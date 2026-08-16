// SPDX-License-Identifier: GPL-2.0
/* Copyright(c) 2020 - 2023 Allwinner Technology Co.,Ltd. All rights reserved. */
/*
 * Copyright (c) 2024 haili@allwinnertech.com
 */

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/pinctrl/pinctrl.h>
#include <linux/io.h>

#include "pinctrl-sunxi.h"

#define SUNXI_RTC_PINCTRL_VERSION   "0.0.2"

static const struct sunxi_desc_pin sun8iw22_rtc_pins[] = {
	/* bank PWQ_IRQ */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(W, 0),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x2, "nmi"),           /* nmi */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 0, 0),   /* nmi_eint0 */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(W, 1),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x2, "pwr_irq0"),      /* pwr_irq0 */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 0, 1),   /* pwr_irq_eint1 */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(W, 2),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x2, "pwr_irq1"),      /* pwr_irq1 */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 0, 2),   /* pwr_irq_eint2 */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(W, 3),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x2, "pwr_irq2"),      /* pwr_irq2 */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 0, 3),   /* pwr_irq_eint3 */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(W, 4),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x2, "pwr_irq3"),      /* pwr_irq3 */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 0, 4),   /* pwr_irq_eint4 */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(W, 5),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x2, "pwr_irq4"),      /* pwr_irq4 */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 0, 5),   /* pwr_irq_eint5 */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
};

static const unsigned int sun8iw22_rtc_bank_base[] = {
	SUNXI_BANK_OFFSET('W', 'W'),	/* PWR_IRQ */
};

static const unsigned int sun8iw22_rtc_irq_bank_map[] = {
	SUNXI_BANK_OFFSET('W', 'W'),
};

static const struct sunxi_pinctrl_desc sun8iw22_rtc_pinctrl_data = {
	.pins = sun8iw22_rtc_pins,
	.npins = ARRAY_SIZE(sun8iw22_rtc_pins),
	.banks = ARRAY_SIZE(sun8iw22_rtc_bank_base),
	.bank_base = sun8iw22_rtc_bank_base,
	.irq_banks = ARRAY_SIZE(sun8iw22_rtc_irq_bank_map),
	.irq_bank_map = sun8iw22_rtc_irq_bank_map,
	.pin_base = SUNXI_PIN_BASE('W'),
	.auto_power_source_switch = true,
	.hw_type = SUNXI_PCTL_HW_TYPE_8,
};

static int sun8iw22_rtc_pinctrl_probe(struct platform_device *pdev)
{
	return sunxi_bsp_pinctrl_init(pdev, &sun8iw22_rtc_pinctrl_data);
}

static struct of_device_id sun8iw22_rtc_pinctrl_match[] = {
	{ .compatible = "allwinner,sun8iw22-rtc-pinctrl", },
	{}
};

MODULE_DEVICE_TABLE(of, sun8iw22_pinctrl_match);

static struct platform_driver sun8iw22_rtc_pinctrl_driver = {
	.probe	= sun8iw22_rtc_pinctrl_probe,
	.driver	= {
		.name		= "sun8iw22-rtc-pinctrl",
		.of_match_table	= sun8iw22_rtc_pinctrl_match,
	},
};

static int __init sun8iw22_rtc_pio_init(void)
{
	return platform_driver_register(&sun8iw22_rtc_pinctrl_driver);
}
fs_initcall(sun8iw22_rtc_pio_init);

MODULE_DESCRIPTION("Allwinner sun8iw22 rtc pinctrl driver");
MODULE_AUTHOR("<haili@allwinnertech>");
MODULE_LICENSE("GPL");
MODULE_VERSION(SUNXI_RTC_PINCTRL_VERSION);
