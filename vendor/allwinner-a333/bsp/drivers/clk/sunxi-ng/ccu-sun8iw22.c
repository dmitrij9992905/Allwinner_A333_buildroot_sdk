// SPDX-License-Identifier: GPL-2.0
/* Copyright(c) 2020 - 2023 Allwinner Technology Co.,Ltd. All rights reserved. */
/*
 * Copyright (c) 2024 haili@allwinnertech.com
 */

#include <linux/clk-provider.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/of_address.h>
#include <linux/platform_device.h>

#include "ccu_common.h"
#include "ccu_reset.h"

#include "ccu_div.h"
#include "ccu_gate.h"
#include "ccu_mp.h"
#include "ccu_mult.h"
#include "ccu_nk.h"
#include "ccu_nkm.h"
#include "ccu_nkmp.h"
#include "ccu_nm.h"
#include "ccu_mux.h"
#include "ccu_sdm.h"

#include "ccu-sun8iw22.h"

#define SUNXI_CCU_VERSION	"0.0.7"
#define UPD_KEY_VALUE           0x8000000

/*
 * The CPU PLL is actually NP clock, with P being /1, /2 or /4. However
 * P should only be used for output frequencies lower than 288 MHz.
 *
 * For now we can just model it as a multiplier clock, and force P to /1.
 *
 * The M factor is present in the register's description, but not in the
 * frequency formula, and it's documented as "M is only used for backdoor
 * testing", so it's not modelled and then force to 0.
 */

/* ccu_des_start */

#define SUN8IW22_PLL_PERI0_CTRL_REG   0x00A0
static struct ccu_nm pll_peri0_clk = {
	.output			= BIT(27),
	.lock			= BIT(28),
	.lock_enable		= BIT(29),
	.enable			= BIT(31),
	.n			= _SUNXI_CCU_MULT_MIN_MAX(8, 8, 52, 105),
	.min_rate		= 1248000000,
	.max_rate		= 2520000000UL,
	.sdm			= _SUNXI_CCU_SDM_INFO(BIT(24), 0x00A8),
	.common			= {
		.reg		= 0x00A0,
		.hw.init	= CLK_HW_INIT("pll-peri0", "dcxo24M",
					&ccu_nm_ops,
					CLK_SET_RATE_UNGATE |
					CLK_IS_CRITICAL),
	},
};

static SUNXI_CCU_M(pll_peri0_2x_clk, "pll-peri0-2x",
			"pll-peri0", 0x00a0,
			16, 3, 0);

static SUNXI_CCU_M(pll_peri0_800m_clk, "pll-peri0-800m",
			"pll-peri0", 0x00a0,
			20, 3, 0);

static SUNXI_CCU_M(pll_peri0_480m_clk, "pll-peri0-480m",
			"pll-peri0", 0x00a0,
			2, 3, 0);

static CLK_FIXED_FACTOR(pll_peri0_600m_clk, "pll-peri0-600m",
			"pll-peri0-2x", 2, 1, 0);

static CLK_FIXED_FACTOR(pll_peri0_400m_clk, "pll-peri0-400m",
			"pll-peri0-2x", 3, 1, 0);

static CLK_FIXED_FACTOR(pll_peri0_300m_clk, "pll-peri0-300m",
			"pll-peri0-600m", 2, 1, 0);

static CLK_FIXED_FACTOR(pll_peri0_200m_clk, "pll-peri0-200m",
			"pll-peri0-400m", 2, 1, 0);

static CLK_FIXED_FACTOR(pll_peri0_160m_clk, "pll-peri0-160m",
			"pll-peri0-480m", 3, 1, 0);

static CLK_FIXED_FACTOR(pll_peri0_150m_clk, "pll-peri0-150m",
			"pll-peri0-300m", 2, 1, 0);

#define SUN8IW22_PLL_PERI1_CTRL_REG   0x00C0
static struct ccu_nm pll_peri1_clk = {
	.output			= BIT(27),
	.lock			= BIT(28),
	.lock_enable		= BIT(29),
	.enable			= BIT(31),
	.n			= _SUNXI_CCU_MULT_MIN_MAX(8, 8, 52, 105),
	.min_rate		= 1248000000,
	.max_rate		= 2520000000UL,
	.sdm			= _SUNXI_CCU_SDM_INFO(BIT(24), 0x00C8),
	.common			= {
		.reg		= 0x00C0,
		.hw.init	= CLK_HW_INIT("pll-peri1", "dcxo24M",
					&ccu_nm_ops,
					CLK_SET_RATE_UNGATE |
					CLK_IS_CRITICAL),
	},
};

static SUNXI_CCU_M(pll_peri1_2x_clk, "pll-peri1-2x",
			"pll-peri1", 0x00c0,
			16, 3, 0);

static SUNXI_CCU_M(pll_peri1_800m_clk, "pll-peri1-800m",
			"pll-peri1", 0x00c0,
			20, 3, 0);

static SUNXI_CCU_M(pll_peri1_480m_clk, "pll-peri1-480m",
			"pll-peri1", 0x00c0,
			2, 3, 0);

static CLK_FIXED_FACTOR(pll_peri1_600m_clk, "pll-peri1-600m",
			"pll-peri1-2x", 2, 1, 0);

static CLK_FIXED_FACTOR(pll_peri1_400m_clk, "pll-peri1-400m",
			"pll-peri1-2x", 3, 1, 0);

static CLK_FIXED_FACTOR(pll_peri1_300m_clk, "pll-peri1-300m",
			"pll-peri1-600m", 2, 1, 0);

static CLK_FIXED_FACTOR(pll_peri1_200m_clk, "pll-peri1-200m",
			"pll-peri1-400m", 2, 1, 0);

static CLK_FIXED_FACTOR(pll_peri1_160m_clk, "pll-peri1-160m",
			"pll-peri1-480m", 3, 1, 0);

static CLK_FIXED_FACTOR(pll_peri1_150m_clk, "pll-peri1-150m",
			"pll-peri1-300m", 2, 1, 0);

#define SUN8IW22_PLL_VIDEO0_CTRL_REG   0x0120
static struct ccu_nm pll_video0_clk = {
	.output			= BIT(27),
	.lock			= BIT(28),
	.lock_enable		= BIT(29),
	.enable			= BIT(31),
	.n			= _SUNXI_CCU_MULT_MIN_MAX(8, 8, 52, 105),
	.min_rate		= 1248000000,
	.max_rate		= 2520000000UL,
	.sdm			= _SUNXI_CCU_SDM_INFO(BIT(24), 0x0128),
	.common			= {
		.reg		= 0x0120,
		.hw.init	= CLK_HW_INIT("pll-video0", "dcxo24M",
					&ccu_nm_ops,
					CLK_SET_RATE_UNGATE |
					CLK_IS_CRITICAL),
	},
};

static SUNXI_CCU_M(pll_video0_4x_clk, "pll-video0-4x",
			"pll-video0", 0x00120,
			20, 3, CLK_SET_RATE_PARENT);      /* p0 */

static SUNXI_CCU_M(pll_video0_3x_clk, "pll-video0-3x",
			"pll-video0", 0x00120,
			16, 3, CLK_SET_RATE_PARENT);      /* p0 */

#define SUN8IW22_PLL_AUDIO0_CTRL_REG   0x0260
static struct ccu_sdm_setting pll_audio0_sdm_table[] = {
	{ .rate = 90316800, .pattern = 0xA00179A7, .m = 20, .n = 75 },	/* pll_audio0 22.5792*4 = 90.3168M */
};
static struct ccu_nm pll_audio0_clk = {
	.output			= BIT(27),
	.lock			= BIT(28),
	.lock_enable		= BIT(29),
	.enable			= BIT(31),
	.n			= _SUNXI_CCU_MULT_MIN_MAX(8, 8, 65, 130),
	.m			= _SUNXI_CCU_DIV(16, 6),
	.sdm			= _SUNXI_CCU_SDM(pll_audio0_sdm_table, BIT(24),
						0x0268, BIT(31)),
	.min_rate		= 90316800,
	.common			= {
		.reg		= 0x0260,
		.features	= CCU_FEATURE_SIGMA_DELTA_MOD,
		.hw.init	= CLK_HW_INIT("pll-audio0", "dcxo24M",
					&ccu_nm_ops,
					CLK_SET_RATE_UNGATE |
					CLK_IS_CRITICAL),
	},
};

static const char * const ahb_parents[] = { "dcxo24M", "ext-32k", "rc_16m", "pll-peri0-600m" };

static SUNXI_CCU_M_WITH_MUX(ahb_clk, "ahb", ahb_parents,
			0x0500, 0, 5, 24, 2, 0);

static const char * const apb0_parents[] = { "dcxo24M", "ext-32k", "rc_16m", "pll-peri0-600m" };

static SUNXI_CCU_M_WITH_MUX(apb0_clk, "apb0", apb0_parents,
			0x0510, 0, 5, 24, 2, 0);

static const char * const apb1_parents[] = { "dcxo24M", "ext-32k", "rc_16m", "pll-peri0-600m" };

static SUNXI_CCU_M_WITH_MUX(apb1_clk, "apb1", apb1_parents,
			0x0518, 0, 5, 24, 2, 0);

static const char * const apb_uart_parents[] = { "dcxo24M", "ext-32k", "rc_16m", "pll-peri0-600m", "pll-peri0-480m" };

static SUNXI_CCU_M_WITH_MUX(apb_uart_clk, "apb-uart", apb_uart_parents,
			0x0538, 0, 5, 24, 3, 0);

static const char * const mbus_parents[] = { "dcxo24M", "pll-peri0-480m", "pll-peri0-400m", "pll-peri0-300m", "hdr-clk" };

static SUNXI_CCU_M_WITH_MUX_GATE_KEY(mbus_clk, "mbus",
			mbus_parents, 0x0588,
			0, 5,  /* M */
			24, 3,  /* mux */
			UPD_KEY_VALUE,
			BIT(31),	/* gate */
			CLK_IGNORE_UNUSED | CLK_SET_RATE_NO_REPARENT);

static SUNXI_CCU_GATE(pll_stby_peri0_cfg_bus_clk, "pll-stby-peri0-cfg-bus",
			"dcxo24M",
			0x05C0, BIT(28), 0);

static SUNXI_CCU_GATE(spif_ahb_sw_cfg_bus_clk, "spif-ahb-sw-cfg-bus",
			"dcxo24M",
			0x05C0, BIT(22), 0);

static SUNXI_CCU_GATE(usb2p0_sys_ahb_sw_cfg_bus_clk, "usb2p0-sys-ahb-sw-cfg-bus",
			"dcxo24M",
			0x05C0, BIT(16), 0);

static SUNXI_CCU_GATE(gmac2_ahb_sw_cfg_bus_clk, "gmac2-ahb-sw-cfg-bus",
			"dcxo24M",
			0x05C0, BIT(15), 0);

static SUNXI_CCU_GATE(gmac1_ahb_sw_cfg_bus_clk, "gmac1-ahb-sw-cfg-bus",
			"dcxo24M",
			0x05C0, BIT(14), 0);

static SUNXI_CCU_GATE(gmac0_ahb_sw_cfg_bus_clk, "gmac0-ahb-sw-cfg-bus",
			"dcxo24M",
			0x05C0, BIT(13), 0);

static SUNXI_CCU_GATE(mcu_sys_ahb_sw_cfg_bus_clk, "mcu-sys-ahb-sw-cfg-bus",
			"dcxo24M",
			0x05C0, BIT(12), 0);

static SUNXI_CCU_GATE(smhc3_ahb_sw_cfg_bus_clk, "smhc3-ahb-sw-cfg-bus",
			"dcxo24M",
			0x05C0, BIT(8), 0);

static SUNXI_CCU_GATE(smhc2_ahb_sw_cfg_bus_clk, "smhc2-ahb-sw-cfg-bus",
			"dcxo24M",
			0x05C0, BIT(7), 0);

static SUNXI_CCU_GATE(smhc1_ahb_sw_cfg_bus_clk, "smhc1-ahb-sw-cfg-bus",
			"dcxo24M",
			0x05C0, BIT(6), 0);

static SUNXI_CCU_GATE(smhc0_ahb_sw_cfg_bus_clk, "smhc0-ahb-sw-cfg-bus",
			"dcxo24M",
			0x05C0, BIT(5), 0);

static SUNXI_CCU_GATE(video_out0_ahb_sw_cfg_bus_clk, "video-out0-ahb-sw-cfg-bus",
			"dcxo24M",
			0x05C0, BIT(3), 0);

static SUNXI_CCU_GATE(video_in_ahb_sw_cfg_bus_clk, "video-in-ahb-sw-cfg-bus",
			"dcxo24M",
			0x05C0, BIT(2), 0);

static SUNXI_CCU_GATE(pll_peri_apb0_sw_cfg_bus_clk, "pll-peri-apb0-sw-cfg-bus",
			"dcxo24M",
			0x05D0, BIT(16), 0);

static SUNXI_CCU_GATE(pll_peri_ahb_sw_cfg_bus_clk, "pll-peri-ahb-sw-cfg-bus",
			"dcxo24M",
			0x05D0, BIT(0), 0);

static SUNXI_CCU_GATE(gmac2_mbus_bus_clk, "gmac2-mbus-bus",
			"dcxo24M",
			0x05E0, BIT(13), 0);

static SUNXI_CCU_GATE(gmac1_mbus_bus_clk, "gmac1-mbus-bus",
			"dcxo24M",
			0x05E0, BIT(12), 0);

static SUNXI_CCU_GATE(gmac0_mbus_bus_clk, "gmac0-mbus-bus",
			"dcxo24M",
			0x05E0, BIT(11), 0);

static SUNXI_CCU_GATE(isp_mbus_bus_clk, "isp-mbus-bus",
			"dcxo24M",
			0x05E0, BIT(9), 0);

static SUNXI_CCU_GATE(csi_mbus_bus_clk, "csi-mbus-bus",
			"dcxo24M",
			0x05E0, BIT(8), 0);

static SUNXI_CCU_GATE(dma1_mbus_bus_clk, "dma1-mbus-bus",
			"dcxo24M",
			0x05E0, BIT(3), 0);

static SUNXI_CCU_GATE(ce_sys_mbus_bus_clk, "ce-sys-mbus-bus",
			"dcxo24M",
			0x05E0, BIT(2), 0);

static SUNXI_CCU_GATE(dma0_mbus_bus_clk, "dma0-mbus-bus",
			"dcxo24M",
			0x05E0, BIT(0), 0);

static SUNXI_CCU_GATE(dma1_mbus_sw_cfg_bus_clk, "dma1-mbus-sw-cfg-bus",
			"dcxo24M",
			0x05E4, BIT(29), 0);

static SUNXI_CCU_GATE(dma0_mbus_sw_cfg_bus_clk, "dma0-mbus-sw-cfg-bus",
			"dcxo24M",
			0x05E4, BIT(28), 0);

static SUNXI_CCU_GATE(lbc_mbus_sw_cfg_bus_clk, "lbc-mbus-sw-cfg-bus",
			"dcxo24M",
			0x05E4, BIT(27), 0);

static SUNXI_CCU_GATE(video_out0_mbus_sw_cfg_bus_clk, "video-out0-mbus-sw-cfg-bus",
			"dcxo24M",
			0x05E4, BIT(23), 0);

static SUNXI_CCU_GATE(mcu_sys_mbus_sw_cfg_bus_clk, "mcu-sys-mbus-sw-cfg-bus",
			"dcxo24M",
			0x05E4, BIT(21), 0);

static SUNXI_CCU_GATE(video_in_mbus_sw_cfg_bus_clk, "video-in-mbus-sw-cfg-bus",
			"dcxo24M",
			0x05E4, BIT(20), 0);

static SUNXI_CCU_GATE(ce_sys_mbus_sw_cfg_bus_clk, "ce-sys-mbus-sw-cfg-bus",
			"dcxo24M",
			0x05E4, BIT(18), 0);

static SUNXI_CCU_GATE(gmac2_mbus_sw_cfg_bus_clk, "gmac2-mbus-sw-cfg-bus",
			"dcxo24M",
			0x05E4, BIT(14), 0);

static SUNXI_CCU_GATE(gmac1_mbus_sw_cfg_bus_clk, "gmac1-mbus-sw-cfg-bus",
			"dcxo24M",
			0x05E4, BIT(13), 0);

static SUNXI_CCU_GATE(gmac0_mbus_sw_cfg_bus_clk, "gmac0-mbus-sw-cfg-bus",
			"dcxo24M",
			0x05E4, BIT(12), 0);

static SUNXI_CCU_GATE(gmac_mbus_sw_cfg_bus_clk, "gmac-mbus-sw-cfg-bus",
			"dcxo24M",
			0x05E4, BIT(11), 0);

static SUNXI_CCU_GATE(spif_ahb_auto_en_bus_clk, "spif-ahb-auto-en-bus",
			"dcxo24M",
			0x05F0, BIT(22), 0);

static SUNXI_CCU_GATE(usb2p0_sys_ahb_auto_en_bus_clk, "usb2p0-sys-ahb-auto-en-bus",
			"dcxo24M",
			0x05F0, BIT(16), 0);

static SUNXI_CCU_GATE(gmac2_ahb_auto_en_bus_clk, "gmac2-ahb-auto-en-bus",
			"dcxo24M",
			0x05F0, BIT(15), 0);

static SUNXI_CCU_GATE(gmac1_ahb_auto_en_bus_clk, "gmac1-ahb-auto-en-bus",
			"dcxo24M",
			0x05F0, BIT(14), 0);

static SUNXI_CCU_GATE(gmac0_ahb_auto_en_bus_clk, "gmac0-ahb-auto-en-bus",
			"dcxo24M",
			0x05F0, BIT(13), 0);

static SUNXI_CCU_GATE(mcu_sys_ahb_auto_en_bus_clk, "mcu-sys-ahb-auto-en-bus",
			"dcxo24M",
			0x05F0, BIT(12), 0);

static SUNXI_CCU_GATE(smhc3_ahb_auto_en_bus_clk, "smhc3-ahb-auto-en-bus",
			"dcxo24M",
			0x05F0, BIT(8), 0);

static SUNXI_CCU_GATE(smhc2_ahb_auto_en_bus_clk, "smhc2-ahb-auto-en-bus",
			"dcxo24M",
			0x05F0, BIT(7), 0);

static SUNXI_CCU_GATE(smhc1_ahb_auto_en_bus_clk, "smhc1-ahb-auto-en-bus",
			"dcxo24M",
			0x05F0, BIT(6), 0);

static SUNXI_CCU_GATE(smhc0_ahb_auto_en_bus_clk, "smhc0-ahb-auto-en-bus",
			"dcxo24M",
			0x05F0, BIT(5), 0);

static SUNXI_CCU_GATE(video_out0_ahb_auto_en_bus_clk, "video-out0-ahb-auto-en-bus",
			"dcxo24M",
			0x05F0, BIT(3), 0);

static SUNXI_CCU_GATE(video_in_ahb_auto_en_bus_clk, "video-in-ahb-auto-en-bus",
			"dcxo24M",
			0x05F0, BIT(2), 0);

static SUNXI_CCU_GATE(dma1_mbus_auto_en_bus_clk, "dma1-mbus-auto-en-bus",
			"dcxo24M",
			0x05F4, BIT(29), 0);

static SUNXI_CCU_GATE(dma0_mbus_auto_en_bus_clk, "dma0-mbus-auto-en-bus",
			"dcxo24M",
			0x05F4, BIT(28), 0);

static SUNXI_CCU_GATE(lbc_mbus_auto_en_bus_clk, "lbc-mbus-auto-en-bus",
			"dcxo24M",
			0x05F4, BIT(27), 0);

static SUNXI_CCU_GATE(video_out0_mbus_auto_en_bus_clk, "video-out0-mbus-auto-en-bus",
			"dcxo24M",
			0x05F4, BIT(23), 0);

static SUNXI_CCU_GATE(mcu_sys_mbus_auto_en_bus_clk, "mcu-sys-mbus-auto-en-bus",
			"dcxo24M",
			0x05F4, BIT(21), 0);

static SUNXI_CCU_GATE(video_in_mbus_auto_en_bus_clk, "video-in-mbus-auto-en-bus",
			"dcxo24M",
			0x05F4, BIT(20), 0);

static SUNXI_CCU_GATE(ce_sys_mbus_auto_en_bus_clk, "ce-sys-mbus-auto-en-bus",
			"dcxo24M",
			0x05F4, BIT(18), 0);

static SUNXI_CCU_GATE(gmac2_mbus_auto_en_bus_clk, "gmac2-mbus-auto-en-bus",
			"dcxo24M",
			0x05F4, BIT(14), 0);

static SUNXI_CCU_GATE(gmac1_mbus_auto_en_bus_clk, "gmac1-mbus-auto-en-bus",
			"dcxo24M",
			0x05F4, BIT(13), 0);

static SUNXI_CCU_GATE(gmac0_mbus_auto_en_bus_clk, "gmac0-mbus-auto-en-bus",
			"dcxo24M",
			0x05F4, BIT(12), 0);

static SUNXI_CCU_GATE(gmac_mbus_auto_en_bus_clk, "gmac-mbus-auto-en-bus",
			"dcxo24M",
			0x05F4, BIT(11), 0);

static SUNXI_CCU_GATE(dma0_ahb_bus_clk, "dma0-ahb-bus",
			"dcxo24M",
			0x0704, BIT(0), 0);

static SUNXI_CCU_GATE(dma1_ahb_bus_clk, "dma1-ahb-bus",
			"dcxo24M",
			0x070C, BIT(0), 0);

static SUNXI_CCU_GATE(spinlock_ahb_bus_clk, "spinlock-ahb-bus",
			"dcxo24M",
			0x0724, BIT(0), 0);

static SUNXI_CCU_GATE(msgbox_cpux_ahb_bus_clk, "msgbox-cpux-ahb-bus",
			"dcxo24M",
			0x0744, BIT(0), 0);

static SUNXI_CCU_GATE(msgbox_core0_ahb_bus_clk, "msgbox-core0-ahb-bus",
			"dcxo24M",
			0x074C, BIT(0), 0);

static SUNXI_CCU_GATE(msgbox_core1_ahb_bus_clk, "msgbox-core1-ahb-bus",
			"dcxo24M",
			0x0754, BIT(0), 0);

static SUNXI_CCU_GATE(msgbox_core2_ahb_bus_clk, "msgbox-core2-ahb-bus",
			"dcxo24M",
			0x075C, BIT(0), 0);

static SUNXI_CCU_GATE(msgbox_core3_ahb_bus_clk, "msgbox-core3-ahb-bus",
			"dcxo24M",
			0x0764, BIT(0), 0);

static SUNXI_CCU_GATE(msgbox_rv_ahb_bus_clk, "msgbox-rv-ahb-bus",
			"dcxo24M",
			0x076C, BIT(0), 0);

static SUNXI_CCU_GATE(pwm2_apb_bus_clk, "pwm2-apb-bus",
			"dcxo24M",
			0x0794, BIT(0), 0);

static SUNXI_CCU_GATE(dcu_bus_clk, "dcu-bus",
			"dcxo24M",
			0x07A4, BIT(0), 0);

static SUNXI_CCU_GATE(dap_ahb_bus_clk, "dap-ahb-bus",
			"dcxo24M",
			0x07AC, BIT(0), 0);

static const char * const pwmcs0_parents[] = { "dcxo24M", "pll-peri0-400m" };

static SUNXI_CCU_M_WITH_MUX_GATE(pwmcs0_clk, "pwmcs0",
			pwmcs0_parents, 0x07C0,
			0, 5,  /* M */
			24, 3,  /* mux */
			BIT(31),	/* gate */
			0);

static SUNXI_CCU_GATE(pwmcs0_apb_bus_clk, "pwmcs0-apb-bus",
			"dcxo24M",
			0x07C4, BIT(0), 0);

static const char * const pwmcs1_parents[] = { "dcxo24M", "pll-peri0-400m" };

static SUNXI_CCU_M_WITH_MUX_GATE(pwmcs1_clk, "pwmcs1",
			pwmcs1_parents, 0x07C8,
			0, 5,  /* M */
			24, 3,  /* mux */
			BIT(31),	/* gate */
			0);

static SUNXI_CCU_GATE(pwmcs1_apb_bus_clk, "pwmcs1-apb-bus",
			"dcxo24M",
			0x07CC, BIT(0), 0);

static const char * const timer0_0_clk_parents[] = { "dcxo24M", "rc_16m", "ext-32k", "pll-peri0-200m" };

static struct ccu_div timer0_0_clk_clk = {
	.enable			= BIT(31),
	.div			= _SUNXI_CCU_DIV_FLAGS(0, 3, CLK_DIVIDER_POWER_OF_TWO),
	.mux			= _SUNXI_CCU_MUX(24, 3),  /* mux */
	.common			= {
		.reg		= 0x0800,
		.hw.init	= CLK_HW_INIT_PARENTS("timer0-0-clk",
							timer0_0_clk_parents,
							&ccu_div_ops, 0),
	},
};

static const char * const timer0_1_clk_parents[] = { "dcxo24M", "rc_16m", "ext-32k", "pll-peri0-200m" };

static struct ccu_div timer0_1_clk_clk = {
	.enable			= BIT(31),
	.div			= _SUNXI_CCU_DIV_FLAGS(0, 3, CLK_DIVIDER_POWER_OF_TWO),
	.mux			= _SUNXI_CCU_MUX(24, 3),  /* mux */
	.common			= {
		.reg		= 0x0804,
		.hw.init	= CLK_HW_INIT_PARENTS("timer0-1-clk",
							timer0_1_clk_parents,
							&ccu_div_ops, 0),
	},
};

static const char * const timer0_2_clk_parents[] = { "dcxo24M", "rc_16m", "ext-32k", "pll-peri0-200m" };

static struct ccu_div timer0_2_clk_clk = {
	.enable			= BIT(31),
	.div			= _SUNXI_CCU_DIV_FLAGS(0, 3, CLK_DIVIDER_POWER_OF_TWO),
	.mux			= _SUNXI_CCU_MUX(24, 3),  /* mux */
	.common			= {
		.reg		= 0x0808,
		.hw.init	= CLK_HW_INIT_PARENTS("timer0-2-clk",
							timer0_2_clk_parents,
							&ccu_div_ops, 0),
	},
};

static const char * const timer0_3_clk_parents[] = { "dcxo24M", "rc_16m", "ext-32k", "pll-peri0-200m" };

static struct ccu_div timer0_3_clk_clk = {
	.enable			= BIT(31),
	.div			= _SUNXI_CCU_DIV_FLAGS(0, 3, CLK_DIVIDER_POWER_OF_TWO),
	.mux			= _SUNXI_CCU_MUX(24, 3),  /* mux */
	.common			= {
		.reg		= 0x080C,
		.hw.init	= CLK_HW_INIT_PARENTS("timer0-3-clk",
							timer0_3_clk_parents,
							&ccu_div_ops, 0),
	},
};

static const char * const timer0_4_clk_parents[] = { "dcxo24M", "rc_16m", "ext-32k", "pll-peri0-200m" };

static struct ccu_div timer0_4_clk_clk = {
	.enable			= BIT(31),
	.div			= _SUNXI_CCU_DIV_FLAGS(0, 3, CLK_DIVIDER_POWER_OF_TWO),
	.mux			= _SUNXI_CCU_MUX(24, 3),  /* mux */
	.common			= {
		.reg		= 0x0810,
		.hw.init	= CLK_HW_INIT_PARENTS("timer0-4-clk",
							timer0_4_clk_parents,
							&ccu_div_ops, 0),
	},
};

static const char * const timer0_5_clk_parents[] = { "dcxo24M", "rc_16m", "ext-32k", "pll-peri0-200m" };

static struct ccu_div timer0_5_clk_clk = {
	.enable			= BIT(31),
	.div			= _SUNXI_CCU_DIV_FLAGS(0, 3, CLK_DIVIDER_POWER_OF_TWO),
	.mux			= _SUNXI_CCU_MUX(24, 3),  /* mux */
	.common			= {
		.reg		= 0x0814,
		.hw.init	= CLK_HW_INIT_PARENTS("timer0-5-clk",
							timer0_5_clk_parents,
							&ccu_div_ops, 0),
	},
};

static const char * const timer0_6_clk_parents[] = { "dcxo24M", "rc_16m", "ext-32k", "pll-peri0-200m" };

static struct ccu_div timer0_6_clk_clk = {
	.enable			= BIT(31),
	.div			= _SUNXI_CCU_DIV_FLAGS(0, 3, CLK_DIVIDER_POWER_OF_TWO),
	.mux			= _SUNXI_CCU_MUX(24, 3),  /* mux */
	.common			= {
		.reg		= 0x0818,
		.hw.init	= CLK_HW_INIT_PARENTS("timer0-6-clk",
							timer0_6_clk_parents,
							&ccu_div_ops, 0),
	},
};

static const char * const timer0_7_clk_parents[] = { "dcxo24M", "rc_16m", "ext-32k", "pll-peri0-200m" };

static struct ccu_div timer0_7_clk_clk = {
	.enable			= BIT(31),
	.div			= _SUNXI_CCU_DIV_FLAGS(0, 3, CLK_DIVIDER_POWER_OF_TWO),
	.mux			= _SUNXI_CCU_MUX(24, 3),  /* mux */
	.common			= {
		.reg		= 0x081C,
		.hw.init	= CLK_HW_INIT_PARENTS("timer0-7-clk",
							timer0_7_clk_parents,
							&ccu_div_ops, 0),
	},
};

static SUNXI_CCU_GATE(timer0_ahb_bus_clk, "timer0-ahb-bus",
			"dcxo24M",
			0x0850, BIT(0), 0);

static const char * const timer0_0_rv_clk_parents[] = { "dcxo24M", "rc_16m", "ext-32k", "pll-peri0-200m" };

static struct ccu_div timer0_0_rv_clk_clk = {
	.enable			= BIT(31),
	.div			= _SUNXI_CCU_DIV_FLAGS(0, 3, CLK_DIVIDER_POWER_OF_TWO),
	.mux			= _SUNXI_CCU_MUX(24, 3),  /* mux */
	.common			= {
		.reg		= 0x0860,
		.hw.init	= CLK_HW_INIT_PARENTS("timer0-0-rv-clk",
							timer0_0_rv_clk_parents,
							&ccu_div_ops, 0),
	},
};

static const char * const timer0_1_rv_clk_parents[] = { "dcxo24M", "rc_16m", "ext-32k", "pll-peri0-200m" };

static struct ccu_div timer0_1_rv_clk_clk = {
	.enable			= BIT(31),
	.div			= _SUNXI_CCU_DIV_FLAGS(0, 3, CLK_DIVIDER_POWER_OF_TWO),
	.mux			= _SUNXI_CCU_MUX(24, 3),  /* mux */
	.common			= {
		.reg		= 0x0864,
		.hw.init	= CLK_HW_INIT_PARENTS("timer0-1-rv-clk",
							timer0_1_rv_clk_parents,
							&ccu_div_ops, 0),
	},
};

static const char * const timer0_2_rv_clk_parents[] = { "dcxo24M", "rc_16m", "ext-32k", "pll-peri0-200m" };

static struct ccu_div timer0_2_rv_clk_clk = {
	.enable			= BIT(31),
	.div			= _SUNXI_CCU_DIV_FLAGS(0, 3, CLK_DIVIDER_POWER_OF_TWO),
	.mux			= _SUNXI_CCU_MUX(24, 3),  /* mux */
	.common			= {
		.reg		= 0x0868,
		.hw.init	= CLK_HW_INIT_PARENTS("timer0-2-rv-clk",
							timer0_2_rv_clk_parents,
							&ccu_div_ops, 0),
	},
};

static const char * const timer0_3_rv_clk_parents[] = { "dcxo24M", "rc_16m", "ext-32k", "pll-peri0-200m" };

static struct ccu_div timer0_3_rv_clk_clk = {
	.enable			= BIT(31),
	.div			= _SUNXI_CCU_DIV_FLAGS(0, 3, CLK_DIVIDER_POWER_OF_TWO),
	.mux			= _SUNXI_CCU_MUX(24, 3),  /* mux */
	.common			= {
		.reg		= 0x086C,
		.hw.init	= CLK_HW_INIT_PARENTS("timer0-3-rv-clk",
							timer0_3_rv_clk_parents,
							&ccu_div_ops, 0),
	},
};

static SUNXI_CCU_GATE(timer0_rv_ahb_bus_clk, "timer0-rv-ahb-bus",
			"dcxo24M",
			0x0870, BIT(0), 0);

static const char * const de_parents[] = { "pll-peri0-600m", "pll-peri0-480m", "pll-peri0-400m", "pll-video0-4x", "pll-video0-3x" };

static SUNXI_CCU_M_WITH_MUX_GATE(de_clk, "de",
			de_parents, 0x0A00,
			0, 5,  /* M */
			24, 3,  /* mux */
			BIT(31),	/* gate */
			0);

static SUNXI_CCU_GATE(de0_ahb_bus_clk, "de0-ahb-bus",
			"dcxo24M",
			0x0A04, BIT(0), 0);

static const char * const g2d_parents[] = { "pll-peri0-600m", "pll-peri0-480m", "pll-peri0-400m", "pll-video0-4x", "pll-video0-3x" };

static SUNXI_CCU_M_WITH_MUX_GATE(g2d_clk, "g2d",
			g2d_parents, 0x0A40,
			0, 5,  /* M */
			24, 3,  /* mux */
			BIT(31),	/* gate */
			0);

static SUNXI_CCU_GATE(g2d_ahb_bus_clk, "g2d-ahb-bus",
			"dcxo24M",
			0x0A44, BIT(0), 0);

static const char * const ce_sys_parents[] = { "dcxo24M", "pll-peri0-400m", "pll-peri0-480m", "pll-peri0-600m" };

static SUNXI_CCU_M_WITH_MUX_GATE(ce_sys_clk, "ce-sys",
			ce_sys_parents, 0x0AC0,
			0, 5,  /* M */
			24, 3,  /* mux */
			BIT(31),	/* gate */
			0);

static SUNXI_CCU_GATE(ce_sys_bus_clk, "ce-sys-bus",
			"dcxo24M",
			0x0AC4, BIT(1), 0);

static SUNXI_CCU_GATE(ce_sys_ip_ahb_bus_clk, "ce-sys-ip-ahb-bus",
			"dcxo24M",
			0x0AC4, BIT(0), 0);

static const char * const rv_core_parents[] = { "dcxo24M", "rc_16m", "ext-32k", "pll-peri0-600m", "pll-peri0-480m", "pll-peri0-400m" };

static SUNXI_CCU_M_WITH_MUX_GATE(rv_core_clk, "rv-core",
			rv_core_parents, 0x0B80,
			0, 5,  /* M */
			24, 3,  /* mux */
			BIT(31),	/* gate */
			0);

static SUNXI_CCU_M(e907_axi_clk_clk, "e907-axi-clk",
		"rv-core", 0xB80, 8, 2, 0);

static const char * const rv_ts_parents[] = { "dcxo24M", "rc_16m", "ext-32k" };

static SUNXI_CCU_MUX_WITH_GATE(rv_ts_clk, "rv-ts",
			rv_ts_parents, 0x0B88,
			24, 3,  /* mux */
			BIT(31), 0);

static SUNXI_CCU_GATE(rv_cfg_bus_clk, "rv-cfg-bus",
			"dcxo24M",
			0x0B9C, BIT(0), 0);

static SUNXI_CCU_GATE(dramc_ahb_bus_clk, "dramc-ahb-bus",
			"dcxo24M",
			0x0C0C, BIT(0), 0);

static const char * const smhc0_parents[] = { "dcxo24M", "pll-peri0-400m", "pll-peri0-300m", "pll-peri1-400m", "pll-peri1-300m" };

static SUNXI_CCU_MP_WITH_MUX_GATE_NO_INDEX(smhc0_clk, "smhc0",
			smhc0_parents, 0x0D00,
			0, 5,  /* M */
			8, 5,  /* N */
			24, 3,  /* mux */
			BIT(31), 0);

static SUNXI_CCU_GATE(smhc0_ahb_bus_clk, "smhc0-ahb-bus",
			"dcxo24M",
			0x0D0C, BIT(0), 0);

static const char * const smhc1_parents[] = { "dcxo24M", "pll-peri0-400m", "pll-peri0-300m", "pll-peri1-400m", "pll-peri1-300m" };

static SUNXI_CCU_MP_WITH_MUX_GATE_NO_INDEX(smhc1_clk, "smhc1",
			smhc1_parents, 0x0D10,
			0, 5,  /* M */
			8, 5,  /* N */
			24, 3,  /* mux */
			BIT(31), 0);

static SUNXI_CCU_GATE(smhc1_ahb_bus_clk, "smhc1-ahb-bus",
			"dcxo24M",
			0x0D1C, BIT(0), 0);

static const char * const smhc2_parents[] = { "dcxo24M", "pll-peri0-800m", "pll-peri0-600m", "pll-peri1-800m", "pll-peri1-600m" };

static SUNXI_CCU_MP_WITH_MUX_GATE_NO_INDEX(smhc2_clk, "smhc2",
			smhc2_parents, 0x0D20,
			0, 5,  /* M */
			8, 5,  /* N */
			24, 3,  /* mux */
			BIT(31), 0);

static SUNXI_CCU_GATE(smhc2_ahb_bus_clk, "smhc2-ahb-bus",
			"dcxo24M",
			0x0D2C, BIT(0), 0);

static const char * const smhc3_parents[] = { "dcxo24M", "pll-peri0-400m", "pll-peri0-300m", "pll-peri1-400m", "pll-peri1-300m" };

static SUNXI_CCU_MP_WITH_MUX_GATE_NO_INDEX(smhc3_clk, "smhc3",
			smhc3_parents, 0x0D30,
			0, 5,  /* M */
			8, 5,  /* N */
			24, 3,  /* mux */
			BIT(31), 0);

static SUNXI_CCU_GATE(smhc3_ahb_bus_clk, "smhc3-ahb-bus",
			"dcxo24M",
			0x0D3C, BIT(0), 0);

static SUNXI_CCU_GATE(uart0_apb_bus_clk, "uart0-apb-bus",
			"apb-uart",
			0x0E00, BIT(0), 0);

static SUNXI_CCU_GATE(uart1_apb_bus_clk, "uart1-apb-bus",
			"apb-uart",
			0x0E04, BIT(0), 0);

static SUNXI_CCU_GATE(uart2_apb_bus_clk, "uart2-apb-bus",
			"apb-uart",
			0x0E08, BIT(0), 0);

static SUNXI_CCU_GATE(uart3_apb_bus_clk, "uart3-apb-bus",
			"apb-uart",
			0x0E0C, BIT(0), 0);

static SUNXI_CCU_GATE(uart4_apb_bus_clk, "uart4-apb-bus",
			"apb-uart",
			0x0E10, BIT(0), 0);

static SUNXI_CCU_GATE(uart5_apb_bus_clk, "uart5-apb-bus",
			"apb-uart",
			0x0E14, BIT(0), 0);

static SUNXI_CCU_GATE(uart6_apb_bus_clk, "uart6-apb-bus",
			"apb-uart",
			0x0E18, BIT(0), 0);

static SUNXI_CCU_GATE(uart7_apb_bus_clk, "uart7-apb-bus",
			"apb-uart",
			0x0E20, BIT(0), 0);

static SUNXI_CCU_GATE(uart8_apb_bus_clk, "uart8-apb-bus",
			"apb-uart",
			0x0E24, BIT(0), 0);

static SUNXI_CCU_GATE(uart9_apb_bus_clk, "uart9-apb-bus",
			"apb-uart",
			0x0E28, BIT(0), 0);

static SUNXI_CCU_GATE(twi0_apb_bus_clk, "twi0-apb-bus",
			"apb-uart",
			0x0E80, BIT(0), 0);

static SUNXI_CCU_GATE(twi1_apb_bus_clk, "twi1-apb-bus",
			"apb1",
			0x0E84, BIT(0), 0);

static SUNXI_CCU_GATE(twi2_apb_bus_clk, "twi2-apb-bus",
			"apb1",
			0x0E88, BIT(0), 0);

static SUNXI_CCU_GATE(twi3_apb_bus_clk, "twi3-apb-bus",
			"apb1",
			0x0E8C, BIT(0), 0);

static SUNXI_CCU_GATE(twi4_apb_bus_clk, "twi4-apb-bus",
			"apb1",
			0x0E90, BIT(0), 0);

static SUNXI_CCU_GATE(twi5_apb_bus_clk, "twi5-apb-bus",
			"apb1",
			0x0E94, BIT(0), 0);

static const char * const spi0_parents[] = { "dcxo24M", "pll-peri0-480m", "pll-peri0-300m", "pll-peri0-200m", "pll-peri1-480m", "pll-peri1-300m", "pll-peri1-200m" };

static SUNXI_CCU_MP_WITH_MUX_GATE_NO_INDEX(spi0_clk, "spi0",
			spi0_parents, 0x0F00,
			0, 5,  /* M */
			8, 5,  /* N */
			24, 3,  /* mux */
			BIT(31), 0);

static SUNXI_CCU_GATE(spi0_ahb_bus_clk, "spi0-ahb-bus",
			"dcxo24M",
			0x0F04, BIT(0), 0);

static const char * const spi1_parents[] = { "dcxo24M", "pll-peri0-480m", "pll-peri0-300m", "pll-peri0-200m", "pll-peri1-480m", "pll-peri1-300m", "pll-peri1-200m" };

static SUNXI_CCU_MP_WITH_MUX_GATE_NO_INDEX(spi1_clk, "spi1",
			spi1_parents, 0x0F08,
			0, 5,  /* M */
			8, 5,  /* N */
			24, 3,  /* mux */
			BIT(31), 0);

static SUNXI_CCU_GATE(spi1_ahb_bus_clk, "spi1-ahb-bus",
			"dcxo24M",
			0x0F0C, BIT(0), 0);

static const char * const spi2_parents[] = { "dcxo24M", "pll-peri0-480m", "pll-peri0-300m", "pll-peri0-200m", "pll-peri1-480m", "pll-peri1-300m", "pll-peri1-200m" };

static SUNXI_CCU_MP_WITH_MUX_GATE_NO_INDEX(spi2_clk, "spi2",
			spi2_parents, 0x0F10,
			0, 5,  /* M */
			8, 5,  /* N */
			24, 3,  /* mux */
			BIT(31), 0);

static SUNXI_CCU_GATE(spi2_ahb_bus_clk, "spi2-ahb-bus",
			"dcxo24M",
			0x0F14, BIT(0), 0);

static const char * const spif_parents[] = { "dcxo24M", "pll-peri0-480m", "pll-peri0-400m", "pll-peri0-300m", "pll-peri1-480m", "pll-peri1-400m", "pll-peri1-300m" };

static SUNXI_CCU_MP_WITH_MUX_GATE_NO_INDEX(spif_clk, "spif",
			spif_parents, 0x0F18,
			0, 5,  /* M */
			8, 5,  /* N */
			24, 3,  /* mux */
			BIT(31), 0);

static SUNXI_CCU_GATE(spif_ahb_bus_clk, "spif-ahb-bus",
			"dcxo24M",
			0x0F1C, BIT(0), 0);

static const char * const spi3_parents[] = { "dcxo24M", "pll-peri0-480m", "pll-peri0-300m", "pll-peri0-200m", "pll-peri1-480m", "pll-peri1-300m", "pll-peri1-200m" };

static SUNXI_CCU_MP_WITH_MUX_GATE_NO_INDEX(spi3_clk, "spi3",
			spi3_parents, 0x0F20,
			0, 5,  /* M */
			8, 5,  /* N */
			24, 3,  /* mux */
			BIT(31), 0);

static SUNXI_CCU_GATE(spi3_ahb_bus_clk, "spi3-ahb-bus",
			"dcxo24M",
			0x0F24, BIT(0), 0);

static const char * const gpadc0_parents[] = { "dcxo24M", "clk48m", "pll-peri0-480m" };

static SUNXI_CCU_MUX_WITH_GATE(gpadc0_clk, "gpadc0",
			gpadc0_parents, 0x0FC0,
			24, 3,  /* mux */
			BIT(31), 0);

static SUNXI_CCU_GATE(gpadc0_apb_bus_clk, "gpadc0-apb-bus",
			"dcxo24M",
			0x0FC4, BIT(0), 0);

static const char * const gpadc1_parents[] = { "dcxo24M", "clk48m", "pll-peri0-480m" };

static SUNXI_CCU_MUX_WITH_GATE(gpadc1_clk, "gpadc1",
			gpadc1_parents, 0x0FC8,
			24, 3,  /* mux */
			BIT(31), 0);

static SUNXI_CCU_GATE(gpadc1_apb_bus_clk, "gpadc1-apb-bus",
			"dcxo24M",
			0x0FCC, BIT(0), 0);

static const char * const gpadc2_parents[] = { "dcxo24M", "clk48m", "pll-peri0-480m" };

static SUNXI_CCU_MUX_WITH_GATE(gpadc2_clk, "gpadc2",
			gpadc2_parents, 0x0FD0,
			24, 3,  /* mux */
			BIT(31), 0);

static SUNXI_CCU_GATE(gpadc2_apb_bus_clk, "gpadc2-apb-bus",
			"dcxo24M",
			0x0FD4, BIT(0), 0);

static const char * const gpadc3_parents[] = { "dcxo24M", "clk48m", "pll-peri0-480m" };

static SUNXI_CCU_M_WITH_MUX_GATE(gpadc3_clk, "gpadc3",
			gpadc3_parents, 0x0FD8,
			0, 5,  /* M */
			24, 3,  /* mux */
			BIT(31),	/* gate */
			0);

static SUNXI_CCU_GATE(gpadc3_apb_bus_clk, "gpadc3-apb-bus",
			"dcxo24M",
			0x0FDC, BIT(0), 0);

static SUNXI_CCU_GATE(tsensor_apb_bus_clk, "tsensor-apb-bus",
			"dcxo24M",
			0x0FE4, BIT(0), 0);

static const char * const ir_rx0_parents[] = { "ext-32k", "dcxo24M" };

static SUNXI_CCU_M_WITH_MUX_GATE(ir_rx0_clk, "ir-rx0",
			ir_rx0_parents, 0x1000,
			0, 5,  /* M */
			24, 3,  /* mux */
			BIT(31),	/* gate */
			0);

static SUNXI_CCU_GATE(ir_rx0_apb_bus_clk, "ir-rx0-apb-bus",
			"dcxo24M",
			0x1004, BIT(0), 0);

static const char * const ir_tx_parents[] = { "dcxo24M", "pll-peri1-600m" };

static SUNXI_CCU_M_WITH_MUX_GATE(ir_tx_clk, "ir-tx",
			ir_tx_parents, 0x1008,
			0, 5,  /* M */
			24, 3,  /* mux */
			BIT(31),	/* gate */
			0);

static SUNXI_CCU_GATE(ir_tx_apb_bus_clk, "ir-tx-apb-bus",
			"dcxo24M",
			0x100C, BIT(0), 0);

static SUNXI_CCU_M_WITH_GATE(tpadc_clk, "tpadc",
			"dcxo24M", 0x1030, 0,
			5, BIT(31), 0);

static SUNXI_CCU_GATE(tpadc_apb_bus_clk, "tpadc-apb-bus",
			"dcxo24M",
			0x1034, BIT(0), 0);

static const char * const lbc_parents[] = { "pll-peri0-480m", "pll-peri0-400m", "pll-peri0-300m", "pll-peri1-480m", "pll-peri1-400m", "pll-peri1-300m", "pll-video0-4x", "pll-video0-3x" };

static SUNXI_CCU_MP_WITH_MUX_GATE_NO_INDEX(lbc_clk, "lbc",
			lbc_parents, 0x1040,
			0, 5,  /* M */
			8, 5,  /* N */
			24, 3,  /* mux */
			BIT(31), 0);

static SUNXI_CCU_GATE(lbc_ahb_bus_clk, "lbc-ahb-bus",
			"dcxo24M",
			0x104C, BIT(0), 0);

static const char * const ir_rx1_parents[] = { "ext-32k", "dcxo24M" };

static SUNXI_CCU_M_WITH_MUX_GATE(ir_rx1_clk, "ir-rx1",
			ir_rx1_parents, 0x1100,
			0, 5,  /* M */
			24, 3,  /* mux */
			BIT(31),	/* gate */
			0);

static SUNXI_CCU_GATE(ir_rx1_apb_bus_clk, "ir-rx1-apb-bus",
			"dcxo24M",
			0x1104, BIT(0), 0);

static const char * const ir_rx2_parents[] = { "ext-32k", "dcxo24M" };

static SUNXI_CCU_M_WITH_MUX_GATE(ir_rx2_clk, "ir-rx2",
			ir_rx2_parents, 0x1108,
			0, 5,  /* M */
			24, 3,  /* mux */
			BIT(31),	/* gate */
			0);

static SUNXI_CCU_GATE(ir_rx2_apb_bus_clk, "ir-rx2-apb-bus",
			"dcxo24M",
			0x110C, BIT(0), 0);

static const char * const ir_rx3_parents[] = { "ext-32k", "dcxo24M" };

static SUNXI_CCU_M_WITH_MUX_GATE(ir_rx3_clk, "ir-rx3",
			ir_rx3_parents, 0x1110,
			0, 5,  /* M */
			24, 3,  /* mux */
			BIT(31),	/* gate */
			0);

static SUNXI_CCU_GATE(ir_rx3_apb_bus_clk, "ir-rx3-apb-bus",
			"dcxo24M",
			0x1114, BIT(0), 0);

static const char * const i2s0_parents[] = { "pll-audio0", "pll-peri1-600m" };

static SUNXI_CCU_M_WITH_MUX_GATE(i2s0_clk, "i2s0",
			i2s0_parents, 0x1200,
			0, 5,  /* M */
			24, 3,  /* mux */
			BIT(31),	/* gate */
			CLK_SET_RATE_NO_REPARENT | CLK_SET_RATE_PARENT);

static SUNXI_CCU_GATE(i2s0_apb_bus_clk, "i2s0-apb-bus",
			"dcxo24M",
			0x120C, BIT(0), 0);

static const char * const i2s1_parents[] = { "pll-audio0", "pll-peri1-600m" };

static SUNXI_CCU_M_WITH_MUX_GATE(i2s1_clk, "i2s1",
			i2s1_parents, 0x1210,
			0, 5,  /* M */
			24, 3,  /* mux */
			BIT(31),	/* gate */
			CLK_SET_RATE_NO_REPARENT | CLK_SET_RATE_PARENT);

static SUNXI_CCU_GATE(i2s1_apb_bus_clk, "i2s1-apb-bus",
			"dcxo24M",
			0x121C, BIT(0), 0);

static const char * const i2s2_parents[] = { "pll-audio0", "pll-peri1-600m" };

static SUNXI_CCU_M_WITH_MUX_GATE(i2s2_clk, "i2s2",
			i2s2_parents, 0x1220,
			0, 5,  /* M */
			24, 3,  /* mux */
			BIT(31),	/* gate */
			CLK_SET_RATE_NO_REPARENT | CLK_SET_RATE_PARENT);

static SUNXI_CCU_GATE(i2s2_apb_bus_clk, "i2s2-apb-bus",
			"dcxo24M",
			0x122C, BIT(0), 0);

static const char * const owa0_tx_parents[] = { "pll-audio0", "pll-peri1-600m" };

static SUNXI_CCU_M_WITH_MUX_GATE(owa0_tx_clk, "owa0-tx",
			owa0_tx_parents, 0x1280,
			0, 5,  /* M */
			24, 3,  /* mux */
			BIT(31),	/* gate */
			CLK_SET_RATE_NO_REPARENT | CLK_SET_RATE_PARENT);

static const char * const owa0_rx_parents[] = { "pll-peri0-400m", "pll-peri0-300m"};

static SUNXI_CCU_M_WITH_MUX_GATE(owa0_rx_clk, "owa0-rx",
			owa0_rx_parents, 0x1284,
			0, 5,  /* M */
			24, 3,  /* mux */
			BIT(31),	/* gate */
			CLK_SET_RATE_NO_REPARENT | CLK_SET_RATE_PARENT);

static SUNXI_CCU_GATE(owa0_apb_bus_clk, "owa0-apb-bus",
			"dcxo24M",
			0x128C, BIT(0), 0);

static const char * const dmic_parents[] = { "pll-audio0", "pll-peri1-600m" };

static SUNXI_CCU_M_WITH_MUX_GATE(dmic_clk, "dmic",
			dmic_parents, 0x12C0,
			0, 5,  /* M */
			24, 3,  /* mux */
			BIT(31),	/* gate */
			CLK_SET_RATE_NO_REPARENT | CLK_SET_RATE_PARENT);

static SUNXI_CCU_GATE(dmic_apb_bus_clk, "dmic-apb-bus",
			"dcxo24M",
			0x12CC, BIT(0), 0);

static const char * const audiocodec0_dac_parents[] = { "pll-audio0", "pll-peri1-600m" };

static SUNXI_CCU_M_WITH_MUX_GATE(audiocodec0_dac_clk, "audiocodec0-dac",
			audiocodec0_dac_parents, 0x12E0,
			0, 5,  /* M */
			24, 3,  /* mux */
			BIT(31),	/* gate */
			CLK_SET_RATE_NO_REPARENT | CLK_SET_RATE_PARENT);

static SUNXI_CCU_GATE(audiocodec0_apb_bus_clk, "audiocodec0-apb-bus",
			"dcxo24M",
			0x12EC, BIT(0), 0);

static SUNXI_CCU_GATE(usb0_bus_clk, "usb0-bus",
			"dcxo24M",
			0x1300, BIT(31), 0);

static SUNXI_CCU_GATE(usb0_dev_ahb_bus_clk, "usb0-dev-ahb-bus",
			"dcxo24M",
			0x1304, BIT(8), 0);

static SUNXI_CCU_GATE(usb0_ehci_ahb_bus_clk, "usb0-ehci-ahb-bus",
			"dcxo24M",
			0x1304, BIT(4), 0);

static SUNXI_CCU_GATE(usb0_ohci_ahb_bus_clk, "usb0-ohci-ahb-bus",
			"dcxo24M",
			0x1304, BIT(0), 0);

static SUNXI_CCU_GATE(usb1_bus_clk, "usb1-bus",
			"dcxo24M",
			0x1308, BIT(31), 0);

static SUNXI_CCU_GATE(usb1_ehci_ahb_bus_clk, "usb1-ehci-ahb-bus",
			"dcxo24M",
			0x130C, BIT(4), 0);

static SUNXI_CCU_GATE(usb1_ohci_ahb_bus_clk, "usb1-ohci-ahb-bus",
			"dcxo24M",
			0x130C, BIT(0), 0);

static SUNXI_CCU_GATE(usb2p0_sys_phy_ref_bus_clk, "usb2p0-sys-phy-ref-bus",
			"dcxo24M",
			0x1340, BIT(31), 0);

static SUNXI_CCU_GATE(usb2p0_sys_ahb_bus_clk, "usb2p0-sys-ahb-bus",
			"dcxo24M",
			0x1344, BIT(0), 0);

static SUNXI_CCU_M_WITH_GATE(gmac0_phy_clk, "gmac0-phy",
			"pll-peri0-150m", 0x1400, 0,
			5, BIT(31), 0);

static const char * const gmac0_ptp_ref_parents[] = { "dcxo24M", "pll-peri0-200m" };

static SUNXI_CCU_M_WITH_MUX_GATE(gmac0_ptp_ref_clk, "gmac0-ptp-ref",
			gmac0_ptp_ref_parents, 0x1404,
			0, 5,  /* M */
			24, 3,  /* mux */
			BIT(31),	/* gate */
			0);

static SUNXI_CCU_GATE(gmac0_ahb_bus_clk, "gmac0-ahb-bus",
			"dcxo24M",
			0x140C, BIT(0), 0);

static SUNXI_CCU_M_WITH_GATE(gmac1_phy_clk, "gmac1-phy",
			"pll-peri0-150m", 0x1410, 0,
			5, BIT(31), 0);

static const char * const gmac1_ptp_ref_parents[] = { "dcxo24M", "pll-peri0-200m" };

static SUNXI_CCU_M_WITH_MUX_GATE(gmac1_ptp_ref_clk, "gmac1-ptp-ref",
			gmac1_ptp_ref_parents, 0x1414,
			0, 5,  /* M */
			24, 3,  /* mux */
			BIT(31),	/* gate */
			0);

static SUNXI_CCU_GATE(gmac1_ahb_bus_clk, "gmac1-ahb-bus",
			"dcxo24M",
			0x141C, BIT(0), 0);

static SUNXI_CCU_M_WITH_GATE(gmac2_phy_clk, "gmac2-phy",
			"pll-peri0-150m", 0x1420, 0,
			5, BIT(31), 0);

static const char * const gmac2_ptp_ref_parents[] = { "dcxo24M", "pll-peri0-200m" };

static SUNXI_CCU_M_WITH_MUX_GATE(gmac2_ptp_ref_clk, "gmac2-ptp-ref",
			gmac2_ptp_ref_parents, 0x1424,
			0, 5,  /* M */
			24, 3,  /* mux */
			BIT(31),	/* gate */
			0);

static SUNXI_CCU_GATE(gmac2_ahb_bus_clk, "gmac2-ahb-bus",
			"dcxo24M",
			0x142C, BIT(0), 0);

static const char * const tcon_lcd0_parents[] = { "pll-video0-4x", "pll-peri0-2x", "pll-peri1-2x", "pll-video0-3x" };

static SUNXI_CCU_M_WITH_MUX_GATE(tcon_lcd0_clk, "tcon-lcd0",
			tcon_lcd0_parents, 0x1500,
			0, 5,  /* M */
			24, 3,  /* mux */
			BIT(31),	/* gate */
			0);

static SUNXI_CCU_GATE(tcon_lcd0_ahb_bus_clk, "tcon-lcd0-ahb-bus",
			"dcxo24M",
			0x1504, BIT(0), 0);

static const char * const mipi_dsi0_parents[] = { "dcxo24M", "pll-peri0-200m", "pll-peri0-150m" };

static SUNXI_CCU_M_WITH_MUX_GATE(mipi_dsi0_clk, "mipi-dsi0",
			mipi_dsi0_parents, 0x1580,
			0, 5,  /* M */
			24, 3,  /* mux */
			BIT(31),	/* gate */
			0);

static SUNXI_CCU_GATE(mipi_dsi0_ahb_bus_clk, "mipi-dsi0-ahb-bus",
			"dcxo24M",
			0x1584, BIT(0), 0);

static const char * const combophy0_parents[] = { "pll-video0-4x", "pll-peri0-2x", "pll-peri1-2x", "pll-video0-3x" };

static SUNXI_CCU_M_WITH_MUX_GATE(combophy0_clk, "combophy0",
			combophy0_parents, 0x15C0,
			0, 5,  /* M */
			24, 3,  /* mux */
			BIT(31),	/* gate */
			0);

static SUNXI_CCU_GATE(vo0_reg_ahb_bus_clk, "vo0-reg-ahb-bus",
			"dcxo24M",
			0x16C4, BIT(0), 0);

static const char * const ledc_parents[] = { "dcxo24M", "pll-peri0-600m" };

static SUNXI_CCU_M_WITH_MUX_GATE(ledc_clk, "ledc",
			ledc_parents, 0x1700,
			0, 5,  /* M */
			24, 1,  /* mux */
			BIT(31),	/* gate */
			0);

static SUNXI_CCU_GATE(ledc_apb_bus_clk, "ledc-apb-bus",
			"dcxo24M",
			0x1704, BIT(0), 0);

static const char * const csi_master0_parents[] = { "dcxo24M", "pll-video0-4x", "pll-video0-3x", "pll-peri1-2x", "pll-peri1-480m" };

static SUNXI_CCU_MP_WITH_MUX_GATE_NO_INDEX(csi_master0_clk, "csi-master0",
			csi_master0_parents, 0x1800,
			0, 5,  /* M */
			8, 5,  /* N */
			24, 3,  /* mux */
			BIT(31), 0);

static const char * const csi_master1_parents[] = { "dcxo24M", "pll-video0-4x", "pll-video0-3x", "pll-peri1-2x", "pll-peri1-480m" };

static SUNXI_CCU_MP_WITH_MUX_GATE_NO_INDEX(csi_master1_clk, "csi-master1",
			csi_master1_parents, 0x1804,
			0, 5,  /* M */
			8, 5,  /* N */
			24, 3,  /* mux */
			BIT(31), 0);

static const char * const csi_master2_parents[] = { "dcxo24M", "pll-video0-4x", "pll-video0-3x", "pll-peri1-2x", "pll-peri1-480m" };

static SUNXI_CCU_MP_WITH_MUX_GATE_NO_INDEX(csi_master2_clk, "csi-master2",
			csi_master2_parents, 0x1808,
			0, 5,  /* M */
			8, 5,  /* N */
			24, 3,  /* mux */
			BIT(31), 0);

static const char * const csi_parents[] = { "pll-peri0-300m", "pll-peri0-400m", "pll-peri0-480m", "pll-video0-4x", "pll-video0-3x", "pll-peri1-2x", "pll-peri1-480m" };

static SUNXI_CCU_M_WITH_MUX_GATE(csi_clk, "csi",
			csi_parents, 0x1840,
			0, 5,  /* M */
			24, 3,  /* mux */
			BIT(31),	/* gate */
			0);

static const char * const isp_parents[] = { "pll-peri0-300m", "pll-peri0-400m", "pll-peri0-480m", "pll-video0-4x", "pll-video0-3x", "pll-peri1-2x", "pll-peri1-480m" };

static SUNXI_CCU_M_WITH_MUX_GATE(isp_clk, "isp",
			isp_parents, 0x1860,
			0, 5,  /* M */
			24, 3,  /* mux */
			BIT(31),	/* gate */
			0);

static SUNXI_CCU_GATE(video_in_ahb_bus_clk, "video-in-ahb-bus",
			"dcxo24M",
			0x1884, BIT(0), 0);

static SUNXI_CCU_GATE(rv_aximon_bus_clk, "rv-aximon-bus",
			"dcxo24M",
			0x1C00, BIT(0), 0);

static SUNXI_CCU_GATE(dcu_ahbmon_bus_clk, "dcu-ahbmon-bus",
			"dcxo24M",
			0x1C04, BIT(1), 0);

static SUNXI_CCU_GATE(cpu_sys_ahbmon_bus_clk, "cpu-sys-ahbmon-bus",
			"dcxo24M",
			0x1C04, BIT(0), 0);

static SUNXI_CCU_GATE(clk50m_bus_clk, "clk50m-bus",
			"dcxo24M",
			0x1F30, BIT(4), 0);

static SUNXI_CCU_GATE(clk25m_bus_clk, "clk25m-bus",
			"dcxo24M",
			0x1F30, BIT(3), 0);

static SUNXI_CCU_GATE(clk16m_bus_clk, "clk16m-bus",
			"dcxo24M",
			0x1F30, BIT(2), 0);

static SUNXI_CCU_GATE(clk12m_bus_clk, "clk12m-bus",
			"dcxo24M",
			0x1F30, BIT(1), 0);

static SUNXI_CCU_GATE(clk24m_bus_clk, "clk24m-bus",
			"dcxo24M",
			0x1F30, BIT(0), 0);

/* ccu_des_end */

/* rst_def_start */
static struct ccu_reset_map sun8iw22_ccu_resets[] = {
	[RST_BUS_PLL_SSC_RSTN]		= { 0x0354, BIT(30) },
	[RST_BUS_DMA0]			= { 0x0704, BIT(16) },
	[RST_BUS_DMA1]			= { 0x070c, BIT(16) },
	[RST_BUS_SPINLOCK]		= { 0x0724, BIT(16) },
	[RST_BUS_MSGBOX_CPUX]		= { 0x0744, BIT(16) },
	[RST_BUS_MSGBOX_CORE0]		= { 0x074c, BIT(16) },
	[RST_BUS_MSGBOX_CORE1]		= { 0x0754, BIT(16) },
	[RST_BUS_MSGBOX_CORE2]		= { 0x075c, BIT(16) },
	[RST_BUS_MSGBOX_CORE3]		= { 0x0764, BIT(16) },
	[RST_BUS_MSGBOX_RV]		= { 0x076c, BIT(16) },
	[RST_BUS_PWM2]			= { 0x0794, BIT(16) },
	[RST_BUS_DCU]			= { 0x07a4, BIT(16) },
	[RST_BUS_DAP]			= { 0x07ac, BIT(16) },
	[RST_BUS_PWMCS0]		= { 0x07c4, BIT(16) },
	[RST_BUS_PWMCS1]		= { 0x07cc, BIT(16) },
	[RST_BUS_TIMER0]		= { 0x0850, BIT(16) },
	[RST_BUS_TIMER0_RV]		= { 0x0870, BIT(16) },
	[RST_BUS_DE0]			= { 0x0a04, BIT(16) },
	[RST_BUS_G2D]			= { 0x0a44, BIT(16) },
	[RST_BUS_CE_SY]			= { 0x0ac4, BIT(17) },
	[RST_BUS_RV_SY]			= { 0x0b94, BIT(17) },
	[RST_BUS_RV_CORE]		= { 0x0b94, BIT(16) },
	[RST_BUS_RV_CFG]		= { 0x0b9c, BIT(16) },
	[RST_BUS_DRAMC]			= { 0x0c0c, BIT(16) },
	[RST_BUS_SMHC0]			= { 0x0d0c, BIT(16) },
	[RST_BUS_SMHC1]			= { 0x0d1c, BIT(16) },
	[RST_BUS_SMHC2]			= { 0x0d2c, BIT(16) },
	[RST_BUS_SMHC3]			= { 0x0d3c, BIT(16) },
	[RST_BUS_UART0]			= { 0x0e00, BIT(16) },
	[RST_BUS_UART1]			= { 0x0e04, BIT(16) },
	[RST_BUS_UART2]			= { 0x0e08, BIT(16) },
	[RST_BUS_UART3]			= { 0x0e0c, BIT(16) },
	[RST_BUS_UART4]			= { 0x0e10, BIT(16) },
	[RST_BUS_UART5]			= { 0x0e14, BIT(16) },
	[RST_BUS_UART6]			= { 0x0e18, BIT(16) },
	[RST_BUS_UART7]			= { 0x0e20, BIT(16) },
	[RST_BUS_UART8]			= { 0x0e24, BIT(16) },
	[RST_BUS_UART9]			= { 0x0e28, BIT(16) },
	[RST_BUS_TWI0]			= { 0x0e80, BIT(16) },
	[RST_BUS_TWI1]			= { 0x0e84, BIT(16) },
	[RST_BUS_TWI2]			= { 0x0e88, BIT(16) },
	[RST_BUS_TWI3]			= { 0x0e8c, BIT(16) },
	[RST_BUS_TWI4]			= { 0x0e90, BIT(16) },
	[RST_BUS_TWI5]			= { 0x0e94, BIT(16) },
	[RST_BUS_SPI0]			= { 0x0f04, BIT(16) },
	[RST_BUS_SPI1]			= { 0x0f0c, BIT(16) },
	[RST_BUS_SPI2]			= { 0x0f14, BIT(16) },
	[RST_BUS_SPIF]			= { 0x0f1c, BIT(16) },
	[RST_BUS_SPI3]			= { 0x0f24, BIT(16) },
	[RST_BUS_GPADC0]		= { 0x0fc4, BIT(16) },
	[RST_BUS_GPADC1]		= { 0x0fcc, BIT(16) },
	[RST_BUS_GPADC2]		= { 0x0fd4, BIT(16) },
	[RST_BUS_GPADC3]		= { 0x0fdc, BIT(16) },
	[RST_BUS_TSENSO]		= { 0x0fe4, BIT(16) },
	[RST_BUS_IR_RX0]		= { 0x1004, BIT(16) },
	[RST_BUS_IR_TX]			= { 0x100c, BIT(16) },
	[RST_BUS_TPADC]			= { 0x1034, BIT(16) },
	[RST_BUS_LBC]			= { 0x104c, BIT(16) },
	[RST_BUS_IR_RX1]		= { 0x1104, BIT(16) },
	[RST_BUS_IR_RX2]		= { 0x110c, BIT(16) },
	[RST_BUS_IR_RX3]		= { 0x1114, BIT(16) },
	[RST_BUS_I2S0]			= { 0x120c, BIT(16) },
	[RST_BUS_I2S1]			= { 0x121c, BIT(16) },
	[RST_BUS_I2S2]			= { 0x122c, BIT(16) },
	[RST_BUS_OWA0]			= { 0x128c, BIT(16) },
	[RST_BUS_DMIC]			= { 0x12cc, BIT(16) },
	[RST_BUS_AUDIOCODEC0]		= { 0x12ec, BIT(16) },
	[RST_USB_0_DEV]			= { 0x1304, BIT(24) },
	[RST_USB_0_EHCI]		= { 0x1304, BIT(20) },
	[RST_USB_0_OHCI]		= { 0x1304, BIT(16) },
	[RST_USB_1_EHCI]		= { 0x130c, BIT(20) },
	[RST_USB_1_OHCI]		= { 0x130c, BIT(16) },
	[RST_USB_2P0_SY]		= { 0x1344, BIT(16) },
	[RST_BUS_GMAC0_TOP_AHB]		= { 0x140c, BIT(18) },
	[RST_BUS_GMAC0_AXI]		= { 0x140c, BIT(17) },
	[RST_BUS_GMAC0_AHB]		= { 0x140c, BIT(16) },
	[RST_BUS_GMAC1_TOP_AHB]		= { 0x141c, BIT(18) },
	[RST_BUS_GMAC1_AXI]		= { 0x141c, BIT(17) },
	[RST_BUS_GMAC1_AHB]		= { 0x141c, BIT(16) },
	[RST_BUS_GMAC2_TOP_AHB]		= { 0x142c, BIT(18) },
	[RST_BUS_GMAC2_AXI]		= { 0x142c, BIT(17) },
	[RST_BUS_GMAC2_AHB]		= { 0x142c, BIT(16) },
	[RST_BUS_TCON_LCD0]		= { 0x1504, BIT(16) },
	[RST_BUS_LVDS0]			= { 0x1544, BIT(16) },
	[RST_BUS_MIPI_DSI0]		= { 0x1584, BIT(16) },
	[RST_BUS_VO0_REG]		= { 0x16c4, BIT(16) },
	[RST_BUS_VIDEO_OUT0]		= { 0x16e4, BIT(16) },
	[RST_BUS_LEDC]			= { 0x1704, BIT(16) },
	[RST_BUS_VIDEO_IN]		= { 0x1884, BIT(16) },
	[RST_BUS_RV_AXIMON]		= { 0x1c00, BIT(16) },
	[RST_BUS_DCU_AHBMON]		= { 0x1c04, BIT(17) },
	[RST_BUS_CPU_SYS_AHBMON]	= { 0x1c04, BIT(16) },
};
/* rst_def_end */

/* ccu_def_start */
static struct clk_hw_onecell_data sun8iw22_hw_clks = {
	.hws    = {
		[CLK_PLL_PERI0]				= &pll_peri0_clk.common.hw,
		[CLK_PLL_PERI0_2X]			= &pll_peri0_2x_clk.common.hw,
		[CLK_PLL_PERI0_800M]			= &pll_peri0_800m_clk.common.hw,
		[CLK_PLL_PERI0_480M]			= &pll_peri0_480m_clk.common.hw,
		[CLK_PLL_PERI0_600M]			= &pll_peri0_600m_clk.hw,
		[CLK_PLL_PERI0_400M]			= &pll_peri0_400m_clk.hw,
		[CLK_PLL_PERI0_300M]			= &pll_peri0_300m_clk.hw,
		[CLK_PLL_PERI0_200M]			= &pll_peri0_200m_clk.hw,
		[CLK_PLL_PERI0_160M]			= &pll_peri0_160m_clk.hw,
		[CLK_PLL_PERI0_150M]			= &pll_peri0_150m_clk.hw,
		[CLK_PLL_PERI1]				= &pll_peri1_clk.common.hw,
		[CLK_PLL_PERI1_2X]			= &pll_peri1_2x_clk.common.hw,
		[CLK_PLL_PERI1_800M]			= &pll_peri1_800m_clk.common.hw,
		[CLK_PLL_PERI1_480M]			= &pll_peri1_480m_clk.common.hw,
		[CLK_PLL_PERI1_600M]			= &pll_peri1_600m_clk.hw,
		[CLK_PLL_PERI1_400M]			= &pll_peri1_400m_clk.hw,
		[CLK_PLL_PERI1_300M]			= &pll_peri1_300m_clk.hw,
		[CLK_PLL_PERI1_200M]			= &pll_peri1_200m_clk.hw,
		[CLK_PLL_PERI1_160M]			= &pll_peri1_160m_clk.hw,
		[CLK_PLL_PERI1_150M]			= &pll_peri1_150m_clk.hw,
		[CLK_PLL_VIDEO0]			= &pll_video0_clk.common.hw,
		[CLK_PLL_VIDEO0_4X]			= &pll_video0_4x_clk.common.hw,
		[CLK_PLL_VIDEO0_3X]			= &pll_video0_3x_clk.common.hw,
		[CLK_PLL_AUDIO0]			= &pll_audio0_clk.common.hw,
		[CLK_AHB]				= &ahb_clk.common.hw,
		[CLK_APB0]				= &apb0_clk.common.hw,
		[CLK_APB1]				= &apb1_clk.common.hw,
		[CLK_APB_UART]				= &apb_uart_clk.common.hw,
		[CLK_MBUS]				= &mbus_clk.common.hw,
		[CLK_BUS_PLL_STBY_PERI0_CFG]		= &pll_stby_peri0_cfg_bus_clk.common.hw,
		[CLK_BUS_SPIF_AHB_SW_CFG]		= &spif_ahb_sw_cfg_bus_clk.common.hw,
		[CLK_BUS_USB2P0_SYS_AHB_SW_CFG]		= &usb2p0_sys_ahb_sw_cfg_bus_clk.common.hw,
		[CLK_BUS_GMAC2_AHB_SW_CFG]		= &gmac2_ahb_sw_cfg_bus_clk.common.hw,
		[CLK_BUS_GMAC1_AHB_SW_CFG]		= &gmac1_ahb_sw_cfg_bus_clk.common.hw,
		[CLK_BUS_GMAC0_AHB_SW_CFG]		= &gmac0_ahb_sw_cfg_bus_clk.common.hw,
		[CLK_BUS_MCU_SYS_AHB_SW_CFG]		= &mcu_sys_ahb_sw_cfg_bus_clk.common.hw,
		[CLK_BUS_SMHC3_AHB_SW_CFG]		= &smhc3_ahb_sw_cfg_bus_clk.common.hw,
		[CLK_BUS_SMHC2_AHB_SW_CFG]		= &smhc2_ahb_sw_cfg_bus_clk.common.hw,
		[CLK_BUS_SMHC1_AHB_SW_CFG]		= &smhc1_ahb_sw_cfg_bus_clk.common.hw,
		[CLK_BUS_SMHC0_AHB_SW_CFG]		= &smhc0_ahb_sw_cfg_bus_clk.common.hw,
		[CLK_BUS_VIDEO_OUT0_AHB_SW_CFG]		= &video_out0_ahb_sw_cfg_bus_clk.common.hw,
		[CLK_BUS_VIDEO_IN_AHB_SW_CFG]		= &video_in_ahb_sw_cfg_bus_clk.common.hw,
		[CLK_BUS_PLL_PERI_APB0_SW_CFG]		= &pll_peri_apb0_sw_cfg_bus_clk.common.hw,
		[CLK_BUS_PLL_PERI_AHB_SW_CFG]		= &pll_peri_ahb_sw_cfg_bus_clk.common.hw,
		[CLK_MBUS_BUS_GMAC2]			= &gmac2_mbus_bus_clk.common.hw,
		[CLK_MBUS_BUS_GMAC1]			= &gmac1_mbus_bus_clk.common.hw,
		[CLK_MBUS_BUS_GMAC0]			= &gmac0_mbus_bus_clk.common.hw,
		[CLK_MBUS_BUS_ISP]			= &isp_mbus_bus_clk.common.hw,
		[CLK_MBUS_BUS_CSI]			= &csi_mbus_bus_clk.common.hw,
		[CLK_MBUS_BUS_DMA1]			= &dma1_mbus_bus_clk.common.hw,
		[CLK_MBUS_BUS_CE_SYS]			= &ce_sys_mbus_bus_clk.common.hw,
		[CLK_MBUS_BUS_DMA0]			= &dma0_mbus_bus_clk.common.hw,
		[CLK_MBUS_BUS_DMA1_SW_CFG]		= &dma1_mbus_sw_cfg_bus_clk.common.hw,
		[CLK_MBUS_BUS_DMA0_SW_CFG]		= &dma0_mbus_sw_cfg_bus_clk.common.hw,
		[CLK_MBUS_BUS_LBC_SW_CFG]		= &lbc_mbus_sw_cfg_bus_clk.common.hw,
		[CLK_MBUS_BUS_VIDEO_OUT0_SW_CFG]	= &video_out0_mbus_sw_cfg_bus_clk.common.hw,
		[CLK_MBUS_BUS_MCU_SYS_SW_CFG]		= &mcu_sys_mbus_sw_cfg_bus_clk.common.hw,
		[CLK_MBUS_BUS_VIDEO_IN_SW_CFG]		= &video_in_mbus_sw_cfg_bus_clk.common.hw,
		[CLK_MBUS_BUS_CE_SYS_SW_CFG]		= &ce_sys_mbus_sw_cfg_bus_clk.common.hw,
		[CLK_MBUS_BUS_GMAC2_SW_CFG]		= &gmac2_mbus_sw_cfg_bus_clk.common.hw,
		[CLK_MBUS_BUS_GMAC1_SW_CFG]		= &gmac1_mbus_sw_cfg_bus_clk.common.hw,
		[CLK_MBUS_BUS_GMAC0_SW_CFG]		= &gmac0_mbus_sw_cfg_bus_clk.common.hw,
		[CLK_MBUS_BUS_GMAC_SW_CFG]		= &gmac_mbus_sw_cfg_bus_clk.common.hw,
		[CLK_BUS_SPIF_AHB_AUTO_EN]		= &spif_ahb_auto_en_bus_clk.common.hw,
		[CLK_BUS_USB2P0_SYS_AHB_AUTO_EN]	= &usb2p0_sys_ahb_auto_en_bus_clk.common.hw,
		[CLK_BUS_GMAC2_AHB_AUTO_EN]		= &gmac2_ahb_auto_en_bus_clk.common.hw,
		[CLK_BUS_GMAC1_AHB_AUTO_EN]		= &gmac1_ahb_auto_en_bus_clk.common.hw,
		[CLK_BUS_GMAC0_AHB_AUTO_EN]		= &gmac0_ahb_auto_en_bus_clk.common.hw,
		[CLK_BUS_MCU_SYS_AHB_AUTO_EN]		= &mcu_sys_ahb_auto_en_bus_clk.common.hw,
		[CLK_BUS_SMHC3_AHB_AUTO_EN]		= &smhc3_ahb_auto_en_bus_clk.common.hw,
		[CLK_BUS_SMHC2_AHB_AUTO_EN]		= &smhc2_ahb_auto_en_bus_clk.common.hw,
		[CLK_BUS_SMHC1_AHB_AUTO_EN]		= &smhc1_ahb_auto_en_bus_clk.common.hw,
		[CLK_BUS_SMHC0_AHB_AUTO_EN]		= &smhc0_ahb_auto_en_bus_clk.common.hw,
		[CLK_BUS_VIDEO_OUT0_AHB_AUTO_EN]	= &video_out0_ahb_auto_en_bus_clk.common.hw,
		[CLK_BUS_VIDEO_IN_AHB_AUTO_EN]		= &video_in_ahb_auto_en_bus_clk.common.hw,
		[CLK_MBUS_BUS_DMA1_AUTO_EN]		= &dma1_mbus_auto_en_bus_clk.common.hw,
		[CLK_MBUS_BUS_DMA0_AUTO_EN]		= &dma0_mbus_auto_en_bus_clk.common.hw,
		[CLK_MBUS_BUS_LBC_AUTO_EN]		= &lbc_mbus_auto_en_bus_clk.common.hw,
		[CLK_MBUS_BUS_VIDEO_OUT0_AUTO_EN]	= &video_out0_mbus_auto_en_bus_clk.common.hw,
		[CLK_MBUS_BUS_MCU_SYS_AUTO_EN]		= &mcu_sys_mbus_auto_en_bus_clk.common.hw,
		[CLK_MBUS_BUS_VIDEO_IN_AUTO_EN]		= &video_in_mbus_auto_en_bus_clk.common.hw,
		[CLK_MBUS_BUS_CE_SYS_AUTO_EN]		= &ce_sys_mbus_auto_en_bus_clk.common.hw,
		[CLK_MBUS_BUS_GMAC2_AUTO_EN]		= &gmac2_mbus_auto_en_bus_clk.common.hw,
		[CLK_MBUS_BUS_GMAC1_AUTO_EN]		= &gmac1_mbus_auto_en_bus_clk.common.hw,
		[CLK_MBUS_BUS_GMAC0_AUTO_EN]		= &gmac0_mbus_auto_en_bus_clk.common.hw,
		[CLK_MBUS_BUS_GMAC_AUTO_EN]		= &gmac_mbus_auto_en_bus_clk.common.hw,
		[CLK_BUS_DMA0_AHB]			= &dma0_ahb_bus_clk.common.hw,
		[CLK_BUS_DMA1_AHB]			= &dma1_ahb_bus_clk.common.hw,
		[CLK_BUS_SPINLOCK_AHB]			= &spinlock_ahb_bus_clk.common.hw,
		[CLK_BUS_MSGBOX_CPUX_AHB]		= &msgbox_cpux_ahb_bus_clk.common.hw,
		[CLK_BUS_MSGBOX_CORE0_AHB]		= &msgbox_core0_ahb_bus_clk.common.hw,
		[CLK_BUS_MSGBOX_CORE1_AHB]		= &msgbox_core1_ahb_bus_clk.common.hw,
		[CLK_BUS_MSGBOX_CORE2_AHB]		= &msgbox_core2_ahb_bus_clk.common.hw,
		[CLK_BUS_MSGBOX_CORE3_AHB]		= &msgbox_core3_ahb_bus_clk.common.hw,
		[CLK_BUS_MSGBOX_RV_AHB]			= &msgbox_rv_ahb_bus_clk.common.hw,
		[CLK_BUS_PWM2_APB]			= &pwm2_apb_bus_clk.common.hw,
		[CLK_BUS_DCU]				= &dcu_bus_clk.common.hw,
		[CLK_BUS_DAP_AHB]			= &dap_ahb_bus_clk.common.hw,
		[CLK_PWMCS0]				= &pwmcs0_clk.common.hw,
		[CLK_BUS_PWMCS0_APB]			= &pwmcs0_apb_bus_clk.common.hw,
		[CLK_PWMCS1]				= &pwmcs1_clk.common.hw,
		[CLK_BUS_PWMCS1_APB]			= &pwmcs1_apb_bus_clk.common.hw,
		[CLK_TIMER0_0_CLK]			= &timer0_0_clk_clk.common.hw,
		[CLK_TIMER0_1_CLK]			= &timer0_1_clk_clk.common.hw,
		[CLK_TIMER0_2_CLK]			= &timer0_2_clk_clk.common.hw,
		[CLK_TIMER0_3_CLK]			= &timer0_3_clk_clk.common.hw,
		[CLK_TIMER0_4_CLK]			= &timer0_4_clk_clk.common.hw,
		[CLK_TIMER0_5_CLK]			= &timer0_5_clk_clk.common.hw,
		[CLK_TIMER0_6_CLK]			= &timer0_6_clk_clk.common.hw,
		[CLK_TIMER0_7_CLK]			= &timer0_7_clk_clk.common.hw,
		[CLK_BUS_TIMER0_AHB]			= &timer0_ahb_bus_clk.common.hw,
		[CLK_TIMER0_0_RV_CLK]			= &timer0_0_rv_clk_clk.common.hw,
		[CLK_TIMER0_1_RV_CLK]			= &timer0_1_rv_clk_clk.common.hw,
		[CLK_TIMER0_2_RV_CLK]			= &timer0_2_rv_clk_clk.common.hw,
		[CLK_TIMER0_3_RV_CLK]			= &timer0_3_rv_clk_clk.common.hw,
		[CLK_BUS_TIMER0_RV_AHB]			= &timer0_rv_ahb_bus_clk.common.hw,
		[CLK_DE]				= &de_clk.common.hw,
		[CLK_BUS_DE0_AHB]			= &de0_ahb_bus_clk.common.hw,
		[CLK_G2D]				= &g2d_clk.common.hw,
		[CLK_BUS_G2D_AHB]			= &g2d_ahb_bus_clk.common.hw,
		[CLK_CE_SYS]				= &ce_sys_clk.common.hw,
		[CLK_BUS_CE_SYS]			= &ce_sys_bus_clk.common.hw,
		[CLK_BUS_CE_SYS_IP_AHB]			= &ce_sys_ip_ahb_bus_clk.common.hw,
		[CLK_RV_CORE]				= &rv_core_clk.common.hw,
		[CLK_E907_AXI_CLK]			= &e907_axi_clk_clk.common.hw,
		[CLK_RV_TS]				= &rv_ts_clk.common.hw,
		[CLK_BUS_RV_CFG]			= &rv_cfg_bus_clk.common.hw,
		[CLK_BUS_DRAMC_AHB]			= &dramc_ahb_bus_clk.common.hw,
		[CLK_SMHC0]				= &smhc0_clk.common.hw,
		[CLK_BUS_SMHC0_AHB]			= &smhc0_ahb_bus_clk.common.hw,
		[CLK_SMHC1]				= &smhc1_clk.common.hw,
		[CLK_BUS_SMHC1_AHB]			= &smhc1_ahb_bus_clk.common.hw,
		[CLK_SMHC2]				= &smhc2_clk.common.hw,
		[CLK_BUS_SMHC2_AHB]			= &smhc2_ahb_bus_clk.common.hw,
		[CLK_SMHC3]				= &smhc3_clk.common.hw,
		[CLK_BUS_SMHC3_AHB]			= &smhc3_ahb_bus_clk.common.hw,
		[CLK_BUS_UART0_APB]			= &uart0_apb_bus_clk.common.hw,
		[CLK_BUS_UART1_APB]			= &uart1_apb_bus_clk.common.hw,
		[CLK_BUS_UART2_APB]			= &uart2_apb_bus_clk.common.hw,
		[CLK_BUS_UART3_APB]			= &uart3_apb_bus_clk.common.hw,
		[CLK_BUS_UART4_APB]			= &uart4_apb_bus_clk.common.hw,
		[CLK_BUS_UART5_APB]			= &uart5_apb_bus_clk.common.hw,
		[CLK_BUS_UART6_APB]			= &uart6_apb_bus_clk.common.hw,
		[CLK_BUS_UART7_APB]			= &uart7_apb_bus_clk.common.hw,
		[CLK_BUS_UART8_APB]			= &uart8_apb_bus_clk.common.hw,
		[CLK_BUS_UART9_APB]			= &uart9_apb_bus_clk.common.hw,
		[CLK_BUS_TWI0_APB]			= &twi0_apb_bus_clk.common.hw,
		[CLK_BUS_TWI1_APB]			= &twi1_apb_bus_clk.common.hw,
		[CLK_BUS_TWI2_APB]			= &twi2_apb_bus_clk.common.hw,
		[CLK_BUS_TWI3_APB]			= &twi3_apb_bus_clk.common.hw,
		[CLK_BUS_TWI4_APB]			= &twi4_apb_bus_clk.common.hw,
		[CLK_BUS_TWI5_APB]			= &twi5_apb_bus_clk.common.hw,
		[CLK_SPI0]				= &spi0_clk.common.hw,
		[CLK_BUS_SPI0_AHB]			= &spi0_ahb_bus_clk.common.hw,
		[CLK_SPI1]				= &spi1_clk.common.hw,
		[CLK_BUS_SPI1_AHB]			= &spi1_ahb_bus_clk.common.hw,
		[CLK_SPI2]				= &spi2_clk.common.hw,
		[CLK_BUS_SPI2_AHB]			= &spi2_ahb_bus_clk.common.hw,
		[CLK_SPIF]				= &spif_clk.common.hw,
		[CLK_BUS_SPIF_AHB]			= &spif_ahb_bus_clk.common.hw,
		[CLK_SPI3]				= &spi3_clk.common.hw,
		[CLK_BUS_SPI3_AHB]			= &spi3_ahb_bus_clk.common.hw,
		[CLK_GPADC0]				= &gpadc0_clk.common.hw,
		[CLK_BUS_GPADC0_APB]			= &gpadc0_apb_bus_clk.common.hw,
		[CLK_GPADC1]				= &gpadc1_clk.common.hw,
		[CLK_BUS_GPADC1_APB]			= &gpadc1_apb_bus_clk.common.hw,
		[CLK_GPADC2]				= &gpadc2_clk.common.hw,
		[CLK_BUS_GPADC2_APB]			= &gpadc2_apb_bus_clk.common.hw,
		[CLK_GPADC3]				= &gpadc3_clk.common.hw,
		[CLK_BUS_GPADC3_APB]			= &gpadc3_apb_bus_clk.common.hw,
		[CLK_BUS_TSENSOR_APB]			= &tsensor_apb_bus_clk.common.hw,
		[CLK_IR_RX0]				= &ir_rx0_clk.common.hw,
		[CLK_BUS_IR_RX0_APB]			= &ir_rx0_apb_bus_clk.common.hw,
		[CLK_IR_TX]				= &ir_tx_clk.common.hw,
		[CLK_BUS_IR_TX_APB]			= &ir_tx_apb_bus_clk.common.hw,
		[CLK_TPADC]				= &tpadc_clk.common.hw,
		[CLK_BUS_TPADC_APB]			= &tpadc_apb_bus_clk.common.hw,
		[CLK_LBC]				= &lbc_clk.common.hw,
		[CLK_BUS_LBC_AHB]			= &lbc_ahb_bus_clk.common.hw,
		[CLK_IR_RX1]				= &ir_rx1_clk.common.hw,
		[CLK_BUS_IR_RX1_APB]			= &ir_rx1_apb_bus_clk.common.hw,
		[CLK_IR_RX2]				= &ir_rx2_clk.common.hw,
		[CLK_BUS_IR_RX2_APB]			= &ir_rx2_apb_bus_clk.common.hw,
		[CLK_IR_RX3]				= &ir_rx3_clk.common.hw,
		[CLK_BUS_IR_RX3_APB]			= &ir_rx3_apb_bus_clk.common.hw,
		[CLK_I2S0]				= &i2s0_clk.common.hw,
		[CLK_BUS_I2S0_APB]			= &i2s0_apb_bus_clk.common.hw,
		[CLK_I2S1]				= &i2s1_clk.common.hw,
		[CLK_BUS_I2S1_APB]			= &i2s1_apb_bus_clk.common.hw,
		[CLK_I2S2]				= &i2s2_clk.common.hw,
		[CLK_BUS_I2S2_APB]			= &i2s2_apb_bus_clk.common.hw,
		[CLK_OWA0_TX]				= &owa0_tx_clk.common.hw,
		[CLK_OWA0_RX]				= &owa0_rx_clk.common.hw,
		[CLK_BUS_OWA0_APB]			= &owa0_apb_bus_clk.common.hw,
		[CLK_DMIC]				= &dmic_clk.common.hw,
		[CLK_BUS_DMIC_APB]			= &dmic_apb_bus_clk.common.hw,
		[CLK_AUDIOCODEC0_DAC]			= &audiocodec0_dac_clk.common.hw,
		[CLK_BUS_AUDIOCODEC0_APB]		= &audiocodec0_apb_bus_clk.common.hw,
		[CLK_BUS_USB0]				= &usb0_bus_clk.common.hw,
		[CLK_BUS_USB0_DEV_AHB]			= &usb0_dev_ahb_bus_clk.common.hw,
		[CLK_BUS_USB0_EHCI_AHB]			= &usb0_ehci_ahb_bus_clk.common.hw,
		[CLK_BUS_USB0_OHCI_AHB]			= &usb0_ohci_ahb_bus_clk.common.hw,
		[CLK_BUS_USB1]				= &usb1_bus_clk.common.hw,
		[CLK_BUS_USB1_EHCI_AHB]			= &usb1_ehci_ahb_bus_clk.common.hw,
		[CLK_BUS_USB1_OHCI_AHB]			= &usb1_ohci_ahb_bus_clk.common.hw,
		[CLK_BUS_USB2P0_SYS_PHY_REF]		= &usb2p0_sys_phy_ref_bus_clk.common.hw,
		[CLK_BUS_USB2P0_SYS_AHB]		= &usb2p0_sys_ahb_bus_clk.common.hw,
		[CLK_GMAC0_PHY]				= &gmac0_phy_clk.common.hw,
		[CLK_GMAC0_PTP_REF]			= &gmac0_ptp_ref_clk.common.hw,
		[CLK_BUS_GMAC0_AHB]			= &gmac0_ahb_bus_clk.common.hw,
		[CLK_GMAC1_PHY]				= &gmac1_phy_clk.common.hw,
		[CLK_GMAC1_PTP_REF]			= &gmac1_ptp_ref_clk.common.hw,
		[CLK_BUS_GMAC1_AHB]			= &gmac1_ahb_bus_clk.common.hw,
		[CLK_GMAC2_PHY]				= &gmac2_phy_clk.common.hw,
		[CLK_GMAC2_PTP_REF]			= &gmac2_ptp_ref_clk.common.hw,
		[CLK_BUS_GMAC2_AHB]			= &gmac2_ahb_bus_clk.common.hw,
		[CLK_TCON_LCD0]				= &tcon_lcd0_clk.common.hw,
		[CLK_BUS_TCON_LCD0_AHB]			= &tcon_lcd0_ahb_bus_clk.common.hw,
		[CLK_MIPI_DSI0]				= &mipi_dsi0_clk.common.hw,
		[CLK_BUS_MIPI_DSI0_AHB]			= &mipi_dsi0_ahb_bus_clk.common.hw,
		[CLK_COMBOPHY0]				= &combophy0_clk.common.hw,
		[CLK_BUS_VO0_REG_AHB]			= &vo0_reg_ahb_bus_clk.common.hw,
		[CLK_LEDC]				= &ledc_clk.common.hw,
		[CLK_BUS_LEDC_APB]			= &ledc_apb_bus_clk.common.hw,
		[CLK_CSI_MASTER0]			= &csi_master0_clk.common.hw,
		[CLK_CSI_MASTER1]			= &csi_master1_clk.common.hw,
		[CLK_CSI_MASTER2]			= &csi_master2_clk.common.hw,
		[CLK_CSI]				= &csi_clk.common.hw,
		[CLK_ISP]				= &isp_clk.common.hw,
		[CLK_BUS_VIDEO_IN_AHB]			= &video_in_ahb_bus_clk.common.hw,
		[CLK_BUS_RV_AXIMON]			= &rv_aximon_bus_clk.common.hw,
		[CLK_BUS_DCU_AHBMON]			= &dcu_ahbmon_bus_clk.common.hw,
		[CLK_BUS_CPU_SYS_AHBMON]		= &cpu_sys_ahbmon_bus_clk.common.hw,
		[CLK_BUS_CLK50M]			= &clk50m_bus_clk.common.hw,
		[CLK_BUS_CLK25M]			= &clk25m_bus_clk.common.hw,
		[CLK_BUS_CLK16M]			= &clk16m_bus_clk.common.hw,
		[CLK_BUS_CLK12M]			= &clk12m_bus_clk.common.hw,
		[CLK_BUS_CLK24M]			= &clk24m_bus_clk.common.hw,
	},
	.num = CLK_NUMBER,
};
/* ccu_def_end */

static struct ccu_common *sun8iw22_ccu_clks[] = {
	&pll_peri0_clk.common,
	&pll_peri0_2x_clk.common,
	&pll_peri0_800m_clk.common,
	&pll_peri0_480m_clk.common,
	&pll_peri1_clk.common,
	&pll_peri1_2x_clk.common,
	&pll_peri1_800m_clk.common,
	&pll_peri1_480m_clk.common,
	&pll_video0_clk.common,
	&pll_video0_4x_clk.common,
	&pll_video0_3x_clk.common,
	&pll_audio0_clk.common,
	&ahb_clk.common,
	&apb0_clk.common,
	&apb1_clk.common,
	&apb_uart_clk.common,
	&mbus_clk.common,
	&pll_stby_peri0_cfg_bus_clk.common,
	&spif_ahb_sw_cfg_bus_clk.common,
	&usb2p0_sys_ahb_sw_cfg_bus_clk.common,
	&gmac2_ahb_sw_cfg_bus_clk.common,
	&gmac1_ahb_sw_cfg_bus_clk.common,
	&gmac0_ahb_sw_cfg_bus_clk.common,
	&mcu_sys_ahb_sw_cfg_bus_clk.common,
	&smhc3_ahb_sw_cfg_bus_clk.common,
	&smhc2_ahb_sw_cfg_bus_clk.common,
	&smhc1_ahb_sw_cfg_bus_clk.common,
	&smhc0_ahb_sw_cfg_bus_clk.common,
	&video_out0_ahb_sw_cfg_bus_clk.common,
	&video_in_ahb_sw_cfg_bus_clk.common,
	&pll_peri_apb0_sw_cfg_bus_clk.common,
	&pll_peri_ahb_sw_cfg_bus_clk.common,
	&gmac2_mbus_bus_clk.common,
	&gmac1_mbus_bus_clk.common,
	&gmac0_mbus_bus_clk.common,
	&isp_mbus_bus_clk.common,
	&csi_mbus_bus_clk.common,
	&dma1_mbus_bus_clk.common,
	&ce_sys_mbus_bus_clk.common,
	&dma0_mbus_bus_clk.common,
	&dma1_mbus_sw_cfg_bus_clk.common,
	&dma0_mbus_sw_cfg_bus_clk.common,
	&lbc_mbus_sw_cfg_bus_clk.common,
	&video_out0_mbus_sw_cfg_bus_clk.common,
	&mcu_sys_mbus_sw_cfg_bus_clk.common,
	&video_in_mbus_sw_cfg_bus_clk.common,
	&ce_sys_mbus_sw_cfg_bus_clk.common,
	&gmac2_mbus_sw_cfg_bus_clk.common,
	&gmac1_mbus_sw_cfg_bus_clk.common,
	&gmac0_mbus_sw_cfg_bus_clk.common,
	&gmac_mbus_sw_cfg_bus_clk.common,
	&spif_ahb_auto_en_bus_clk.common,
	&usb2p0_sys_ahb_auto_en_bus_clk.common,
	&gmac2_ahb_auto_en_bus_clk.common,
	&gmac1_ahb_auto_en_bus_clk.common,
	&gmac0_ahb_auto_en_bus_clk.common,
	&mcu_sys_ahb_auto_en_bus_clk.common,
	&smhc3_ahb_auto_en_bus_clk.common,
	&smhc2_ahb_auto_en_bus_clk.common,
	&smhc1_ahb_auto_en_bus_clk.common,
	&smhc0_ahb_auto_en_bus_clk.common,
	&video_out0_ahb_auto_en_bus_clk.common,
	&video_in_ahb_auto_en_bus_clk.common,
	&dma1_mbus_auto_en_bus_clk.common,
	&dma0_mbus_auto_en_bus_clk.common,
	&lbc_mbus_auto_en_bus_clk.common,
	&video_out0_mbus_auto_en_bus_clk.common,
	&mcu_sys_mbus_auto_en_bus_clk.common,
	&video_in_mbus_auto_en_bus_clk.common,
	&ce_sys_mbus_auto_en_bus_clk.common,
	&gmac2_mbus_auto_en_bus_clk.common,
	&gmac1_mbus_auto_en_bus_clk.common,
	&gmac0_mbus_auto_en_bus_clk.common,
	&gmac_mbus_auto_en_bus_clk.common,
	&dma0_ahb_bus_clk.common,
	&dma1_ahb_bus_clk.common,
	&spinlock_ahb_bus_clk.common,
	&msgbox_cpux_ahb_bus_clk.common,
	&msgbox_core0_ahb_bus_clk.common,
	&msgbox_core1_ahb_bus_clk.common,
	&msgbox_core2_ahb_bus_clk.common,
	&msgbox_core3_ahb_bus_clk.common,
	&msgbox_rv_ahb_bus_clk.common,
	&pwm2_apb_bus_clk.common,
	&dcu_bus_clk.common,
	&dap_ahb_bus_clk.common,
	&pwmcs0_clk.common,
	&pwmcs0_apb_bus_clk.common,
	&pwmcs1_clk.common,
	&pwmcs1_apb_bus_clk.common,
	&timer0_0_clk_clk.common,
	&timer0_1_clk_clk.common,
	&timer0_2_clk_clk.common,
	&timer0_3_clk_clk.common,
	&timer0_4_clk_clk.common,
	&timer0_5_clk_clk.common,
	&timer0_6_clk_clk.common,
	&timer0_7_clk_clk.common,
	&timer0_ahb_bus_clk.common,
	&timer0_0_rv_clk_clk.common,
	&timer0_1_rv_clk_clk.common,
	&timer0_2_rv_clk_clk.common,
	&timer0_3_rv_clk_clk.common,
	&timer0_rv_ahb_bus_clk.common,
	&de_clk.common,
	&de0_ahb_bus_clk.common,
	&g2d_clk.common,
	&g2d_ahb_bus_clk.common,
	&ce_sys_clk.common,
	&ce_sys_bus_clk.common,
	&ce_sys_ip_ahb_bus_clk.common,
	&rv_core_clk.common,
	&e907_axi_clk_clk.common,
	&rv_ts_clk.common,
	&rv_cfg_bus_clk.common,
	&dramc_ahb_bus_clk.common,
	&smhc0_clk.common,
	&smhc0_ahb_bus_clk.common,
	&smhc1_clk.common,
	&smhc1_ahb_bus_clk.common,
	&smhc2_clk.common,
	&smhc2_ahb_bus_clk.common,
	&smhc3_clk.common,
	&smhc3_ahb_bus_clk.common,
	&uart0_apb_bus_clk.common,
	&uart1_apb_bus_clk.common,
	&uart2_apb_bus_clk.common,
	&uart3_apb_bus_clk.common,
	&uart4_apb_bus_clk.common,
	&uart5_apb_bus_clk.common,
	&uart6_apb_bus_clk.common,
	&uart7_apb_bus_clk.common,
	&uart8_apb_bus_clk.common,
	&uart9_apb_bus_clk.common,
	&twi0_apb_bus_clk.common,
	&twi1_apb_bus_clk.common,
	&twi2_apb_bus_clk.common,
	&twi3_apb_bus_clk.common,
	&twi4_apb_bus_clk.common,
	&twi5_apb_bus_clk.common,
	&spi0_clk.common,
	&spi0_ahb_bus_clk.common,
	&spi1_clk.common,
	&spi1_ahb_bus_clk.common,
	&spi2_clk.common,
	&spi2_ahb_bus_clk.common,
	&spif_clk.common,
	&spif_ahb_bus_clk.common,
	&spi3_clk.common,
	&spi3_ahb_bus_clk.common,
	&gpadc0_clk.common,
	&gpadc0_apb_bus_clk.common,
	&gpadc1_clk.common,
	&gpadc1_apb_bus_clk.common,
	&gpadc2_clk.common,
	&gpadc2_apb_bus_clk.common,
	&gpadc3_clk.common,
	&gpadc3_apb_bus_clk.common,
	&tsensor_apb_bus_clk.common,
	&ir_rx0_clk.common,
	&ir_rx0_apb_bus_clk.common,
	&ir_tx_clk.common,
	&ir_tx_apb_bus_clk.common,
	&tpadc_clk.common,
	&tpadc_apb_bus_clk.common,
	&lbc_clk.common,
	&lbc_ahb_bus_clk.common,
	&ir_rx1_clk.common,
	&ir_rx1_apb_bus_clk.common,
	&ir_rx2_clk.common,
	&ir_rx2_apb_bus_clk.common,
	&ir_rx3_clk.common,
	&ir_rx3_apb_bus_clk.common,
	&i2s0_clk.common,
	&i2s0_apb_bus_clk.common,
	&i2s1_clk.common,
	&i2s1_apb_bus_clk.common,
	&i2s2_clk.common,
	&i2s2_apb_bus_clk.common,
	&owa0_tx_clk.common,
	&owa0_rx_clk.common,
	&owa0_apb_bus_clk.common,
	&dmic_clk.common,
	&dmic_apb_bus_clk.common,
	&audiocodec0_dac_clk.common,
	&audiocodec0_apb_bus_clk.common,
	&usb0_bus_clk.common,
	&usb0_dev_ahb_bus_clk.common,
	&usb0_ehci_ahb_bus_clk.common,
	&usb0_ohci_ahb_bus_clk.common,
	&usb1_bus_clk.common,
	&usb1_ehci_ahb_bus_clk.common,
	&usb1_ohci_ahb_bus_clk.common,
	&usb2p0_sys_phy_ref_bus_clk.common,
	&usb2p0_sys_ahb_bus_clk.common,
	&gmac0_phy_clk.common,
	&gmac0_ptp_ref_clk.common,
	&gmac0_ahb_bus_clk.common,
	&gmac1_phy_clk.common,
	&gmac1_ptp_ref_clk.common,
	&gmac1_ahb_bus_clk.common,
	&gmac2_phy_clk.common,
	&gmac2_ptp_ref_clk.common,
	&gmac2_ahb_bus_clk.common,
	&tcon_lcd0_clk.common,
	&tcon_lcd0_ahb_bus_clk.common,
	&mipi_dsi0_clk.common,
	&mipi_dsi0_ahb_bus_clk.common,
	&combophy0_clk.common,
	&vo0_reg_ahb_bus_clk.common,
	&ledc_clk.common,
	&ledc_apb_bus_clk.common,
	&csi_master0_clk.common,
	&csi_master1_clk.common,
	&csi_master2_clk.common,
	&csi_clk.common,
	&isp_clk.common,
	&video_in_ahb_bus_clk.common,
	&rv_aximon_bus_clk.common,
	&dcu_ahbmon_bus_clk.common,
	&cpu_sys_ahbmon_bus_clk.common,
	&clk50m_bus_clk.common,
	&clk25m_bus_clk.common,
	&clk16m_bus_clk.common,
	&clk12m_bus_clk.common,
	&clk24m_bus_clk.common,
};

static struct ccu_reg_dump sun8iw22_distbus_restore_clks[] = {
	{0x0588, UPD_KEY_VALUE},
};

static const struct sunxi_ccu_desc sun8iw22_ccu_desc = {
	.ccu_clks	= sun8iw22_ccu_clks,
	.num_ccu_clks	= ARRAY_SIZE(sun8iw22_ccu_clks),

	.hw_clks	= &sun8iw22_hw_clks,

	.resets		= sun8iw22_ccu_resets,
	.num_resets	= ARRAY_SIZE(sun8iw22_ccu_resets),
};

static int sun8iw22_ccu_really_probe(struct device_node *node)
{
	void __iomem *reg;
	int ret;

	reg = of_iomap(node, 0);
	if (IS_ERR(reg))
		return PTR_ERR(reg);

	ret = sunxi_parse_sdm_info(node);
	if (ret)
		pr_debug("%s: sdm_info not enabled\n", __func__);

	/* Enable AHB_MONITOR_EN and SD_MONITOR_EN.
	 * When this feature is enabled, it will automatically monitor the traffic of AHB (Advanced High-performance Bus).
	 * If there is no data, it will automatically turn off the clock of the relevant bus decoder,
	 * which helps reduce power consumption.*/
	set_reg(reg + SUN8IW22_AHB_GATE_EN_REG, 0x1, 1, SUN8IW22_AHB_MONITOR_ENABLE);
	set_reg(reg + SUN8IW22_AHB_GATE_EN_REG, 0x1, 1, SUN8IW22_SD_MONITOR_ENABLE);

	/*
	 * 1. enable pll clk gate
	 * 2. enable pll clk auto gate
	 */
	set_reg(reg + SUN8IW22_PLL_PERI0_GATE_EN_REG, 0xfff0fff, 32, 0);
	set_reg(reg + SUN8IW22_PLL_PERI1_GATE_EN_REG, 0x1fff1fff, 32, 0);
	set_reg(reg + SUN8IW22_PLL_VIDEO_GATE_EN_REG, 0x110011, 32, 0);
	set_reg(reg + SUN8IW22_PLL_AUDIO_GATE_EN_REG, 0x10001, 32, 0);

	ret = sunxi_ccu_probe(node, reg, &sun8iw22_ccu_desc);
	if (ret)
		return ret;

	sunxi_ccu_sleep_init(reg, sun8iw22_ccu_clks,
			ARRAY_SIZE(sun8iw22_ccu_clks),
			sun8iw22_distbus_restore_clks,
			ARRAY_SIZE(sun8iw22_distbus_restore_clks));

	return 0;
}

#if IS_ENABLED(CONFIG_AW_KERNEL_ORIGIN)
static void __init of_sun8iw22_ccu_init(struct device_node *node)
{
	sun8iw22_ccu_really_probe(node);
}

CLK_OF_DECLARE(sun8iw22_ccu_init, "allwinner,sun8iw22-ccu", of_sun8iw22_ccu_init);
#else
static int sun8iw22_ccu_probe(struct platform_device *pdev)
{
	struct device_node *node = pdev->dev.of_node;

	return sun8iw22_ccu_really_probe(node);
}

static const struct of_device_id sun8iw22_ccu_ids[] = {
	{ .compatible = "allwinner,sun8iw22-ccu" },
	{ }
};

static struct platform_driver sun8iw22_ccu_driver = {
	.probe	= sun8iw22_ccu_probe,
	.driver	= {
		.name	= "sun8iw22-ccu",
		.of_match_table	= sun8iw22_ccu_ids,
	},
};

static int __init sun8iw22_ccu_init(void)
{
	int err;

	err = platform_driver_register(&sun8iw22_ccu_driver);
	if (err)
		pr_err("register ccu sun8iw22 failed\n");

	return err;
}

core_initcall(sun8iw22_ccu_init);

static void __exit sun8iw22_ccu_exit(void)
{
	platform_driver_unregister(&sun8iw22_ccu_driver);
}
module_exit(sun8iw22_ccu_exit);
#endif

MODULE_DESCRIPTION("Allwinner sun8iw22 clk driver");
MODULE_AUTHOR("haili");
MODULE_LICENSE("GPL v2");
MODULE_VERSION(SUNXI_CCU_VERSION);
