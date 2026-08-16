// SPDX-License-Identifier: GPL-2.0
/* Copyright(c) 2020 - 2024 Allwinner Technology Co.,Ltd. All rights reserved. */
/*
 * Allwinner USB2.0 (AW) Phy driver
 *
 * Copyright (C) 2024 Allwinner Electronics Co., Ltd.
 */

#include <linux/bitfield.h>
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/phy/phy.h>
#include <linux/reset.h>
#include <linux/usb/otg.h>
#include <dt-bindings/phy/phy.h>
#include <linux/version.h>

#include <sunxi-sid.h>

/* USB2.0 PHY Control Register */
#define USBC_REG_o_PHY_CTRL(n)			(0x0010 + (0x20 * (n)))
#define   BIST_EN_A				BIT(16)
#define   VC_ADDR				GENMASK(15, 8)
#define   VC_DI					BIT(7)
#define   SIDDQ					BIT(3) /* No need for common */
#define   VC_EN					BIT(1)
#define   VC_CLK				BIT(0)

/* USB2.0 PHY Control Register */
#define USBC_REG_o_PHY_STATUS(n)		(0x0024 + (0x20 * (n)))
#define   BIST_ERR				BIT(17)
#define   BIST_DONE				BIT(16)
#define   VC_DO					BIT(0)

/* USB2.0 PHY Reset Control Register */
#define USBC_REG_o_RST_CTRL(n)			(0x0028 + (0x20 * (n)))
#define   PHY_RST				BIT(0)

/* USB2.0 PHY Version Number Register */
#define USBC_REG_o_PHY_VER			(0x00F8)
#define   PHY_GEN_VER				GENMASK(31, 24)
#define   PHY_SUB_VER				GENMASK(23, 16)
#define   PHY_PRJ_VER				GENMASK(15, 8)

/* USB2.0 subsystem digital app reg : PHY control regsiter */
#define USB_DCTRL				0x08
#define U2_MAP_SEL				BIT(0)

/* Registers */
#define  USBC_REG_PHY_CTRL(phy_base_addr, n)	\
				((phy_base_addr) + USBC_REG_o_PHY_CTRL(n))
#define  USBC_REG_PHY_STATUS(phy_base_addr, n)	\
				((phy_base_addr) + USBC_REG_o_PHY_STATUS(n))
#define  USBC_REG_RST_CTRL(phy_base_addr, n)	\
				((phy_base_addr) + USBC_REG_o_RST_CTRL(n))
#define  USBC_REG_PHY_VER(phy_base_addr)	\
				((phy_base_addr) + USBC_REG_o_PHY_VER)

/* USB2.0 AW-PHY Inner Register */
#define AWPHY_REG_VREF_TUNE			(0x60)
#define AWPHY_REG_VREF_TUNE_LEN			(4)

#define AWPHY_REG_PREEMPAMP_TUNE		(0x64)
#define AWPHY_REG_PREEMPAMP_TUNE_LEN		(2)

#define AWPHY_REG_TX_RISTUNE			(0x68)
#define AWPHY_REG_TX_RISTUNE_LEN		(2)

#define AWPHY_REG_TX_PREEMPPULSE_TUNE		(0x6A)
#define AWPHY_REG_TX_PREEMPPULSE_TUNE_LEN	(1)

struct sunxi_phy_cal {
	u32 role;
	u32 addr;
	u32 data;
	u32 len;
};

struct sunxi_phy {
	struct device		dev;
	const char		*name;
	struct phy		*phy;
	struct clk		*ref_clk;
	struct clk		*u2_only_clk;
	struct sunxi_phy_plat	*sunxi_phy;

	__u8			num; /* phy number */
	enum phy_mode		mode;

	u32			cal_num;
	struct sunxi_phy_cal	*cal_data;

	u32			dpdm_bypass_quirk;
	struct sunxi_phy_cal	*dpdm_bypass_data;

	u32			u2u3_only_quirk;
};

struct sunxi_phy_plat {
	struct device		*dev;
	void __iomem		*phy_reg;
	void __iomem		*top_reg;

	struct clk		*pclk;
	struct clk		*mclk;
	struct reset_control	*reset;
	struct reset_control	*usb_reset;

	struct sunxi_phy	*uphy0;
	struct sunxi_phy	*uphy1;
	struct sunxi_phy	*uphy2;

	__u32			vernum; /* PHY Version number */

	const struct sunxi_phy_config *cfg;
};

enum sunxi_phy_type_e {
	SUNXI_PHY0_TYPE = 0,
	SUNXI_PHY1_TYPE,
	SUNXI_PHY2_TYPE,
};

struct sunxi_phy_config {
	bool has_rst;		// some SOC don't have HSI rst: sun8iw22p1;
};

enum phy_mode_e {
	PHY_MODE_USB2 = 0,
	PHY_MODE_USB3,
};

/* The AW PHY support x_p0_transceiver, x_p1_transceiver, x_p2_transceiver and x_common module */
#define  USB_XCVR0_PHY_NO		(0)
#define  USB_XCVR1_PHY_NO		(1)
#define  USB_XCVR2_PHY_NO		(2)
#define  USB_COMMON_PHY_NO		(3)

#define SUNXI_PHY_CMD_NUM		(5)
#define __PHY_DEV_ATTR(_name)		(&(dev_attr_##_name.attr))
#define __PHY_CLASS_ATTR(_name)		(&(class_attr_##_name.attr))

static u32 reg_addr, reg_lenth;

/*****************************************************************
 *                 Sub-System USB2.0 PHY Support
 *****************************************************************/

static u32 phy_ver_get(struct sunxi_phy_plat *sunxi_phy)
{
	u32 reg;

	reg = readl(USBC_REG_PHY_VER(sunxi_phy->phy_reg));

	return reg;
}

static void sunxi_phy_set(struct sunxi_phy *phy, bool enable)
{
	struct sunxi_phy_plat *sunxi_phy = phy->sunxi_phy;
	u32 val;

	val = readl(USBC_REG_PHY_CTRL(sunxi_phy->phy_reg, phy->num));
	if (enable)
		val &= ~SIDDQ; /* write 0 to enable phy */
	else
		val |= SIDDQ; /* write 1 to disable phy */
	writel(val, USBC_REG_PHY_CTRL(sunxi_phy->phy_reg, phy->num));

	val = readl(USBC_REG_RST_CTRL(sunxi_phy->phy_reg, phy->num));
	if (enable)
		val |= PHY_RST;
	else
		val &= ~PHY_RST;
	writel(val, USBC_REG_RST_CTRL(sunxi_phy->phy_reg, phy->num));

	/* default use USB2.0_SYS register control mapping */
	val = readl(sunxi_phy->top_reg + USB_DCTRL);
	val |= U2_MAP_SEL;
	writel(val, sunxi_phy->top_reg + USB_DCTRL);
}

static int sunxi_phy_VCbus_write(struct sunxi_phy *phy, u32 addr, u32 data, u32 len)
{
	u32 j = 0;
	u32 temp = 0;
	u32 dtmp = data;
	u32 phy_offset = phy->num; /* transciver register 0-2 */
	struct sunxi_phy_plat *sunxi_phy = phy->sunxi_phy;

	if (addr < 0x60) /* common register */
		phy_offset = 3;

	/*VC_EN enable*/
	temp = readl(USBC_REG_PHY_CTRL(sunxi_phy->phy_reg, phy_offset));
	temp |= VC_EN;
	writel(temp, USBC_REG_PHY_CTRL(sunxi_phy->phy_reg, phy_offset));

	for (j = 0; j < len; j++) {
		/*ensure VC_CLK low*/
		temp = readl(USBC_REG_PHY_CTRL(sunxi_phy->phy_reg, phy_offset));
		temp &= ~VC_CLK;
		writel(temp, USBC_REG_PHY_CTRL(sunxi_phy->phy_reg, phy_offset));

		/*set write address*/
		temp = readl(USBC_REG_PHY_CTRL(sunxi_phy->phy_reg, phy_offset));
		temp &= ~VC_ADDR;//clear
		temp |= FIELD_PREP(VC_ADDR, addr + j);  // write
		writel(temp, USBC_REG_PHY_CTRL(sunxi_phy->phy_reg, phy_offset));

		/*write data to VC_DI*/
		temp = readl(USBC_REG_PHY_CTRL(sunxi_phy->phy_reg, phy_offset));
		temp &= ~VC_DI;//clear
		temp |= FIELD_PREP(VC_DI, dtmp & 0x01);  // write
		writel(temp, USBC_REG_PHY_CTRL(sunxi_phy->phy_reg, phy_offset));

		/*set VC_CLK high*/
		temp |= VC_CLK;
		writel(temp, USBC_REG_PHY_CTRL(sunxi_phy->phy_reg, phy_offset));

		/*right move one bit*/
		dtmp >>= 1;
	}

	/*set VC_CLK low*/
	temp = readl(USBC_REG_PHY_CTRL(sunxi_phy->phy_reg, phy_offset));
	temp &= ~VC_CLK;
	writel(temp, USBC_REG_PHY_CTRL(sunxi_phy->phy_reg, phy_offset));

	/*VC_EN disable*/
	temp = readl(USBC_REG_PHY_CTRL(sunxi_phy->phy_reg, phy_offset));
	temp &= ~VC_EN;
	writel(temp, USBC_REG_PHY_CTRL(sunxi_phy->phy_reg, phy_offset));

	return 0;
}

static u32 sunxi_phy_VCbus_read(struct sunxi_phy *phy, u32 addr, u32 len)
{
	u32 j = 0;
	u32 ret = 0;
	u32 temp = 0;
	u32 phy_offset = phy->num; /* transciver register 0-2 */
	struct sunxi_phy_plat *sunxi_phy = phy->sunxi_phy;

	if (addr < 0x60) /* common register */
		phy_offset = 3;

	/*VC_EN enable*/
	temp = readl(USBC_REG_PHY_CTRL(sunxi_phy->phy_reg, phy_offset));
	temp |= VC_EN;
	writel(temp, USBC_REG_PHY_CTRL(sunxi_phy->phy_reg, phy_offset));

	for (j = len; j > 0; j--) {
		/*set write address*/
		temp = readl(USBC_REG_PHY_CTRL(sunxi_phy->phy_reg, phy_offset));
		temp &= ~VC_ADDR;//clear
		temp |= FIELD_PREP(VC_ADDR, addr + j - 1);  // write
		writel(temp, USBC_REG_PHY_CTRL(sunxi_phy->phy_reg, phy_offset));

		/*delsy 1us*/
		udelay(1);

		/*read data from VC_DO*/
		temp = readl(USBC_REG_PHY_STATUS(sunxi_phy->phy_reg, phy_offset));
		ret <<= 1;
		ret |= temp & VC_DO;
	}

	/*VC_EN disable*/
	temp = readl(USBC_REG_PHY_CTRL(sunxi_phy->phy_reg, phy_offset));
	temp &= ~VC_EN;
	writel(temp, USBC_REG_PHY_CTRL(sunxi_phy->phy_reg, phy_offset));

	return ret;
}

static int sunxi_phy_set_calibrate(struct phy *_phy)
{
	struct sunxi_phy *uphy = phy_get_drvdata(_phy);
	struct sunxi_phy_cal *cal;
	u32 i, phy_role = 0;

	switch (uphy->mode) {
	case PHY_MODE_USB_DEVICE:
	case PHY_MODE_USB_DEVICE_LS:
	case PHY_MODE_USB_DEVICE_FS:
	case PHY_MODE_USB_DEVICE_HS:
	case PHY_MODE_USB_DEVICE_SS:
		phy_role = USB_DR_MODE_PERIPHERAL;
		break;
	case PHY_MODE_USB_HOST:
	case PHY_MODE_USB_HOST_LS:
	case PHY_MODE_USB_HOST_FS:
	case PHY_MODE_USB_HOST_HS:
	case PHY_MODE_USB_HOST_SS:
		phy_role = USB_DR_MODE_HOST;
		break;
	case PHY_MODE_USB_OTG:
		phy_role = USB_DR_MODE_OTG;
		break;
	default:
		/* TODO */
		break;
	}

	/* set normal calibrate*/
	if (uphy->cal_data && uphy->cal_num) {
		for (i = 0; i < uphy->cal_num; i++) {
			cal = &uphy->cal_data[i];
			if (cal->role == USB_DR_MODE_OTG || cal->role == phy_role) {
				sunxi_phy_VCbus_write(uphy, cal->addr, cal->data, cal->len);
				dev_dbg(uphy->sunxi_phy->dev,
					"[phy%d] PHY calibrate set addr[%02x]:%02x\n",
					uphy->num, cal->addr,
					sunxi_phy_VCbus_read(uphy, cal->addr, cal->len));
			}
		}
	}

	/* fix dpdm pull down*/
	if (uphy->dpdm_bypass_quirk && uphy->dpdm_bypass_data) {
		for (i = 0; i < uphy->dpdm_bypass_quirk; i++) {
			cal = &uphy->dpdm_bypass_data[i];
			if (cal->role == USB_DR_MODE_OTG || cal->role == phy_role) {
				sunxi_phy_VCbus_write(uphy, cal->addr, cal->data, cal->len);
				dev_dbg(uphy->sunxi_phy->dev,
					"[phy%d] PHY bypass set addr[%02x]:%02x\n",
					uphy->num, cal->addr,
					sunxi_phy_VCbus_read(uphy, cal->addr, cal->len));
			}
		}
	}

	return 0;
}

static int sunxi_phy_set_mode(struct phy *_phy, enum phy_mode mode, int submode)
{
	struct sunxi_phy *uphy = phy_get_drvdata(_phy);

	uphy->mode = mode;
	sunxi_phy_set_calibrate(_phy);

	dev_dbg(uphy->sunxi_phy->dev, "[phy%d] PHY mode set %x\n", uphy->num, mode);

	return 0;
}

/*****************************************************************
 *                          device attribute
 *****************************************************************/

static int parse_arg(char **argv, const char *buf, size_t count)
{
	int argc = 0;
	char *p = (char *)buf;

	pr_debug("%s\n", buf);

	while (p && (p[0] != '\0') && (argc < SUNXI_PHY_CMD_NUM)) {
		argv[argc++] = strsep(&p, " ,");
		pr_debug("[%d]%s\n", argc, argv[argc-1]);
	}

	return argc;
}

static ssize_t phy_reg_print(struct sunxi_phy *uphy, char *buf, u32 addr, u32 len)
{
	if (len > 1) /* read multi phy reg */
		return sprintf(buf, "AWPHY addr[0x%x--0x%x] = 0x%x\n\n",
			addr, addr + len - 1,
			sunxi_phy_VCbus_read(uphy, addr, len));
	else /* read single phy reg */
		return sprintf(buf, "AWPHY addr[0x%x] = 0x%x\n\n",
			addr, sunxi_phy_VCbus_read(uphy, addr, len));
}

static ssize_t help_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	return sprintf(buf,
		"\n----------------------------------------------------------------------\n\n"
		"NAME:\n"
		"    reg_dump & reg_write:\n"
		"USAGE:\n"
		"    read  single register: echo (0x)addr > reg_dump;cat reg_dump\n"
		"    read  multi  register: echo (0x)addr,(0x)len > reg_dump;cat reg_dump\n"
		"    write single register: echo (0x)addr,(0x)data > reg_write\n"
		"    write multi  register: echo (0x)addr,(0x)data,(0x)len > reg_write\n"
		"RANGE:\n"
		"    addr:[0x0-0xe3]\n"
		"    Each register address corresponds to 1 bit\n"
		"DESCRIPTION:\n"
		"    AWPHY Register description please refer to SPEC/FAE.\n"
		"\n\n"
		"NAME:\n"
		"    tx_vref_tune\n"
		"USAGE:\n"
		"    read:  cat tx_vref_tune\n"
		"    write: echo (0x)data > tx_vref_tune\n"
		"RANGE:\n"
		"    [0x0-0xf]\n"
		"DESCRIPTION:\n"
		"    TX Voltage Reference Tune.Can adjust the height of Eye Diagram.Used to\n"
		"    optimize the transmission voltage levels for improved signal integrity\n"
		"    and performance.\n"
		"\n\n"
		"NAME:\n"
		"    tx_preempamp_tune\n"
		"USAGE:\n"
		"    read:  cat tx_preempamp_tune\n"
		"    write: echo (0x)data > tx_preempamp_tune\n"
		"RANGE:\n"
		"    [0x0-0x3]\n"
		"DESCRIPTION:\n"
		"    TX Amplitude of Pre-emphasis Pulse.Can adjust the overshoot or collapse\n"
		"    of the Eye Diagram edges.Used to offset the effects of long cables.\n"
		"\n\n"
		"NAME:\n"
		"    tx_ristune\n"
		"USAGE:\n"
		"    read:  cat tx_ristune\n"
		"    write: echo (0x)data > tx_ristune\n"
		"RANGE:\n"
		"    [0x0-0x3]\n"
		"DESCRIPTION:\n"
		"    TX Transmitter Rise Time.Can adjust the eye diagram slope.Used to optimize\n"
		"    the transmitted signal for enhanced signal quality and performance."
		"\n\n"
		"NAME:\n"
		"    tx_preemppulse_tune\n"
		"USAGE:\n"
		"    read:  cat tx_preemppulse_tune\n"
		"    write: echo (0x)data > tx_preemppulse_tune\n"
		"RANGE:\n"
		"    [0x0-0x1]\n"
		"DESCRIPTION:\n"
		"    TX Width of Pre-emphasis Pulse.Can adjust the overshoot or collapse of the\n"
		"    Eye Diagram edges.Used to offset the effects of long cables.\n"
		"\n----------------------------------------------------------------------\n\n");
}
static DEVICE_ATTR_RO(help);

static ssize_t reg_dump_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct sunxi_phy *uphy = container_of(dev, struct sunxi_phy, dev);

	return phy_reg_print(uphy, buf, reg_addr, reg_lenth);
}

static ssize_t reg_dump_store(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t count)
{
	int err, argc;
	u16 addr, len;
	char *argv[SUNXI_PHY_CMD_NUM];

	argc = parse_arg(argv, buf, count);

	if (argc == 1) { /* read single phy reg */
		err = kstrtou16(argv[0], 16, &addr);
		if (err)
			return err;

		if (addr > 0xe3 || addr < 0x0) {
			printk("Invalid addr 0x%x, data range[0x0-0xe3]\n", addr);
			goto finish;
		}

		reg_addr = addr;
		reg_lenth = 1;
	} else if (argc == 2) { /* read multi phy reg */
		err = kstrtou16(argv[0], 16, &addr);
		if (err)
			return err;

		err = kstrtou16(argv[1], 16, &len);
		if (err)
			return err;

		if (addr > 0xe3 || addr < 0x0) {
			printk("Invalid addr 0x%x, data range[0x0-0xe3]\n", (u32)addr);
			goto finish;
		}

		if (len > (0xe3 - addr + 1)) {
			printk("Invalid len 0x%x, len range[0x%x-0x%x]\n", len, (u32)addr, 0xe3 - addr + 1);
			goto finish;
		}

		reg_addr = addr;
		reg_lenth = len;
	} else
		printk("Invalid param! Please 'cat help'.\n");

finish:
	return count;
}
static DEVICE_ATTR_RW(reg_dump);

static ssize_t reg_write_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct sunxi_phy *uphy = container_of(dev, struct sunxi_phy, dev);

	return phy_reg_print(uphy, buf, reg_addr, reg_lenth);
}

static ssize_t reg_write_store(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t count)
{
	int err, argc;
	u16 addr, data, len;
	char *argv[SUNXI_PHY_CMD_NUM];
	struct sunxi_phy *uphy = container_of(dev, struct sunxi_phy, dev);

	argc = parse_arg(argv, buf, count);

	if (argc == 2) { /* set single phy reg */
		err = kstrtou16(argv[0], 16, &addr);
		if (err)
			return err;

		err = kstrtou16(argv[1], 16, &data);
		if (err)
			return err;

		if (addr > 0xe3 || addr < 0x0) {
			printk("Invalid addr 0x%x, data range[0x0-0xe3]\n", addr);
			goto finish;
		}

		if (data > 0x01 || data < 0x0) {
			printk("Invalid data 0x%x, data range[0x0-0x1]\n", data);
			goto finish;
		}

		reg_addr = addr;
		reg_lenth = 1;

		sunxi_phy_VCbus_write(uphy, reg_addr, data, reg_lenth);
	} else if (argc == 3) { /* set multi phy reg */
		err = kstrtou16(argv[0], 16, &addr);
		if (err)
			return err;

		err = kstrtou16(argv[1], 16, &data);
		if (err)
			return err;

		err = kstrtou16(argv[2], 16, &len);
		if (err)
			return err;

		if (addr > 0xe3 || addr < 0x0) {
			printk("Invalid addr 0x%x, data range[0x0-0xe3]\n", (u32)addr);
			goto finish;
		}

		if (data > GENMASK(len, 0) || data < 0x0) {
			printk("Invalid data 0x%x, data range[0x0-0x%x]\n", (u32)data, (u32)GENMASK(len, 0));
			goto finish;
		}

		if (len > (0xe3 - addr + 1)) {
			printk("Invalid len 0x%x, len range[0x%x-0x%x]\n", len, (u32)addr, 0xe3 - addr + 1);
			goto finish;
		}

		reg_addr = addr;
		reg_lenth = len;

		sunxi_phy_VCbus_write(uphy, reg_addr, data, reg_lenth);
	} else
		printk("Invalid param! Please 'cat help'.\n");

finish:
	return count;
}
static DEVICE_ATTR_RW(reg_write);

static ssize_t tx_vref_tune_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct sunxi_phy *uphy = container_of(dev, struct sunxi_phy, dev);

	return phy_reg_print(uphy, buf, AWPHY_REG_VREF_TUNE, AWPHY_REG_VREF_TUNE_LEN);
}

static ssize_t tx_vref_tune_store(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t count)
{
	int err;
	u32 data;
	struct sunxi_phy *uphy = container_of(dev, struct sunxi_phy, dev);

	err = kstrtoint(buf, 16, &data);
	if (err)
		return err;

	if (data > 0x0f || data < 0x0) {
		printk("Invalid addr 0x%x, data range[0x0-0xf]\n", data);
		return count;
	}

	sunxi_phy_VCbus_write(uphy, AWPHY_REG_VREF_TUNE, data, AWPHY_REG_VREF_TUNE_LEN);
	return count;
}
static DEVICE_ATTR_RW(tx_vref_tune);

static ssize_t tx_preempamp_tune_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct sunxi_phy *uphy = container_of(dev, struct sunxi_phy, dev);

	return phy_reg_print(uphy, buf, AWPHY_REG_PREEMPAMP_TUNE, AWPHY_REG_PREEMPAMP_TUNE_LEN);
}

static ssize_t tx_preempamp_tune_store(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t count)
{
	int err;
	u32 data;
	struct sunxi_phy *uphy = container_of(dev, struct sunxi_phy, dev);

	err = kstrtoint(buf, 16, &data);
	if (err)
		return err;

	if (data > 0x03 || data < 0x0) {
		printk("Invalid addr 0x%x, data range[0x0-0x3]\n", data);
		return count;
	}

	sunxi_phy_VCbus_write(uphy, AWPHY_REG_PREEMPAMP_TUNE, data, AWPHY_REG_PREEMPAMP_TUNE_LEN);
	return count;
}
static DEVICE_ATTR_RW(tx_preempamp_tune);

static ssize_t tx_ristune_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct sunxi_phy *uphy = container_of(dev, struct sunxi_phy, dev);

	return phy_reg_print(uphy, buf, AWPHY_REG_TX_RISTUNE, AWPHY_REG_TX_RISTUNE_LEN);
}

static ssize_t tx_ristune_store(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t count)
{
	int err;
	u32 data;
	struct sunxi_phy *uphy = container_of(dev, struct sunxi_phy, dev);

	err = kstrtoint(buf, 16, &data);
	if (err)
		return err;

	if (data > 0x03 || data < 0x0) {
		printk("Invalid addr 0x%x, data range[0x0-0x3]\n", data);
		return count;
	}

	sunxi_phy_VCbus_write(uphy, AWPHY_REG_TX_RISTUNE, data, AWPHY_REG_TX_RISTUNE_LEN);
	return count;
}
static DEVICE_ATTR_RW(tx_ristune);

static ssize_t tx_preemppulse_tune_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct sunxi_phy *uphy = container_of(dev, struct sunxi_phy, dev);

	return phy_reg_print(uphy, buf, AWPHY_REG_TX_PREEMPPULSE_TUNE, AWPHY_REG_TX_PREEMPPULSE_TUNE_LEN);
}

static ssize_t tx_preemppulse_tune_store(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t count)
{
	int err;
	u32 data;
	struct sunxi_phy *uphy = container_of(dev, struct sunxi_phy, dev);

	err = kstrtoint(buf, 16, &data);
	if (err)
		return err;

	if (data > 0x01 || data < 0x0) {
		printk("Invalid addr 0x%x, data range[0x0-0x1]\n", data);
		return count;
	}

	sunxi_phy_VCbus_write(uphy, AWPHY_REG_TX_PREEMPPULSE_TUNE, data,
					AWPHY_REG_TX_PREEMPPULSE_TUNE_LEN);
	return count;
}
static DEVICE_ATTR_RW(tx_preemppulse_tune);

/*****************************************************************
 *                           class attribute
 *****************************************************************/

static int phy_device_match(struct device *dev, const void *data)
{
	/* each phy device can point to upper sunxi_phy*/
	return 1;
}

static ssize_t
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(6, 6, 0))
version_show(const struct class *class, const struct class_attribute *attr, char *buf)
#else
version_show(struct class *class, struct class_attribute *attr, char *buf)
#endif
{
	struct device *dev;

	dev = class_find_device(class, NULL, "u2-phy", phy_device_match);

	if (dev) {
		struct sunxi_phy *uphy = container_of(dev, struct sunxi_phy, dev);
		return sprintf(buf, "Allwinner USB2.0 PHY Version v%lu.%lu.%lu\n\n",
			 FIELD_GET(PHY_GEN_VER, uphy->sunxi_phy->vernum),
			 FIELD_GET(PHY_SUB_VER, uphy->sunxi_phy->vernum),
			 FIELD_GET(PHY_PRJ_VER, uphy->sunxi_phy->vernum));
	}
	return sprintf(buf, "Device not found: %s\n", "u2-phy");
}
static CLASS_ATTR_RO(version);


static struct attribute *awphy_attrs[] = {
	__PHY_DEV_ATTR(help),
	__PHY_DEV_ATTR(reg_dump),
	__PHY_DEV_ATTR(reg_write),
	__PHY_DEV_ATTR(tx_vref_tune),
	__PHY_DEV_ATTR(tx_preempamp_tune),
	__PHY_DEV_ATTR(tx_ristune),
	__PHY_DEV_ATTR(tx_preemppulse_tune),
	NULL,
};
ATTRIBUTE_GROUPS(awphy);


static struct attribute *awphy_class_attrs[] = {
	__PHY_CLASS_ATTR(version),
	NULL,
};
ATTRIBUTE_GROUPS(awphy_class);

static struct class awphy_class = {
	.name = "awphy",
	.class_groups = awphy_class_groups,
};

static int sunxi_phy_sysfs_init(struct sunxi_phy *phy)
{
	int ret;
	struct device *dev;

	dev = &phy->dev;

	dev->class = &awphy_class;
	dev_set_name(dev, "%s", phy->name);

	ret = device_register(dev);
	if (ret)
		goto err_dev_register;

	ret = sysfs_create_groups(&dev->kobj, awphy_groups);
	if (ret)
		goto err_sysfs_create;

	dev_info(dev, "init awphy sysfs for %s, ret: %d\n", phy->name, ret);

	return 0;

err_sysfs_create:
	sysfs_remove_groups(&dev->kobj, awphy_groups);
err_dev_register:
	device_unregister(dev);

	return ret;
}

static int sunxi_phy_sysfs_deinit(struct sunxi_phy *phy)
{
	struct device *dev = &phy->dev;

	sysfs_remove_groups(&dev->kobj, awphy_groups);
	device_unregister(dev);

	return 0;
}

/*****************************************************************
 *                           aw-phyx
 *****************************************************************/

static int sunxi_phy0_init(struct phy *_phy)
{
	struct sunxi_phy *uphy = phy_get_drvdata(_phy);
	struct sunxi_phy_plat *sunxi_phy = uphy->sunxi_phy;
	int ret;

	if (uphy->ref_clk) {
		ret = clk_prepare_enable(uphy->ref_clk);
		if (ret) {
			dev_err(sunxi_phy->dev, "%s enable ref_clk err, return %d\n",
				uphy->name, ret);
			return ret;
		}
	}

	sunxi_phy_set(uphy, true);

	return 0;
}

static int sunxi_phy0_exit(struct phy *_phy)
{
	struct sunxi_phy *uphy = phy_get_drvdata(_phy);

	sunxi_phy_set(uphy, false);

	if (uphy->ref_clk)
		clk_disable_unprepare(uphy->ref_clk);

	return 0;
}

static const struct phy_ops sunxi_phy0_ops = {
	.init		= sunxi_phy0_init,
	.exit		= sunxi_phy0_exit,
	.set_mode	= sunxi_phy_set_mode,
	.owner		= THIS_MODULE,
};

static int sunxi_phy1_init(struct phy *_phy)
{
	struct sunxi_phy *uphy = phy_get_drvdata(_phy);
	struct sunxi_phy_plat *sunxi_phy = uphy->sunxi_phy;
	int ret;

	if (uphy->ref_clk) {
		ret = clk_prepare_enable(uphy->ref_clk);
		if (ret) {
			dev_err(sunxi_phy->dev, "%s enable ref_clk err, return %d\n",
				uphy->name, ret);
			return ret;
		}
	}

	sunxi_phy_set(uphy, true);

	return 0;
}

static int sunxi_phy1_exit(struct phy *_phy)
{
	struct sunxi_phy *uphy = phy_get_drvdata(_phy);

	sunxi_phy_set(uphy, false);

	if (uphy->ref_clk)
		clk_disable_unprepare(uphy->ref_clk);

	return 0;
}

static const struct phy_ops sunxi_phy1_ops = {
	.init		= sunxi_phy1_init,
	.exit		= sunxi_phy1_exit,
	.set_mode	= sunxi_phy_set_mode,
	.owner		= THIS_MODULE,
};

static int sunxi_phy2_init(struct phy *_phy)
{
	struct sunxi_phy *uphy = phy_get_drvdata(_phy);
	struct sunxi_phy_plat *sunxi_phy = uphy->sunxi_phy;
	int ret;

	/* if only use usb3.1_u3, don't init usb3.1_u2 phy*/
	if (uphy->u2u3_only_quirk == PHY_MODE_USB3)
		return 0;

	if (uphy->ref_clk) {
		ret = clk_prepare_enable(uphy->ref_clk);
		if (ret) {
			dev_err(sunxi_phy->dev, "%s enable ref_clk err, return %d\n",
				uphy->name, ret);
			return ret;
		}
	}

	if (uphy->u2_only_clk) {
		ret = clk_set_rate(uphy->u2_only_clk, 120000000);
		if (ret) {
			dev_err(sunxi_phy->dev, "set u2_only_clk rate 120MHz err, return %d\n", ret);
			return ret;
		}

		ret = clk_prepare_enable(uphy->u2_only_clk);
		if (ret) {
			dev_err(sunxi_phy->dev, "%s enable u2_only_clk err, return %d\n",
				uphy->name, ret);
			return ret;
		}
	}

	sunxi_phy_set(uphy, true);

	return 0;
}

static int sunxi_phy2_exit(struct phy *_phy)
{
	struct sunxi_phy *uphy = phy_get_drvdata(_phy);

	if (uphy->u2u3_only_quirk == PHY_MODE_USB3)
		return 0;

	sunxi_phy_set(uphy, false);

	if (uphy->u2_only_clk)
		clk_disable_unprepare(uphy->u2_only_clk);

	if (uphy->ref_clk)
		clk_disable_unprepare(uphy->ref_clk);

	return 0;
}

static const struct phy_ops sunxi_phy2_ops = {
	.init		= sunxi_phy2_init,
	.exit		= sunxi_phy2_exit,
	.set_mode	= sunxi_phy_set_mode,
	.owner		= THIS_MODULE,
};

/*****************************************************************
 *                        sunxi phy platform
 *****************************************************************/

int sunxi_phy_plat_create(struct device *dev, struct device_node *np,
			  struct sunxi_phy *uphy, enum sunxi_phy_type_e type)
{
	struct sunxi_phy_plat *sunxi_phy = dev_get_drvdata(dev);
	const struct phy_ops *ops;
	const __be32 *p;
	struct property *prop;
	int chip_ver, len, ret;

	switch (type) {
	case SUNXI_PHY0_TYPE:
		uphy->num = USB_XCVR0_PHY_NO;
		uphy->name = kstrdup_const("u2-phy0", GFP_KERNEL);
		ops = &sunxi_phy0_ops;
		break;
	case SUNXI_PHY1_TYPE:
		uphy->num = USB_XCVR1_PHY_NO;
		uphy->name = kstrdup_const("u2-phy1", GFP_KERNEL);
		ops = &sunxi_phy1_ops;
		break;
	case SUNXI_PHY2_TYPE:
		uphy->num = USB_XCVR2_PHY_NO;
		uphy->name = kstrdup_const("u2-phy2", GFP_KERNEL);
		ops = &sunxi_phy2_ops;
		break;
	default:
		pr_err("not support phy type (%d)\n", type);
		return -EINVAL;
	}

	uphy->ref_clk = devm_get_clk_from_child(dev, np, "ref_clk");
	if (IS_ERR(uphy->ref_clk)) {
		uphy->ref_clk = NULL;
		pr_debug("Maybe there is no ref clk for phy (%s)\n", uphy->name);
	}

	uphy->u2_only_clk = devm_get_clk_from_child(dev, np, "u2_only_clk");
	if (IS_ERR(uphy->u2_only_clk)) {
		uphy->u2_only_clk = NULL;
		pr_debug("Maybe there is no u2 only pipe clk for phy (%s)\n", uphy->name);
	}

	uphy->phy = devm_phy_create(dev, np, ops);
	if (IS_ERR(uphy->phy)) {
		ret = PTR_ERR(uphy->phy);
		dev_err(dev, "failed to create phy for %s, ret: %d\n", uphy->name, ret);
		return ret;
	}

	if (of_get_property(np, "calibrate", &len)) {
		if (len && (len % sizeof(*uphy->cal_data) == 0)) {
			uphy->cal_data = devm_kzalloc(dev, len, GFP_KERNEL);
			if (!uphy->cal_data) {
				dev_err(dev, "%s Failed allocate calibrate!\n", uphy->name);
				return (-ENOMEM);
			}

			ret = of_property_read_u32_array(np, "calibrate",
							 (u32 *)uphy->cal_data, len / sizeof(len));
			if (ret) {
				dev_err(dev, "%s Failed read calibrate %d!\n", uphy->name, ret);
				return ret;
			}

			uphy->cal_num = len / sizeof(*uphy->cal_data);
		} else {
			dev_err(dev, "%s Invalid calibrate elements (%d)!\n", uphy->name, len);
		}
	}

	of_property_for_each_u32(np, "aw,dpdm-bypass-ver", prop, p, chip_ver) {
		if (chip_ver == sunxi_get_soc_ver()) { /*need bypass*/
			if (of_get_property(np, "dpdm-bypass-quirk", &len)) {
				if (!len || (len % sizeof(*uphy->dpdm_bypass_data))) {
					dev_err(dev, "%s Invalid dpdm-bypass-quirk (%d)!\n", uphy->name, len);
					break;
				}

				uphy->dpdm_bypass_data = devm_kzalloc(dev, len, GFP_KERNEL);
				if (!uphy->dpdm_bypass_data) {
					dev_err(dev, "%s Fail alloc dpdm_bypass_data!\n", uphy->name);
					return -ENOMEM;
				}

				ret = of_property_read_u32_array(np, "dpdm-bypass-quirk",
									(u32 *)uphy->dpdm_bypass_data,
									len / sizeof(len));
				if (ret) {
					dev_err(dev, "%s Failed read dpdm_bypass_data %d!\n", uphy->name, ret);
					return ret;
				}

				uphy->dpdm_bypass_quirk = len / sizeof(*uphy->dpdm_bypass_data);
			}
			break;
		}
	}

	of_property_for_each_u32(np, "aw,u2u3-only-ver", prop, p, chip_ver) {
		if (chip_ver == sunxi_get_soc_ver()) {
			of_property_read_u32(np, "aw,u2u3-only-quirk", &uphy->u2u3_only_quirk);
			break;
		}
	}

	uphy->sunxi_phy = sunxi_phy;
	phy_set_drvdata(uphy->phy, uphy);

	ret = sunxi_phy_sysfs_init(uphy);
	if (ret) {
		dev_err(&uphy->dev, "failed to init awphy sysfs for %s, ret: %d\n", uphy->name, ret);
		return ret;
	}

	return 0;
}

static struct phy *sunxi_phy_plat_xlate(struct device *dev,
					struct of_phandle_args *args)
{
	struct phy *phy = NULL;

	phy = of_phy_simple_xlate(dev, args);
	if (IS_ERR(phy)) {
		pr_err("%s fail\n", __func__);
		return phy;
	}

	/* TODO: if need */

	return phy;
}

static int sunxi_phy_plat_init(struct sunxi_phy_plat *sunxi_phy)
{
	int ret;

	if (sunxi_phy->pclk) {
		ret = clk_prepare_enable(sunxi_phy->pclk);
		if (ret) {
			dev_err(sunxi_phy->dev, "enable ahb master clk err, return %d\n", ret);
			return ret;
		}
	}

	if (sunxi_phy->reset) {
		ret = reset_control_deassert(sunxi_phy->reset);
		if (ret) {
			dev_err(sunxi_phy->dev, "reset bus err, return %d\n", ret);
			return ret;
		}
	}

	if (sunxi_phy->mclk) {
		ret = clk_prepare_enable(sunxi_phy->mclk);
		if (ret) {
			dev_err(sunxi_phy->dev, "enable ahb clk err, return %d\n", ret);
			return ret;
		}
	}

	if (sunxi_phy->usb_reset) {
		ret = reset_control_deassert(sunxi_phy->usb_reset);
		if (ret) {
			dev_err(sunxi_phy->dev, "reset usb err, return %d\n", ret);
			return ret;
		}
	}

	sunxi_phy->vernum = phy_ver_get(sunxi_phy);

	return 0;
}

static void sunxi_phy_plat_exit(struct sunxi_phy_plat *sunxi_phy)
{
	if (sunxi_phy->usb_reset)
		reset_control_assert(sunxi_phy->usb_reset);

	if (sunxi_phy->mclk)
		clk_disable_unprepare(sunxi_phy->mclk);

	if (sunxi_phy->reset)
		reset_control_assert(sunxi_phy->reset);

	if (sunxi_phy->pclk)
		clk_disable_unprepare(sunxi_phy->pclk);
}

static int sunxi_phy_plat_parse_dt(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct sunxi_phy_plat *sunxi_phy = dev_get_drvdata(dev);
	struct device_node *child;
	int ret;

	/* parse top register, which determide general configuration such as mode */
	sunxi_phy->phy_reg = devm_platform_ioremap_resource_byname(pdev, "phy_base");
	if (IS_ERR(sunxi_phy->phy_reg))
		return PTR_ERR(sunxi_phy->phy_reg);

	sunxi_phy->top_reg = devm_platform_ioremap_resource_byname(pdev, "top_base");
	if (IS_ERR(sunxi_phy->top_reg))
		return PTR_ERR(sunxi_phy->top_reg);

	sunxi_phy->pclk = devm_clk_get(dev, "pclk");
	if (IS_ERR(sunxi_phy->pclk)) {
		return dev_err_probe(dev, PTR_ERR(sunxi_phy->pclk),
				     "failed to get ahb master clock for sunxi phy\n");
	}

	sunxi_phy->mclk = devm_clk_get(dev, "mclk");
	if (IS_ERR(sunxi_phy->mclk)) {
		return dev_err_probe(dev, PTR_ERR(sunxi_phy->mclk),
				     "failed to get ahb clock for sunxi phy\n");
	}

	if (sunxi_phy->cfg->has_rst) {
		sunxi_phy->reset = devm_reset_control_get_shared(dev, "rst");
		if (IS_ERR(sunxi_phy->reset)) {
			return dev_err_probe(dev, PTR_ERR(sunxi_phy->reset),
				     "failed to get reset for sunxi phy\n");
		}
	}

	sunxi_phy->usb_reset = devm_reset_control_get_shared(dev, "usb_rst");
	if (IS_ERR(sunxi_phy->usb_reset)) {
		return dev_err_probe(dev, PTR_ERR(sunxi_phy->usb_reset),
				     "failed to get usb reset for sunxi phy\n");
	}

	for_each_available_child_of_node(dev->of_node, child) {
		if (of_node_name_eq(child, "u2-phy0")) {
			sunxi_phy->uphy0 = devm_kzalloc(dev, sizeof(*sunxi_phy->uphy0), GFP_KERNEL);
			if (!sunxi_phy->uphy0)
				return -ENOMEM;

			/* create u2 phy0 */
			ret = sunxi_phy_plat_create(dev, child, sunxi_phy->uphy0, SUNXI_PHY0_TYPE);
			if (ret) {
				dev_err(dev, "failed to create u2phy0, ret:%d\n", ret);
				goto err_node_put;
			}

		} else if (of_node_name_eq(child, "u2-phy1")) {
			sunxi_phy->uphy1 = devm_kzalloc(dev, sizeof(*sunxi_phy->uphy1), GFP_KERNEL);
			if (!sunxi_phy->uphy1)
				return -ENOMEM;

			/* create u2 phy1 */
			ret = sunxi_phy_plat_create(dev, child, sunxi_phy->uphy1, SUNXI_PHY1_TYPE);
			if (ret) {
				dev_err(dev, "failed to create u2phy1, ret:%d\n", ret);
				goto err_node_put;
			}

		} else if (of_node_name_eq(child, "u2-phy2")) {
			sunxi_phy->uphy2 = devm_kzalloc(dev, sizeof(*sunxi_phy->uphy2), GFP_KERNEL);
			if (!sunxi_phy->uphy2)
				return -ENOMEM;

			/* create u2 phy2 */
			ret = sunxi_phy_plat_create(dev, child, sunxi_phy->uphy2, SUNXI_PHY2_TYPE);
			if (ret) {
				dev_err(dev, "failed to create u2phy2, ret:%d\n", ret);
				goto err_node_put;
			}
		}
	}

	return 0;

err_node_put:
	of_node_put(child);

	return ret;
}

static int sunxi_phy_plat_probe(struct platform_device *pdev)
{
	struct sunxi_phy_plat *sunxi_phy;
	struct device *dev = &pdev->dev;
	struct phy_provider *phy_provider;
	int ret;

	/* create dir: /sys/class/awphy */
	ret = class_register(&awphy_class);
	if (ret) {
		dev_err(dev, "dir awphy creat fail %x!\n", ret);
		class_unregister(&awphy_class);
		return -EINVAL;
	}

	sunxi_phy = devm_kzalloc(dev, sizeof(*sunxi_phy), GFP_KERNEL);
	if (!sunxi_phy)
		return -ENOMEM;

	sunxi_phy->dev = dev;
	dev_set_drvdata(dev, sunxi_phy);

	sunxi_phy->cfg = of_device_get_match_data(&pdev->dev);
	if (!sunxi_phy->cfg)
		return -ENODEV;

	ret = sunxi_phy_plat_parse_dt(pdev);
	if (ret)
		return -EINVAL;

	phy_provider = devm_of_phy_provider_register(dev, sunxi_phy_plat_xlate);
	if (IS_ERR(phy_provider))
		return PTR_ERR_OR_ZERO(phy_provider);

	ret = sunxi_phy_plat_init(sunxi_phy);
	if (ret)
		return -EINVAL;

	dev_info(dev, "Allwinner USB2.0 PHY Version v%lu.%lu.%lu\n",
		 FIELD_GET(PHY_GEN_VER, sunxi_phy->vernum),
		 FIELD_GET(PHY_SUB_VER, sunxi_phy->vernum),
		 FIELD_GET(PHY_PRJ_VER, sunxi_phy->vernum));

	return 0;
}

static int sunxi_phy_plat_remove(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct sunxi_phy_plat *sunxi_phy = dev_get_drvdata(dev);

	if (sunxi_phy->uphy0)
		sunxi_phy_sysfs_deinit(sunxi_phy->uphy0);

	if (sunxi_phy->uphy1)
		sunxi_phy_sysfs_deinit(sunxi_phy->uphy1);

	if (sunxi_phy->uphy2)
		sunxi_phy_sysfs_deinit(sunxi_phy->uphy2);

	sunxi_phy_plat_exit(sunxi_phy);
	class_unregister(&awphy_class);

	return 0;
}

static int __maybe_unused sunxi_phy_plat_suspend(struct device *dev)
{
	struct sunxi_phy_plat *sunxi_phy = dev_get_drvdata(dev);

	sunxi_phy_plat_exit(sunxi_phy);

	return 0;
}

static int __maybe_unused sunxi_phy_plat_resume(struct device *dev)
{
	struct sunxi_phy_plat *sunxi_phy = dev_get_drvdata(dev);
	int ret;

	ret = sunxi_phy_plat_init(sunxi_phy);
	if (ret) {
		dev_err(dev, "failed to resume awphy\n");
		return ret;
	}

	return 0;
}

static struct dev_pm_ops sunxi_phy_plat_pm_ops = {
	SET_SYSTEM_SLEEP_PM_OPS(sunxi_phy_plat_suspend, sunxi_phy_plat_resume)
};

static struct sunxi_phy_config sunxi_phy_v100 = {
	.has_rst = true,
};

static struct sunxi_phy_config sunxi_phy_v101 = {
	.has_rst	= false,
};

static const struct of_device_id sunxi_phy_plat_of_match_table[] = {
	{ .compatible = "allwinner,sunxi-awphy-v100", .data = &sunxi_phy_v100 },
	{ .compatible = "allwinner,sunxi-awphy-v101", .data = &sunxi_phy_v101 },
	{ /* Sentinel */ }
};

static struct platform_driver sunxi_phy_plat_driver = {
	.probe		= sunxi_phy_plat_probe,
	.remove		= sunxi_phy_plat_remove,
	.driver = {
		.name	= "sunxi-plat-awphy",
		.pm	= &sunxi_phy_plat_pm_ops,
		.of_match_table = sunxi_phy_plat_of_match_table,
	},
};
module_platform_driver(sunxi_phy_plat_driver);

MODULE_ALIAS("platform:sunxi-plat-awphy");
MODULE_DESCRIPTION("Allwinner Platform USB2.0 AW PHY driver");
MODULE_AUTHOR("kanghoupeng<kanghoupeng@allwinnertech.com>");
MODULE_LICENSE("GPL v2");
MODULE_VERSION("0.0.11");
