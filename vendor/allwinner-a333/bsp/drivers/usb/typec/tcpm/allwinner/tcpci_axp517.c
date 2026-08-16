// SPDX-License-Identifier: GPL-2.0
/* Copyright(c) 2020 - 2023 Allwinner Technology Co.,Ltd. All rights reserved. */
/*
 * Copyright (C) 2025 Allwinner Technology Co., Ltd.
 *
 * Allwinner PMIC USB Type-C Port Controller Interface Driver
 */

#include <linux/gpio/consumer.h>
#include <linux/of_gpio.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/pm_wakeirq.h>
#include <linux/version.h>
#include <linux/ktime.h>
#include <linux/regmap.h>
#include <linux/regulator/consumer.h>
#include <linux/usb/tcpm.h>
#include <linux/usb/role.h>
#include <linux/extcon.h>
#include <linux/extcon-provider.h>
#include "tcpci.h"

#include "sunxi-power-supply.h"
#include "sunxi-power-notifier.h"
#include "axp2101.h"

#define AXP517_REG_COMM_STAT1			(0x01)
#define AXP517_PWR_OK				BIT(4)

#define AXP517_REG_CLK_EN			(0x0B)
#define AXP517_CC_CLK_EN			BIT(3)

#define AXP517_REG_MODULE_EN			(0x19)
#define AXP517_BOOST_EN				BIT(4)

#define AXP517_VENDOR_ID			(0x1F3A)

#define AXP517_REG_AWAKE_EN			(0xE0)
#define AXP517_AWAKE_MODE			BIT(7)
#define AXP517_AWAKE_STATE			GENMASK(6, 5)
#define AXP517_HARD_AWAKE_EN			BIT(1)
#define AXP517_SOFT_AWAKE_EN			BIT(0)

#define AXP517_REG_PD_STATE			(0xE3)
#define AXP517_AWAKE_SEL			BIT(3)

#define AXP517_REG_CC_GENERAL_CONTROL		(0xE8)
#define AXP517_PD_RX_GATE_EN			BIT(7)
#define AXP517_PD_TXRX_RESET			BIT(7)
#define AXP517_GLOBAL_SW_RESET			BIT(5)
#define AXP517_DRP_DUTY_CYCLE_MASK		GENMASK(1, 0)
/* Percent of time that DRP advertises DFP during tDRP */
#define AXP517_DRP_DUTY_CYCLE(n)		((n) & 0x3)

#define AXP517_REG_PHY_BMC_TX_CTRL		(0xE9)
#define AXP517_TX_CARRIER_MODE2_SEL		BIT(3)
#define AXP517_TX_FAST_ROLE_SWAP		BIT(2)
#define AXP517_TX_FAST_ROLE_RX_EN		BIT(1)

#define AXP517_REG_VBUS_CC_PERIOD_FREQ		(0xEA)
#define AXP517_CONFIG_OCP			BIT(4)
#define AXP517_ADC_PERIOD_MASK			GENMASK(3, 2)
#define AXP517_ADC_PERIOD(n)			((n) & 0x3)
#define AXP517_VBUS_DETECT_FREQUENCY_MASK	GENMASK(1, 0)
#define AXP517_VBUS_DETECT_FREQUENCY(n)		((n) & 0x3)

#define AXP517_REG_TWI_ADDR_STATIC		(0xEB)
#define AXP517_I2C_ADDR_STATIC			BIT(0)

#define TCPC_ROLE_CTRL_SET(drp, rp, cc1, cc2) \
	((drp) << 6 | (rp) << 4 | (cc2) << 2 | (cc1))

/* Resources */
#define AXP517_TCPC_MAX_IRQS			(0x10)
#define AXP517_TCPM_DEBOUNCE_MS			500 /* ms */

#define TCPC_RECEIVE_BUFFER_LEN				32
#define TCPC_RECEIVE_BUFFER_COUNT_OFFSET		0
#define TCPC_RECEIVE_BUFFER_FRAME_TYPE_OFFSET		1
#define TCPC_RECEIVE_BUFFER_RX_BYTE_BUF_OFFSET		2

struct tcpci {
	struct device *dev;

	struct tcpm_port *port;

	struct regmap *regmap;
	unsigned int alert_mask;

	bool controls_vbus;

	struct tcpc_dev tcpc;
	struct tcpci_data *data;
};

struct irq_params {
	int virq;
	char *irq_name;
};

struct axp517_tcpc_resources {
	unsigned int nr_irqs;
	struct irq_params irq_params[AXP517_TCPC_MAX_IRQS];
};

struct axp517_chip {
	struct tcpci_data data;
	struct tcpci *tcpci;
	struct device *dev;
	struct regulator *vbus;
	struct power_supply *usb_psy;
	struct usb_role_switch *role_sw;
	unsigned long debounce_jiffies;
	struct delayed_work wq_detcable;
	struct extcon_dev *charger_edev;
	struct notifier_block charger_nb;
	struct extcon_dev *edev;

	bool vbus_on;
	bool port_reset_quirk;
	bool vbus_float_quirk;
	bool battery_exist;
	u32 current_limit;

	u16 vendor_id;
	struct delayed_work  vbus_check_mon;
	struct delayed_work  power_save_mon;
	struct delayed_work  resume_mon;
};

static const char * const typec_cc_status_name[] = {
	[TYPEC_CC_OPEN]		= "Open",
	[TYPEC_CC_RA]		= "Ra",
	[TYPEC_CC_RD]		= "Rd",
	[TYPEC_CC_RP_DEF]	= "Rp-def",
	[TYPEC_CC_RP_1_5]	= "Rp-1.5",
	[TYPEC_CC_RP_3_0]	= "Rp-3.0",
};

static const char * const usb_role_name[] = {
	[USB_ROLE_NONE]		= "NONE",
	[USB_ROLE_HOST]		= "HOST",
	[USB_ROLE_DEVICE]	= "DEVICE",
};

#define tcpci_cc_is_sink(cc) \
	((cc) == TYPEC_CC_RP_DEF || (cc) == TYPEC_CC_RP_1_5 || \
	 (cc) == TYPEC_CC_RP_3_0)

/* As long as cc is pulled up, we can consider it as sink. */
#define tcpci_port_is_sink(cc1, cc2) \
	(tcpci_cc_is_sink(cc1) || tcpci_cc_is_sink(cc2))

#define tcpci_cc_is_source(cc) ((cc) == TYPEC_CC_RD)
#define tcpci_cc_is_audio(cc) ((cc) == TYPEC_CC_RA)
#define tcpci_cc_is_open(cc) ((cc) == TYPEC_CC_OPEN)

#define tcpci_port_is_source(cc1, cc2) \
	((tcpci_cc_is_source(cc1) && !tcpci_cc_is_source(cc2)) || \
	 (tcpci_cc_is_source(cc2) && !tcpci_cc_is_source(cc1)))

#define tcpci_port_is_audio(cc1, cc2) \
	(tcpci_cc_is_audio(cc1) && tcpci_cc_is_audio(cc2))

#define tcpci_port_is_open(cc1, cc2) \
	(tcpci_cc_is_open(cc1) && tcpci_cc_is_open(cc2))

#define tcpci_port_is_close(cc1, cc2) \
	(!tcpci_cc_is_open(cc1) && !tcpci_cc_is_open(cc2))

static inline int axp_tcpci_read16(struct axp517_chip *chip, unsigned int reg, u16 *val)
{
	return regmap_raw_read(chip->data.regmap, reg, val, sizeof(u16));
}

static inline int axp_tcpci_write16(struct axp517_chip *chip, unsigned int reg, u16 val)
{
	return regmap_raw_write(chip->data.regmap, reg, &val, sizeof(u16));
}

static inline int axp_tcpci_read8(struct axp517_chip *chip, unsigned int reg, u8 *val)
{
	return regmap_raw_read(chip->data.regmap, reg, val, sizeof(u8));
}

static inline int axp_tcpci_write8(struct axp517_chip *chip, unsigned int reg, u8 val)
{
	return regmap_raw_write(chip->data.regmap, reg, &val, sizeof(u8));
}

static struct axp517_chip *tdata_to_axp517(struct tcpci_data *tdata)
{
	return container_of(tdata, struct axp517_chip, data);
}

static const unsigned int usb_extcon_cable[] = {
	EXTCON_JACK_HEADPHONE,
	EXTCON_NONE,
};

static struct axp517_tcpc_resources axp517_tcpc_res = {
	.irq_params = {
		{ .irq_name = "rx-msg-change",		.virq = 0x0, },
		{ .irq_name = "rx-hw-rst",		.virq = 0x0, },
		{ .irq_name = "tx-fail",		.virq = 0x0, },
		{ .irq_name = "tx-discard",		.virq = 0x0, },
		{ .irq_name = "tx-success",		.virq = 0x0, },
		{ .irq_name = "rxbuf-overflow",		.virq = 0x0, },
		{ .irq_name = "cc-state-change",	.virq = 0x0, },
		{ .irq_name = "vbus-change",		.virq = 0x0, },
		{ .irq_name = "high-voltage-alarm",	.virq = 0x0, },
		{ .irq_name = "low-voltage-alarm",	.virq = 0x0, },
		{ .irq_name = "fault",			.virq = 0x0, },
		{ .irq_name = "snk-disconnect-detect",	.virq = 0x0, },
		{ .irq_name = "vendor",			.virq = 0x0, },
	},
	.nr_irqs = 13,
};

static void axp517_tcpc_irq_set(bool enable)
{
	int i = 0, irq;

	for (i = 0; i < axp517_tcpc_res.nr_irqs; i++) {
		irq = axp517_tcpc_res.irq_params[i].virq;
		if (enable)
			enable_irq(irq);
		else
			disable_irq(irq);
	}
}

static inline const char *to_irq_name(int irq)
{
	int i;

	for (i = 0; i < axp517_tcpc_res.nr_irqs; i++) {
		if (axp517_tcpc_res.irq_params[i].virq == irq)
			return axp517_tcpc_res.irq_params[i].irq_name;
	}

	return "unknown";
}

static int axp517_sw_reset(struct axp517_chip *chip)
{
	struct regmap *regmap = chip->data.regmap;
	int ret;

	/* soft reset */
	ret = regmap_update_bits(regmap, AXP517_REG_CC_GENERAL_CONTROL, AXP517_GLOBAL_SW_RESET, BIT(5));

	if (ret < 0) {
		dev_err(chip->dev, " fail to soft reset chip(%d)\n", ret);
		return ret;
	}

	ret = regmap_update_bits(regmap, AXP517_REG_CC_GENERAL_CONTROL, AXP517_GLOBAL_SW_RESET, 0);
	if (ret < 0) {
		dev_err(chip->dev, " fail to clear soft reset registers(%d)\n", ret);
		return ret;
	}

	return ret;
}

static int axp517_init_chip(struct axp517_chip *chip)
{
	struct regmap *regmap = chip->data.regmap;
	unsigned int reg;
	int ret;

	/* System status indication */
	ret = regmap_read(regmap, AXP517_REG_COMM_STAT1, &reg);
	if ((ret < 0) || !(AXP517_PWR_OK & reg))
		dev_err(chip->dev, " fail to power on(%d) %#x\n", ret, reg);

	/* CC module clock enable */
	ret = regmap_update_bits(regmap, AXP517_REG_CLK_EN, AXP517_CC_CLK_EN, BIT(3));

	if (ret < 0)
		dev_err(chip->dev, " fail to init chip(%d)\n", ret);

	/* Wait for CC module ready */
	mdelay(20);

	return ret;
}

static int axp517_init(struct tcpci *tcpci, struct tcpci_data *tdata)
{
	int ret = 0;
	struct axp517_chip *chip = tdata_to_axp517(tdata);
	struct regmap *regmap = chip->data.regmap;

	/* UFP Both RD setting : DRP = 0, RpVal = 0 (Default), Rd, Rd */
	ret = axp_tcpci_write8(chip, TCPC_ROLE_CTRL, TCPC_ROLE_CTRL_SET(0, 0, TCPC_ROLE_CTRL_CC_RD, TCPC_ROLE_CTRL_CC_RD));
	/* tTCPCfilter : (26.7 * val) us */

	/* tDRP : (51.2 + 6.4 * val) ms */

	/* dcSRC.DRP : 33% */

	/* Vconn OC */

	/* CK_300K from 320K, SHIPPING off, AUTOIDLE enable, TIMEOUT = 6.4ms */

	/* software low-power mode */
	ret |= regmap_update_bits(regmap, AXP517_REG_PD_STATE, AXP517_AWAKE_SEL, BIT(3));

	if (ret < 0)
		dev_err(chip->dev, " fail to init registers(%d)\n", ret);

	/* Enable I2C ADDR Static */
	ret = regmap_update_bits(regmap, AXP517_REG_TWI_ADDR_STATIC, AXP517_I2C_ADDR_STATIC, BIT(0));
	if (ret < 0)
		dev_err(chip->dev, " fail to enable i2c addr static registers(%d)\n", ret);

	return ret;
}

static int axp517_set_vbus(struct tcpci *tcpci, struct tcpci_data *tdata,
			    bool on, bool charge)
{
	struct axp517_chip *chip = tdata_to_axp517(tdata);
	struct regmap *regmap = chip->data.regmap;
	int ret = 0;

	if (chip->vbus_on == on) {
		dev_info(chip->dev, " vbus is already %s", on ? "On" : "Off");
		goto done;
	}

	dev_info(chip->dev, " set vbus %s", on ? "On" : "Off");

	if (on)
		ret = regulator_enable(chip->vbus);
	else
		ret = regulator_disable(chip->vbus);
	if (ret < 0) {
		dev_err(chip->dev, " cannot %s vbus regulator, ret=%d",
			on ? "enable" : "disable", ret);
		goto done;
	}

	chip->vbus_on = on;
	/**
	 * FIXME:
	 * 1. Trigger Power interrupt When Use External BOOST.
	 * 2. Boost module enable for PD communication.
	 */
	if (tcpci->port && chip->vbus_float_quirk) {
		tcpm_vbus_change(tcpci->port);
		if (on)
			regmap_update_bits(regmap, AXP517_REG_MODULE_EN, AXP517_BOOST_EN, AXP517_BOOST_EN);
		else
			regmap_update_bits(regmap, AXP517_REG_MODULE_EN, AXP517_BOOST_EN, 0);
	}

done:
	return ret;
}

static void process_rx(struct tcpci *tcpci, u16 status)
{
	struct axp517_chip *chip = tdata_to_axp517(tcpci->data);
	struct pd_message msg;
	u8 count, frame_type, rx_buf[TCPC_RECEIVE_BUFFER_LEN];
	int ret, payload_index;
	u8 *rx_buf_ptr;
	enum tcpm_transmit_type rx_type;

	/*
	 * READABLE_BYTE_COUNT: Indicates the number of bytes in the RX_BUF_BYTE_x registers
	 * plus one (for the RX_BUF_FRAME_TYPE) Table 4-36.
	 * Read the count and frame type.
	 */
	ret = regmap_noinc_read(chip->data.regmap, TCPC_RX_BYTE_CNT, rx_buf, 2);
	if (ret < 0) {
		dev_err(chip->dev, "TCPC_RX_BYTE_CNT read failed ret:%d\n", ret);
		return;
	}

	count = rx_buf[TCPC_RECEIVE_BUFFER_COUNT_OFFSET];
	frame_type = rx_buf[TCPC_RECEIVE_BUFFER_FRAME_TYPE_OFFSET];

	switch (frame_type) {
	case TCPC_RX_BUF_FRAME_TYPE_SOP1:
		rx_type = TCPC_TX_SOP_PRIME;
		break;
	case TCPC_RX_BUF_FRAME_TYPE_SOP:
		rx_type = TCPC_TX_SOP;
		break;
	default:
		rx_type = TCPC_TX_SOP;
		break;
	}

	if (count == 0 || (frame_type != TCPC_RX_BUF_FRAME_TYPE_SOP &&
	    frame_type != TCPC_RX_BUF_FRAME_TYPE_SOP1)) {
		axp_tcpci_write16(chip, TCPC_ALERT, TCPC_ALERT_RX_STATUS);
		dev_err(chip->dev, "%s\n", count ==  0 ? "error: count is 0" :
			"error frame_type is not SOP/SOP'");
		return;
	}

	if ((count > (sizeof(struct pd_message) + 1)) || (count + 1 > TCPC_RECEIVE_BUFFER_LEN)) {
		dev_err(chip->dev, "Invalid TCPC_RX_BYTE_CNT %d\n", count);
		return;
	}

	/*
	 * Read count + 1 as RX_BUF_BYTE_x is hidden and can only be read through
	 * TCPC_RX_BYTE_CNT
	 */
	count += 1;
	ret = regmap_noinc_read(chip->data.regmap, TCPC_RX_BYTE_CNT, rx_buf, count);
	if (ret < 0) {
		dev_err(chip->dev, "Error: TCPC_RX_BYTE_CNT read failed: %d\n", ret);
		return;
	}

	rx_buf_ptr = rx_buf + TCPC_RECEIVE_BUFFER_RX_BYTE_BUF_OFFSET;
	msg.header = cpu_to_le16(*(u16 *)rx_buf_ptr);
	rx_buf_ptr = rx_buf_ptr + sizeof(msg.header);
	for (payload_index = 0; payload_index < pd_header_cnt_le(msg.header); payload_index++,
	     rx_buf_ptr += sizeof(msg.payload[0]))
		msg.payload[payload_index] = cpu_to_le32(*(u32 *)rx_buf_ptr);

	/* Read complete, clear RX status alert bit */
	axp_tcpci_write16(chip, TCPC_ALERT, TCPC_ALERT_RX_STATUS);

	tcpm_pd_receive(tcpci->port, &msg);
}

static inline struct tcpci *tcpc_to_tcpci(struct tcpc_dev *tcpc)
{
	return container_of(tcpc, struct tcpci, tcpc);
}

static int axp517_set_roles(struct tcpc_dev *tcpc, bool attached,
			     enum typec_role role, enum typec_data_role data)
{
	struct tcpci *tcpci = tcpc_to_tcpci(tcpc);
	struct axp517_chip *chip = tdata_to_axp517(tcpci->data);
	enum typec_cc_status cc1, cc2;
	unsigned int reg;
	int ret;

	reg = PD_REV20 << TCPC_MSG_HDR_INFO_REV_SHIFT;
	if (role == TYPEC_SOURCE)
		reg |= TCPC_MSG_HDR_INFO_PWR_ROLE;
	if (data == TYPEC_HOST)
		reg |= TCPC_MSG_HDR_INFO_DATA_ROLE;
	ret = regmap_write(tcpci->regmap, TCPC_MSG_HDR_INFO, reg);
	if (ret < 0)
		return ret;

	/* Support for Audio Accessory Mode. */
	if (tcpc->get_cc(tcpc, &cc1, &cc2) == 0) {
		if (tcpci_port_is_audio(cc1, cc2))
			extcon_set_state_sync(chip->edev, EXTCON_JACK_HEADPHONE, true);
		else
			extcon_set_state_sync(chip->edev, EXTCON_JACK_HEADPHONE, false);
	}

	return 0;
}

static int axp517_get_vbus(struct tcpc_dev *tcpc)
{
	struct tcpci *tcpci = tcpc_to_tcpci(tcpc);
	struct axp517_chip *chip = tdata_to_axp517(tcpci->data);
	enum typec_cc_status cc1, cc2;
	unsigned int reg;
	int ret;

	ret = tcpc->get_cc(tcpc, &cc1, &cc2);
	if (ret < 0)
		return ret;

	ret = regmap_read(tcpci->regmap, TCPC_POWER_STATUS, &reg);
	if (ret < 0)
		return ret;

	return (tcpci_port_is_sink(cc1, cc2) ? !!(reg & TCPC_POWER_STATUS_VBUS_PRES) : 0)  || chip->vbus_on;
}

static int axp517_get_current_limit(struct tcpc_dev *tcpc)
{
	union power_supply_propval temp;
	struct tcpci *tcpci = tcpc_to_tcpci(tcpc);
	struct axp517_chip *chip = tdata_to_axp517(tcpci->data);
	int limit;

	power_supply_get_property(chip->usb_psy, POWER_SUPPLY_PROP_INPUT_CURRENT_LIMIT, &temp);
	if (temp.intval)
		limit = temp.intval;
	else
		limit = 0;
	dev_info(chip->dev, " get current limit %u mA", limit);

	return limit;
}

static int axp517_set_current_limit(struct tcpc_dev *tcpc, u32 max_ma, u32 mv)
{
	union power_supply_propval temp;
	struct tcpci *tcpci = tcpc_to_tcpci(tcpc);
	struct axp517_chip *chip = tdata_to_axp517(tcpci->data);
	int ret = 0;

	dev_info(chip->dev, " Setting voltage/current limit %u mV %u mA", mv, max_ma);

	temp.intval = mv;
	power_supply_set_property(chip->usb_psy, POWER_SUPPLY_PROP_VOLTAGE_NOW, &temp);

	if (mv == 5000 && max_ma != 500)
		max_ma = chip->current_limit;
	temp.intval = max_ma;
	power_supply_set_property(chip->usb_psy, POWER_SUPPLY_PROP_INPUT_CURRENT_LIMIT, &temp);

	return ret;
}

static int axp517_check_battery(struct axp517_chip *chip)
{
	struct device_node *np = NULL;
	static int battery_check;

	if (!battery_check) {
		battery_check = 1;
		np = of_parse_phandle(chip->dev->of_node, "det_battery_supply", 0);
		if (np) {
			if (of_device_is_available(np))
				battery_check = 2;
		}
	}

	return battery_check - 1;
}

static void axp517_init_tcpci_data(struct axp517_chip *chip)
{
	if (chip->vbus)
		chip->data.set_vbus = axp517_set_vbus;
	chip->data.init = axp517_init;
	chip->data.process_rx = process_rx;
	chip->data.RX_BUF_BYTE_x_hidden = true;
	chip->data.TX_BUF_BYTE_x_hidden = true;
	chip->data.RX_TX_FIFO_supported = true;
	chip->data.vbus_floated = chip->vbus_float_quirk;
	chip->battery_exist = axp517_check_battery(chip) ? true : false;
	chip->data.self_powered = chip->battery_exist;
}

static void axp517_init_tcpci_data_late(struct axp517_chip *chip)
{
	union power_supply_propval temp;
	struct device_node *np = NULL;
	if (chip->vbus_float_quirk)
		chip->tcpci->tcpc.get_vbus = axp517_get_vbus;

	chip->tcpci->tcpc.set_roles = axp517_set_roles;
	if (!chip->usb_psy)
		return;

	np = of_parse_phandle(chip->dev->of_node, "det_usb_supply", 0);
	if (np)
		of_property_read_u32(np, "pmu_usbad_cur", &temp.intval);
	chip->current_limit = temp.intval;
	dev_info(chip->dev, " battery exist: %s", chip->battery_exist ? "yes" : "no");

	chip->tcpci->tcpc.set_current_limit = axp517_set_current_limit;
	chip->tcpci->tcpc.get_current_limit = axp517_get_current_limit;
}

static irqreturn_t axp517_irq(int irq, void *dev_id)
{
	struct axp517_chip *chip = dev_id;
	u16 status;

	axp_tcpci_read16(chip, TCPC_ALERT, &status);
	dev_dbg(chip->dev, " irq %d name %s\n", irq, to_irq_name(irq));

	/**
	 * NOTE:
	 * We provide tcpci_irq_overrides instead of tcpci_irq for vendor hooks.
	 */

	queue_delayed_work(system_power_efficient_wq, &chip->wq_detcable,
			   chip->debounce_jiffies);

	cancel_delayed_work_sync(&chip->power_save_mon);
	schedule_delayed_work(&chip->power_save_mon, msecs_to_jiffies(500));

	if (status & TCPC_ALERT_CC_STATUS) {
		cancel_delayed_work_sync(&chip->vbus_check_mon);
		schedule_delayed_work(&chip->vbus_check_mon, msecs_to_jiffies(100));
	}

	return tcpci_irq_overrides(chip->tcpci);
}

static int axp517_check_revision(struct axp517_chip *chip)
{
	u16 vendor_id;
	int ret;

	ret = axp_tcpci_read16(chip, TCPC_VENDOR_ID, &vendor_id);
	if (ret < 0) {
		dev_err(chip->dev, " fail to read Vendor id(%d)\n", ret);
		return ret;
	}

	if (vendor_id != AXP517_VENDOR_ID) {
		dev_err(chip->dev, " vid is not correct, 0x%04x\n", vendor_id);
		return -ENODEV;
	}

	chip->vendor_id = vendor_id;

	return 0;
}

static void axp517_detect_cable(struct work_struct *work)
{
	struct axp517_chip *chip = container_of(to_delayed_work(work), struct axp517_chip,
						 wq_detcable);
	struct tcpm_port *port = tcpci_get_tcpm_port_overrides(chip->tcpci);
	enum typec_cc_status cc1, cc2;
	union power_supply_propval temp;
	int reg_val;

	chip->tcpci->tcpc.get_cc(&chip->tcpci->tcpc, &cc1, &cc2);

	dev_dbg(chip->dev, " CC1: %d - %s, CC2: %d - %s\n",
		 cc1, typec_cc_status_name[cc1], cc2, typec_cc_status_name[cc2]);

	/**
	 * FIXME:
	 * 1. Support double Rp to Vbus cable as sink and device.
	 * 2. Update symbol list for 'usb_role_switch_get_role' function.
	 */
	if (tcpci_port_is_audio(cc1, cc2)) {
		/* Nothing to do */
	} else if ((tcpci_cc_is_audio(cc1) && tcpci_cc_is_open(cc2)) ||
		   (tcpci_cc_is_audio(cc2) && tcpci_cc_is_open(cc1))) {
		regmap_read(chip->data.regmap, TCPC_CC_STATUS, &reg_val);
		if (!(reg_val & TCPC_CC_STATUS_TOGGLING))
			regmap_write(chip->data.regmap, TCPC_ROLE_CTRL, TCPC_ROLE_CTRL_SET(0, 0, TCPC_ROLE_CTRL_CC_RD, TCPC_ROLE_CTRL_CC_RD));
	} else if (tcpci_port_is_open(cc1, cc2)) {

		dev_dbg(chip->dev, "Setting Role [%s]\n", usb_role_name[USB_ROLE_NONE]);
		if (chip->usb_psy) {
			temp.intval = 0;
			power_supply_set_property(chip->usb_psy, POWER_SUPPLY_PROP_INPUT_CURRENT_LIMIT, &temp);
		}
		usb_role_switch_set_role(chip->role_sw, USB_ROLE_NONE);

		/* FIXME: Enable DRP toggling for Ra Cable */
		regmap_write(chip->data.regmap, TCPC_COMMAND, TCPC_CMD_LOOK4CONNECTION);
	} else if (tcpci_cc_is_sink(cc1) && tcpci_cc_is_sink(cc2)) {

		dev_dbg(chip->dev, "Setting Role [%s]\n", usb_role_name[USB_ROLE_DEVICE]);
		if (chip->usb_psy) {
			temp.intval = chip->current_limit ? chip->current_limit : 500;
			power_supply_set_property(chip->usb_psy, POWER_SUPPLY_PROP_INPUT_CURRENT_LIMIT, &temp);
		}
		usb_role_switch_set_role(chip->role_sw, USB_ROLE_DEVICE);
	}
	/* FIXME: Trigger CC interrupt When System Power On. */
	DO_ONCE_LITE(tcpm_cc_change, port);
	chip->debounce_jiffies = msecs_to_jiffies(AXP517_TCPM_DEBOUNCE_MS);
}

static bool axp517_is_disconnected(struct axp517_chip *chip)
{
	enum typec_cc_status cc1, cc2;

	chip->tcpci->tcpc.get_cc(&chip->tcpci->tcpc, &cc1, &cc2);
	if (tcpci_port_is_close(cc1, cc2))
		return true;

	return false;
}

static void axp517_power_save(struct work_struct *work)
{
	struct axp517_chip *chip = container_of(to_delayed_work(work), struct axp517_chip,
						 power_save_mon);
	struct regmap *regmap = chip->data.regmap;
	int ret = 0;

	/* FIXME: Disable cc clock if port in disconnect after 500ms. */
	/*
	 * Disable 24M oscillator to save power consumption, and it will be
	 * enabled automatically when INT occur after system resume.
	 */

	if (axp517_is_disconnected(chip)) {
		ret = regmap_update_bits(regmap, AXP517_REG_AWAKE_EN, AXP517_SOFT_AWAKE_EN, BIT(0));
		if (ret < 0) {
			dev_err(chip->dev, " fail to soft LPM(%d)\n", ret);
		}
	}
}

static void axp517_detect_call(struct work_struct *work)
{
	struct axp517_chip *chip = container_of(to_delayed_work(work), struct axp517_chip,
						 vbus_check_mon);

	if (chip->usb_psy && chip->vbus_float_quirk)
		sunxi_call_power_supply_notifier(AW_PSY_EVENT_VBUS_ONLINE_CHECK);
}

static void axp517_resume_call(struct work_struct *work)
{
	struct axp517_chip *chip = container_of(to_delayed_work(work), struct axp517_chip,
						 resume_mon);
	enum typec_cc_status cc1, cc2;
	union power_supply_propval temp;

	chip->tcpci->tcpc.get_cc(&chip->tcpci->tcpc, &cc1, &cc2);

	dev_dbg(chip->dev, " CC1: %d - %s, CC2: %d - %s\n",
		 cc1, typec_cc_status_name[cc1], cc2, typec_cc_status_name[cc2]);

	if (tcpci_cc_is_sink(cc1) && tcpci_cc_is_sink(cc2)) {
		power_supply_get_property(chip->usb_psy, POWER_SUPPLY_PROP_VOLTAGE_NOW, &temp);
		if (temp.intval < 6000) {
		/* Transmit Hard Reset: nRetryCount is 3 in PD2.0 spec where 2 in PD3.0 spec */
			axp_tcpci_write8(chip, TCPC_TRANSMIT,
				  (0x2 << TCPC_TRANSMIT_RETRY_SHIFT) | (TCPC_TX_HARD_RESET << TCPC_TRANSMIT_TYPE_SHIFT));
			tcpm_tcpc_reset(tcpci_get_tcpm_port_overrides(chip->tcpci));
		}
	}
}

static int axp517_extcon_charge_notifier(struct notifier_block *nb,
	unsigned long event, void *ptr)
{
	struct axp517_chip *chip = container_of(nb, struct axp517_chip, charger_nb);
	struct regmap *regmap = chip->data.regmap;
	int ret;

	dev_info(chip->dev, "charger event %s\n", event ? "enable" : "disable");

	if (!event) {
		/* Transmit Hard Reset: nRetryCount is 3 in PD2.0 spec where 2 in PD3.0 spec */
		ret = axp_tcpci_write8(chip, TCPC_TRANSMIT,
				(0x2 << TCPC_TRANSMIT_RETRY_SHIFT) | (TCPC_TX_HARD_RESET << TCPC_TRANSMIT_TYPE_SHIFT));
		if (ret < 0)
			dev_err(chip->dev, "cannot send hardreset, ret=%d", ret);
		/* CC module clock disable */
		ret = regmap_update_bits(regmap, AXP517_REG_CLK_EN, AXP517_CC_CLK_EN, 0);

		if (ret < 0)
			dev_err(chip->dev, " fail to init chip(%d)\n", ret);
	} else {
		/* CC module clock enable */
		ret = regmap_update_bits(regmap, AXP517_REG_CLK_EN, AXP517_CC_CLK_EN, BIT(3));

		if (ret < 0)
			dev_err(chip->dev, " fail to init chip(%d)\n", ret);

		/* Wait for CC module ready */
		mdelay(20);

		/* Transmit Hard Reset: nRetryCount is 3 in PD2.0 spec where 2 in PD3.0 spec */
		ret = axp_tcpci_write8(chip, TCPC_TRANSMIT,
				(0x2 << TCPC_TRANSMIT_RETRY_SHIFT) | (TCPC_TX_HARD_RESET << TCPC_TRANSMIT_TYPE_SHIFT));
		if (ret < 0)
			dev_err(chip->dev, "cannot send hardreset, ret=%d", ret);

		tcpm_tcpc_reset(tcpci_get_tcpm_port_overrides(chip->tcpci));
	}

	return NOTIFY_DONE;
}

static int axp517_probe(struct platform_device *pdev)
{
	struct sunxi_power_dev *axp_dev = dev_get_drvdata(pdev->dev.parent);
	struct regmap_irq_chip_data *irq_data = axp_dev->regmap_pdirqc;
	struct device *dev = &pdev->dev;
	struct device_node *node = pdev->dev.of_node;
	struct axp517_chip *chip;
	int i = 0, irq;
	u32 base;
	int ret;

	if (!of_device_is_available(node)) {
		pr_err(" axp517 device not configured\n");
		return -ENODEV;
	}

	if (!axp_dev->irq || !irq_data) {
		pr_err(" can't register pmic tcpc without irq or irq_data\n");
		return -EINVAL;
	}

	chip = devm_kzalloc(&pdev->dev, sizeof(*chip), GFP_KERNEL);
	if (!chip) {
		dev_err(&pdev->dev, " alloc failed\n");
		return -ENOMEM;
	}

	chip->data.regmap = dev_get_regmap(dev->parent, NULL);
	if (!chip->data.regmap) {
		dev_err(&pdev->dev, " Failed to get regmap\n");
		return -ENODEV;
	}

	ret = of_property_read_u32(node, "reg", &base);
	if (ret) {
		dev_err(&pdev->dev, " Failed to get 'reg' property\n");
		return ret;
	}
	chip->dev = &pdev->dev;

	ret = axp517_init_chip(chip);
	if (ret < 0) {
		dev_err(chip->dev, " pmic chip init failed\n");
		ret = -EINVAL;
		return ret;
	}

	ret = axp517_check_revision(chip);
	if (ret < 0) {
		dev_err(chip->dev, " check vid/pid fail(%d)\n", ret);
		return ret;
	}

	if (of_find_property(chip->dev->of_node, "det_usb_supply", NULL)) {
		chip->usb_psy = devm_power_supply_get_by_phandle(chip->dev, "det_usb_supply");
	} else {
		pr_err(" failed to find usb power\n");
		chip->usb_psy =  NULL;
	}

	chip->vbus = devm_regulator_get_optional(chip->dev, "vbus");
	if (IS_ERR(chip->vbus)) {
		ret = PTR_ERR(chip->vbus);
		chip->vbus = NULL;
		if (ret != -ENODEV)
			return ret;
	}

	/* Here are some compatible issues for Type-C Port Controller. */
	chip->port_reset_quirk = device_property_read_bool(chip->dev, "aw,port-reset-quirk");
	chip->vbus_float_quirk = device_property_read_bool(chip->dev, "aw,vbus-float-quirk");

	chip->debounce_jiffies = msecs_to_jiffies(AXP517_TCPM_DEBOUNCE_MS * 3);
	INIT_DELAYED_WORK(&chip->wq_detcable, axp517_detect_cable);
	INIT_DELAYED_WORK(&chip->vbus_check_mon, axp517_detect_call);
	INIT_DELAYED_WORK(&chip->power_save_mon, axp517_power_save);
	INIT_DELAYED_WORK(&chip->resume_mon, axp517_resume_call);

	ret = axp517_sw_reset(chip);
	if (ret < 0) {
		dev_err(chip->dev, " fail to soft reset, ret = %d\n", ret);
		return ret;
	}

	axp517_init_tcpci_data(chip);

	chip->tcpci = tcpci_register_port_overrides(chip->dev, &chip->data);
	if (IS_ERR(chip->tcpci)) {
		dev_err(chip->dev, " fail to register tcpci port, %ld\n", PTR_ERR(chip->tcpci));
		return PTR_ERR(chip->tcpci);
	}

	axp517_init_tcpci_data_late(chip);

	chip->role_sw = usb_role_switch_get(chip->dev);
	if (IS_ERR(chip->role_sw)) {
		ret = PTR_ERR(chip->role_sw);
		dev_err(chip->dev, "fail to get usb role switch, %ld\n", PTR_ERR(chip->role_sw));
		return ret;
	}

	for (i = 0; i < axp517_tcpc_res.nr_irqs; i++) {
		irq = platform_get_irq_byname(pdev, axp517_tcpc_res.irq_params[i].irq_name);
		if (irq < 0)
			continue;

		irq = regmap_irq_get_virq(irq_data, irq);
		if (irq < 0) {
			dev_err(chip->dev, " can't get irq %s\n", axp517_tcpc_res.irq_params[i].irq_name);
			tcpci_unregister_port_overrides(chip->tcpci);
			return irq;
		}
		/* we use this variable to suspend irq */
		axp517_tcpc_res.irq_params[i].virq = irq;
		ret = devm_request_any_context_irq(&pdev->dev, irq,
						   axp517_irq, 0,
						   axp517_tcpc_res.irq_params[i].irq_name, chip);
		if (ret < 0) {
			dev_err(chip->dev, " failed to request %s IRQ %d: %d\n",
				axp517_tcpc_res.irq_params[i].irq_name, irq, ret);
			tcpci_unregister_port_overrides(chip->tcpci);
			return ret;
		} else {
			ret = 0;
		}

		dev_dbg(chip->dev, " Requested %s IRQ %d: %d\n", axp517_tcpc_res.irq_params[i].irq_name, irq, ret);
	}

	if (!device_property_read_bool(chip->dev, "wakeup-source")) {
		dev_info(chip->dev, "wakeup source is disabled!\n");
	} else {
		device_init_wakeup(dev, true);
	}

	if (of_property_read_bool(node, "extcon")) {
		chip->charger_edev = extcon_get_edev_by_phandle(chip->dev, 0);
		if (IS_ERR_OR_NULL(chip->charger_edev)) {
			dev_vdbg(chip->dev, "couldn't get extcon device\n");
			return -EPROBE_DEFER;
		}

		chip->charger_nb.notifier_call = axp517_extcon_charge_notifier;
		ret = devm_extcon_register_notifier(chip->dev, chip->charger_edev,
						    EXTCON_CHG_USB_PD, &chip->charger_nb);
		if (ret < 0)
			dev_vdbg(chip->dev, "failed to register notifier for Charger\n");
	}

	chip->edev = devm_extcon_dev_allocate(chip->dev, usb_extcon_cable);
	if (IS_ERR(chip->edev)) {
		dev_err(chip->dev, "failed to allocate extcon device\n");
		return -ENOMEM;
	}

	ret = devm_extcon_dev_register(chip->dev, chip->edev);
	if (ret < 0) {
		dev_err(chip->dev, "failed to register extcon device\n");
		return ret;
	}

	platform_set_drvdata(pdev, chip);

	dev_info(chip->dev, " Vendor ID: 0x%04x Base: 0x%x, probe success\n", chip->vendor_id, base);

	return 0;
}

static int axp517_remove(struct platform_device *pdev)
{
	struct axp517_chip *chip = platform_get_drvdata(pdev);

	if (device_may_wakeup(chip->dev))
		device_init_wakeup(chip->dev, false);
	cancel_delayed_work_sync(&chip->wq_detcable);
	cancel_delayed_work_sync(&chip->vbus_check_mon);
	usb_role_switch_put(chip->role_sw);
	tcpci_unregister_port_overrides(chip->tcpci);

	return 0;
}

/*
 * NOTE: The system is about to shutdown and poweroff, that is to say,
 * we'd better hard reset the port and notify attached device.
 */
static void axp517_shutdown(struct platform_device *pdev)
{
	struct axp517_chip *chip = platform_get_drvdata(pdev);
	int ret;

	axp517_tcpc_irq_set(false);
	if (!chip->battery_exist) {
		dev_dbg(chip->dev, " no battery, hardreset forbidden");
		return;
	}
	/* Transmit Hard Reset: nRetryCount is 3 in PD2.0 spec where 2 in PD3.0 spec */
	ret = axp_tcpci_write8(chip, TCPC_TRANSMIT,
			  (0x2 << TCPC_TRANSMIT_RETRY_SHIFT) | (TCPC_TX_HARD_RESET << TCPC_TRANSMIT_TYPE_SHIFT));
	if (ret < 0)
		dev_err(chip->dev, "cannot send hardreset, ret=%d", ret);
}

static int axp517_pm_suspend(struct device *dev)
{
	struct axp517_chip *chip = dev_get_drvdata(dev);

	cancel_delayed_work_sync(&chip->wq_detcable);
	cancel_delayed_work_sync(&chip->vbus_check_mon);
	/*
	 * When system suspend, disable irq to prevent interrupt trigger
	 * during I2C bus suspend
	 */
	if (!device_may_wakeup(chip->dev))
		axp517_tcpc_irq_set(false);

	return 0;
}

static int axp517_pm_resume(struct device *dev)
{
	struct axp517_chip *chip = dev_get_drvdata(dev);
	int ret = 0;
	u8 pwr;

	/* Enable irq after I2C bus already resume */
	if (!device_may_wakeup(chip->dev))
		axp517_tcpc_irq_set(true);

	queue_delayed_work(system_power_efficient_wq,
			   &chip->wq_detcable, chip->debounce_jiffies);
	schedule_delayed_work(&chip->vbus_check_mon, msecs_to_jiffies(100));

	if (!chip->port_reset_quirk) {
		dev_info(chip->dev, " disable reset this port\n");
		return 0;
	}

	/*
	 * When the power of axp517 is lost or i2c read failed in PM S/R
	 * process, we must reset the tcpm port first to ensure the devices
	 * can attach again.
	 */
	ret = axp_tcpci_read8(chip, AXP517_REG_AWAKE_EN, &pwr);
	if (pwr & BIT(0) || ret < 0) {
		ret = axp517_sw_reset(chip);
		if (ret < 0) {
			dev_err(chip->dev, " fail to soft reset, ret = %d\n", ret);
			return ret;
		}

		tcpm_tcpc_reset(tcpci_get_tcpm_port_overrides(chip->tcpci));
		dev_info(chip->dev, " reset the port success(%d)\n", pwr);
	}

	schedule_delayed_work(&chip->resume_mon, msecs_to_jiffies(5 * 1000));

	return 0;
}

static SIMPLE_DEV_PM_OPS(axp517_pm_ops, axp517_pm_suspend, axp517_pm_resume);

static const struct of_device_id axp517_of_id[] = {
	{ .compatible = "x-powers,axp517-tcpc", .data = &axp517_tcpc_res },
	{},
};
MODULE_DEVICE_TABLE(of, axp517_of_id);

static struct platform_driver axp517_plat_driver = {
	.driver = {
		.name = "axp517-tcpc",
		.pm = &axp517_pm_ops,
		.of_match_table = axp517_of_id,
	},
	.probe = axp517_probe,
	.remove = axp517_remove,
	.shutdown = axp517_shutdown,
};
module_platform_driver(axp517_plat_driver);

MODULE_ALIAS("platform:axp517-plat-driver");
MODULE_DESCRIPTION("AXP517 USB Type-C Port Controller Interface Driver");
MODULE_AUTHOR("kanghoupeng<kanghoupeng@allwinnertech.com>");
MODULE_LICENSE("GPL v2");
MODULE_VERSION("1.0.1");
