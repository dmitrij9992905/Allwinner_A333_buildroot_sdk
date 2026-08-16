/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Copyright(c) 2020 - 2023 Allwinner Technology Co.,Ltd. All rights reserved. */
#define pr_fmt(x) KBUILD_MODNAME ": " x "\n"

#include "axp517_charger.h"

struct axp517_usb_power {
	/* base */
	char                      *name;
	struct device             *dev;
	struct regmap             *regmap;
	struct power_supply       *usb_supply;
	struct axp_config_info    dts_info;
	struct delayed_work       usb_supply_mon;

	/* input limit */
	struct delayed_work       usb_chg_state;
	struct wakeup_source	  *vbus_input_check;
	atomic_t                  set_current_limit;
	atomic_t                  set_input_vol;

	/* bat supply */
	bool                      battery_supply_exist;
	bool                      battery_supply_enable;

	/* acin_power_supply */
	struct power_supply       *acin_supply;

	/* gpio_vbus_detect */
	struct axp_gpio_para      axp_vbus_det;
	struct delayed_work       usb_det_mon;

	/* vbus_detect_type */
	int                       vbus_detect_type;

	/* usb notifier */
	struct notifier_block	  usb_nb;
};

static enum power_supply_property axp517_usb_props[] = {
	/* real_time */
	POWER_SUPPLY_PROP_ONLINE,
	POWER_SUPPLY_PROP_PRESENT,
	POWER_SUPPLY_PROP_VOLTAGE_NOW,
	POWER_SUPPLY_PROP_INPUT_CURRENT_LIMIT,
	POWER_SUPPLY_PROP_TEMP,
	POWER_SUPPLY_PROP_SCOPE,
	POWER_SUPPLY_PROP_USB_TYPE,
	/* static */
	POWER_SUPPLY_PROP_MANUFACTURER,
	POWER_SUPPLY_PROP_VOLTAGE_MIN_DESIGN,
};

const enum power_supply_usb_type axp517_usb_types_array[] = {
    POWER_SUPPLY_USB_TYPE_SDP,
    POWER_SUPPLY_USB_TYPE_DCP,
};

static int axp517_acin_function_init(struct axp517_usb_power *usb_power)
{
	struct device_node *np = NULL;
	static int acin_check;

	if (!acin_check) {
		usb_power->acin_supply = NULL;
		np = of_parse_phandle(usb_power->dev->of_node, "det_acin_supply", 0);

		if (!of_device_is_available(np)) {
			acin_check = 1;
		}
		acin_check = 2;
	}

	return acin_check - 1;
}

static int axp517_check_acin(struct axp517_usb_power *usb_power)
{
	union power_supply_propval temp;

	if (!axp517_acin_function_init(usb_power))
		return 0;

	if (usb_power->vbus_detect_type == DETEC_BY_VBUS)
		return 0;

	if (usb_power->acin_supply == NULL) {
		usb_power->acin_supply = devm_power_supply_get_by_phandle(usb_power->dev,
									"det_acin_supply");
		if (!usb_power->acin_supply)
			return 0;
	}
	power_supply_get_property(usb_power->acin_supply, POWER_SUPPLY_PROP_ONLINE, &temp);

	return temp.intval;
}

static int axp517_check_battery(struct axp517_usb_power *usb_power)
{
	struct device_node *np = NULL;
	static int battery_check;

	if (!battery_check) {
		battery_check = 1;
		np = of_parse_phandle(usb_power->dev->of_node, "det_battery_supply", 0);
		if (np) {
			if (of_device_is_available(np))
				battery_check = 2;
		}
	}

	return battery_check - 1;
}

static int _axp517_get_vbus_state(struct regmap *regmap)
{
	unsigned int data;
	int ret = 0;

	ret = regmap_read(regmap, AXP517_STATUS0, &data);
	if (ret < 0)
		return ret;

	ret = !!(data & AXP517_MASK_VBUS_STAT);

	return ret;
}

static int _axp517_get_vbus_state_by_typec_status(struct regmap *regmap)
{
	unsigned int data;
	int ret = 0;

	ret = regmap_read(regmap, AXP517_CC_CNNT_STA, &data);
	if (ret < 0)
		return ret;

	data &= AXP515_MASK_VBUS_STAT_BY_TYPEC;
	if (data == AXP515_TYPEC_STAT_DEBUG) {
		mdelay(30);

		ret = regmap_read(regmap, AXP517_CC_STATUS, &data);
		if (ret < 0)
			return ret;
		ret = (data & BIT(4)) ? 1 : 0;

	} else {
		ret = ((data > 0) && ((data <= AXP515_VBUS_STAT_BY_TYPEC_EXIST_MAX) || (data == AXP515_MASK_VBUS_STAT_BY_TYPEC))) ? 1 : 0;
	}

	return ret;
}

static int axp517_get_vbus_state(struct power_supply *ps,
				   union power_supply_propval *val)
{
	struct axp517_usb_power *usb_power = power_supply_get_drvdata(ps);
	struct regmap *regmap = usb_power->regmap;
	int ret = 0;

	switch (usb_power->vbus_detect_type) {
	case DETEC_BY_TYPEC:
		ret = _axp517_get_vbus_state_by_typec_status(regmap);
		break;
	case DETEC_BY_GPIO:
		ret = gpio_get_value(usb_power->axp_vbus_det.gpio);
		break;
	default:
		ret = _axp517_get_vbus_state(regmap);
		break;
	}

	if (ret < 0)
		return ret;

	val->intval = ret;
	return 0;
}

static int _axp517_get_iin_limit(struct axp517_usb_power *usb_supply)
{
	struct regmap *regmap = usb_supply->regmap;
	unsigned int data;
	int limit_cur, ret = 0;

	ret = regmap_read(regmap, AXP517_IIN_LIM, &data);
	if (ret < 0)
		return ret;

	data &= ~(0x3);

	limit_cur = (data >> 2) * 50 + 100;

	return limit_cur;
}

static int axp517_get_iin_limit(struct power_supply *ps,
				   union power_supply_propval *val)
{
	struct axp517_usb_power *usb_supply = power_supply_get_drvdata(ps);
	int limit_cur;

	limit_cur = _axp517_get_iin_limit(usb_supply);
	if (limit_cur < 0)
		return limit_cur;

	val->intval = limit_cur;

	return 0;
}

static int axp517_get_vindpm(struct power_supply *ps,
				   union power_supply_propval *val)
{
	struct axp517_usb_power *usb_power = power_supply_get_drvdata(ps);
	struct regmap *regmap = usb_power->regmap;
	unsigned int data;
	int ret = 0;

	ret = regmap_read(regmap, AXP517_VINDPM_CFG, &data);
	if (ret < 0)
		return ret;

	data &= ~(0x80);
	if (!data)
		data = 1;

	data = ((data - 1) * 100) + 3600;
	val->intval = data;

	return 0;
}

static int axp517_get_usb_type(struct power_supply *ps,
				   union power_supply_propval *val)
{
	struct axp517_usb_power *usb_power = power_supply_get_drvdata(ps);

	if (atomic_read(&usb_power->set_current_limit)) {
		val->intval = POWER_SUPPLY_USB_TYPE_SDP;
	} else {
		val->intval = POWER_SUPPLY_USB_TYPE_DCP;
	}

	return 0;
}

/* read temperature */
static inline int axp517_tdie_to_temp(u32 reg)
{
	return (721 * 5 - ((int)(((reg >> 8) << 4) | (reg & 0x000F)))) * 100 / 191 / 5;
}

static inline int axp517_get_tdie_adc_temp(struct regmap *regmap)
{
	unsigned char temp_val[2];
	unsigned int reg_value;
	u32 ts_res;
	int die_temp;
	int ret = 0;

	ret = regmap_read(regmap, AXP517_ADC_CONTROL, &reg_value);
	if (ret < 0)
		return ret;

	if ((reg_value & 0xf) != AXP517_ADC_TDIE) {
		reg_value &= ~(0xf);
		reg_value |= AXP517_ADC_TDIE;
		ret = regmap_write(regmap, AXP517_ADC_CONTROL, reg_value);
		if (ret < 0)
			return ret;
		mdelay(1);
	}

	ret = regmap_bulk_read(regmap, AXP517_ADC_RES, temp_val, 2);
	if (ret < 0)
		return ret;

	temp_val[0] &= GENMASK(5, 0);
	ts_res = (temp_val[0] << 8) | temp_val[1];
	die_temp = axp517_tdie_to_temp(ts_res);

	return die_temp;
}

static int axp517_get_ic_temp(struct power_supply *ps,
			     union power_supply_propval *val)
{
	struct axp517_usb_power *usb_power = power_supply_get_drvdata(ps);
	struct regmap *regmap = usb_power->regmap;

	int i = 0, temp, old_temp;

	old_temp = axp517_get_tdie_adc_temp(regmap);

	/* read until abs(old_temp - temp) < 10*/
	temp = axp517_get_tdie_adc_temp(regmap);

	PMIC_DEBUG("old_temp:%d, temp:%d\n", old_temp, temp);
	while ((abs(old_temp - temp) > 100) && (i < 10)) {
		old_temp = temp;
		temp = axp517_get_tdie_adc_temp(regmap);
		i++;
		PMIC_DEBUG("turn[%d]:old_temp:%d, temp:%d\n", i, old_temp, temp);
	}

	val->intval = temp;

	return 0;
}

static int axp517_set_iin_limit(struct regmap *regmap, int mA)
{
	unsigned int data;
	int ret = 0;

	data = mA;
	if (data > 3250)
		data = 3250;

	if	(data < 100)
		data = 100;

	data = ((data - 100) / 50) << 2;

	ret = regmap_update_bits(regmap, AXP517_IIN_LIM, GENMASK(7, 0),
				 data);

	if (ret < 0)
		return ret;

	return 0;
}

static int axp517_set_vindpm(struct regmap *regmap, int mV)
{
	unsigned int data;
	int ret = 0;

	data = mV;

	if (data > 16200)
		data = 16200;
	if	(data < 3600)
		data = 3600;

	data = ((data - 3600) / 100) + 1;
	ret = regmap_update_bits(regmap, AXP517_VINDPM_CFG, GENMASK(6, 0),
				 data);
	if (ret < 0)
		return ret;

	return 0;
}

static int axp517_usb_get_property(struct power_supply *psy,
				       enum power_supply_property psp,
				       union power_supply_propval *val)
{
	struct axp517_usb_power *usb_power = power_supply_get_drvdata(psy);
	int ret = 0;

	switch (psp) {
	case POWER_SUPPLY_PROP_ONLINE:
		ret = axp517_get_vbus_state(psy, val);
		break;
	case POWER_SUPPLY_PROP_PRESENT:
		ret = axp517_get_vbus_state(psy, val);
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_NOW:
		val->intval = atomic_read(&usb_power->set_input_vol);
		break;
	case POWER_SUPPLY_PROP_INPUT_CURRENT_LIMIT:
		ret = axp517_get_iin_limit(psy, val);
		break;
	case POWER_SUPPLY_PROP_TEMP:
		ret = axp517_get_ic_temp(psy, val);
		break;
	case POWER_SUPPLY_PROP_USB_TYPE:
		ret = axp517_get_usb_type(psy, val);
		break;
	case POWER_SUPPLY_PROP_MANUFACTURER:
		val->strval = AXP517_MANUFACTURER;
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_MIN_DESIGN:
		ret = axp517_get_vindpm(psy, val);
		break;
	default:
		break;
	}

	return ret;
}

static int axp517_usb_set_property(struct power_supply *psy,
				enum power_supply_property psp,
				const union power_supply_propval *val)
{
	struct axp517_usb_power *usb_power = power_supply_get_drvdata(psy);
	struct axp_config_info *dinfo = &usb_power->dts_info;

	struct regmap *regmap = usb_power->regmap;
	int ret = 0, usb_cur;

	switch (psp) {
	case POWER_SUPPLY_PROP_VOLTAGE_NOW:
		if (!val->intval)
			break;
		atomic_set(&usb_power->set_input_vol, val->intval);
		if (val->intval >= 9000)
			axp517_set_vindpm(regmap, 5500);
		else
			axp517_set_vindpm(regmap, dinfo->pmu_usbad_vol);
		break;
	case POWER_SUPPLY_PROP_INPUT_CURRENT_LIMIT:
		if (!val->intval)
			break;
		if (!axp517_check_battery(usb_power))
			break;

		usb_cur = val->intval;
		if (usb_cur < usb_power->dts_info.pmu_usbad_cur) {
			atomic_set(&usb_power->set_current_limit, 1);
			if (axp517_check_acin(usb_power))
				return ret;
		}

		ret = axp517_set_iin_limit(regmap, usb_cur);
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_MIN_DESIGN:
		ret = axp517_set_vindpm(regmap, val->intval);
		break;
	default:
		ret = -EINVAL;
	}
	return ret;
}

static int axp517_usb_power_property_is_writeable(struct power_supply *psy,
			     enum power_supply_property psp)
{
	int ret = 0;
	switch (psp) {
	case POWER_SUPPLY_PROP_VOLTAGE_NOW:
		ret = 0;
		break;
	case POWER_SUPPLY_PROP_INPUT_CURRENT_LIMIT:
		ret = 0;
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_MIN_DESIGN:
		ret = 0;
		break;
	default:
		ret = -EINVAL;
	}
	return ret;

}

static const struct power_supply_desc axp517_usb_desc = {
	.name = "axp517-usb",
	.type = POWER_SUPPLY_TYPE_USB,
	.usb_types = axp517_usb_types_array,
	.num_usb_types = ARRAY_SIZE(axp517_usb_types_array),
	.get_property = axp517_usb_get_property,
	.properties = axp517_usb_props,
	.set_property = axp517_usb_set_property,
	.num_properties = ARRAY_SIZE(axp517_usb_props),
	.property_is_writeable = axp517_usb_power_property_is_writeable,
};

static void axp517_irq_limit_input_process(struct axp517_usb_power *usb_power)
{
	struct axp_config_info *axp_config = &usb_power->dts_info;

	if (!axp_config->pmu_bc12_en) {
		if (!axp517_check_acin(usb_power)) {
			if (usb_power->vbus_detect_type != DETEC_BY_TYPEC)
				axp517_set_iin_limit(usb_power->regmap, axp_config->pmu_usbpc_cur);
			cancel_delayed_work_sync(&usb_power->usb_chg_state);
			__pm_stay_awake(usb_power->vbus_input_check);
			schedule_delayed_work(&usb_power->usb_chg_state, msecs_to_jiffies(5 * 1000));
		}
		atomic_set(&usb_power->set_current_limit, 0);
	}
}

static irqreturn_t axp517_irq_handler_usb_in(int irq, void *data)
{
	struct axp517_usb_power *usb_power = data;

	if (usb_power->vbus_detect_type != DETEC_BY_VBUS)
		return IRQ_HANDLED;

	power_supply_changed(usb_power->usb_supply);

	if (!axp517_check_battery(usb_power))
		return IRQ_HANDLED;

	axp517_irq_limit_input_process(usb_power);

	return IRQ_HANDLED;
}

static irqreturn_t axp517_irq_handler_usb_out(int irq, void *data)
{
	struct axp517_usb_power *usb_power = data;

	if (usb_power->vbus_detect_type != DETEC_BY_VBUS)
		return IRQ_HANDLED;

	atomic_set(&usb_power->set_current_limit, 0);
	power_supply_changed(usb_power->usb_supply);

	return IRQ_HANDLED;
}

static irqreturn_t axp517_acin_vbus_det_isr(int irq, void *data)
{
	struct axp517_usb_power *usb_power = data;

	cancel_delayed_work_sync(&usb_power->usb_det_mon);
	schedule_delayed_work(&usb_power->usb_det_mon, 0);

	return IRQ_HANDLED;
}

enum axp517_usb_virq_index {
	AXP517_VIRQ_USB_IN,
	AXP517_VIRQ_USB_OUT,

	AXP517_USB_VIRQ_MAX_VIRQ,
};

static struct axp_interrupts axp_usb_irq[] = {
	[AXP517_VIRQ_USB_IN] = { "vbus_insert", axp517_irq_handler_usb_in },
	[AXP517_VIRQ_USB_OUT] = { "vbus_remove", axp517_irq_handler_usb_out },
};

static void axp517_usb_power_monitor(struct work_struct *work)
{
	struct axp517_usb_power *usb_power =
		container_of(work, typeof(*usb_power), usb_supply_mon.work);

	schedule_delayed_work(&usb_power->usb_supply_mon, msecs_to_jiffies(500));
}

static void axp517_usb_det_monitor(struct work_struct *work)
{
	struct axp517_usb_power *usb_power =
		container_of(work, typeof(*usb_power), usb_det_mon.work);
	int vbus_det_gpio_value;
	static int vbus_det_gpio_value_old;

	if (usb_power->vbus_detect_type == DETEC_BY_GPIO) {
		PMIC_ERR("[usb_det_gpio] acin_usb_det not used\n");
		return;
	}

	vbus_det_gpio_value = gpio_get_value(usb_power->axp_vbus_det.gpio);

	if (vbus_det_gpio_value_old == vbus_det_gpio_value) {
		return;
	} else {
		vbus_det_gpio_value_old = vbus_det_gpio_value;
	}

	power_supply_changed(usb_power->usb_supply);

	PMIC_INFO("[usb_det_gpio] vbus_dev_flag = %d\n", vbus_det_gpio_value);

	if (vbus_det_gpio_value) {
		if (!axp517_check_battery(usb_power))
			return;

		axp517_irq_limit_input_process(usb_power);
	}
}

static int axp517_usb_power_notifier(struct notifier_block *nb, unsigned long event, void *data)
{
	struct axp517_usb_power *usb_power = container_of(nb, struct axp517_usb_power, usb_nb);
	struct regmap *regmap = usb_power->regmap;
	int ret = 0;

	PMIC_INFO("notifier event %lu\n", event);

	switch (event) {
	case AW_PSY_EVENT_VBUS_ONLINE_CHECK:
		PMIC_INFO("vbus_online_check notify\n");
		mdelay(100);
		power_supply_changed(usb_power->usb_supply);
		if (_axp517_get_vbus_state_by_typec_status(regmap)) {
			if (axp517_check_battery(usb_power)) {
				if (atomic_read(&usb_power->set_input_vol) > 5000)
					break;
				axp517_irq_limit_input_process(usb_power);
			}
		} else {
			atomic_set(&usb_power->set_current_limit, 0);
		}
		break;
	default:
		ret = -EINVAL;
		break;
	}

	return NOTIFY_DONE;
}

static int axp517_acin_vbus_det_init(struct axp517_usb_power *usb_power)
{
	int ret = 0;
	struct axp_gpio_para axp_vbus_det;
	unsigned long irq_flags = 0;

	usb_power->axp_vbus_det.gpio = 0;
	usb_power->axp_vbus_det.irq_num = 0;

	axp_vbus_det.gpio =
		of_get_named_gpio(usb_power->dev->of_node,
				"pmu_vbus_det_gpio", 0);
	if (axp_vbus_det.gpio < 0) {
		PMIC_INFO("axp517 usb not detect by gpio\n");
		return 0;
	}

	if (!gpio_is_valid(axp_vbus_det.gpio)) {
		PMIC_ERR("get axp_vbus_det_gpio is fail\n");
		return -EPROBE_DEFER;
	}

	/* set vbus_det input usbid output */
	ret = gpio_request(axp_vbus_det.gpio,
			"pmu_vbus_det_gpio");
	if (ret != 0) {
		PMIC_ERR("pmu_vbus_det_gpio gpio_request failed\n");
		return -EINVAL;
	}
	gpio_direction_input(axp_vbus_det.gpio);

	/* init delay work */
	INIT_DELAYED_WORK(&usb_power->usb_det_mon, axp517_usb_det_monitor);

	/* irq config setting */
	irq_flags = IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING |
			IRQF_ONESHOT | IRQF_NO_SUSPEND;
	axp_vbus_det.irq_num = gpio_to_irq(axp_vbus_det.gpio);

	ret = devm_request_threaded_irq(usb_power->dev, axp_vbus_det.irq_num, NULL, axp517_acin_vbus_det_isr, irq_flags,
				"pmu_vbus_det_gpio", usb_power);
	if (IS_ERR_VALUE((unsigned long)ret)) {
		cancel_delayed_work_sync(&usb_power->usb_det_mon);
		PMIC_ERR("Requested pmu_vbus_det_gpio IRQ failed, err %d\n", ret);
		return -EINVAL;
	}

	usb_power->vbus_detect_type = DETEC_BY_GPIO;
	usb_power->axp_vbus_det = axp_vbus_det;

	PMIC_DEV_DEBUG(usb_power->dev, "Requested pmu_vbus_det_gpio IRQ successed: %d\n", ret);

	return 0;
}

static int axp517_usb_detect_type_init(struct axp517_usb_power *usb_power)
{
	int ret = 0;

	usb_power->vbus_detect_type = DETEC_UNKNOWN;

	/* detect by gpio */
	ret = axp517_acin_vbus_det_init(usb_power);
	if (ret != 0) {
		PMIC_ERR("gpio init failed\n");
		return ret;
	}

	if (usb_power->vbus_detect_type == DETEC_BY_GPIO) {
		PMIC_INFO("axp517 usb detect by gpio\n");
		return 0;
	}

	/* detect by cc-status */
	if (of_property_read_bool(usb_power->dev->of_node, "pmu_vbus_det_typec")) {
		usb_power->usb_nb.notifier_call = axp517_usb_power_notifier;
		ret = sunxi_power_supply_reg_notifier(&usb_power->usb_nb);
		if (ret < 0) {
			PMIC_ERR("failed to register notifier :%d\n", ret);
			return ret;
		}
		usb_power->vbus_detect_type = DETEC_BY_TYPEC;
		PMIC_INFO("axp517 usb detect by typec notify\n");
		return 0;
	}

	return -EINVAL;
}

static void axp517_usb_set_current_fsm(struct work_struct *work)
{
	struct axp517_usb_power *usb_power =
		container_of(work, typeof(*usb_power), usb_chg_state.work);
	struct axp_config_info *axp_config = &usb_power->dts_info;
	int limit_cur;

	if (atomic_read(&usb_power->set_input_vol) > 5000) {
		PMIC_INFO("current limit setted: usb pd type\n");
		atomic_set(&usb_power->set_current_limit, 0);
	} else {
		if (atomic_read(&usb_power->set_current_limit)) {
			PMIC_INFO("current limit setted: usb pc type\n");
		} else {
			axp517_set_iin_limit(usb_power->regmap, axp_config->pmu_usbad_cur);
			PMIC_INFO("current limit not set: usb adapter type\n");
		}
		limit_cur = _axp517_get_iin_limit(usb_power);
	}
	__pm_relax(usb_power->vbus_input_check);
}

static void axp517_usb_power_init(struct axp517_usb_power *usb_power)
{
	struct regmap *regmap = usb_power->regmap;
	struct axp_config_info *dinfo = &usb_power->dts_info;

	/* set default value */
	atomic_set(&usb_power->set_input_vol, 5000);

	/* set vindpm value */
	if (axp517_check_battery(usb_power))
		axp517_set_vindpm(regmap, dinfo->pmu_usbad_vol);

	/* enable die adc */
	regmap_update_bits(regmap, AXP517_ADC_CH_EN0, BIT(4), BIT(4));

	/* set bc12 en/disable  */
	if (!dinfo->pmu_bc12_en) {
		if (!axp517_check_battery(usb_power))
			axp517_set_iin_limit(usb_power->regmap, dinfo->pmu_usbad_cur);
	}
}

static int axp517_usb_dt_parse(struct device_node *node,
			 struct axp_config_info *axp_config)
{
	if (!of_device_is_available(node)) {
		PMIC_ERR("%s: failed\n", __func__);
		return -1;
	}


	AXP_OF_PROP_READ(pmu_usbpc_vol,                    4600);
	AXP_OF_PROP_READ(pmu_usbpc_cur,                    500);
	AXP_OF_PROP_READ(pmu_usbad_vol,                    4600);
	AXP_OF_PROP_READ(pmu_usbad_cur,                    1500);
	AXP_OF_PROP_READ(pmu_bc12_en,                         0);

	axp_config->wakeup_usb_in =
		of_property_read_bool(node, "wakeup_usb_in");
	axp_config->wakeup_usb_out =
		of_property_read_bool(node, "wakeup_usb_out");

	return 0;
}

static void axp517_usb_parse_device_tree(struct axp517_usb_power *usb_power)
{
	int ret;
	struct axp_config_info *cfg;

	/* set input current limit */
	if (!usb_power->dev->of_node) {
		PMIC_INFO("can not find device tree\n");
		return;
	}

	cfg = &usb_power->dts_info;
	ret = axp517_usb_dt_parse(usb_power->dev->of_node, cfg);
	if (ret) {
		PMIC_INFO("can not parse device tree err\n");
		return;
	}

	/*init axp517 usb by device tree*/
	axp517_usb_power_init(usb_power);
}

static int axp517_usb_probe(struct platform_device *pdev)
{
	int ret = 0;
	int i = 0, irq;

	struct axp517_usb_power *usb_power;

	struct sunxi_power_dev *axp_dev = dev_get_drvdata(pdev->dev.parent);
	struct power_supply_config psy_cfg = {};

	if (!axp_dev->irq) {
		PMIC_ERR("can not register axp517-usb without irq\n");
		return -EINVAL;
	}

	usb_power = devm_kzalloc(&pdev->dev, sizeof(*usb_power), GFP_KERNEL);
	if (usb_power == NULL) {
		PMIC_ERR("axp517_usb_power alloc failed\n");
		ret = -ENOMEM;
		goto err;
	}

	usb_power->name = "axp517_usb";
	usb_power->dev = &pdev->dev;
	usb_power->regmap = axp_dev->regmap;

	/* parse device tree and set register */
	axp517_usb_parse_device_tree(usb_power);

	psy_cfg.of_node = pdev->dev.of_node;
	psy_cfg.drv_data = usb_power;

	usb_power->usb_supply = devm_power_supply_register(usb_power->dev,
			&axp517_usb_desc, &psy_cfg);

	if (IS_ERR(usb_power->usb_supply)) {
		PMIC_ERR("axp517 failed to register usb power\n");
		ret = PTR_ERR(usb_power->usb_supply);
		return ret;
	}

	axp517_usb_detect_type_init(usb_power);

	if (!usb_power->vbus_detect_type) {
		PMIC_ERR("axp517 use normal detect ways\n");
		usb_power->vbus_detect_type = DETEC_BY_VBUS;
	} else {
		ret = axp517_acin_function_init(usb_power);
		if (!ret) {
			PMIC_INFO("axp517-acin device is not configed\n");
			return ret;
		}
	}

	if (!usb_power->dts_info.pmu_bc12_en) {
		INIT_DELAYED_WORK(&usb_power->usb_supply_mon, axp517_usb_power_monitor);
		INIT_DELAYED_WORK(&usb_power->usb_chg_state, axp517_usb_set_current_fsm);
	}

	for (i = 0; i < ARRAY_SIZE(axp_usb_irq); i++) {
		irq = platform_get_irq_byname(pdev, axp_usb_irq[i].name);
		if (irq < 0)
			continue;

		irq = regmap_irq_get_virq(axp_dev->regmap_irqc, irq);
		if (irq < 0) {
			PMIC_DEV_ERR(&pdev->dev, "can not get irq\n");
			ret = irq;
			goto cancel_work;
		}
		/* we use this variable to suspend irq */
		axp_usb_irq[i].irq = irq;
		ret = devm_request_any_context_irq(&pdev->dev, irq,
						   axp_usb_irq[i].isr, 0,
						   axp_usb_irq[i].name, usb_power);
		if (ret < 0) {
			PMIC_DEV_ERR(&pdev->dev, "failed to request %s IRQ %d: %d\n",
				axp_usb_irq[i].name, irq, ret);
			goto cancel_work;
		} else {
			ret = 0;
		}

		PMIC_DEV_DEBUG(&pdev->dev, "Requested %s IRQ %d: %d\n",
			axp_usb_irq[i].name, irq, ret);
	}

	if (!usb_power->dts_info.pmu_bc12_en) {
		schedule_delayed_work(&usb_power->usb_supply_mon, msecs_to_jiffies(500));
		usb_power->vbus_input_check = wakeup_source_register(usb_power->dev, "vbus_input_check");
		__pm_stay_awake(usb_power->vbus_input_check);
		schedule_delayed_work(&usb_power->usb_chg_state, msecs_to_jiffies(20 * 1000));
	}

	platform_set_drvdata(pdev, usb_power);
	sunxi_call_power_supply_notifier(AW_PSY_EVENT_USB_INIT_DONE);

	return ret;

cancel_work:
	if (!usb_power->dts_info.pmu_bc12_en) {
		cancel_delayed_work_sync(&usb_power->usb_supply_mon);
		cancel_delayed_work_sync(&usb_power->usb_chg_state);
	}


err:
	PMIC_ERR("%s,probe fail, ret = %d\n", __func__, ret);

	return ret;
}

static int axp517_usb_remove(struct platform_device *pdev)
{
	struct axp517_usb_power *usb_power = platform_get_drvdata(pdev);

	if (!usb_power->dts_info.pmu_bc12_en) {
		cancel_delayed_work_sync(&usb_power->usb_supply_mon);
		cancel_delayed_work_sync(&usb_power->usb_chg_state);
	}
	if (usb_power->vbus_detect_type == DETEC_BY_GPIO)
		cancel_delayed_work_sync(&usb_power->usb_det_mon);

	PMIC_DEV_DEBUG(&pdev->dev, "==============AXP517 usb unegister==============\n");
	if (usb_power->usb_supply)
		power_supply_unregister(usb_power->usb_supply);
	PMIC_DEV_DEBUG(&pdev->dev, "axp517 teardown usb dev\n");

	return 0;
}

static inline void axp517_usb_irq_set(unsigned int irq, bool enable)
{
	if (enable)
		enable_irq(irq);
	else
		disable_irq(irq);
}

static void axp517_usb_virq_dts_set(struct axp517_usb_power *usb_power, bool enable)
{
	struct axp_config_info *dts_info = &usb_power->dts_info;

	if (!dts_info->wakeup_usb_in)
		axp517_usb_irq_set(axp_usb_irq[AXP517_VIRQ_USB_IN].irq,
				enable);

	if (!dts_info->wakeup_usb_out)
		axp517_usb_irq_set(axp_usb_irq[AXP517_VIRQ_USB_OUT].irq,
				enable);

	if (usb_power->vbus_detect_type == DETEC_BY_GPIO) {
		axp517_usb_irq_set(usb_power->axp_vbus_det.irq_num,
				enable);
	}
}

static void axp517_usb_shutdown(struct platform_device *pdev)
{
	struct axp517_usb_power *usb_power = platform_get_drvdata(pdev);

	if (!usb_power->dts_info.pmu_bc12_en) {
		cancel_delayed_work_sync(&usb_power->usb_supply_mon);
		cancel_delayed_work_sync(&usb_power->usb_chg_state);
	}
	if (usb_power->vbus_detect_type == DETEC_BY_GPIO) {
		cancel_delayed_work_sync(&usb_power->usb_det_mon);
		axp517_usb_irq_set(usb_power->axp_vbus_det.irq_num,
				false);
	}
}

static int axp517_usb_suspend(struct platform_device *p, pm_message_t state)
{
	struct axp517_usb_power *usb_power = platform_get_drvdata(p);

	axp517_usb_virq_dts_set(usb_power, false);

	if (!usb_power->dts_info.pmu_bc12_en) {
		cancel_delayed_work_sync(&usb_power->usb_supply_mon);
		cancel_delayed_work_sync(&usb_power->usb_chg_state);
	}
	if (usb_power->vbus_detect_type == DETEC_BY_GPIO)
		cancel_delayed_work_sync(&usb_power->usb_det_mon);

	return 0;
}

static int axp517_usb_resume(struct platform_device *p)
{
	struct axp517_usb_power *usb_power = platform_get_drvdata(p);

	if (!usb_power->dts_info.pmu_bc12_en) {
		schedule_delayed_work(&usb_power->usb_supply_mon, 0);
		schedule_delayed_work(&usb_power->usb_chg_state, 0);
	}
	if (usb_power->vbus_detect_type == DETEC_BY_GPIO)
		schedule_delayed_work(&usb_power->usb_det_mon, 0);

	axp517_usb_virq_dts_set(usb_power, true);

	return 0;
}

static const struct of_device_id axp517_usb_power_match[] = {
	{
		.compatible = "x-powers,axp517-usb-power-supply",
		.data = (void *)AXP517_ID,
	}, {/* sentinel */}
};
MODULE_DEVICE_TABLE(of, axp517_usb_power_match);

static struct platform_driver axp517_usb_power_driver = {
	.driver = {
		.name = "axp517-usb-power-supply",
		.of_match_table = axp517_usb_power_match,
	},
	.probe = axp517_usb_probe,
	.remove = axp517_usb_remove,
	.shutdown = axp517_usb_shutdown,
	.suspend = axp517_usb_suspend,
	.resume = axp517_usb_resume,
};

module_platform_driver(axp517_usb_power_driver);

MODULE_AUTHOR("wangxiaoliang <wangxiaoliang@x-powers.com>");
MODULE_DESCRIPTION("axp517 usb driver");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.0.0");
