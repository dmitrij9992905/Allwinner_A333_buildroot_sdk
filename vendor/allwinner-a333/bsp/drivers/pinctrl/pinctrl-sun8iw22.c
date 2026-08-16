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

#define SUNXI_PINCTRL_VERSION   "0.0.5"

static const struct sunxi_desc_pin sun8iw22_pins[] = {
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
		SUNXI_FUNCTION(0x1, "gpio_out"),         /* gpioa[14] */
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
	/* Pin banks are: A B C D E F G J K */

	/* bank A */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(A, 0),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x2, "gpadc2"),        /* gpadc2 */
		SUNXI_FUNCTION(0x3, "tp"),            /* tp */
		SUNXI_FUNCTION(0x6, "lbus"),          /* lbus */
		SUNXI_FUNCTION(0x7, "lbus_lbe2"),          /* lbus_lbe2 */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 0, 0),   /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(A, 1),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x2, "gpadc2"),        /* gpadc2 */
		SUNXI_FUNCTION(0x3, "tp"),            /* tp */
		SUNXI_FUNCTION(0x6, "lbus"),          /* lbus */
		SUNXI_FUNCTION(0x7, "lbus_dp1"),          /* lbus_dp1 */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 0, 1),   /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(A, 2),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x2, "gpadc2"),        /* gpadc2 */
		SUNXI_FUNCTION(0x3, "tp"),            /* tp */
		SUNXI_FUNCTION(0x6, "lbus"),          /* lbus */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 0, 2),   /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(A, 3),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x2, "gpadc2"),        /* gpadc2 */
		SUNXI_FUNCTION(0x3, "tp"),            /* tp */
		SUNXI_FUNCTION(0x6, "lbus"),          /* lbus */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 0, 3),   /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(A, 4),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x2, "gpadc0"),        /* gpadc0 */
		SUNXI_FUNCTION(0x4, "rgmii0"),        /* rgmii0 */
		SUNXI_FUNCTION(0x5, "ncsi"),          /* ncsi */
		SUNXI_FUNCTION(0x6, "lbus"),          /* lbus */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 0, 4),   /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(A, 5),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x2, "gpadc0"),        /* gpadc0 */
		SUNXI_FUNCTION(0x4, "rgmii0"),        /* rgmii0 */
		SUNXI_FUNCTION(0x5, "ncsi"),          /* ncsi */
		SUNXI_FUNCTION(0x6, "lbus"),          /* lbus */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 0, 5),   /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(A, 6),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x2, "gpadc0"),        /* gpadc0 */
		SUNXI_FUNCTION(0x4, "rgmii0"),        /* rgmii0 */
		SUNXI_FUNCTION(0x5, "ncsi"),          /* ncsi */
		SUNXI_FUNCTION(0x6, "lbus"),          /* lbus */
		SUNXI_FUNCTION(0x7, "pwmcs_i8"),      /* pwmcs_i8 */
		SUNXI_FUNCTION(0x9, "lcd"),           /* lcd */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 0, 6),   /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(A, 7),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x2, "gpadc0"),        /* gpadc0 */
		SUNXI_FUNCTION(0x4, "rgmii0"),        /* rgmii0 */
		SUNXI_FUNCTION(0x5, "ncsi"),          /* ncsi */
		SUNXI_FUNCTION(0x6, "lbus"),          /* lbus */
		SUNXI_FUNCTION(0x7, "pwmcs_i9"),      /* pwmcs_i9 */
		SUNXI_FUNCTION(0x9, "lcd"),           /* lcd */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 0, 7),   /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(A, 8),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x2, "gpadc0"),        /* gpadc0 */
		SUNXI_FUNCTION(0x4, "rgmii0"),        /* rgmii0 */
		SUNXI_FUNCTION(0x5, "ncsi"),          /* ncsi */
		SUNXI_FUNCTION(0x6, "lbus"),          /* lbus */
		SUNXI_FUNCTION(0x7, "pwmcs_ia"),      /* pwmcs_ia */
		SUNXI_FUNCTION(0x9, "lcd"),           /* lcd */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 0, 8),   /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(A, 9),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x2, "gpadc0"),        /* gpadc0 */
		SUNXI_FUNCTION(0x4, "rgmii0"),        /* rgmii0 */
		SUNXI_FUNCTION(0x5, "ncsi"),          /* ncsi */
		SUNXI_FUNCTION(0x6, "lbus"),          /* lbus */
		SUNXI_FUNCTION(0x7, "pwmcs_ib"),      /* pwmcs_ib */
		SUNXI_FUNCTION(0x8, "lbus_lbe3"),          /* lbus_lbe3 */
		SUNXI_FUNCTION(0x9, "lcd"),           /* lcd */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 0, 9),   /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(A, 10),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x2, "gpadc0"),        /* gpadc0 */
		SUNXI_FUNCTION(0x4, "rgmii0"),        /* rgmii0 */
		SUNXI_FUNCTION(0x5, "ncsi"),          /* ncsi */
		SUNXI_FUNCTION(0x6, "lbus"),          /* lbus */
		SUNXI_FUNCTION(0x7, "pwmcs_ic"),      /* pwmcs_ic */
		SUNXI_FUNCTION(0x9, "lcd"),           /* lcd */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 0, 10),  /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(A, 11),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x2, "gpadc0"),        /* gpadc0 */
		SUNXI_FUNCTION(0x4, "rgmii0"),        /* rgmii0 */
		SUNXI_FUNCTION(0x5, "ncsi"),          /* ncsi */
		SUNXI_FUNCTION(0x6, "lbus"),          /* lbus */
		SUNXI_FUNCTION(0x7, "pwmcs_id"),      /* pwmcs_id */
		SUNXI_FUNCTION(0x9, "lcd"),           /* lcd */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 0, 11),  /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(A, 12),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x2, "gpadc0"),        /* gpadc0 */
		SUNXI_FUNCTION(0x4, "rgmii0"),        /* rgmii0 */
		SUNXI_FUNCTION(0x5, "ncsi"),          /* ncsi */
		SUNXI_FUNCTION(0x6, "lbus"),          /* lbus */
		SUNXI_FUNCTION(0x7, "pwmcs_ie"),      /* pwmcs_ie */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 0, 12),  /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(A, 13),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x2, "gpadc0"),        /* gpadc0 */
		SUNXI_FUNCTION(0x4, "rgmii0"),        /* rgmii0 */
		SUNXI_FUNCTION(0x5, "ncsi"),          /* ncsi */
		SUNXI_FUNCTION(0x6, "lbus"),          /* lbus */
		SUNXI_FUNCTION(0x7, "pwmcs_if"),      /* pwmcs_if */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 0, 13),  /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(A, 14),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x2, "gpadc1"),        /* gpadc1 */
		SUNXI_FUNCTION(0x4, "rgmii0"),        /* rgmii0 */
		SUNXI_FUNCTION(0x5, "ncsi"),          /* ncsi */
		SUNXI_FUNCTION(0x6, "lbus"),          /* lbus */
		SUNXI_FUNCTION(0x7, "uart6"),         /* uart6 */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 0, 14),  /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(A, 15),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x2, "gpadc1"),        /* gpadc1 */
		SUNXI_FUNCTION(0x4, "rgmii0"),        /* rgmii0 */
		SUNXI_FUNCTION(0x5, "ncsi"),          /* ncsi */
		SUNXI_FUNCTION(0x6, "lbus"),          /* lbus */
		SUNXI_FUNCTION(0x7, "uart6"),         /* uart6 */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 0, 15),  /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(A, 16),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x2, "gpadc1"),        /* gpadc1 */
		SUNXI_FUNCTION(0x4, "rgmii0"),        /* rgmii0 */
		SUNXI_FUNCTION(0x5, "ncsi"),          /* ncsi */
		SUNXI_FUNCTION(0x6, "lbus"),          /* lbus */
		SUNXI_FUNCTION(0x7, "twi3"),          /* twi3 */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 0, 16),  /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(A, 17),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x2, "gpadc1"),        /* gpadc1 */
		SUNXI_FUNCTION(0x4, "rgmii0"),        /* rgmii0 */
		SUNXI_FUNCTION(0x5, "ncsi"),          /* ncsi */
		SUNXI_FUNCTION(0x6, "lbus"),          /* lbus */
		SUNXI_FUNCTION(0x7, "twi3"),          /* twi3 */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 0, 17),  /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(A, 18),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x2, "gpadc1"),        /* gpadc1 */
		SUNXI_FUNCTION(0x4, "rgmii0"),        /* rgmii0 */
		SUNXI_FUNCTION(0x5, "ncsi"),          /* ncsi */
		SUNXI_FUNCTION(0x6, "lbus"),          /* lbus */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 0, 18),  /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(A, 19),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x2, "gpadc1"),        /* gpadc1 */
		SUNXI_FUNCTION(0x4, "rgmii0"),        /* rgmii0 */
		SUNXI_FUNCTION(0x5, "ncsi"),          /* ncsi */
		SUNXI_FUNCTION(0x6, "lbus"),          /* lbus */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 0, 19),  /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(A, 20),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x2, "gpadc1"),        /* gpadc1 */
		SUNXI_FUNCTION(0x5, "ncsi"),          /* ncsi */
		SUNXI_FUNCTION(0x6, "lbus"),          /* lbus */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 0, 20),  /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(A, 21),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x2, "gpadc1"),        /* gpadc1 */
		SUNXI_FUNCTION(0x5, "ncsi"),          /* ncsi */
		SUNXI_FUNCTION(0x6, "lbus"),          /* lbus */
		SUNXI_FUNCTION(0x7, "spi2"),          /* spi2 */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 0, 21),  /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(A, 22),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x2, "gpadc1"),        /* gpadc1 */
		SUNXI_FUNCTION(0x5, "ncsi"),          /* ncsi */
		SUNXI_FUNCTION(0x6, "lbus"),          /* lbus */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 0, 22),  /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(A, 23),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x2, "gpadc1"),        /* gpadc1 */
		SUNXI_FUNCTION(0x5, "ncsi"),          /* ncsi */
		SUNXI_FUNCTION(0x6, "lbus"),          /* lbus */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 0, 23),  /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	/* bank B */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(B, 0),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x2, "uart2"),         /* uart2 */
		SUNXI_FUNCTION(0x3, "spi3"),          /* spi3 */
		SUNXI_FUNCTION(0x4, "jtag"),          /* jtag */
		SUNXI_FUNCTION(0x5, "pwm0_6"),        /* pwm0_6 */
		SUNXI_FUNCTION(0x7, "rjtag"),         /* rjtag */
		SUNXI_FUNCTION(0x9, "rgmii2"),        /* rgmii2 */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 1, 0),   /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(B, 1),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x2, "uart2"),         /* uart2 */
		SUNXI_FUNCTION(0x3, "spi3"),          /* spi3 */
		SUNXI_FUNCTION(0x4, "jtag"),          /* jtag */
		SUNXI_FUNCTION(0x5, "pwm0_7"),        /* pwm0_7 */
		SUNXI_FUNCTION(0x7, "rjtag"),         /* rjtag */
		SUNXI_FUNCTION(0x9, "rgmii2"),        /* rgmii2 */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 1, 1),   /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(B, 2),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x2, "uart2"),         /* uart2 */
		SUNXI_FUNCTION(0x3, "spi3"),          /* spi3 */
		SUNXI_FUNCTION(0x4, "jtag"),          /* jtag */
		SUNXI_FUNCTION(0x5, "twi2"),          /* twi2 */
		SUNXI_FUNCTION(0x7, "rjtag"),         /* rjtag */
		SUNXI_FUNCTION(0x9, "rgmii2"),        /* rgmii2 */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 1, 2),   /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(B, 3),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x2, "uart2"),         /* uart2 */
		SUNXI_FUNCTION(0x3, "spi3"),          /* spi3 */
		SUNXI_FUNCTION(0x4, "jtag"),          /* jtag */
		SUNXI_FUNCTION(0x5, "twi2"),          /* twi2 */
		SUNXI_FUNCTION(0x7, "rjtag"),         /* rjtag */
		SUNXI_FUNCTION(0x9, "rgmii2"),        /* rgmii2 */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 1, 3),   /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(B, 4),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x2, "twi1"),          /* twi1 */
		SUNXI_FUNCTION(0x3, "i2s0_mclk"),     /* i2s0_mclk */
		SUNXI_FUNCTION(0x4, "spi3"),          /* spi3 */
		SUNXI_FUNCTION(0x5, "pwm1_0"),        /* pwm1_0 */
		SUNXI_FUNCTION(0x6, "ir0"),           /* ir0 */
		SUNXI_FUNCTION(0x7, "uart8"),         /* uart8 */
		SUNXI_FUNCTION(0x9, "rgmii2"),        /* rgmii2 */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 1, 4),   /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(B, 5),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x2, "twi1"),          /* twi1 */
		SUNXI_FUNCTION(0x3, "i2s0_bclk"),     /* i2s0_bclk */
		SUNXI_FUNCTION(0x4, "spi3"),          /* spi3 */
		SUNXI_FUNCTION(0x5, "pwm1_1"),        /* pwm1_1 */
		SUNXI_FUNCTION(0x6, "ir1"),           /* ir1 */
		SUNXI_FUNCTION(0x7, "uart8"),         /* uart8 */
		SUNXI_FUNCTION(0x9, "rgmii2"),        /* rgmii2 */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 1, 5),   /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(B, 6),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x3, "i2s0_lrck"),     /* i2s0_lrck */
		SUNXI_FUNCTION(0x5, "pwm1_2"),        /* pwm1_2 */
		SUNXI_FUNCTION(0x6, "ir"),            /* ir */
		SUNXI_FUNCTION(0x7, "uart8"),         /* uart8 */
		SUNXI_FUNCTION(0x8, "twi0"),          /* twi0 */
		SUNXI_FUNCTION(0x9, "rgmii2"),        /* rgmii2 */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 1, 6),   /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(B, 7),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x2, "owa"),           /* owa */
		SUNXI_FUNCTION(0x3, "i2s0_dout0"),    /* i2s0_dout0 */
		SUNXI_FUNCTION(0x4, "i2s0_din1"),     /* i2s0_din1 */
		SUNXI_FUNCTION(0x5, "pwm1_3"),        /* pwm1_3 */
		SUNXI_FUNCTION(0x7, "uart8"),         /* uart8 */
		SUNXI_FUNCTION(0x8, "twi0"),          /* twi0 */
		SUNXI_FUNCTION(0x9, "rgmii2"),        /* rgmii2 */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 1, 7),   /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(B, 8),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x2, "owa"),           /* owa */
		SUNXI_FUNCTION(0x3, "i2s0_din0"),     /* i2s0_din0 */
		SUNXI_FUNCTION(0x4, "i2s0_dout1"),    /* i2s0_dout1 */
		SUNXI_FUNCTION(0x5, "pwm0_0"),        /* pwm0_0 */
		SUNXI_FUNCTION(0x7, "uart8"),         /* uart8 */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 1, 8),   /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(B, 9),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x2, "uart0"),         /* uart0 */
		SUNXI_FUNCTION(0x3, "i2s0_din2"),     /* i2s0_din2 */
		SUNXI_FUNCTION(0x4, "i2s0_dout2"),    /* i2s0_dout2 */
		SUNXI_FUNCTION(0x6, "twi0"),          /* twi0 */
		SUNXI_FUNCTION(0x7, "uart8"),         /* uart8 */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 1, 9),   /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(B, 10),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x2, "uart0"),         /* uart0 */
		SUNXI_FUNCTION(0x3, "i2s0_din3"),     /* i2s0_din3 */
		SUNXI_FUNCTION(0x4, "i2s0_dout3"),    /* i2s0_dout3 */
		SUNXI_FUNCTION(0x5, "pwm0_1"),        /* pwm0_1 */
		SUNXI_FUNCTION(0x6, "twi0"),          /* twi0 */
		SUNXI_FUNCTION(0x7, "uart8"),         /* uart8 */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 1, 10),  /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(B, 11),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x2, "twi5"),          /* twi5 */
		SUNXI_FUNCTION(0x3, "uart7"),         /* uart7 */
		SUNXI_FUNCTION(0x4, "spi3"),          /* spi3 */
		SUNXI_FUNCTION(0x5, "pwm0_2"),        /* pwm0_2 */
		SUNXI_FUNCTION(0x6, "watchdog"),      /* watchdog */
		SUNXI_FUNCTION(0x7, "uart8"),         /* uart8 */
		SUNXI_FUNCTION(0x9, "rgmii2"),        /* rgmii2 */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 1, 11),  /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(B, 12),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x2, "twi5"),          /* twi5 */
		SUNXI_FUNCTION(0x3, "uart7"),         /* uart7 */
		SUNXI_FUNCTION(0x4, "spi3"),          /* spi3 */
		SUNXI_FUNCTION(0x5, "pwm0_3"),        /* pwm0_3 */
		SUNXI_FUNCTION(0x6, "pll"),           /* pll */
		SUNXI_FUNCTION(0x9, "rgmii2"),        /* rgmii2 */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 1, 12),  /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(B, 13),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x2, "twi4"),          /* twi4 */
		SUNXI_FUNCTION(0x3, "uart7"),         /* uart7 */
		SUNXI_FUNCTION(0x4, "spi3"),          /* spi3 */
		SUNXI_FUNCTION(0x5, "pwm0_4"),        /* pwm0_4 */
		SUNXI_FUNCTION(0x6, "ir2"),           /* ir2 */
		SUNXI_FUNCTION(0x7, "clk"),           /* clk */
		SUNXI_FUNCTION(0x9, "rgmii2"),        /* rgmii2 */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 1, 13),  /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(B, 14),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x2, "twi4"),          /* twi4 */
		SUNXI_FUNCTION(0x3, "uart7"),         /* uart7 */
		SUNXI_FUNCTION(0x4, "spi3"),          /* spi3 */
		SUNXI_FUNCTION(0x5, "pwm0_5"),        /* pwm0_5 */
		SUNXI_FUNCTION(0x6, "ir3"),           /* ir3 */
		SUNXI_FUNCTION(0x7, "clk"),           /* clk */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 1, 14),  /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	/* bank C */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(C, 0),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x2, "sdc2"),          /* sdc2 */
		SUNXI_FUNCTION(0x3, "spi0"),          /* spi0 */
		SUNXI_FUNCTION(0x4, "spif0"),         /* spif0 */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 2, 0),   /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(C, 1),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x2, "sdc2"),          /* sdc2 */
		SUNXI_FUNCTION(0x3, "spi0"),          /* spi0 */
		SUNXI_FUNCTION(0x4, "spif0"),         /* spif0 */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 2, 1),   /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(C, 2),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x2, "sdc2"),          /* sdc2 */
		SUNXI_FUNCTION(0x3, "spi0"),          /* spi0 */
		SUNXI_FUNCTION(0x4, "spif0"),         /* spif0 */
		SUNXI_FUNCTION(0x5, "boot"),          /* boot */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 2, 2),   /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(C, 3),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x2, "sdc2"),          /* sdc2 */
		SUNXI_FUNCTION(0x3, "spi0"),          /* spi0 */
		SUNXI_FUNCTION(0x4, "spif0"),         /* spif0 */
		SUNXI_FUNCTION(0x5, "boot"),          /* boot */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 2, 3),   /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(C, 4),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x2, "sdc2"),          /* sdc2 */
		SUNXI_FUNCTION(0x3, "spi0"),          /* spi0 */
		SUNXI_FUNCTION(0x4, "spif0"),         /* spif0 */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 2, 4),   /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(C, 5),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x2, "sdc2"),          /* sdc2 */
		SUNXI_FUNCTION(0x3, "spi0"),          /* spi0 */
		SUNXI_FUNCTION(0x4, "spif0"),         /* spif0 */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 2, 5),   /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(C, 6),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x2, "sdc2"),          /* sdc2 */
		SUNXI_FUNCTION(0x3, "spi0"),          /* spi0 */
		SUNXI_FUNCTION(0x4, "spif0"),         /* spif0 */
		SUNXI_FUNCTION(0x6, "uart6"),         /* uart6 */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 2, 6),   /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(C, 7),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x2, "sdc2"),          /* sdc2 */
		SUNXI_FUNCTION(0x4, "spif0"),         /* spif0 */
		SUNXI_FUNCTION(0x6, "uart6"),         /* uart6 */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 2, 7),   /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(C, 8),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x2, "sdc2"),          /* sdc2 */
		SUNXI_FUNCTION(0x4, "spif0"),         /* spif0 */
		SUNXI_FUNCTION(0x5, "uart5"),         /* uart5 */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 2, 8),   /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(C, 9),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x2, "sdc2"),          /* sdc2 */
		SUNXI_FUNCTION(0x4, "spif0"),         /* spif0 */
		SUNXI_FUNCTION(0x5, "uart5"),         /* uart5 */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 2, 9),   /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(C, 10),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x2, "sdc2"),          /* sdc2 */
		SUNXI_FUNCTION(0x4, "spif0"),         /* spif0 */
		SUNXI_FUNCTION(0x5, "uart5"),         /* uart5 */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 2, 10),  /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(C, 11),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x2, "sdc2"),          /* sdc2 */
		SUNXI_FUNCTION(0x5, "uart5"),         /* uart5 */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 2, 11),  /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	/* bank D */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(D, 0),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x2, "lcd"),           /* lcd */
		SUNXI_FUNCTION(0x3, "lvds0"),         /* lvds0 */
		SUNXI_FUNCTION(0x4, "dsi"),           /* dsi */
		SUNXI_FUNCTION(0x5, "pwm0_0"),        /* pwm0_0 */
		SUNXI_FUNCTION(0x6, "uart2"),         /* uart2 */
		SUNXI_FUNCTION(0x7, "uart9"),         /* uart9 */
		SUNXI_FUNCTION(0x8, "lbus"),          /* lbus */
		SUNXI_FUNCTION(0x9, "pwmcs_o0"),      /* pwmcs_o0 */
		SUNXI_FUNCTION(0xb, "rgmii2"),        /* rgmii2 */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 3, 0),   /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(D, 1),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x2, "lcd"),           /* lcd */
		SUNXI_FUNCTION(0x3, "lvds0"),         /* lvds0 */
		SUNXI_FUNCTION(0x4, "dsi"),           /* dsi */
		SUNXI_FUNCTION(0x5, "pwm0_1"),        /* pwm0_1 */
		SUNXI_FUNCTION(0x6, "uart2"),         /* uart2 */
		SUNXI_FUNCTION(0x7, "uart9"),         /* uart9 */
		SUNXI_FUNCTION(0x8, "lbus"),          /* lbus */
		SUNXI_FUNCTION(0x9, "pwmcs_o1"),      /* pwmcs_o1 */
		SUNXI_FUNCTION(0xb, "rgmii2"),        /* rgmii2 */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 3, 1),   /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(D, 2),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x2, "lcd"),           /* lcd */
		SUNXI_FUNCTION(0x3, "lvds0"),         /* lvds0 */
		SUNXI_FUNCTION(0x4, "dsi"),           /* dsi */
		SUNXI_FUNCTION(0x5, "pwm0_2"),        /* pwm0_2 */
		SUNXI_FUNCTION(0x6, "uart2"),         /* uart2 */
		SUNXI_FUNCTION(0x7, "uart9"),         /* uart9 */
		SUNXI_FUNCTION(0x8, "lbus"),          /* lbus */
		SUNXI_FUNCTION(0x9, "pwmcs_o2"),      /* pwmcs_o2 */
		SUNXI_FUNCTION(0xb, "rgmii2"),        /* rgmii2 */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 3, 2),   /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(D, 3),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x2, "lcd"),           /* lcd */
		SUNXI_FUNCTION(0x3, "lvds0"),         /* lvds0 */
		SUNXI_FUNCTION(0x4, "dsi"),           /* dsi */
		SUNXI_FUNCTION(0x5, "pwm0_3"),        /* pwm0_3 */
		SUNXI_FUNCTION(0x6, "uart2"),         /* uart2 */
		SUNXI_FUNCTION(0x7, "uart9"),         /* uart9 */
		SUNXI_FUNCTION(0x8, "lbus"),          /* lbus */
		SUNXI_FUNCTION(0x9, "pwmcs_o3"),      /* pwmcs_o3 */
		SUNXI_FUNCTION(0xb, "rgmii2"),        /* rgmii2 */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 3, 3),   /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(D, 4),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x2, "lcd"),           /* lcd */
		SUNXI_FUNCTION(0x3, "lvds0"),         /* lvds0 */
		SUNXI_FUNCTION(0x4, "dsi"),           /* dsi */
		SUNXI_FUNCTION(0x5, "pwm0_4"),        /* pwm0_4 */
		SUNXI_FUNCTION(0x6, "uart4"),         /* uart4 */
		SUNXI_FUNCTION(0x7, "uart6"),         /* uart6 */
		SUNXI_FUNCTION(0x8, "lbus"),          /* lbus */
		SUNXI_FUNCTION(0x9, "pwmcs_o4"),      /* pwmcs_o4 */
		SUNXI_FUNCTION(0xb, "rgmii2"),        /* rgmii2 */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 3, 4),   /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(D, 5),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x2, "lcd"),           /* lcd */
		SUNXI_FUNCTION(0x3, "lvds0"),         /* lvds0 */
		SUNXI_FUNCTION(0x4, "dsi"),           /* dsi */
		SUNXI_FUNCTION(0x5, "pwm0_5"),        /* pwm0_5 */
		SUNXI_FUNCTION(0x6, "uart4"),         /* uart4 */
		SUNXI_FUNCTION(0x7, "uart6"),         /* uart6 */
		SUNXI_FUNCTION(0x8, "lbus"),          /* lbus */
		SUNXI_FUNCTION(0x9, "pwmcs_o5"),      /* pwmcs_o5 */
		SUNXI_FUNCTION(0xb, "rgmii2"),        /* rgmii2 */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 3, 5),   /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(D, 6),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x2, "lcd"),           /* lcd */
		SUNXI_FUNCTION(0x3, "lvds0"),         /* lvds0 */
		SUNXI_FUNCTION(0x4, "dsi"),           /* dsi */
		SUNXI_FUNCTION(0x5, "pwm0_6"),        /* pwm0_6 */
		SUNXI_FUNCTION(0x6, "uart4"),         /* uart4 */
		SUNXI_FUNCTION(0x7, "uart6"),         /* uart6 */
		SUNXI_FUNCTION(0x8, "lbus"),          /* lbus */
		SUNXI_FUNCTION(0x9, "pwmcs_o6"),      /* pwmcs_o6 */
		SUNXI_FUNCTION(0xb, "rgmii2"),        /* rgmii2 */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 3, 6),   /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(D, 7),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x2, "lcd"),           /* lcd */
		SUNXI_FUNCTION(0x3, "lvds0"),         /* lvds0 */
		SUNXI_FUNCTION(0x4, "dsi"),           /* dsi */
		SUNXI_FUNCTION(0x5, "pwm0_7"),        /* pwm0_7 */
		SUNXI_FUNCTION(0x6, "uart4"),         /* uart4 */
		SUNXI_FUNCTION(0x7, "uart6"),         /* uart6 */
		SUNXI_FUNCTION(0x8, "lbus"),          /* lbus */
		SUNXI_FUNCTION(0x9, "pwmcs_o7"),      /* pwmcs_o7 */
		SUNXI_FUNCTION(0xb, "rgmii2"),        /* rgmii2 */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 3, 7),   /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(D, 8),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x2, "lcd"),           /* lcd */
		SUNXI_FUNCTION(0x3, "lvds0"),         /* lvds0 */
		SUNXI_FUNCTION(0x4, "dsi"),           /* dsi */
		SUNXI_FUNCTION(0x5, "pwm1_0"),        /* pwm1_0 */
		SUNXI_FUNCTION(0x8, "lbus"),          /* lbus */
		SUNXI_FUNCTION(0x9, "pwmcs_o8"),      /* pwmcs_o8 */
		SUNXI_FUNCTION(0xb, "rgmii2"),        /* rgmii2 */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 3, 8),   /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(D, 9),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x2, "lcd"),           /* lcd */
		SUNXI_FUNCTION(0x3, "lvds0"),         /* lvds0 */
		SUNXI_FUNCTION(0x4, "dsi"),           /* dsi */
		SUNXI_FUNCTION(0x5, "pwm1_1"),        /* pwm1_1 */
		SUNXI_FUNCTION(0x6, "dmic"),          /* dmic */
		SUNXI_FUNCTION(0x8, "lbus"),          /* lbus */
		SUNXI_FUNCTION(0x9, "pwmcs_o9"),      /* pwmcs_o9 */
		SUNXI_FUNCTION(0xb, "rgmii2"),        /* rgmii2 */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 3, 9),   /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(D, 10),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x2, "lcd"),           /* lcd */
		SUNXI_FUNCTION(0x3, "lvds1"),         /* lvds1 */
		SUNXI_FUNCTION(0x4, "spi1"),          /* spi1 */
		SUNXI_FUNCTION(0x5, "pwm1_2"),        /* pwm1_2 */
		SUNXI_FUNCTION(0x6, "dmic"),          /* dmic */
		SUNXI_FUNCTION(0x7, "uart8"),         /* uart8 */
		SUNXI_FUNCTION(0x8, "lbus"),          /* lbus */
		SUNXI_FUNCTION(0x9, "pwmcs_oa"),      /* pwmcs_oa */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 3, 10),  /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(D, 11),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x2, "lcd"),           /* lcd */
		SUNXI_FUNCTION(0x3, "lvds1"),         /* lvds1 */
		SUNXI_FUNCTION(0x4, "spi1"),          /* spi1 */
		SUNXI_FUNCTION(0x5, "pwm1_3"),        /* pwm1_3 */
		SUNXI_FUNCTION(0x6, "dmic"),          /* dmic */
		SUNXI_FUNCTION(0x7, "uart8"),         /* uart8 */
		SUNXI_FUNCTION(0x8, "lbus"),          /* lbus */
		SUNXI_FUNCTION(0x9, "pwmcs_ob"),      /* pwmcs_ob */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 3, 11),  /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(D, 12),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x2, "lcd"),           /* lcd */
		SUNXI_FUNCTION(0x3, "lvds1"),         /* lvds1 */
		SUNXI_FUNCTION(0x4, "spi1"),          /* spi1 */
		SUNXI_FUNCTION(0x5, "pwm1_4"),        /* pwm1_4 */
		SUNXI_FUNCTION(0x6, "dmic"),          /* dmic */
		SUNXI_FUNCTION(0x7, "uart8"),         /* uart8 */
		SUNXI_FUNCTION(0x8, "lbus"),          /* lbus */
		SUNXI_FUNCTION(0x9, "pwmcs_oc"),      /* pwmcs_oc */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 3, 12),  /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(D, 13),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x2, "lcd"),           /* lcd */
		SUNXI_FUNCTION(0x3, "lvds1"),         /* lvds1 */
		SUNXI_FUNCTION(0x4, "spi1"),          /* spi1 */
		SUNXI_FUNCTION(0x5, "pwm1_5"),        /* pwm1_5 */
		SUNXI_FUNCTION(0x6, "dmic"),          /* dmic */
		SUNXI_FUNCTION(0x7, "uart8"),         /* uart8 */
		SUNXI_FUNCTION(0x8, "lbus"),          /* lbus */
		SUNXI_FUNCTION(0x9, "pwmcs_od"),      /* pwmcs_od */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 3, 13),  /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(D, 14),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x2, "lcd"),           /* lcd */
		SUNXI_FUNCTION(0x3, "lvds1"),         /* lvds1 */
		SUNXI_FUNCTION(0x4, "spi1"),          /* spi1 */
		SUNXI_FUNCTION(0x5, "pwm1_6"),        /* pwm1_6 */
		SUNXI_FUNCTION(0x6, "uart3"),         /* uart3 */
		SUNXI_FUNCTION(0x7, "uart8"),         /* uart8 */
		SUNXI_FUNCTION(0x8, "lbus"),          /* lbus */
		SUNXI_FUNCTION(0x9, "pwmcs_oe"),      /* pwmcs_oe */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 3, 14),  /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(D, 15),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x2, "lcd"),           /* lcd */
		SUNXI_FUNCTION(0x3, "lvds1"),         /* lvds1 */
		SUNXI_FUNCTION(0x4, "spi1"),          /* spi1 */
		SUNXI_FUNCTION(0x5, "pwm1_7"),        /* pwm1_7 */
		SUNXI_FUNCTION(0x6, "uart3"),         /* uart3 */
		SUNXI_FUNCTION(0x7, "uart8"),         /* uart8 */
		SUNXI_FUNCTION(0x8, "lbus"),          /* lbus */
		SUNXI_FUNCTION(0x9, "pwmcs_of"),      /* pwmcs_of */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 3, 15),  /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(D, 16),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x2, "lcd"),           /* lcd */
		SUNXI_FUNCTION(0x3, "lvds1"),         /* lvds1 */
		SUNXI_FUNCTION(0x5, "pwm2_0"),        /* pwm2_0 */
		SUNXI_FUNCTION(0x6, "uart3"),         /* uart3 */
		SUNXI_FUNCTION(0x7, "uart8"),         /* uart8 */
		SUNXI_FUNCTION(0x8, "lbus"),          /* lbus */
		SUNXI_FUNCTION(0x9, "pwmcs_i0"),      /* pwmcs_i0 */
		SUNXI_FUNCTION(0xb, "rgmii2"),        /* rgmii2 */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 3, 16),  /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(D, 17),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x2, "lcd"),           /* lcd */
		SUNXI_FUNCTION(0x3, "lvds1"),         /* lvds1 */
		SUNXI_FUNCTION(0x5, "pwm2_1"),        /* pwm2_1 */
		SUNXI_FUNCTION(0x6, "uart3"),         /* uart3 */
		SUNXI_FUNCTION(0x7, "uart8"),         /* uart8 */
		SUNXI_FUNCTION(0x8, "lbus"),          /* lbus */
		SUNXI_FUNCTION(0x9, "pwmcs_i1"),      /* pwmcs_i1 */
		SUNXI_FUNCTION(0xb, "rgmii2"),        /* rgmii2 */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 3, 17),  /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(D, 18),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x2, "lcd"),           /* lcd */
		SUNXI_FUNCTION(0x3, "lvds1"),         /* lvds1 */
		SUNXI_FUNCTION(0x4, "twi5"),          /* twi5 */
		SUNXI_FUNCTION(0x5, "pwm2_2"),        /* pwm2_2 */
		SUNXI_FUNCTION(0x8, "lbus"),          /* lbus */
		SUNXI_FUNCTION(0x9, "pwmcs_i2"),      /* pwmcs_i2 */
		SUNXI_FUNCTION(0xb, "rgmii2"),        /* rgmii2 */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 3, 18),  /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(D, 19),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x2, "lcd"),           /* lcd */
		SUNXI_FUNCTION(0x3, "lvds1"),         /* lvds1 */
		SUNXI_FUNCTION(0x4, "twi5"),          /* twi5 */
		SUNXI_FUNCTION(0x5, "pwm2_3"),        /* pwm2_3 */
		SUNXI_FUNCTION(0x8, "lbus"),          /* lbus */
		SUNXI_FUNCTION(0x9, "pwmcs_i3"),      /* pwmcs_i3 */
		SUNXI_FUNCTION(0xb, "rgmii2"),        /* rgmii2 */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 3, 19),  /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(D, 20),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x2, "lcd"),           /* lcd */
		SUNXI_FUNCTION(0x3, "pwm2_4"),        /* pwm2_4 */
		SUNXI_FUNCTION(0x4, "uart3"),         /* uart3 */
		SUNXI_FUNCTION(0x5, "twi5"),          /* twi5 */
		SUNXI_FUNCTION(0x8, "lbus"),          /* lbus */
		SUNXI_FUNCTION(0x9, "pwmcs_i4"),      /* pwmcs_i4 */
		SUNXI_FUNCTION(0xb, "rgmii2"),        /* rgmii2 */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 3, 20),  /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(D, 21),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x2, "lcd"),           /* lcd */
		SUNXI_FUNCTION(0x3, "pwm2_5"),        /* pwm2_5 */
		SUNXI_FUNCTION(0x4, "uart3"),         /* uart3 */
		SUNXI_FUNCTION(0x5, "twi5"),          /* twi5 */
		SUNXI_FUNCTION(0x8, "lbus"),          /* lbus */
		SUNXI_FUNCTION(0x9, "pwmcs_i5"),      /* pwmcs_i5 */
		SUNXI_FUNCTION(0xb, "rgmii2"),        /* rgmii2 */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 3, 21),  /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(D, 22),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x3, "pwm2_6"),        /* pwm2_6 */
		SUNXI_FUNCTION(0x4, "uart3"),         /* uart3 */
		SUNXI_FUNCTION(0x5, "twi4"),          /* twi4 */
		SUNXI_FUNCTION(0x6, "twi0"),          /* twi0 */
		SUNXI_FUNCTION(0x7, "csi0"),          /* csi0 */
		SUNXI_FUNCTION(0x8, "lbus"),          /* lbus */
		SUNXI_FUNCTION(0x9, "pwmcs_i6"),      /* pwmcs_i6 */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 3, 22),  /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(D, 23),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x3, "pwm2_7"),        /* pwm2_7 */
		SUNXI_FUNCTION(0x4, "uart3"),         /* uart3 */
		SUNXI_FUNCTION(0x5, "twi4"),          /* twi4 */
		SUNXI_FUNCTION(0x6, "twi0"),          /* twi0 */
		SUNXI_FUNCTION(0x7, "csi1"),          /* csi1 */
		SUNXI_FUNCTION(0x8, "lbus"),          /* lbus */
		SUNXI_FUNCTION(0x9, "pwmcs_i7"),      /* pwmcs_i7 */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 3, 23),  /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	/* bank E */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(E, 0),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x2, "mcsi0"),         /* mcsi0 */
		SUNXI_FUNCTION(0x4, "ncsi"),          /* ncsi */
		SUNXI_FUNCTION(0x5, "clk"),           /* clk */
		SUNXI_FUNCTION(0x7, "dmic"),          /* dmic */
		SUNXI_FUNCTION(0x9, "sdc3"),          /* sdc3 */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 4, 0),   /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(E, 1),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x2, "twi1"),          /* twi1 */
		SUNXI_FUNCTION(0x4, "ncsi"),          /* ncsi */
		SUNXI_FUNCTION(0x6, "spi2"),          /* spi2 */
		SUNXI_FUNCTION(0x7, "dmic"),          /* dmic */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 4, 1),   /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(E, 2),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x2, "twi1"),          /* twi1 */
		SUNXI_FUNCTION(0x3, "pwm0_3"),        /* pwm0_3 */
		SUNXI_FUNCTION(0x4, "ncsi"),          /* ncsi */
		SUNXI_FUNCTION(0x6, "spi2"),          /* spi2 */
		SUNXI_FUNCTION(0x7, "dmic"),          /* dmic */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 4, 2),   /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(E, 3),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x2, "twi2"),          /* twi2 */
		SUNXI_FUNCTION(0x3, "uart5"),         /* uart5 */
		SUNXI_FUNCTION(0x4, "ncsi"),          /* ncsi */
		SUNXI_FUNCTION(0x5, "uart6"),         /* uart6 */
		SUNXI_FUNCTION(0x6, "spi2"),          /* spi2 */
		SUNXI_FUNCTION(0x7, "dmic"),          /* dmic */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 4, 3),   /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(E, 4),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x2, "twi2"),          /* twi2 */
		SUNXI_FUNCTION(0x3, "uart5"),         /* uart5 */
		SUNXI_FUNCTION(0x4, "ncsi"),          /* ncsi */
		SUNXI_FUNCTION(0x5, "uart6"),         /* uart6 */
		SUNXI_FUNCTION(0x6, "spi2"),          /* spi2 */
		SUNXI_FUNCTION(0x7, "dmic"),          /* dmic */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 4, 4),   /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(E, 5),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x2, "mcsi1"),         /* mcsi1 */
		SUNXI_FUNCTION(0x3, "uart5"),         /* uart5 */
		SUNXI_FUNCTION(0x4, "ncsi"),          /* ncsi */
		SUNXI_FUNCTION(0x5, "uart6"),         /* uart6 */
		SUNXI_FUNCTION(0x6, "spi2"),          /* spi2 */
		SUNXI_FUNCTION(0x9, "sdc3"),          /* sdc3 */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 4, 5),   /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(E, 6),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x2, "twi3"),          /* twi3 */
		SUNXI_FUNCTION(0x3, "uart5"),         /* uart5 */
		SUNXI_FUNCTION(0x4, "ncsi"),          /* ncsi */
		SUNXI_FUNCTION(0x5, "uart6"),         /* uart6 */
		SUNXI_FUNCTION(0x6, "spi2"),          /* spi2 */
		SUNXI_FUNCTION(0x9, "sdc3"),          /* sdc3 */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 4, 6),   /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(E, 7),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x2, "twi3"),          /* twi3 */
		SUNXI_FUNCTION(0x3, "pwm0_2"),        /* pwm0_2 */
		SUNXI_FUNCTION(0x4, "ncsi"),          /* ncsi */
		SUNXI_FUNCTION(0x6, "spi2"),          /* spi2 */
		SUNXI_FUNCTION(0x9, "sdc3"),          /* sdc3 */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 4, 7),   /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(E, 8),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x2, "mcsi2"),         /* mcsi2 */
		SUNXI_FUNCTION(0x3, "pwm0_4"),        /* pwm0_4 */
		SUNXI_FUNCTION(0x4, "csi0"),          /* csi0 */
		SUNXI_FUNCTION(0x5, "clk"),           /* clk */
		SUNXI_FUNCTION(0x6, "spi2"),          /* spi2 */
		SUNXI_FUNCTION(0x9, "sdc3"),          /* sdc3 */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 4, 8),   /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(E, 9),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x2, "tcon"),          /* tcon */
		SUNXI_FUNCTION(0x3, "pwm0_5"),        /* pwm0_5 */
		SUNXI_FUNCTION(0x4, "csi1"),          /* csi1 */
		SUNXI_FUNCTION(0x9, "sdc3"),          /* sdc3 */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 4, 9),   /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	/* bank F */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(F, 0),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x2, "sdc0"),          /* sdc0 */
		SUNXI_FUNCTION(0x3, "jtag"),          /* jtag */
		SUNXI_FUNCTION(0x4, "uart1"),         /* uart1 */
		SUNXI_FUNCTION(0x7, "rjtag"),         /* rjtag */
		SUNXI_FUNCTION(0x8, "ir0"),           /* ir0 */
		SUNXI_FUNCTION(0x9, "lbus"),          /* lbus */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 5, 0),   /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(F, 1),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x2, "sdc0"),          /* sdc0 */
		SUNXI_FUNCTION(0x3, "jtag"),          /* jtag */
		SUNXI_FUNCTION(0x4, "uart1"),         /* uart1 */
		SUNXI_FUNCTION(0x7, "rjtag"),         /* rjtag */
		SUNXI_FUNCTION(0x8, "ir1"),           /* ir1 */
		SUNXI_FUNCTION(0x9, "lbus"),          /* lbus */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 5, 1),   /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(F, 2),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x2, "sdc0"),          /* sdc0 */
		SUNXI_FUNCTION(0x3, "uart0"),         /* uart0 */
		SUNXI_FUNCTION(0x4, "dmic"),          /* dmic */
		SUNXI_FUNCTION(0x6, "spi3"),          /* spi3 */
		SUNXI_FUNCTION(0x8, "ir2"),           /* ir2 */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 5, 2),   /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(F, 3),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x2, "sdc0"),          /* sdc0 */
		SUNXI_FUNCTION(0x3, "jtag"),          /* jtag */
		SUNXI_FUNCTION(0x4, "dmic"),          /* dmic */
		SUNXI_FUNCTION(0x6, "spi3"),          /* spi3 */
		SUNXI_FUNCTION(0x7, "rjtag"),         /* rjtag */
		SUNXI_FUNCTION(0x8, "ir3"),           /* ir3 */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 5, 3),   /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(F, 4),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x2, "sdc0"),          /* sdc0 */
		SUNXI_FUNCTION(0x3, "uart0"),         /* uart0 */
		SUNXI_FUNCTION(0x4, "dmic"),          /* dmic */
		SUNXI_FUNCTION(0x6, "spi3"),          /* spi3 */
		SUNXI_FUNCTION(0x8, "ir"),            /* ir */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 5, 4),   /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(F, 5),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x2, "sdc0"),          /* sdc0 */
		SUNXI_FUNCTION(0x3, "jtag"),          /* jtag */
		SUNXI_FUNCTION(0x4, "dmic"),          /* dmic */
		SUNXI_FUNCTION(0x6, "spi3"),          /* spi3 */
		SUNXI_FUNCTION(0x7, "rjtag"),         /* rjtag */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 5, 5),   /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(F, 6),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x4, "dmic"),          /* dmic */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 5, 6),   /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	/* bank G */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(G, 0),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x2, "sdc1"),          /* sdc1 */
		SUNXI_FUNCTION(0x4, "uart8"),         /* uart8 */
		SUNXI_FUNCTION(0x5, "rgmii1"),        /* rgmii1 */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 6, 0),   /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(G, 1),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x2, "sdc1"),          /* sdc1 */
		SUNXI_FUNCTION(0x4, "uart8"),         /* uart8 */
		SUNXI_FUNCTION(0x5, "rgmii1"),        /* rgmii1 */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 6, 1),   /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(G, 2),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x2, "sdc1"),          /* sdc1 */
		SUNXI_FUNCTION(0x4, "uart8"),         /* uart8 */
		SUNXI_FUNCTION(0x5, "rgmii1"),        /* rgmii1 */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 6, 2),   /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(G, 3),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x2, "sdc1"),          /* sdc1 */
		SUNXI_FUNCTION(0x4, "uart8"),         /* uart8 */
		SUNXI_FUNCTION(0x5, "rgmii1"),        /* rgmii1 */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 6, 3),   /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(G, 4),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x2, "sdc1"),          /* sdc1 */
		SUNXI_FUNCTION(0x5, "rgmii1"),        /* rgmii1 */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 6, 4),   /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(G, 5),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x2, "sdc1"),          /* sdc1 */
		SUNXI_FUNCTION(0x5, "rgmii1"),        /* rgmii1 */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 6, 5),   /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(G, 6),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x2, "uart1"),         /* uart1 */
		SUNXI_FUNCTION(0x3, "twi4"),          /* twi4 */
		SUNXI_FUNCTION(0x4, "uart9"),         /* uart9 */
		SUNXI_FUNCTION(0x5, "rgmii1"),        /* rgmii1 */
		SUNXI_FUNCTION(0x6, "i2s2_mclk"),     /* i2s2_mclk */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 6, 6),   /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(G, 7),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x2, "uart1"),         /* uart1 */
		SUNXI_FUNCTION(0x3, "twi4"),          /* twi4 */
		SUNXI_FUNCTION(0x4, "uart9"),         /* uart9 */
		SUNXI_FUNCTION(0x5, "rgmii1"),        /* rgmii1 */
		SUNXI_FUNCTION(0x6, "i2s2_bclk"),     /* i2s2_bclk */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 6, 7),   /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(G, 8),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x2, "uart1"),         /* uart1 */
		SUNXI_FUNCTION(0x3, "twi5"),          /* twi5 */
		SUNXI_FUNCTION(0x4, "uart9"),         /* uart9 */
		SUNXI_FUNCTION(0x5, "rgmii1"),        /* rgmii1 */
		SUNXI_FUNCTION(0x6, "i2s2_lrck"),     /* i2s2_lrck */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 6, 8),   /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(G, 9),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x2, "uart1"),         /* uart1 */
		SUNXI_FUNCTION(0x3, "twi5"),          /* twi5 */
		SUNXI_FUNCTION(0x4, "uart9"),         /* uart9 */
		SUNXI_FUNCTION(0x5, "rgmii1"),        /* rgmii1 */
		SUNXI_FUNCTION(0x6, "i2s2_dout0"),    /* i2s2_dout0 */
		SUNXI_FUNCTION(0x7, "i2s2_din1"),     /* i2s2_din1 */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 6, 9),   /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(G, 10),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x2, "uart4"),         /* uart4 */
		SUNXI_FUNCTION(0x3, "i2s1_mclk"),     /* i2s1_mclk */
		SUNXI_FUNCTION(0x4, "twi2"),          /* twi2 */
		SUNXI_FUNCTION(0x5, "rgmii1"),        /* rgmii1 */
		SUNXI_FUNCTION(0x6, "i2s2_din0"),     /* i2s2_din0 */
		SUNXI_FUNCTION(0x7, "i2s2_dout1"),    /* i2s2_dout1 */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 6, 10),  /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(G, 11),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x2, "uart4"),         /* uart4 */
		SUNXI_FUNCTION(0x3, "i2s1_bclk"),     /* i2s1_bclk */
		SUNXI_FUNCTION(0x4, "twi2"),          /* twi2 */
		SUNXI_FUNCTION(0x5, "rgmii1"),        /* rgmii1 */
		SUNXI_FUNCTION(0x6, "dmic"),          /* dmic */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 6, 11),  /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(G, 12),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x2, "uart4"),         /* uart4 */
		SUNXI_FUNCTION(0x3, "i2s1_lrck"),     /* i2s1_lrck */
		SUNXI_FUNCTION(0x5, "rgmii1"),        /* rgmii1 */
		SUNXI_FUNCTION(0x6, "dmic"),          /* dmic */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 6, 12),  /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(G, 13),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x2, "uart4"),         /* uart4 */
		SUNXI_FUNCTION(0x3, "i2s1_dout0"),    /* i2s1_dout0 */
		SUNXI_FUNCTION(0x4, "i2s1_din1"),     /* i2s1_din1 */
		SUNXI_FUNCTION(0x5, "rgmii1"),        /* rgmii1 */
		SUNXI_FUNCTION(0x6, "dmic"),          /* dmic */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 6, 13),  /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(G, 14),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x3, "i2s1_din0"),     /* i2s1_din0 */
		SUNXI_FUNCTION(0x4, "i2s1_dout1"),    /* i2s1_dout1 */
		SUNXI_FUNCTION(0x5, "rgmii1"),        /* rgmii1 */
		SUNXI_FUNCTION(0x6, "dmic"),          /* dmic */
		SUNXI_FUNCTION(0x8, "lbus"),          /* lbus */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 6, 14),  /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(G, 15),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x5, "rgmii1"),        /* rgmii1 */
		SUNXI_FUNCTION(0x6, "dmic"),          /* dmic */
		SUNXI_FUNCTION(0x8, "lbus"),          /* lbus */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 6, 15),  /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	/* bank J */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(J, 0),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x2, "pwm2_0"),        /* pwm2_0 */
		SUNXI_FUNCTION(0x3, "i2s2_mclk"),     /* i2s2_mclk */
		SUNXI_FUNCTION(0x4, "twi4"),          /* twi4 */
		SUNXI_FUNCTION(0x5, "rgmii0"),        /* rgmii0 */
		SUNXI_FUNCTION(0x6, "dmic"),          /* dmic */
		SUNXI_FUNCTION(0x7, "uart5"),         /* uart5 */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 7, 0),   /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(J, 1),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x2, "pwm2_1"),        /* pwm2_1 */
		SUNXI_FUNCTION(0x3, "i2s2_bclk"),     /* i2s2_bclk */
		SUNXI_FUNCTION(0x4, "twi4"),          /* twi4 */
		SUNXI_FUNCTION(0x5, "rgmii0"),        /* rgmii0 */
		SUNXI_FUNCTION(0x6, "dmic"),          /* dmic */
		SUNXI_FUNCTION(0x7, "uart5"),         /* uart5 */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 7, 1),   /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(J, 2),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x2, "pwm2_2"),        /* pwm2_2 */
		SUNXI_FUNCTION(0x3, "i2s2_lrck"),     /* i2s2_lrck */
		SUNXI_FUNCTION(0x5, "rgmii0"),        /* rgmii0 */
		SUNXI_FUNCTION(0x6, "dmic"),          /* dmic */
		SUNXI_FUNCTION(0x7, "uart5"),         /* uart5 */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 7, 2),   /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(J, 3),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x2, "pwm2_3"),        /* pwm2_3 */
		SUNXI_FUNCTION(0x3, "i2s2_dout0"),    /* i2s2_dout0 */
		SUNXI_FUNCTION(0x4, "i2s2_din1"),     /* i2s2_din1 */
		SUNXI_FUNCTION(0x5, "rgmii0"),        /* rgmii0 */
		SUNXI_FUNCTION(0x6, "dmic"),          /* dmic */
		SUNXI_FUNCTION(0x7, "uart5"),         /* uart5 */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 7, 3),   /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(J, 4),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x2, "pwm2_4"),        /* pwm2_4 */
		SUNXI_FUNCTION(0x3, "i2s2_din0"),     /* i2s2_din0 */
		SUNXI_FUNCTION(0x4, "i2s2_dout1"),    /* i2s2_dout1 */
		SUNXI_FUNCTION(0x5, "rgmii0"),        /* rgmii0 */
		SUNXI_FUNCTION(0x6, "dmic"),          /* dmic */
		SUNXI_FUNCTION(0x7, "uart7"),         /* uart7 */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 7, 4),   /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(J, 5),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x2, "pwm2_5"),        /* pwm2_5 */
		SUNXI_FUNCTION(0x3, "i2s2_din2"),     /* i2s2_din2 */
		SUNXI_FUNCTION(0x4, "i2s2_dout2"),    /* i2s2_dout2 */
		SUNXI_FUNCTION(0x5, "rgmii0"),        /* rgmii0 */
		SUNXI_FUNCTION(0x7, "uart7"),         /* uart7 */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 7, 5),   /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(J, 6),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x2, "pwm2_6"),        /* pwm2_6 */
		SUNXI_FUNCTION(0x3, "i2s2_din3"),     /* i2s2_din3 */
		SUNXI_FUNCTION(0x4, "i2s2_dout3"),    /* i2s2_dout3 */
		SUNXI_FUNCTION(0x5, "rgmii0"),        /* rgmii0 */
		SUNXI_FUNCTION(0x7, "uart7"),         /* uart7 */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 7, 6),   /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(J, 7),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x2, "pwm2_7"),        /* pwm2_7 */
		SUNXI_FUNCTION(0x3, "i2s1_mclk"),     /* i2s1_mclk */
		SUNXI_FUNCTION(0x5, "rgmii0"),        /* rgmii0 */
		SUNXI_FUNCTION(0x7, "uart7"),         /* uart7 */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 7, 7),   /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(J, 8),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x2, "pwm2_8"),        /* pwm2_8 */
		SUNXI_FUNCTION(0x3, "i2s1_bclk"),     /* i2s1_bclk */
		SUNXI_FUNCTION(0x4, "ledc"),          /* ledc */
		SUNXI_FUNCTION(0x5, "rgmii0"),        /* rgmii0 */
		SUNXI_FUNCTION(0x7, "uart2"),         /* uart2 */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 7, 8),   /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(J, 9),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x2, "pwm2_9"),        /* pwm2_9 */
		SUNXI_FUNCTION(0x3, "i2s1_lrck"),     /* i2s1_lrck */
		SUNXI_FUNCTION(0x4, "ir"),            /* ir */
		SUNXI_FUNCTION(0x5, "rgmii0"),        /* rgmii0 */
		SUNXI_FUNCTION(0x7, "uart2"),         /* uart2 */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 7, 9),   /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(J, 10),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x2, "pwm2_10"),       /* pwm2_10 */
		SUNXI_FUNCTION(0x3, "i2s1_dout0"),    /* i2s1_dout0 */
		SUNXI_FUNCTION(0x4, "i2s1_din1"),     /* i2s1_din1 */
		SUNXI_FUNCTION(0x5, "rgmii0"),        /* rgmii0 */
		SUNXI_FUNCTION(0x7, "uart2"),         /* uart2 */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 7, 10),  /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(J, 11),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x2, "pwm2_11"),       /* pwm2_11 */
		SUNXI_FUNCTION(0x3, "i2s1_din0"),     /* i2s1_din0 */
		SUNXI_FUNCTION(0x4, "i2s1_dout1"),    /* i2s1_dout1 */
		SUNXI_FUNCTION(0x5, "rgmii0"),        /* rgmii0 */
		SUNXI_FUNCTION(0x7, "uart2"),         /* uart2 */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 7, 11),  /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(J, 12),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x2, "pwm2_12"),       /* pwm2_12 */
		SUNXI_FUNCTION(0x4, "ir0"),           /* ir0 */
		SUNXI_FUNCTION(0x5, "rgmii0"),        /* rgmii0 */
		SUNXI_FUNCTION(0x6, "twi0"),          /* twi0 */
		SUNXI_FUNCTION(0x7, "uart9"),         /* uart9 */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 7, 12),  /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(J, 13),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x2, "pwm2_13"),       /* pwm2_13 */
		SUNXI_FUNCTION(0x4, "ir1"),           /* ir1 */
		SUNXI_FUNCTION(0x5, "rgmii0"),        /* rgmii0 */
		SUNXI_FUNCTION(0x6, "twi0"),          /* twi0 */
		SUNXI_FUNCTION(0x7, "uart9"),         /* uart9 */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 7, 13),  /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(J, 14),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x4, "ir2"),           /* ir2 */
		SUNXI_FUNCTION(0x5, "rgmii0"),        /* rgmii0 */
		SUNXI_FUNCTION(0x6, "twi5"),          /* twi5 */
		SUNXI_FUNCTION(0x7, "uart9"),         /* uart9 */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 7, 14),  /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(J, 15),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x4, "ir3"),           /* ir3 */
		SUNXI_FUNCTION(0x5, "rgmii0"),        /* rgmii0 */
		SUNXI_FUNCTION(0x6, "twi5"),          /* twi5 */
		SUNXI_FUNCTION(0x7, "uart9"),         /* uart9 */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 7, 15),  /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	/* bank K */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(K, 0),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x3, "mcsia"),         /* mcsia */
		SUNXI_FUNCTION(0x5, "uart7"),         /* uart7 */
		SUNXI_FUNCTION(0x6, "twi1"),          /* twi1 */
		SUNXI_FUNCTION(0x7, "ncsi"),          /* ncsi */
		SUNXI_FUNCTION(0x8, "lbus"),          /* lbus */
		SUNXI_FUNCTION(0x9, "i2s1_mclk"),     /* i2s1_mclk */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 8, 0),   /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(K, 1),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x3, "mcsia"),         /* mcsia */
		SUNXI_FUNCTION(0x5, "uart7"),         /* uart7 */
		SUNXI_FUNCTION(0x6, "twi1"),          /* twi1 */
		SUNXI_FUNCTION(0x7, "ncsi"),          /* ncsi */
		SUNXI_FUNCTION(0x8, "lbus"),          /* lbus */
		SUNXI_FUNCTION(0x9, "i2s1_bclk"),     /* i2s1_bclk */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 8, 1),   /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(K, 2),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x3, "mcsia"),         /* mcsia */
		SUNXI_FUNCTION(0x5, "uart7"),         /* uart7 */
		SUNXI_FUNCTION(0x6, "spi1"),          /* spi1 */
		SUNXI_FUNCTION(0x7, "ncsi"),          /* ncsi */
		SUNXI_FUNCTION(0x8, "lbus"),          /* lbus */
		SUNXI_FUNCTION(0x9, "i2s1_lrck"),     /* i2s1_lrck */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 8, 2),   /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(K, 3),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x3, "mcsia"),         /* mcsia */
		SUNXI_FUNCTION(0x5, "uart7"),         /* uart7 */
		SUNXI_FUNCTION(0x6, "spi1"),          /* spi1 */
		SUNXI_FUNCTION(0x7, "ncsi"),          /* ncsi */
		SUNXI_FUNCTION(0x8, "lbus"),          /* lbus */
		SUNXI_FUNCTION(0x9, "i2s1_dout0"),    /* i2s1_dout0 */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 8, 3),   /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(K, 4),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x3, "mcsia"),         /* mcsia */
		SUNXI_FUNCTION(0x5, "twi1"),          /* twi1 */
		SUNXI_FUNCTION(0x6, "spi1"),          /* spi1 */
		SUNXI_FUNCTION(0x7, "ncsi"),          /* ncsi */
		SUNXI_FUNCTION(0x8, "lbus"),          /* lbus */
		SUNXI_FUNCTION(0x9, "i2s1_din0"),     /* i2s1_din0 */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 8, 4),   /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(K, 5),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x3, "mcsia"),         /* mcsia */
		SUNXI_FUNCTION(0x5, "twi1"),          /* twi1 */
		SUNXI_FUNCTION(0x6, "spi1"),          /* spi1 */
		SUNXI_FUNCTION(0x7, "ncsi"),          /* ncsi */
		SUNXI_FUNCTION(0x8, "lbus"),          /* lbus */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 8, 5),   /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(K, 6),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x3, "mcsib"),         /* mcsib */
		SUNXI_FUNCTION(0x5, "mcsi0"),         /* mcsi0 */
		SUNXI_FUNCTION(0x6, "spi1"),          /* spi1 */
		SUNXI_FUNCTION(0x7, "ncsi"),          /* ncsi */
		SUNXI_FUNCTION(0x8, "lbus"),          /* lbus */
		SUNXI_FUNCTION(0x9, "uart3"),         /* uart3 */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 8, 6),   /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(K, 7),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x3, "mcsib"),         /* mcsib */
		SUNXI_FUNCTION(0x5, "twi2"),          /* twi2 */
		SUNXI_FUNCTION(0x6, "spi1"),          /* spi1 */
		SUNXI_FUNCTION(0x7, "ncsi"),          /* ncsi */
		SUNXI_FUNCTION(0x8, "lbus"),          /* lbus */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 8, 7),   /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(K, 8),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x3, "mcsib"),         /* mcsib */
		SUNXI_FUNCTION(0x5, "twi2"),          /* twi2 */
		SUNXI_FUNCTION(0x6, "spi1"),          /* spi1 */
		SUNXI_FUNCTION(0x7, "ncsi"),          /* ncsi */
		SUNXI_FUNCTION(0x8, "lbus"),          /* lbus */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 8, 8),   /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(K, 9),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x3, "mcsib"),         /* mcsib */
		SUNXI_FUNCTION(0x5, "mcsi1"),         /* mcsi1 */
		SUNXI_FUNCTION(0x6, "spi1"),          /* spi1 */
		SUNXI_FUNCTION(0x7, "ncsi"),          /* ncsi */
		SUNXI_FUNCTION(0x8, "lbus"),          /* lbus */
		SUNXI_FUNCTION(0x9, "uart3"),         /* uart3 */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 8, 9),   /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(K, 10),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x3, "mcsib"),         /* mcsib */
		SUNXI_FUNCTION(0x5, "twi3"),          /* twi3 */
		SUNXI_FUNCTION(0x6, "pwm0_6"),        /* pwm0_6 */
		SUNXI_FUNCTION(0x7, "ncsi"),          /* ncsi */
		SUNXI_FUNCTION(0x8, "lbus"),          /* lbus */
		SUNXI_FUNCTION(0x9, "uart3"),         /* uart3 */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 8, 10),  /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
	SUNXI_PIN(SUNXI_PINCTRL_PIN(K, 11),
		SUNXI_FUNCTION(0x0, "gpio_in"),       /* gpio_in */
		SUNXI_FUNCTION(0x1, "gpio_out"),      /* gpio_out */
		SUNXI_FUNCTION(0x3, "mcsib"),         /* mcsib */
		SUNXI_FUNCTION(0x5, "twi3"),          /* twi3 */
		SUNXI_FUNCTION(0x6, "pwm0_7"),        /* pwm0_7 */
		SUNXI_FUNCTION(0x7, "ncsi"),          /* ncsi */
		SUNXI_FUNCTION(0x8, "lbus"),          /* lbus */
		SUNXI_FUNCTION(0x9, "uart3"),         /* uart3 */
		SUNXI_FUNCTION_IRQ_BANK(0xe, 8, 11),  /* eint */
		SUNXI_FUNCTION(0xf, "io_disabled")),  /* io_disabled */
#endif
};

static const unsigned int sun8iw22_bank_base[] = {
	SUNXI_BANK_OFFSET('A', 'A'),
#if IS_ENABLED(CONFIG_AW_IC_BOARD)
	SUNXI_BANK_OFFSET('B', 'A'),
	SUNXI_BANK_OFFSET('C', 'A'),
#endif
	SUNXI_BANK_OFFSET('D', 'A'),
#if IS_ENABLED(CONFIG_AW_IC_BOARD)
	SUNXI_BANK_OFFSET('E', 'A'),
#endif
	SUNXI_BANK_OFFSET('F', 'A'),
#if IS_ENABLED(CONFIG_AW_IC_BOARD)
	SUNXI_BANK_OFFSET('G', 'A'),
	SUNXI_BANK_OFFSET('J', 'A'),
	SUNXI_BANK_OFFSET('K', 'A'),
#endif
};

static const unsigned int sun8iw22_irq_bank_map[] = {
	SUNXI_BANK_OFFSET('A', 'A'),
#if IS_ENABLED(CONFIG_AW_IC_BOARD)
	SUNXI_BANK_OFFSET('B', 'A'),
	SUNXI_BANK_OFFSET('C', 'A'),
#endif
	SUNXI_BANK_OFFSET('D', 'A'),
#if IS_ENABLED(CONFIG_AW_IC_BOARD)
	SUNXI_BANK_OFFSET('E', 'A'),
#endif
	SUNXI_BANK_OFFSET('F', 'A'),
#if IS_ENABLED(CONFIG_AW_IC_BOARD)
	SUNXI_BANK_OFFSET('G', 'A'),
	SUNXI_BANK_OFFSET('J', 'A'),
	SUNXI_BANK_OFFSET('K', 'A'),
#endif

};

static const struct sunxi_pinctrl_desc sun8iw22_pinctrl_data = {
	.pins = sun8iw22_pins,
	.npins = ARRAY_SIZE(sun8iw22_pins),
	.banks = ARRAY_SIZE(sun8iw22_bank_base),
	.bank_base = sun8iw22_bank_base,
	.irq_banks = ARRAY_SIZE(sun8iw22_irq_bank_map),
	.irq_bank_map = sun8iw22_irq_bank_map,
	.auto_power_source_switch = true,
	.pf_power_source_switch = true,
	.is_data_reg_atomic_access = true,
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

static int sun8iw22_pinctrl_suspend_noirq(struct device *dev)
{
	struct sunxi_pinctrl *pctl = dev_get_drvdata(dev);
	unsigned long flags;

	dev_info(dev, "pinctrl suspend\n");

	raw_spin_lock_irqsave(&pctl->lock, flags);
	memcpy_fromio(mem, pctl->membase, mem_size);
	raw_spin_unlock_irqrestore(&pctl->lock, flags);

	return 0;
}

static int sun8iw22_pinctrl_resume_noirq(struct device *dev)
{
	struct sunxi_pinctrl_desc const *desc = &sun8iw22_pinctrl_data;
	struct sunxi_pinctrl *pctl = dev_get_drvdata(dev);
	unsigned long flags;
	int idx, bank;
	int initial_bank_offset = sunxi_pinctrl_hw_info[desc->hw_type].initial_bank_offset;
	int bank_mem_size = sunxi_pinctrl_hw_info[desc->hw_type].bank_mem_size;
	int irq_cfg_reg = sunxi_pinctrl_hw_info[desc->hw_type].irq_cfg_reg;
	int irq_mem_size = sunxi_pinctrl_hw_info[desc->hw_type].irq_mem_size;
	int irq_debounce_reg = sunxi_pinctrl_hw_info[desc->hw_type].irq_debounce_reg;
	int data_clr_regs_offset = sunxi_pinctrl_hw_info[desc->hw_type].data_clr_regs_offset;

	raw_spin_lock_irqsave(&pctl->lock, flags);

	/* recover PX_REG_BASE and GPIO_POW_MOD/CTL/VAL */
	memcpy_toio(pctl->membase, mem, 0x80);

	/* clear data_clr_reg before recover data_reg to ensure data_reg correctness */
	for (idx = 0; idx < desc->banks; idx++) {
		bank = desc->bank_base[idx];
		*(unsigned int *)(mem + initial_bank_offset + bank * bank_mem_size + data_clr_regs_offset) = 0x0;
	}

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

static const struct dev_pm_ops sun8iw22_pinctrl_pm_ops = {
	.suspend_noirq = sun8iw22_pinctrl_suspend_noirq,
	.resume_noirq = sun8iw22_pinctrl_resume_noirq,
};
#define PINCTRL_PM_OPS	(&sun8iw22_pinctrl_pm_ops)

#else
static int pinctrl_pm_alloc_mem(struct platform_device *pdev)
{
	return 0;
}
#define PINCTRL_PM_OPS	NULL
#endif

static int sun8iw22_pinctrl_probe(struct platform_device *pdev)
{
	int ret;
	ret = pinctrl_pm_alloc_mem(pdev);
	if (ret) {
		dev_err(&pdev->dev, "alloc pm mem err\n");
		return ret;
	}

	return sunxi_bsp_pinctrl_init(pdev, &sun8iw22_pinctrl_data);
}

static struct of_device_id sun8iw22_pinctrl_match[] = {
	{ .compatible = "allwinner,sun8iw22-pinctrl", },
	{}
};

MODULE_DEVICE_TABLE(of, sun8iw22_pinctrl_match);

static struct platform_driver sun8iw22_pinctrl_driver = {
	.probe	= sun8iw22_pinctrl_probe,
	.driver	= {
		.name		= "sun8iw22-pinctrl",
		.pm		= PINCTRL_PM_OPS,
		.of_match_table	= sun8iw22_pinctrl_match,
	},
};

static int __init sun8iw22_pio_init(void)
{
	return platform_driver_register(&sun8iw22_pinctrl_driver);
}
fs_initcall(sun8iw22_pio_init);

MODULE_DESCRIPTION("Allwinner sun8iw22 pio pinctrl driver");
MODULE_AUTHOR("<haili@allwinnertech>");
MODULE_LICENSE("GPL");
MODULE_VERSION(SUNXI_PINCTRL_VERSION);
