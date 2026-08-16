// SPDX-License-Identifier: GPL-2.0-or-later
/* Copyright(c) 2020 - 2023 Allwinner Technology Co.,Ltd. All rights reserved. */
/*
 * Copyright (c) 2007-2019 Allwinnertech Co., Ltd.
 * Author: zhengxiaobin <zhengxiaobin@allwinnertech.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#include "include.h"

#include "disp_lcd.h"
#include "disp_display.h"
#include "dev_fb.h"
#include "dev_lcd_fb.h"

#include "./panels/panels.h"

struct sunxi_lcd_fb_dev_lcd_fb_t g_drv_info;

static struct attribute *lcd_fb_attributes[] = { NULL };

static struct attribute_group lcd_fb_attribute_group = {
	.name = "attr",
	.attrs = lcd_fb_attributes
};

static void sunxi_lcd_fb_module_start_work(struct work_struct *work)
{
	int i = 0;
	struct sunxi_lcd_fb_device *dispdev = NULL;

	for (i = 0; i < g_drv_info.lcd_panel_count; ++i) {
		dispdev = sunxi_sunxi_lcd_fb_device_get(i);
		if (dispdev)
			dispdev->enable(dispdev);
	}
}

static s32 sunxi_lcd_fb_module_start_process(void)
{
	flush_work(&g_drv_info.start_work);
	schedule_work(&g_drv_info.start_work);
	return 0;
}

static int sunxi_lcd_fb_module_probe(struct platform_device *pdev)
{
	int ret = 0;
	struct device_node *np = pdev->dev.of_node;

	LCDFB_DBG("running sunxi_lcd_fb_module_probe\n");

	g_drv_info.device = &pdev->dev;

	ret = sysfs_create_group(&pdev->dev.kobj, &lcd_fb_attribute_group);

	INIT_WORK(&g_drv_info.start_work, sunxi_lcd_fb_module_start_work);

	g_drv_info.lcd_panel_count = of_graph_get_endpoint_count(np);
	if (g_drv_info.lcd_panel_count <= 0) {
		LCDFB_WRN("no endpoints found for node\n");
		return -ENODEV;
	}

	LCDFB_DBG("get endpoint count %d\n", g_drv_info.lcd_panel_count);

	sunxi_lcd_fb_disp_init_lcd(&g_drv_info);

	if (!g_drv_info.lcd_fb_num) {
		LCDFB_WRN("no panel found for lcd fb\n");
		return -ENODEV;
	}

	sunxi_lcd_fb_lcd_panel_init();

	sunxi_lcd_fb_init(&g_drv_info);

	sunxi_lcd_fb_module_start_process();

	return ret;
}

static void sunxi_lcd_fb_module_shutdown(struct platform_device *pdev)
{
	int i = 0;
	struct sunxi_lcd_fb_device *dispdev = NULL;

	for (i = 0; i < g_drv_info.lcd_panel_count; ++i) {
		dispdev = sunxi_sunxi_lcd_fb_device_get(i);
		if (dispdev)
			dispdev->disable(dispdev);
	}

	LCDFB_DBG("running sunxi_lcd_fb_module_shutdown\n");
}

static int sunxi_lcd_fb_module_remove(struct platform_device *pdev)
{
	sunxi_lcd_fb_module_shutdown(pdev);

	sunxi_lcd_fb_exit();

	sunxi_lcd_fb_disp_exit_lcd();

	sysfs_remove_group(&pdev->dev.kobj, &lcd_fb_attribute_group);

	platform_set_drvdata(pdev, NULL);

	LCDFB_DBG("running sunxi_lcd_fb_module_remove\n");

	return 0;
}

static const struct of_device_id sunxi_lcd_fb_module_match[] = {
	{
		.compatible = "allwinner,lcd-fb",
	},
	{},
};

int sunxi_lcd_fb_module_suspend(struct device *dev)
{
	int i = 0;
	int ret = 0;
	struct sunxi_lcd_fb_device *dispdev = NULL;

	for (i = 0; i < g_drv_info.lcd_panel_count; ++i) {
		dispdev = sunxi_sunxi_lcd_fb_device_get(i);
		if (dispdev)
			ret = dispdev->disable(dispdev);
	}
	return ret;
}

int sunxi_lcd_fb_module_resume(struct device *dev)
{
	int i = 0;
	int ret = 0;
	struct sunxi_lcd_fb_device *dispdev = NULL;

	for (i = 0; i < g_drv_info.lcd_panel_count; ++i) {
		dispdev = sunxi_sunxi_lcd_fb_device_get(i);
		if (dispdev)
			ret = dispdev->enable(dispdev);
	}
	return ret;
}

static const struct dev_pm_ops sunxi_lcd_fb_module_runtime_pm_ops = {
	.suspend = sunxi_lcd_fb_module_suspend,
	.resume = sunxi_lcd_fb_module_resume,
};

struct platform_driver sunxi_lcd_fb_driver = {
	.probe = sunxi_lcd_fb_module_probe,
	.remove = sunxi_lcd_fb_module_remove,
	.shutdown = sunxi_lcd_fb_module_shutdown,
	.driver = {
		.name = "lcd_fb",
		.owner = THIS_MODULE,
		.pm = &sunxi_lcd_fb_module_runtime_pm_ops,
		.of_match_table = sunxi_lcd_fb_module_match,
	},
};

static int __init sunxi_lcd_fb_module_init(void)
{
	s32 ret = -1;
	LCDFB_DBG("running sunxi_lcd_fb_module_init\n");
	ret = platform_driver_register(&sunxi_lcd_fb_driver);

	if (ret) {
		LCDFB_WRN("lcd fb register driver failed!\n");
		return ret;
	}

	return ret;
}

static void __exit sunxi_lcd_fb_module_exit(void)
{
	platform_driver_unregister(&sunxi_lcd_fb_driver);
	LCDFB_HERE;
}

int sunxi_lcd_fb_disp_get_source_ops(
	struct sunxi_lcd_fb_disp_source_ops *src_ops)
{
	memset((void *)src_ops, 0, sizeof(*src_ops));

	src_ops->sunxi_lcd_fb_set_panel_funs_ops =
		sunxi_lcd_fb_bsp_disp_lcd_set_panel_funs;
	src_ops->sunxi_lcd_fb_delay_ms_ops = sunxi_lcd_fb_disp_delay_ms;
	src_ops->sunxi_lcd_fb_delay_us_ops = sunxi_lcd_fb_disp_delay_us;
	src_ops->sunxi_lcd_fb_backlight_enable_ops =
		sunxi_lcd_fb_bsp_disp_lcd_backlight_enable;
	src_ops->sunxi_lcd_fb_backlight_disable_ops =
		sunxi_lcd_fb_bsp_disp_lcd_backlight_disable;
	src_ops->sunxi_lcd_fb_pwm_enable_ops =
		sunxi_lcd_fb_bsp_disp_lcd_pwm_enable;
	src_ops->sunxi_lcd_fb_pwm_disable_ops =
		sunxi_lcd_fb_bsp_disp_lcd_pwm_disable;
	src_ops->sunxi_lcd_fb_power_enable_ops =
		sunxi_lcd_fb_bsp_disp_lcd_power_enable;
	src_ops->sunxi_lcd_fb_power_disable_ops =
		sunxi_lcd_fb_bsp_disp_lcd_power_disable;
	src_ops->sunxi_lcd_fb_pin_cfg_ops = sunxi_lcd_fb_bsp_disp_lcd_pin_cfg;
	src_ops->sunxi_lcd_fb_gpio_set_value_ops =
		sunxi_lcd_fb_bsp_disp_lcd_gpio_set_value;
	src_ops->sunxi_lcd_fb_gpio_set_direction_ops =
		sunxi_lcd_fb_bsp_disp_lcd_gpio_set_direction;
	src_ops->sunxi_lcd_fb_cmd_write_ops =
		sunxi_lcd_fb_bsp_disp_lcd_cmd_write;
	src_ops->sunxi_lcd_fb_para_write_ops =
		sunxi_lcd_fb_bsp_disp_lcd_para_write;
	src_ops->sunxi_lcd_fb_cmd_read_ops = sunxi_lcd_fb_bsp_disp_lcd_cmd_read;
	src_ops->sunxi_lcd_fb_qspi_cmd_write_ops =
		sunxi_lcd_fb_bsp_disp_qspi_lcd_cmd_write;
	src_ops->sunxi_lcd_fb_qspi_para_write_ops =
		sunxi_lcd_fb_bsp_disp_qspi_lcd_para_write;
	src_ops->sunxi_lcd_fb_qspi_multi_para_write_ops =
		sunxi_lcd_fb_bsp_disp_qspi_lcd_multi_para_write;
	src_ops->sunxi_lcd_fb_qspi_cmd_read_ops =
		sunxi_lcd_fb_bsp_disp_qspi_lcd_cmd_read;
	return 0;
}

module_init(sunxi_lcd_fb_module_init);
module_exit(sunxi_lcd_fb_module_exit);

MODULE_AUTHOR("zhengxiaobin <zhengxiaobin@allwinnertech.com>");
MODULE_AUTHOR("zhangyuanjing <zhangyuanjing@allwinnertech.com>");
MODULE_DESCRIPTION("sunxi lcd fb module");
MODULE_VERSION("1.0.2");
MODULE_LICENSE("GPL");
