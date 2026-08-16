/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Copyright(c) 2020 - 2023 Allwinner Technology Co.,Ltd. All rights reserved. */
/*
 * Allwinner SoCs pmic driver.
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2.  This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */

#define pr_fmt(x) KBUILD_MODNAME ": " x "\n"

#include "sunxi_multi_charge.h"

struct sunxi_multi_charge {
	/* base */
	const char                *name;
	struct device             *dev;
	struct power_supply       *mc_supply;
	struct mc_config_info     dts_info;
	struct delayed_work       mc_supply_mon;
	struct delayed_work       mc_chg_state;

	/* detect_type */
	int                       mc_detect_type;

	/* gpio_config */
	struct mc_gpio_para       mc_det;

	/* extcon */
	struct extcon_dev         *mc_edev;
	struct notifier_block	  mc_nb;
	atomic_t                  mc_online;

	/* usb_power_supply */
	bool                      usb_supply_exist;
	struct power_supply       *usb_psy;
};

static enum power_supply_property sunxi_multi_charge_props[] = {
	POWER_SUPPLY_PROP_MODEL_NAME,
	POWER_SUPPLY_PROP_ONLINE,
};

static void sunxi_multi_charge_init_usb_para(struct sunxi_multi_charge *mc_power)
{
	struct device_node *np = NULL;
	struct mc_config_info *mc_config = &mc_power->dts_info;
	int paras = -1;

	np = of_parse_phandle(mc_power->dev->of_node, "det_usb_supply", 0);
	if (!of_device_is_available(np)) {
		PMIC_ERR("sunxi_multi_charge need usb power, but is not available\n");
		return;
	}

	mc_power->usb_supply_exist = true;

	of_property_read_u32(np, "pmu_usbad_cur", &paras);
	if (paras > 0)
		mc_config->pmu_usbad_cur = paras;
	else
		mc_config->pmu_usbad_cur = 2500;

	of_property_read_u32(np, "pmu_usbpc_cur", &paras);
	if (paras > 0)
		mc_config->pmu_usbpc_cur = paras;
	else
		mc_config->pmu_usbpc_cur = 500;

	return;
}

static int sunxi_multi_charge_check_usb(struct sunxi_multi_charge *mc_power)
{
	if (!mc_power->usb_supply_exist)
		return 0;

	if (mc_power->usb_psy == NULL) {
		mc_power->usb_psy = devm_power_supply_get_by_phandle(mc_power->dev,
									"det_usb_supply");
	}
	if (!(mc_power->usb_psy && (!IS_ERR(mc_power->usb_psy))))
		return 0;

	return 1;
}

static int sunxi_multi_charge_get_property(struct power_supply *psy,
				       enum power_supply_property psp,
				       union power_supply_propval *val)
{
	struct sunxi_multi_charge *mc_power = power_supply_get_drvdata(psy);

	int ret = 0;

	switch (psp) {
	case POWER_SUPPLY_PROP_MODEL_NAME:
		val->strval = psy->desc->name;
		break;
	case POWER_SUPPLY_PROP_ONLINE:
		switch (mc_power->mc_detect_type) {
		case DETEC_BY_GPIO:
			val->intval = gpio_get_value(mc_power->mc_det.gpio);
			break;
		case DETEC_BY_EXTCON:
			val->intval = atomic_read(&mc_power->mc_online);
			break;
		default:
			ret = -EINVAL;
		}
		break;
	default:
		ret = -EINVAL;
	}

	return ret;
}

static struct power_supply_desc sunxi_multi_charge_desc = {
	.name = "sunxi-multi-charge",
	.type = POWER_SUPPLY_TYPE_MAINS,
	.get_property = sunxi_multi_charge_get_property,
	.properties = sunxi_multi_charge_props,
	.num_properties = ARRAY_SIZE(sunxi_multi_charge_props),
};

int sunxi_multi_charge_irq_handler_in(void *data)
{
	struct sunxi_multi_charge *mc_power = data;
	struct mc_config_info *mc_config = &mc_power->dts_info;
	union power_supply_propval temp;

	PMIC_INFO("[mc_irq]sunxi_multi_charge_in\n");
	power_supply_changed(mc_power->mc_supply);
	if (sunxi_multi_charge_check_usb(mc_power)) {
		temp.intval = mc_config->pmu_usbad_cur;
		PMIC_INFO("[mc_irq] ad_current_limit = %d\n", temp.intval);
		power_supply_set_property(mc_power->usb_psy,
						POWER_SUPPLY_PROP_INPUT_CURRENT_LIMIT, &temp);
	}
	return 0;
}

int sunxi_multi_charge_irq_handler_out(void *data)
{
	struct sunxi_multi_charge *mc_power = data;
	struct mc_config_info *mc_config = &mc_power->dts_info;
	union power_supply_propval temp;

	PMIC_INFO("[mc_irq]sunxi_multi_charge_out\n");
	power_supply_changed(mc_power->mc_supply);
	if (sunxi_multi_charge_check_usb(mc_power)) {
		power_supply_get_property(mc_power->usb_psy, POWER_SUPPLY_PROP_ONLINE, &temp);
		if (temp.intval) {
			power_supply_get_property(mc_power->usb_psy, POWER_SUPPLY_PROP_USB_TYPE, &temp);
			if (temp.intval == POWER_SUPPLY_USB_TYPE_SDP) {
				temp.intval = mc_config->pmu_usbpc_cur;
				PMIC_INFO("[mc_irq] pc_current_limit = %d\n", temp.intval);
				power_supply_set_property(mc_power->usb_psy,
							POWER_SUPPLY_PROP_INPUT_CURRENT_LIMIT, &temp);
			}
		}
	}
	return 0;
}

static irqreturn_t multi_charge_isr(int irq, void *data)
{
	struct sunxi_multi_charge *mc_power = data;

	cancel_delayed_work_sync(&mc_power->mc_chg_state);
	schedule_delayed_work(&mc_power->mc_chg_state, 1);

	return IRQ_HANDLED;
}

static void sunxi_multi_charge_set_current_fsm(struct work_struct *work)
{
	struct sunxi_multi_charge *mc_power =
		container_of(work, typeof(*mc_power), mc_chg_state.work);
	int mc_det_status_value;
	static int mc_det_status_value_old;

	if (mc_power->mc_detect_type == DETEC_BY_GPIO) {
		mc_det_status_value = gpio_get_value(mc_power->mc_det.gpio);
	} else if (mc_power->mc_detect_type == DETEC_BY_EXTCON) {
		mc_det_status_value = atomic_read(&mc_power->mc_online);
	} else {
		return;
	}

	if (mc_det_status_value_old == mc_det_status_value) {
		return;
	} else {
		mc_det_status_value_old = mc_det_status_value;
	}

	PMIC_INFO("[ac_status] ac_in_flag :%d\n", mc_det_status_value);
	if (mc_det_status_value == 1) {
		sunxi_multi_charge_irq_handler_in(mc_power);
	} else {
		sunxi_multi_charge_irq_handler_out(mc_power);
	}
}

static int sunxi_multi_charge_gpio_init(struct sunxi_multi_charge *mc_power)
{
	int ret = 0;
	unsigned long irq_flags = 0;

	if (!gpio_is_valid(mc_power->mc_det.gpio)) {
		PMIC_WARN("get pmu_mc_det_gpio is fail\n");
		return -EPROBE_DEFER;
	}

	ret = gpio_request(
			mc_power->mc_det.gpio,
			"pmu_mc_det_gpio");
	if (ret != 0) {
		PMIC_WARN("pmu_mc_det gpio_request failed\n");
		return -EPROBE_DEFER;
	}
	/* set acin input */
	gpio_direction_input(mc_power->mc_det.gpio);

	/* irq config setting */
	irq_flags = IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING |
			IRQF_ONESHOT | IRQF_NO_SUSPEND;
	mc_power->mc_det.irq_num = gpio_to_irq(mc_power->mc_det.gpio);

	/* init delay work */
	mc_power->mc_detect_type = DETEC_BY_GPIO;
	ret = devm_request_threaded_irq(mc_power->dev, mc_power->mc_det.irq_num, NULL, multi_charge_isr, irq_flags,
				"pmu_mc_det_gpio", mc_power);

	if (IS_ERR_VALUE((unsigned long)ret)) {
		PMIC_ERR("Requested pmu_mc_det_gpio IRQ failed, err %d\n", ret);
		mc_power->mc_detect_type = DETEC_UNKNOWN;
		return -EINVAL;
	}
	PMIC_DEV_DEBUG(mc_power->dev, "Requested pmu_mc_det_gpio IRQ %d: %d\n",
		mc_power->mc_det.irq_num, ret);

	return 0;
}

static int sunxi_multi_charge_extcon_notifier(struct notifier_block *nb,
	unsigned long event, void *ptr)
{
	struct sunxi_multi_charge *mc_power = container_of(nb, struct sunxi_multi_charge, mc_nb);

	PMIC_INFO("acin event %lu\n", event);
	if (event) {
		atomic_set(&mc_power->mc_online, 1);
	} else {
		atomic_set(&mc_power->mc_online, 0);
	}
	schedule_delayed_work(&mc_power->mc_chg_state, 1);

	return NOTIFY_DONE;
}

static int sunxi_multi_charge_extcon_init(struct sunxi_multi_charge *mc_power)
{
	int ret = 0;

	mc_power->mc_nb.notifier_call = sunxi_multi_charge_extcon_notifier;
	ret = devm_extcon_register_notifier(mc_power->dev, mc_power->mc_edev,
					EXTCON_CHG_USB_DCP, &mc_power->mc_nb);
	if (ret < 0) {
		PMIC_ERR("failed to register notifier :%d\n", ret);
		return ret;
	}

	mc_power->mc_detect_type = DETEC_BY_EXTCON;
	if (extcon_get_state(mc_power->mc_edev, EXTCON_CHG_USB_DCP) == true) {
		atomic_set(&mc_power->mc_online, 1);
		sunxi_multi_charge_irq_handler_in(mc_power);
	} else {
		atomic_set(&mc_power->mc_online, 0);
		sunxi_multi_charge_irq_handler_out(mc_power);
	}

	return ret;
}

static int sunxi_multi_charge_detect_type_init(struct sunxi_multi_charge *mc_power)
{
	struct extcon_dev *edev;
	int ret = 0;

	mc_power->mc_detect_type = DETEC_UNKNOWN;

	/* detect by gpio */
	ret = of_get_named_gpio(mc_power->dev->of_node, "multi_charge_det_gpio", 0);

	if (ret < 0) {
		mc_power->mc_det.gpio = 0;
	} else {
		mc_power->mc_det.gpio = ret;
		ret = sunxi_multi_charge_gpio_init(mc_power);
		if (ret != 0) {
			PMIC_ERR("sunxi multi charge init failed\n");
			return ret;
		}
		PMIC_INFO("sunxi multi charge detect by gpio\n");
		return 0;
	}

	/* detect by extcon */
	if (of_property_read_bool(mc_power->dev->of_node, "extcon")) {
		edev = extcon_get_edev_by_phandle(mc_power->dev, 0);
		if (IS_ERR_OR_NULL(edev)) {
			PMIC_ERR("couldn't get extcon device\n");
			return -EPROBE_DEFER;
		}
		mc_power->mc_edev = edev;
		ret = sunxi_multi_charge_extcon_init(mc_power);
		if (ret != 0) {
			PMIC_ERR("sunxi multi charge extcon init failed\n");
			return ret;
		}
		PMIC_INFO("sunxi multi charge detect by extcon\n");
		return 0;
	}

	return -EINVAL;
}

static void sunxi_multi_charge_monitor(struct work_struct *work)
{
	struct sunxi_multi_charge *mc_power =
		container_of(work, typeof(*mc_power), mc_supply_mon.work);

	schedule_delayed_work(&mc_power->mc_supply_mon, msecs_to_jiffies(1000));
}

int sunxi_multi_charge_dt_parse(struct device_node *node,
			 struct mc_config_info *mc_config)
{
	return 0;
}

static void sunxi_multi_charge_parse_device_tree(struct sunxi_multi_charge *mc_power)
{
	int ret;
	struct mc_config_info *cfg;

	if (!mc_power->dev->of_node) {
		PMIC_INFO("can not find device tree\n");
		return;
	}

	cfg = &mc_power->dts_info;
	ret = sunxi_multi_charge_dt_parse(mc_power->dev->of_node, cfg);
	if (ret) {
		PMIC_INFO("can not parse device tree err\n");
		return;
	}
}

static int sunxi_multi_charge_probe(struct platform_device *pdev)
{
	struct sunxi_multi_charge *mc_power;
	struct power_supply_config psy_cfg = {};
	int ret = 0;

	mc_power = devm_kzalloc(&pdev->dev, sizeof(*mc_power), GFP_KERNEL);
	if (mc_power == NULL) {
		PMIC_ERR("sunxi_multi_charge alloc failed\n");
		ret = -ENOMEM;
		goto err;
	}

	mc_power->name = pdev->mfd_cell->name;
	mc_power->dev = &pdev->dev;

	/* parse device tree and set register */
	sunxi_multi_charge_parse_device_tree(mc_power);

	psy_cfg.of_node = pdev->dev.of_node;
	psy_cfg.drv_data = mc_power;

	sunxi_multi_charge_desc.name = mc_power->name;
	mc_power->mc_supply = devm_power_supply_register(mc_power->dev,
			&sunxi_multi_charge_desc, &psy_cfg);

	if (IS_ERR(mc_power->mc_supply)) {
		PMIC_ERR("sunxi multi charge failed to register power-sypply\n");
		ret = PTR_ERR(mc_power->mc_supply);
		return ret;
	}

	mc_power->usb_psy = NULL;
	mc_power->usb_supply_exist = false;
	if (of_find_property(mc_power->dev->of_node, "det_usb_supply", NULL)) {
		sunxi_multi_charge_init_usb_para(mc_power);
	} else {
		PMIC_ERR("sunxi multi charge failed to find usb power\n");
	}

	INIT_DELAYED_WORK(&mc_power->mc_supply_mon, sunxi_multi_charge_monitor);
	INIT_DELAYED_WORK(&mc_power->mc_chg_state, sunxi_multi_charge_set_current_fsm);

	ret = sunxi_multi_charge_detect_type_init(mc_power);
	if (ret != 0) {
		PMIC_ERR("sunxi multi charge failed to init detect type\n");
		return ret;
	}

	schedule_delayed_work(&mc_power->mc_supply_mon, msecs_to_jiffies(1000));
	platform_set_drvdata(pdev, mc_power);
	PMIC_INFO("sunxi multi charge probe finish: %s , %d \n", __func__, __LINE__);

	return ret;

err:
	PMIC_ERR("%s,probe fail, ret = %d\n", __func__, ret);

	return ret;
}

static int sunxi_multi_charge_remove(struct platform_device *pdev)
{
	struct sunxi_multi_charge *mc_power = platform_get_drvdata(pdev);

	cancel_delayed_work_sync(&mc_power->mc_supply_mon);
	cancel_delayed_work_sync(&mc_power->mc_chg_state);

	PMIC_DEV_DEBUG(&pdev->dev, "==============sunxi multi charge unegister==============\n");
	if (mc_power->mc_supply)
		power_supply_unregister(mc_power->mc_supply);
	PMIC_DEV_DEBUG(&pdev->dev, "sunxi multi charge teardown dev\n");

	return 0;
}

static inline void sunxi_multi_charge_irq_set(unsigned int irq, bool enable)
{
	if (enable)
		enable_irq(irq);
	else
		disable_irq(irq);
}

static void sunxi_multi_charge_virq_dts_set(struct sunxi_multi_charge *mc_power, bool enable)
{
	if (mc_power->mc_detect_type == DETEC_BY_GPIO) {
		sunxi_multi_charge_irq_set(mc_power->mc_det.irq_num,
				enable);
	}
}

static void sunxi_multi_charge_shutdown(struct platform_device *pdev)
{
	struct sunxi_multi_charge *mc_power = platform_get_drvdata(pdev);

	sunxi_multi_charge_virq_dts_set(mc_power, false);

	cancel_delayed_work_sync(&mc_power->mc_supply_mon);
	cancel_delayed_work_sync(&mc_power->mc_chg_state);
}

static int sunxi_multi_charge_suspend(struct platform_device *pdev, pm_message_t state)
{
	struct sunxi_multi_charge *mc_power = platform_get_drvdata(pdev);

	sunxi_multi_charge_virq_dts_set(mc_power, false);

	cancel_delayed_work_sync(&mc_power->mc_supply_mon);
	cancel_delayed_work_sync(&mc_power->mc_chg_state);

	return 0;
}

static int sunxi_multi_charge_resume(struct platform_device *pdev)
{
	struct sunxi_multi_charge *mc_power = platform_get_drvdata(pdev);

	schedule_delayed_work(&mc_power->mc_supply_mon, 0);
	schedule_delayed_work(&mc_power->mc_chg_state, 0);

	sunxi_multi_charge_virq_dts_set(mc_power, true);
	return 0;
}

static struct of_device_id sunxi_multi_charge_match[] = {
	{ .compatible = "x-powers,axp515-multi-charge-supply" },
	{ .compatible = "x-powers,axp517-multi-charge-supply" },
	{ /* sentinel */ },
};

static struct platform_driver sunxi_multi_charge_driver = {
	.driver = {
		.name = "sunxi-multi-charge-supply",
		.of_match_table = sunxi_multi_charge_match,
	},
	.probe = sunxi_multi_charge_probe,
	.remove = sunxi_multi_charge_remove,
	.shutdown = sunxi_multi_charge_shutdown,
	.suspend = sunxi_multi_charge_suspend,
	.resume = sunxi_multi_charge_resume,
};

module_platform_driver(sunxi_multi_charge_driver);

MODULE_VERSION("1.0.0");
MODULE_AUTHOR("xinouyang <xinouyang@allwinnertech.com>");
MODULE_DESCRIPTION("sunxi multi charge driver");
MODULE_LICENSE("GPL");
