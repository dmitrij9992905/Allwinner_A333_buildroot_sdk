// SPDX-License-Identifier: GPL-2.0
/* Copyright(c) 2020 - 2025 Allwinner Technology Co.,Ltd. All rights reserved. */
/*
 * Copyright (c) 2025 haili@allwinnertech.com
 */

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/pinctrl/pinctrl.h>
#include <linux/io.h>

#include "pinctrl-sunxi.h"

#define SUNXI_PINCTRL_VERSION   "0.0.1"

static const struct sunxi_desc_pin sun55iw7_pins[] = {
#if IS_ENABLED(CONFIG_AW_FPGA_BOARD)
	/* Pin banks are: PA  PD  PF */

	/* bank A */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(A, 0),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpxi[0] */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpxo[0] */
		SUNXI_FUNCTION(0x2, "gmac0"),         /* gmac0_rxd_di[3] */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 0, 0),   /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(A, 1),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpxi[0] */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpxo[0] */
		SUNXI_FUNCTION(0x2, "gmac0"),         /* gmac0_rxd_di[2] */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 0, 1),   /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(A, 2),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpxi[0] */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpxo[0] */
		SUNXI_FUNCTION(0x2, "gmac0"),         /* gmac0_rxd_di[1] */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 0, 2),   /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(A, 3),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpxi[0] */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpxo[0] */
		SUNXI_FUNCTION(0x2, "gmac0"),         /* gmac0_rxd_di[0] */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 0, 3),   /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(A, 4),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpxi[0] */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpxo[0] */
		SUNXI_FUNCTION(0x2, "gmac0"),         /* gmac0_txd_do[3] */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 0, 4),   /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(A, 5),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpxi[0] */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpxo[0] */
		SUNXI_FUNCTION(0x2, "gmac0"),         /* gmac0_txd_do[2] */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 0, 5),   /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(A, 6),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpxi[0] */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpxo[0] */
		SUNXI_FUNCTION(0x2, "gmac0"),         /* gmac0_txd_do[1] */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 0, 6),   /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(A, 7),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpxi[0] */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpxo[0] */
		SUNXI_FUNCTION(0x2, "gmac0"),         /* gmac0_txd_do[0] */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 0, 7),   /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(A, 8),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpxi[0] */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpxo[0] */
		SUNXI_FUNCTION(0x2, "gmac0"),         /* gmac0_rxclk_di */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 0, 8),   /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(A, 10),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpxi[0] */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpxo[0] */
		SUNXI_FUNCTION(0x2, "gmac0"),         /* gmac0_rxdv_di */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 0, 10),  /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(A, 11),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpxi[0] */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpxo[0] */
		SUNXI_FUNCTION(0x2, "gmac0"),         /* gmac0_mdc_do */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 0, 11),  /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(A, 12),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpxi[0] */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpxo[0] */
		SUNXI_FUNCTION(0x2, "gmac0"),         /* gmac0_md_do/di */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 0, 12),  /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(A, 13),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpxi[0] */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpxo[0] */
		SUNXI_FUNCTION(0x2, "gmac0"),         /* gmac0_txen_do */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 0, 13),  /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(A, 14),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpxi[0] */
		SUNXI_FUNCTION(0x1, "gpioa"),         /* gpioa[14] */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 0, 14),  /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(A, 18),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpxi[0] */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpxo[0] */
		SUNXI_FUNCTION(0x2, "gmac0"),         /* gmac0_clktx_do */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 0, 18),  /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	/* bank D */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(D, 29),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpxi[0] */
		SUNXI_FUNCTION(0x1, "lcd"),           /* lcd_pio0 */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 1, 29),  /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(D, 30),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpxi[0] */
		SUNXI_FUNCTION(0x1, "lcd"),           /* lcd_pio1 */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 1, 30),  /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(D, 31),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpxi[0] */
		SUNXI_FUNCTION(0x1, "lcd"),           /* lcd_pio2 */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 1, 31),  /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	/* bank F */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(F, 8),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpxi[0] */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpxo[0] */
		SUNXI_FUNCTION(0x2, "sd2"),           /* sd2_ccmd_do/di */
		SUNXI_FUNCTION(0x3, "spi0"),          /* spi0_ss_do[0]/di[0] */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 2, 8),   /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(F, 9),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpxi[0] */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpxo[0] */
		SUNXI_FUNCTION(0x2, "sd2"),           /* sd2_ds_di */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 2, 9),   /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(F, 10),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpxi[0] */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpxo[0] */
		SUNXI_FUNCTION(0x3, "spi0"),          /* spi0_miso_do/di */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 2, 10),  /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(F, 11),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpxi[0] */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpxo[0] */
		SUNXI_FUNCTION(0x2, "sd2"),           /* sd2_cclk_do */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 2, 11),  /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(F, 14),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpxi[0] */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpxo[0] */
		SUNXI_FUNCTION(0x3, "spi0"),          /* spi0_mosi_do/di */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 2, 14),  /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(F, 17),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpxi[0] */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpxo[0] */
		SUNXI_FUNCTION(0x3, "spi0"),          /* spi0_sck_do/di */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 2, 17),  /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(F, 19),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpxi[0] */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpxo[0] */
		SUNXI_FUNCTION(0x2, "sd2"),           /* sd2_rstb_do */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 2, 19),  /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(F, 20),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpxi[0] */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpxo[0] */
		SUNXI_FUNCTION(0x2, "sd2"),           /* sd2_cdat_do[7]/di[7] */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 2, 20),  /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(F, 21),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpxi[0] */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpxo[0] */
		SUNXI_FUNCTION(0x2, "sd2"),           /* sd2_cdat_do[6]/di[6] */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 2, 21),  /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(F, 22),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpxi[0] */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpxo[0] */
		SUNXI_FUNCTION(0x2, "sd2"),           /* sd2_cdat_do[5]/di[5] */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 2, 22),  /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(F, 23),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpxi[0] */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpxo[0] */
		SUNXI_FUNCTION(0x2, "sd2"),           /* sd2_cdat_do[4]/di[4] */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 2, 23),  /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(F, 24),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpxi[0] */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpxo[0] */
		SUNXI_FUNCTION(0x2, "sd2"),           /* sd2_cdat_do[3]/di[3] */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 2, 24),  /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(F, 25),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpxi[0] */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpxo[0] */
		SUNXI_FUNCTION(0x2, "sd2"),           /* sd2_cdat_do[2]/di[2] */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 2, 25),  /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(F, 26),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpxi[0] */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpxo[0] */
		SUNXI_FUNCTION(0x2, "sd2"),           /* sd2_cdat_do[1]/di[1] */
		SUNXI_FUNCTION(0x3, "spi0"),          /* spi0_wp_do/di */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 2, 26),  /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(F, 27),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpxi[0] */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpxo[0] */
		SUNXI_FUNCTION(0x2, "sd2"),           /* sd2_cdat_do[0]/di[0] */
		SUNXI_FUNCTION(0x3, "spi0"),          /* spi0_hold_do/di */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 2, 27),  /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
#else
#endif
};

static const unsigned int sun55iw7_bank_base[] = {
	SUNXI_BANK_OFFSET('A', 'A'),
	SUNXI_BANK_OFFSET('D', 'A'),
	SUNXI_BANK_OFFSET('F', 'A'),
};

static const unsigned int sun55iw7_irq_bank_map[] = {
	SUNXI_BANK_OFFSET('A', 'A'),
	SUNXI_BANK_OFFSET('D', 'A'),
	SUNXI_BANK_OFFSET('F', 'A'),
};

static const struct sunxi_pinctrl_desc sun55iw7_pinctrl_data = {
	.pins = sun55iw7_pins,
	.npins = ARRAY_SIZE(sun55iw7_pins),
	.banks = ARRAY_SIZE(sun55iw7_bank_base),
	.bank_base = sun55iw7_bank_base,
	.irq_banks = ARRAY_SIZE(sun55iw7_irq_bank_map),
	.irq_bank_map = sun55iw7_irq_bank_map,
	.auto_power_source_switch = true,
	.pf_power_source_switch = true,
	.hw_type = SUNXI_PCTL_HW_TYPE_4,
};

/* PINCTRL power management code */
#if IS_ENABLED(CONFIG_PM_SLEEP)

static void *mem;
static int mem_size;

static int pinctrl_pm_alloc_mem(struct platform_device *pdev)
{
	struct resource *res;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res)
		return -EINVAL;
	mem_size = resource_size(res);

	if (mem)
		return -ENOMEM;
	mem = devm_kzalloc(&pdev->dev, mem_size, GFP_KERNEL);
	if (!mem)
		return -ENOMEM;
	return 0;
}

static int sun55iw7_pinctrl_suspend_noirq(struct device *dev)
{
	struct sunxi_pinctrl *pctl = dev_get_drvdata(dev);
	unsigned long flags;

	dev_info(dev, "pinctrl suspend\n");

	raw_spin_lock_irqsave(&pctl->lock, flags);
	memcpy_fromio(mem, pctl->membase, mem_size);
	raw_spin_unlock_irqrestore(&pctl->lock, flags);

	return 0;
}

static int sun55iw7_pinctrl_resume_noirq(struct device *dev)
{
	struct sunxi_pinctrl_desc const *desc = &sun55iw7_pinctrl_data;
	struct sunxi_pinctrl *pctl = dev_get_drvdata(dev);
	unsigned long flags;
	int idx, bank;
	int initial_bank_offset = sunxi_pinctrl_hw_info[desc->hw_type].initial_bank_offset;
	int bank_mem_size = sunxi_pinctrl_hw_info[desc->hw_type].bank_mem_size;
	int irq_cfg_reg = sunxi_pinctrl_hw_info[desc->hw_type].irq_cfg_reg;
	int irq_mem_size = sunxi_pinctrl_hw_info[desc->hw_type].irq_mem_size;
	int irq_debounce_reg = sunxi_pinctrl_hw_info[desc->hw_type].irq_debounce_reg;

	raw_spin_lock_irqsave(&pctl->lock, flags);

	for (idx = 0; idx < desc->banks; idx++) {
		bank = desc->bank_base[idx];
		/* cfg/dat/drv/pull */
		memcpy_toio(pctl->membase + initial_bank_offset + bank * bank_mem_size,
			    mem + initial_bank_offset + bank * bank_mem_size,
			    bank_mem_size);
	}

	for (idx = 0; idx < desc->irq_banks; idx++) {
		bank = desc->irq_bank_map[idx];
		/* irq cfg */
		memcpy_toio(pctl->membase + irq_cfg_reg + bank * irq_mem_size,
			    mem + irq_cfg_reg + bank * irq_mem_size,
			   0x10);
		/* irq deb */
		writel(readl(mem + irq_debounce_reg + bank * irq_mem_size),
			pctl->membase + irq_debounce_reg + bank * irq_mem_size);
	}

	raw_spin_unlock_irqrestore(&pctl->lock, flags);

	sunxi_info(dev, "pinctrl resume\n");
	return 0;
}

static const struct dev_pm_ops sun55iw7_pinctrl_pm_ops = {
	.suspend_noirq = sun55iw7_pinctrl_suspend_noirq,
	.resume_noirq = sun55iw7_pinctrl_resume_noirq,
};
#define PINCTRL_PM_OPS	(&sun55iw7_pinctrl_pm_ops)

#else
static int pinctrl_pm_alloc_mem(struct platform_device *pdev)
{
	return 0;
}
#define PINCTRL_PM_OPS	NULL
#endif

static int sun55iw7_pinctrl_probe(struct platform_device *pdev)
{
	int ret;
	ret = pinctrl_pm_alloc_mem(pdev);
	if (ret) {
		dev_err(&pdev->dev, "alloc pm mem err\n");
		return ret;
	}

	return sunxi_bsp_pinctrl_init(pdev, &sun55iw7_pinctrl_data);
}

static struct of_device_id sun55iw7_pinctrl_match[] = {
	{ .compatible = "allwinner,sun55iw7-pinctrl", },
	{}
};

MODULE_DEVICE_TABLE(of, sun55iw7_pinctrl_match);

static struct platform_driver sun55iw7_pinctrl_driver = {
	.probe	= sun55iw7_pinctrl_probe,
	.driver	= {
		.name		= "sun55iw7-pinctrl",
		.pm		= PINCTRL_PM_OPS,
		.of_match_table	= sun55iw7_pinctrl_match,
	},
};

static int __init sun55iw7_pio_init(void)
{
	return platform_driver_register(&sun55iw7_pinctrl_driver);
}
fs_initcall(sun55iw7_pio_init);

MODULE_DESCRIPTION("Allwinner sun55iw7 pio pinctrl driver");
MODULE_AUTHOR("<haili@allwinnertech>");
MODULE_LICENSE("GPL");
MODULE_VERSION(SUNXI_PINCTRL_VERSION);
