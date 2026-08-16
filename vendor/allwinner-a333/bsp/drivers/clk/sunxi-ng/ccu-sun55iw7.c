// SPDX-License-Identifier: GPL-2.0
/* Copyright(c) 2020 - 2025 Allwinner Technology Co.,Ltd. All rights reserved. */
/*
 * Copyright (c) 2025 haili@allwinnertech.com
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
#include "ccu-sun55iw7.h"

#define SUNXI_CCU_VERSION	"0.0.1"
#define UPD_KEY_VALUE           0x8000000

/* ccu_des_start */
#define SUN55IW7_PLL_PERI0_CTRL_REG   0x00A0
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
		.reg		= SUN55IW7_PLL_PERI0_CTRL_REG,
		.hw.init	= CLK_HW_INIT("pll-peri0", "dcxo24M",
					&ccu_nm_ops,
					CLK_SET_RATE_UNGATE |
					CLK_IS_CRITICAL),
	},
};

static SUNXI_CCU_M(pll_peri0_2x_clk, "pll-peri0-2x",
			"pll-peri0", SUN55IW7_PLL_PERI0_CTRL_REG,
			16, 3, 0);

static SUNXI_CCU_M(pll_peri0_800m_clk, "pll-peri0-800m",
			"pll-peri0", SUN55IW7_PLL_PERI0_CTRL_REG,
			20, 3, 0);

static SUNXI_CCU_M(pll_peri0_480m_clk, "pll-peri0-480m",
			"pll-peri0", SUN55IW7_PLL_PERI0_CTRL_REG,
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

#define SUN55IW7_PLL_PERI1_CTRL_REG   0x00C0
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
		.reg		= SUN55IW7_PLL_PERI1_CTRL_REG,
		.hw.init	= CLK_HW_INIT("pll-peri1", "dcxo24M",
					&ccu_nm_ops,
					CLK_SET_RATE_UNGATE |
					CLK_IS_CRITICAL),
	},
};

static SUNXI_CCU_M(pll_peri1_2x_clk, "pll-peri1-2x",
			"pll-peri1", SUN55IW7_PLL_PERI1_CTRL_REG,
			16, 3, 0);

static SUNXI_CCU_M(pll_peri1_800m_clk, "pll-peri1-800m",
			"pll-peri1", SUN55IW7_PLL_PERI1_CTRL_REG,
			20, 3, 0);

static SUNXI_CCU_M(pll_peri1_480m_clk, "pll-peri1-480m",
			"pll-peri1", SUN55IW7_PLL_PERI1_CTRL_REG,
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

#define SUN55IW7_PLL_GPU_CTRL_REG   0x00E0
static struct ccu_nm pll_gpu_vco_clk = {
	.output			= BIT(27),
	.lock			= BIT(28),
	.lock_enable		= BIT(29),
	.enable			= BIT(31),
	.n			= _SUNXI_CCU_MULT_MIN_MAX(8, 8, 53, 105),
	//.min_rate		= 1272000000,
	//.max_rate		= 2520000000,
	.sdm			= _SUNXI_CCU_SDM_INFO(BIT(24), 0x00E8),
	.common			= {
		.reg		= SUN55IW7_PLL_GPU_CTRL_REG,
		.hw.init	= CLK_HW_INIT("pll-gpu-vco", "dcxo24M",
				&ccu_nm_ops,
				CLK_SET_RATE_UNGATE |
				CLK_IGNORE_UNUSED),
	},
};

static SUNXI_CCU_M(pll_gpu_clk, "pll-gpu",
			"pll-gpu-vco", SUN55IW7_PLL_GPU_CTRL_REG,
			20, 3, CLK_SET_RATE_PARENT);

#define SUN55IW7_PLL_VIDEO0_CTRL_REG   0x0120
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
		.reg		= SUN55IW7_PLL_VIDEO0_CTRL_REG,
		.hw.init	= CLK_HW_INIT("pll-video0", "dcxo24M",
					&ccu_nm_ops,
					CLK_SET_RATE_UNGATE |
					CLK_IS_CRITICAL),
	},
};

static SUNXI_CCU_M(pll_video0_4x_clk, "pll-video0-4x",
			"pll-video0", SUN55IW7_PLL_VIDEO0_CTRL_REG,
			20, 3, CLK_SET_RATE_PARENT);      /* p0 */

static SUNXI_CCU_M(pll_video0_3x_clk, "pll-video0-3x",
			"pll-video0", SUN55IW7_PLL_VIDEO0_CTRL_REG,
			16, 3, CLK_SET_RATE_PARENT);      /* p0 */

#define SUN55IW7_PLL_VIDEO1_CTRL_REG   0x0140
static struct ccu_nm pll_video1_clk = {
	.output			= BIT(27),
	.lock			= BIT(28),
	.lock_enable		= BIT(29),
	.enable			= BIT(31),
	.n			= _SUNXI_CCU_MULT_MIN_MAX(8, 8, 52, 105),
	.min_rate		= 1248000000,
	.max_rate		= 2520000000UL,
	.sdm			= _SUNXI_CCU_SDM_INFO(BIT(24), 0x0128),
	.common			= {
		.reg		= SUN55IW7_PLL_VIDEO1_CTRL_REG,
		.hw.init	= CLK_HW_INIT("pll-video1", "dcxo24M",
					&ccu_nm_ops,
					CLK_SET_RATE_UNGATE |
					CLK_IS_CRITICAL),
	},
};

static SUNXI_CCU_M(pll_video1_4x_clk, "pll-video1-4x",
			"pll-video1", SUN55IW7_PLL_VIDEO1_CTRL_REG,
			20, 3, CLK_SET_RATE_PARENT);      /* p0 */

static SUNXI_CCU_M(pll_video1_3x_clk, "pll-video1-3x",
			"pll-video1", SUN55IW7_PLL_VIDEO1_CTRL_REG,
			16, 3, CLK_SET_RATE_PARENT);      /* p0 */

#define SUN55IW7_PLL_VE_CTRL_REG   0x0220
static struct ccu_nm pll_ve_vco_clk = {
	.output			= BIT(27),
	.lock			= BIT(28),
	.lock_enable		= BIT(29),
	.enable			= BIT(31),
	.n			= _SUNXI_CCU_MULT_MIN_MAX(8, 8, 53, 105),
	//.min_rate		= 1272000000,
	//.max_rate		= 2520000000,
	.sdm			= _SUNXI_CCU_SDM_INFO(BIT(24), 0x0228),
	.common			= {
		.reg		= SUN55IW7_PLL_VE_CTRL_REG,
		.hw.init	= CLK_HW_INIT("pll-ve-vco", "dcxo24M",
				&ccu_nm_ops,
				CLK_SET_RATE_UNGATE),
	},
};

static SUNXI_CCU_M(pll_ve_clk, "pll-ve",
			"pll-ve-vco", SUN55IW7_PLL_VE_CTRL_REG,
			20, 3, CLK_SET_RATE_PARENT);

#define SUN55IW7_PLL_ADC_CTRL_REG   0x0240
static struct ccu_nm pll_adc_vco_clk = {
	.output			= BIT(27),
	.lock			= BIT(28),
	.lock_enable		= BIT(29),
	.enable			= BIT(31),
	.n			= _SUNXI_CCU_MULT_MIN_MAX(8, 8, 12, 20),
	//.min_rate		= ,
	//.max_rate		= ,
	.common			= {
		.reg		= SUN55IW7_PLL_ADC_CTRL_REG,
		.hw.init	= CLK_HW_INIT("pll-adc-vco", "dcxo24M",
					&ccu_nm_ops,
					0),
	},
};

static SUNXI_CCU_M(pll_adc_clk, "pll-adc",
			"pll-adc-vco", SUN55IW7_PLL_ADC_CTRL_REG,
			20, 3, CLK_SET_RATE_PARENT);

#define SUN55IW7_PLL_AUDIO0_CTRL_REG   0x0260
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
		.reg		= SUN55IW7_PLL_AUDIO0_CTRL_REG,
		.features	= CCU_FEATURE_SIGMA_DELTA_MOD,
		.hw.init	= CLK_HW_INIT("pll-audio0", "dcxo24M",
					&ccu_nm_ops,
					CLK_SET_RATE_UNGATE |
					CLK_IS_CRITICAL),
	},
};

static const char * const ahb_parents[] = { "dcxo24M", "ext-32k", "rc_16m", "pll-peri0-600m-bus" };

static SUNXI_CCU_M_WITH_MUX(ahb_clk, "ahb", ahb_parents,
			0x0500, 0, 5, 24, 2, 0);

static const char * const apb0_parents[] = { "dcxo24M", "ext-32k", "rc_16m", "pll-peri0-600m-bus" };

static SUNXI_CCU_M_WITH_MUX(apb0_clk, "apb0", apb0_parents,
			0x0510, 0, 5, 24, 2, 0);

static const char * const apb1_parents[] = { "dcxo24M", "ext-32k", "rc_16m", "pll-peri0-600m-bus" };

static SUNXI_CCU_M_WITH_MUX(apb1_clk, "apb1", apb1_parents,
			0x0518, 0, 5, 24, 2, 0);

static const char * const apb_uart_parents[] = { "dcxo24M", "ext-32k", "rc_16m", "pll-peri0-600m-bus", "pll-peri0-480m-bus" };

static SUNXI_CCU_M_WITH_MUX(apb_uart_clk, "apb-uart", apb_uart_parents,
			0x0538, 0, 5, 24, 3, 0);

static const char * const cpux_gic_parents[] = { "dcxo24M", "ext-32k", "rc_16m", "pll-peri0-600m", "pll-peri0-480m", "pll-peri0-300m" };

/*Noting: This formula maybe need to modify*/
static SUNXI_CCU_M_WITH_MUX_GATE(cpux_gic_clk, "cpux-gic",
			cpux_gic_parents, 0x0560,
			0, 5,  /* M */
			24, 3,  /* mux */
			BIT(31),	/* gate */
			0);

static const char * const nsi_parents[] = { "dcxo24M", "pll-video1-3x", "pll-peri0-600m-bus", "pll-peri0-480m", "pll-peri0-400m", "hdr-clk" };

static SUNXI_CCU_M_WITH_MUX_GATE(nsi_clk, "nsi",
			nsi_parents, 0x0580,
			0, 5,  /* M */
			24, 3,  /* mux */
			BIT(31),	/* gate */
			0);

static SUNXI_CCU_GATE(nsi_cfg_bus_clk, "nsi-cfg-bus",
			"dcxo24M",
			0x0584, BIT(0), 0);

static const char * const mbus_parents[] = { "dcxo24M", "pll-peri0-600m-bus", "pll-peri0-480m", "pll-peri0-400m", "hdr-clk" };

static SUNXI_CCU_M_WITH_MUX_GATE_KEY(mbus_clk, "mbus",
			mbus_parents, 0x0588,
			0, 5,  /* M */
			24, 3,  /* mux */
			UPD_KEY_VALUE,
			BIT(31),	/* gate */
			0);

static SUNXI_CCU_GATE(iommu_apb_bus_clk, "iommu-apb-bus",
			"dcxo24M",
			0x058C, BIT(0), 0);

static SUNXI_CCU_GATE(pll_stby_peri0_clk_sw_cfg_bus_clk, "pll-stby-peri0-clk-sw-cfg-bus",
			"dcxo24M",
			0x05C0, BIT(28), 0);

static SUNXI_CCU_GATE(smhc2_ahb_sw_cfg_bus_clk, "smhc2-ahb-sw-cfg-bus",
			"dcxo24M",
			0x05C0, BIT(15), 0);

static SUNXI_CCU_GATE(smhc1_ahb_sw_cfg_bus_clk, "smhc1-ahb-sw-cfg-bus",
			"dcxo24M",
			0x05C0, BIT(14), 0);

static SUNXI_CCU_GATE(smhc0_ahb_sw_cfg_bus_clk, "smhc0-ahb-sw-cfg-bus",
			"dcxo24M",
			0x05C0, BIT(13), 0);

static SUNXI_CCU_GATE(hsi_ahb_sw_cfg_bus_clk, "hsi-ahb-sw-cfg-bus",
			"dcxo24M",
			0x05C0, BIT(11), 0);

static SUNXI_CCU_GATE(secure_sys_ahb_sw_cfg_bus_clk, "secure-sys-ahb-sw-cfg-bus",
			"dcxo24M",
			0x05C0, BIT(8), 0);

static SUNXI_CCU_GATE(gpu_ahb_sw_cfg_bus_clk, "gpu-ahb-sw-cfg-bus",
			"dcxo24M",
			0x05C0, BIT(7), 0);

static SUNXI_CCU_GATE(video_out0_ahb_sw_cfg_bus_clk, "video-out0-ahb-sw-cfg-bus",
			"dcxo24M",
			0x05C0, BIT(3), 0);

static SUNXI_CCU_GATE(video_in_ahb_sw_cfg_bus_clk, "video-in-ahb-sw-cfg-bus",
			"dcxo24M",
			0x05C0, BIT(2), 0);

static SUNXI_CCU_GATE(ve0_ahb_sw_cfg_bus_clk, "ve0-ahb-sw-cfg-bus",
			"dcxo24M",
			0x05C0, BIT(1), 0);

static SUNXI_CCU_GATE(pll_peri_apb0_sw_cfg_bus_clk, "pll-peri-apb0-sw-cfg-bus",
			"dcxo24M",
			0x05D0, BIT(16), 0);

static SUNXI_CCU_GATE(pll_peri_ahb_sw_cfg_bus_clk, "pll-peri-ahb-sw-cfg-bus",
			"dcxo24M",
			0x05D0, BIT(0), 0);

static SUNXI_CCU_GATE(dma0_mbus_sw_cfg_bus_clk, "dma0-mbus-sw-cfg-bus",
			"dcxo24M",
			0x05E0, BIT(28), 0);

static SUNXI_CCU_GATE(ce_sys_axi_sw_cfg_bus_clk, "ce-sys-axi-sw-cfg-bus",
			"dcxo24M",
			0x05E0, BIT(8), 0);

static SUNXI_CCU_GATE(gpu_axi_sw_cfg_bus_clk, "gpu-axi-sw-cfg-bus",
			"dcxo24M",
			0x05E0, BIT(7), 0);

static SUNXI_CCU_GATE(de_sys_mbus_sw_cfg_bus_clk, "de-sys-mbus-sw-cfg-bus",
			"dcxo24M",
			0x05E0, BIT(5), 0);

static SUNXI_CCU_GATE(video_in_mbus_sw_cfg_bus_clk, "video-in-mbus-sw-cfg-bus",
			"dcxo24M",
			0x05E0, BIT(2), 0);

static SUNXI_CCU_GATE(ve0_mbus_sw_cfg_bus_clk, "ve0-mbus-sw-cfg-bus",
			"dcxo24M",
			0x05E0, BIT(1), 0);

static SUNXI_CCU_GATE(gmac0_axi_bus_clk, "gmac0-axi-bus",
			"dcxo24M",
			0x05E4, BIT(11), 0);

static SUNXI_CCU_GATE(iommu_mbus_bus_clk, "iommu-mbus-bus",
			"dcxo24M",
			0x05E4, BIT(4), 0);

static SUNXI_CCU_GATE(video_out_mbus_bus_clk, "video-out-mbus-bus",
			"dcxo24M",
			0x05E4, BIT(3), 0);

static SUNXI_CCU_GATE(ce_sys_axi_bus_clk, "ce-sys-axi-bus",
			"dcxo24M",
			0x05E4, BIT(2), 0);

static SUNXI_CCU_GATE(ve0_mbus_bus_clk, "ve0-mbus-bus",
			"dcxo24M",
			0x05E4, BIT(1), 0);

static SUNXI_CCU_GATE(dma0_mbus_bus_clk, "dma0-mbus-bus",
			"dcxo24M",
			0x05E4, BIT(0), 0);

static SUNXI_CCU_GATE(smhc2_ahb_auto_en_bus_clk, "smhc2-ahb-auto-en-bus",
			"dcxo24M",
			0x05F0, BIT(15), 0);

static SUNXI_CCU_GATE(smhc1_ahb_auto_en_bus_clk, "smhc1-ahb-auto-en-bus",
			"dcxo24M",
			0x05F0, BIT(14), 0);

static SUNXI_CCU_GATE(smhc0_ahb_auto_en_bus_clk, "smhc0-ahb-auto-en-bus",
			"dcxo24M",
			0x05F0, BIT(13), 0);

static SUNXI_CCU_GATE(hsi_ahb_auto_en_bus_clk, "hsi-ahb-auto-en-bus",
			"dcxo24M",
			0x05F0, BIT(11), 0);

static SUNXI_CCU_GATE(secure_sys_ahb_auto_en_bus_clk, "secure-sys-ahb-auto-en-bus",
			"dcxo24M",
			0x05F0, BIT(8), 0);

static SUNXI_CCU_GATE(gpu_ahb_auto_en_bus_clk, "gpu-ahb-auto-en-bus",
			"dcxo24M",
			0x05F0, BIT(7), 0);

static SUNXI_CCU_GATE(video_out0_ahb_auto_en_bus_clk, "video-out0-ahb-auto-en-bus",
			"dcxo24M",
			0x05F0, BIT(3), 0);

static SUNXI_CCU_GATE(video_in_ahb_auto_en_bus_clk, "video-in-ahb-auto-en-bus",
			"dcxo24M",
			0x05F0, BIT(2), 0);

static SUNXI_CCU_GATE(ve0_ahb_auto_en_bus_clk, "ve0-ahb-auto-en-bus",
			"dcxo24M",
			0x05F0, BIT(1), 0);

static SUNXI_CCU_GATE(dma0_mbus_auto_en_bus_clk, "dma0-mbus-auto-en-bus",
			"dcxo24M",
			0x05F4, BIT(28), 0);

static SUNXI_CCU_GATE(ce_sys_axi_auto_en_bus_clk, "ce-sys-axi-auto-en-bus",
			"dcxo24M",
			0x05F4, BIT(8), 0);

static SUNXI_CCU_GATE(gpu_axi_auto_en_bus_clk, "gpu-axi-auto-en-bus",
			"dcxo24M",
			0x05F4, BIT(7), 0);

static SUNXI_CCU_GATE(de_sys_mbus_auto_en_bus_clk, "de-sys-mbus-auto-en-bus",
			"dcxo24M",
			0x05F4, BIT(5), 0);

static SUNXI_CCU_GATE(video_in_mbus_auto_en_bus_clk, "video-in-mbus-auto-en-bus",
			"dcxo24M",
			0x05F4, BIT(2), 0);

static SUNXI_CCU_GATE(ve0_mbus_auto_en_bus_clk, "ve0-mbus-auto-en-bus",
			"dcxo24M",
			0x05F4, BIT(1), 0);

static SUNXI_CCU_GATE(dma0_ahb_bus_clk, "dma0-ahb-bus",
			"dcxo24M",
			0x0704, BIT(0), 0);

static SUNXI_CCU_GATE(spinlock_ahb_bus_clk, "spinlock-ahb-bus",
			"dcxo24M",
			0x0724, BIT(0), 0);

static SUNXI_CCU_GATE(msgbox_cpux_ahb_bus_clk, "msgbox-cpux-ahb-bus",
			"dcxo24M",
			0x0744, BIT(0), 0);

static SUNXI_CCU_GATE(msgbox_cpus_ahb_bus_clk, "msgbox-cpus-ahb-bus",
			"dcxo24M",
			0x074C, BIT(0), 0);

static SUNXI_CCU_GATE(smcard_apb_bus_clk, "smcard-apb-bus",
			"dcxo24M",
			0x0770, BIT(0), 0);

static SUNXI_CCU_GATE(pwm0_apb_bus_clk, "pwm0-apb-bus",
			"dcxo24M",
			0x0784, BIT(0), 0);

static SUNXI_CCU_GATE(dcu_bus_clk, "dcu-bus",
			"dcxo24M",
			0x07A4, BIT(0), 0);

static SUNXI_CCU_GATE(dap_ahb_bus_clk, "dap-ahb-bus",
			"dcxo24M",
			0x07AC, BIT(0), 0);

static SUNXI_CCU_GATE(pll_cec_peri_bus_clk, "pll-cec-peri-bus",
			"dcxo24M",
			0x07B0, BIT(0), 0);

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

static SUNXI_CCU_GATE(timer0_ahb_bus_clk, "timer0-ahb-bus",
			"dcxo24M",
			0x0850, BIT(0), 0);

static const char * const de_parents[] = { "pll-video1-3x", "pll-video0-3x", "pll-adc", "pll-gpu", "pll-ve", "pll-peri0-600m", "pll-peri0-480m", "pll-peri0-400m" };

static SUNXI_CCU_M_WITH_MUX_GATE(de_clk, "de",
			de_parents, 0x0A00,
			0, 5,  /* M */
			24, 3,  /* mux */
			BIT(31),	/* gate */
			0);

static SUNXI_CCU_GATE(de0_ahb_bus_clk, "de0-ahb-bus",
			"dcxo24M",
			0x0A04, BIT(0), 0);

static const char * const vep_parents[] = { "pll-video1-3x", "pll-video0-3x", "pll-adc", "pll-gpu", "pll-ve", "pll-peri0-600m", "pll-peri0-480m", "pll-peri0-400m" };

static SUNXI_CCU_M_WITH_MUX_GATE(vep_clk, "vep",
			vep_parents, 0x0A10,
			0, 5,  /* M */
			24, 3,  /* mux */
			BIT(31),	/* gate */
			0);

static SUNXI_CCU_GATE(vep_ahb_bus_clk, "vep-ahb-bus",
			"dcxo24M",
			0x0A14, BIT(0), 0);

static const char * const ve0_parents[] = { "pll-ve", "pll-peri0-600m", "pll-peri0-480m", "pll-peri0-400m", "pll-peri0-300m" };

static SUNXI_CCU_M_WITH_MUX_GATE(ve0_clk, "ve0",
			ve0_parents, 0x0A80,
			0, 5,  /* M */
			24, 3,  /* mux */
			BIT(31),	/* gate */
			0);

static SUNXI_CCU_GATE(ve0_ahb_bus_clk, "ve0-ahb-bus",
			"dcxo24M",
			0x0A8C, BIT(0), 0);

static const char * const ce_sys_parents[] = { "dcxo24M", "pll-peri0-600m", "pll-peri0-480m", "pll-peri0-400m" };

static SUNXI_CCU_M_WITH_MUX_GATE(ce_sys_clk, "ce-sys",
			ce_sys_parents, 0x0AC0,
			0, 5,  /* M */
			24, 3,  /* mux */
			BIT(31),	/* gate */
			0);

static SUNXI_CCU_GATE(ce_keysram_bus_clk, "ce-keysram-bus",
			"dcxo24M",
			0x0AC4, BIT(1), 0);

static SUNXI_CCU_GATE(ce_sys_ip_ahb_bus_clk, "ce-sys-ip-ahb-bus",
			"dcxo24M",
			0x0AC4, BIT(0), 0);

static SUNXI_CCU_GATE(gpu_ahb_bus_clk, "gpu-ahb-bus",
			"dcxo24M",
			0x0B24, BIT(0), 0);

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

static SUNXI_CCU_GATE(twi0_apb_bus_clk, "twi0-apb-bus",
			"apb1",
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

static const char * const i2s0_parents[] = { "pll-audio0", "pll-peri1-600m" };

static SUNXI_CCU_M_WITH_MUX_GATE(i2s0_clk, "i2s0",
			i2s0_parents, 0x1200,
			0, 5,  /* M */
			24, 3,  /* mux */
			BIT(31),	/* gate */
			0);

static SUNXI_CCU_GATE(i2s0_apb_bus_clk, "i2s0-apb-bus",
			"dcxo24M",
			0x120C, BIT(0), 0);

static const char * const i2s1_parents[] = { "pll-audio0", "pll-peri1-600m" };

static SUNXI_CCU_M_WITH_MUX_GATE(i2s1_clk, "i2s1",
			i2s1_parents, 0x1210,
			0, 5,  /* M */
			24, 3,  /* mux */
			BIT(31),	/* gate */
			0);

static SUNXI_CCU_GATE(i2s1_apb_bus_clk, "i2s1-apb-bus",
			"dcxo24M",
			0x121C, BIT(0), 0);

static const char * const i2s2_parents[] = { "pll-audio0", "pll-peri1-600m" };

static SUNXI_CCU_M_WITH_MUX_GATE(i2s2_clk, "i2s2",
			i2s2_parents, 0x1220,
			0, 5,  /* M */
			24, 3,  /* mux */
			BIT(31),	/* gate */
			0);

static SUNXI_CCU_GATE(i2s2_apb_bus_clk, "i2s2-apb-bus",
			"dcxo24M",
			0x122C, BIT(0), 0);

static const char * const i2s3_parents[] = { "pll-audio0", "pll-peri1-600m" };

static SUNXI_CCU_M_WITH_MUX_GATE(i2s3_clk, "i2s3",
			i2s3_parents, 0x1230,
			0, 5,  /* M */
			24, 3,  /* mux */
			BIT(31),	/* gate */
			0);

static SUNXI_CCU_GATE(i2s3_apb_bus_clk, "i2s3-apb-bus",
			"dcxo24M",
			0x123C, BIT(0), 0);

static const char * const i2s4_parents[] = { "pll-audio0", "pll-peri1-600m" };

static SUNXI_CCU_M_WITH_MUX_GATE(i2s4_clk, "i2s4",
			i2s4_parents, 0x1240,
			0, 5,  /* M */
			24, 3,  /* mux */
			BIT(31),	/* gate */
			0);

static SUNXI_CCU_GATE(i2s4_apb_bus_clk, "i2s4-apb-bus",
			"dcxo24M",
			0x124C, BIT(0), 0);

static const char * const owa0_tx_parents[] = { "pll-audio0", "pll-peri1-600m" };

static SUNXI_CCU_M_WITH_MUX_GATE(owa0_tx_clk, "owa0-tx",
			owa0_tx_parents, 0x1280,
			0, 5,  /* M */
			24, 3,  /* mux */
			BIT(31),	/* gate */
			0);

static const char * const owa0_rx_parents[] = { "pll-peri0-400m", "pll-peri0-300m", "pll-audio0" };

static SUNXI_CCU_M_WITH_MUX_GATE(owa0_rx_clk, "owa0-rx",
			owa0_rx_parents, 0x1284,
			0, 5,  /* M */
			24, 3,  /* mux */
			BIT(31),	/* gate */
			0);

static SUNXI_CCU_GATE(owa0_apb_bus_clk, "owa0-apb-bus",
			"dcxo24M",
			0x128C, BIT(0), 0);

static const char * const owa1_tx_parents[] = { "pll-audio0", "pll-peri1-600m" };

static SUNXI_CCU_M_WITH_MUX_GATE(owa1_tx_clk, "owa1-tx",
			owa1_tx_parents, 0x12A0,
			0, 5,  /* M */
			24, 3,  /* mux */
			BIT(31),	/* gate */
			0);

static const char * const owa1_rx_parents[] = { "pll-peri0-400m", "pll-peri0-300m", "pll-audio0" };

static SUNXI_CCU_M_WITH_MUX_GATE(owa1_rx_clk, "owa1-rx",
			owa1_rx_parents, 0x12A4,
			0, 5,  /* M */
			24, 3,  /* mux */
			BIT(31),	/* gate */
			0);

static SUNXI_CCU_GATE(owa1_apb_bus_clk, "owa1-apb-bus",
			"dcxo24M",
			0x12AC, BIT(0), 0);

static const char * const audiocodec0_dac_parents[] = { "pll-audio0", "pll-peri1-600m" };

static SUNXI_CCU_M_WITH_MUX_GATE(audiocodec0_dac_clk, "audiocodec0-dac",
			audiocodec0_dac_parents, 0x12E0,
			0, 5,  /* M */
			24, 3,  /* mux */
			BIT(31),	/* gate */
			0);

static const char * const audiocodec0_adc_parents[] = { "pll-audio0", "pll-peri1-600m" };

static SUNXI_CCU_M_WITH_MUX_GATE(audiocodec0_adc_clk, "audiocodec0-adc",
			audiocodec0_adc_parents, 0x12E8,
			0, 5,  /* M */
			24, 3,  /* mux */
			BIT(31),	/* gate */
			0);

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

static SUNXI_CCU_GATE(usb2_u2_phy_ref_bus_clk, "usb2-u2-phy-ref-bus",
			"dcxo24M",
			0x1348, BIT(31), 0);

static const char * const usb2_suspend_parents[] = { "ext-32k", "dcxo24M" };

static SUNXI_CCU_M_WITH_MUX_GATE(usb2_suspend_clk, "usb2-suspend",
			usb2_suspend_parents, 0x1350,
			0, 5,  /* M */
			24, 1,  /* mux */
			BIT(31),	/* gate */
			0);

static const char * const usb2_ref_parents[] = { "dcxo24M", "pll-peri0-300m" };

static SUNXI_CCU_M_WITH_MUX_GATE(usb2_ref_clk, "usb2-ref",
			usb2_ref_parents, 0x1354,
			0, 5,  /* M */
			24, 3,  /* mux */
			BIT(31),	/* gate */
			0);

static const char * const usb2_u3_only_utmi_parents[] = { "dcxo24M", "pll-peri0-300m" };

static SUNXI_CCU_M_WITH_MUX_GATE(usb2_u3_only_utmi_clk, "usb2-u3-only-utmi",
			usb2_u3_only_utmi_parents, 0x1360,
			0, 5,  /* M */
			24, 3,  /* mux */
			BIT(31),	/* gate */
			0);

static const char * const usb2_u2_only_pipe_parents[] = { "dcxo24M", "pll-peri0-480m" };

static SUNXI_CCU_M_WITH_MUX_GATE(usb2_u2_only_pipe_clk, "usb2-u2-only-pipe",
			usb2_u2_only_pipe_parents, 0x1364,
			0, 5,  /* M */
			24, 3,  /* mux */
			BIT(31),	/* gate */
			0);

static const char * const hsi_comb0_phy_cfg_parents[] = { "pll-peri0-600m", "pll-peri0-400m" };

static SUNXI_CCU_M_WITH_MUX_GATE(hsi_comb0_phy_cfg_clk, "hsi-comb0-phy-cfg",
			hsi_comb0_phy_cfg_parents, 0x13C0,
			0, 5,  /* M */
			24, 1,  /* mux */
			BIT(31),	/* gate */
			0);

static const char * const hsi_comb0_phy_ref_parents[] = { "dcxo24M", "pll-peri0-200m" };

static SUNXI_CCU_M_WITH_MUX_GATE(hsi_comb0_phy_ref_clk, "hsi-comb0-phy-ref",
			hsi_comb0_phy_ref_parents, 0x13C4,
			0, 5,  /* M */
			24, 3,  /* mux */
			BIT(31),	/* gate */
			0);

static SUNXI_CCU_GATE(hsi_axi_bus_clk, "hsi-axi-bus",
			"dcxo24M",
			0x13CC, BIT(1), 0);

static SUNXI_CCU_GATE(hsi_ahb_bus_clk, "hsi-ahb-bus",
			"dcxo24M",
			0x13CC, BIT(0), 0);

static const char * const hsi_axi_parents[] = { "dcxo24M", "pll-peri0-600m-bus", "pll-peri0-480m", "pll-peri0-400m" };

static SUNXI_CCU_M_WITH_MUX_GATE(hsi_axi_clk, "hsi-axi",
			hsi_axi_parents, 0x13E0,
			0, 5,  /* M */
			24, 3,  /* mux */
			BIT(31),	/* gate */
			0);

static SUNXI_CCU_M_WITH_GATE(gmac0_phy_clk, "gmac0-phy",
			"pll-peri0-150m", 0x1400, 0,
			5, BIT(31), 0);

static SUNXI_CCU_GATE(gmac0_ahb_bus_clk, "gmac0-ahb-bus",
			"dcxo24M",
			0x140C, BIT(0), 0);

static const char * const tcon_lcd0_parents[] = { "pll-video0-4x", "pll-video1-4x", "pll-peri0-2x", "pll-peri1-2x", "pll-video0-3x", "pll-video1-3x" };

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

static const char * const combophy0_parents[] = { "pll-video0-4x", "pll-video1-4x", "pll-peri0-2x", "pll-peri1-2x", "pll-video0-3x", "pll-video1-3x" };

static SUNXI_CCU_M_WITH_MUX_GATE(combophy0_clk, "combophy0",
			combophy0_parents, 0x15C0,
			0, 5,  /* M */
			24, 3,  /* mux */
			BIT(31),	/* gate */
			0);

static const char * const tcon_tv0_parents[] = { "pll-peri0-600m", "pll-video0-3x", "pll-video0-4x", "pll-video1-3x", "pll-video1-4x" };

static SUNXI_CCU_M_WITH_MUX_GATE(tcon_tv0_clk, "tcon-tv0",
			tcon_tv0_parents, 0x1600,
			0, 5,  /* M */
			24, 3,  /* mux */
			BIT(31),	/* gate */
			0);

static SUNXI_CCU_GATE(tcon_tv0_ahb_bus_clk, "tcon-tv0-ahb-bus",
			"dcxo24M",
			0x1604, BIT(0), 0);

static const char * const tve_parents[] = { "pll-adc", "pll-video0-3x", "pll-video0-4x", "pll-video1-3x", "pll-video1-4x", "pll-peri0-300m" };

static SUNXI_CCU_M_WITH_MUX_GATE(tve_clk, "tve",
			tve_parents, 0x1620,
			0, 5,  /* M */
			24, 3,  /* mux */
			BIT(31),	/* gate */
			0);

static SUNXI_CCU_GATE(tve_ahb_bus_clk, "tve-ahb-bus",
			"dcxo24M",
			0x1624, BIT(0), 0);

static const char * const tcon_tv1_parents[] = { "pll-peri0-600m", "pll-video0-3x", "pll-video0-4x", "pll-video1-3x", "pll-video1-4x" };

static SUNXI_CCU_M_WITH_MUX_GATE(tcon_tv1_clk, "tcon-tv1",
			tcon_tv1_parents, 0x1630,
			0, 5,  /* M */
			24, 3,  /* mux */
			BIT(31),	/* gate */
			0);

static SUNXI_CCU_GATE(tcon_tv1_ahb_bus_clk, "tcon-tv1-ahb-bus",
			"dcxo24M",
			0x1634, BIT(0), 0);

static SUNXI_CCU_GATE(hdmi_ref_bus_clk, "hdmi-ref-bus",
			"dcxo24M",
			0x1680, BIT(31), 0);

static const char * const hdmi_prep_parents[] = { "pll-video0-4x", "pll-peri0-300m" };

static SUNXI_CCU_M_WITH_MUX_GATE(hdmi_prep_clk, "hdmi-prep",
			hdmi_prep_parents, 0x1684,
			0, 5,  /* M */
			24, 3,  /* mux */
			BIT(31),	/* gate */
			0);

static const char * const hdmi_linkqpclk_parents[] = { "pll-video0-4x", "pll-peri0-300m" };

static SUNXI_CCU_M_WITH_MUX_GATE(hdmi_linkqpclk_clk, "hdmi-linkqpclk",
			hdmi_linkqpclk_parents, 0x1688,
			0, 5,  /* M */
			24, 3,  /* mux */
			BIT(31),	/* gate */
			0);

static const char * const hdmi_pixel_parents[] = { "pll-video0-4x", "pll-peri0-300m" };

static SUNXI_CCU_M_WITH_MUX_GATE(hdmi_pixel_clk, "hdmi-pixel",
			hdmi_pixel_parents, 0x168C,
			0, 5,  /* M */
			24, 3,  /* mux */
			BIT(31),	/* gate */
			0);

static SUNXI_CCU_GATE(hdmi_ahb_bus_clk, "hdmi-ahb-bus",
			"dcxo24M",
			0x1690, BIT(0), 0);

static SUNXI_CCU_GATE(vo0_reg_ahb_bus_clk, "vo0-reg-ahb-bus",
			"dcxo24M",
			0x16C4, BIT(0), 0);

static SUNXI_CCU_GATE(vo1_reg_ahb_bus_clk, "vo1-reg-ahb-bus",
			"dcxo24M",
			0x16D4, BIT(0), 0);

static SUNXI_CCU_GATE(video_out0_ahb_bus_clk, "video-out0-ahb-bus",
			"dcxo24M",
			0x16E4, BIT(0), 0);

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

static const char * const csi_master0_parents[] = { "dcxo24M", "pll-video1-4x", "pll-video1-3x", "pll-video0-4x", "pll-video0-3x", "pll-peri1-2x", "pll-peri1-480m" };

static SUNXI_CCU_MP_WITH_MUX_GATE_NO_INDEX(csi_master0_clk, "csi-master0",
			csi_master0_parents, 0x1800,
			0, 5,  /* M */
			8, 5,  /* N */
			24, 3,  /* mux */
			BIT(31), 0);

static const char * const csi_parents[] = { "pll-peri0-300m", "pll-peri0-400m", "pll-peri0-480m", "pll-video0-4x", "pll-video0-3x", "pll-peri1-2x", "pll-peri1-480m" };

static SUNXI_CCU_M_WITH_MUX_GATE(csi_clk, "csi",
			csi_parents, 0x1810,
			0, 5,  /* M */
			24, 3,  /* mux */
			BIT(31),	/* gate */
			0);

static const char * const hrc_parents[] = { "pll-video1-3x", "pll-video0-3x", "pll-adc", "pll-gpu", "pll-ve", "pll-peri0-600m", "pll-peri0-480m", "pll-peri0-400m" };

static SUNXI_CCU_M_WITH_MUX_GATE(hrc_clk, "hrc",
			hrc_parents, 0x1820,
			0, 5,  /* M */
			24, 3,  /* mux */
			BIT(31),	/* gate */
			0);

static SUNXI_CCU_GATE(hrc_bus_clk, "hrc-bus",
			"dcxo24M",
			0x1824, BIT(0), 0);

static const char * const hdmi_tcon_pclk_parents[] = { "pll-peri0-600m", "pll-peri0-480m", "pll-peri0-400m" };

static SUNXI_CCU_M_WITH_MUX_GATE(hdmi_tcon_pclk_clk, "hdmi-tcon-pclk",
			hdmi_tcon_pclk_parents, 0x1830,
			0, 5,  /* M */
			24, 3,  /* mux */
			BIT(31),	/* gate */
			0);

static SUNXI_CCU_GATE(hdmi_rx_bus_clk, "hdmi-rx-bus",
			"dcxo24M",
			0x1834, BIT(0), 0);

static const char * const tvd_parents[] = { "dcxo24M", "pll-adc", "pll-video0-4x", "pll-video0-3x", "pll-video1-4x", "pll-video1-3x" };

static SUNXI_CCU_M_WITH_MUX_GATE(tvd_clk, "tvd",
			tvd_parents, 0x1840,
			0, 5,  /* M */
			24, 3,  /* mux */
			BIT(31),	/* gate */
			0);

static SUNXI_CCU_GATE(tvd_bus_clk, "tvd-bus",
			"dcxo24M",
			0x1844, BIT(0), 0);

static const char * const tsdm_parents[] = { "pll-peri0-600m", "pll-peri0-480m", "pll-peri0-200m" };

static SUNXI_CCU_M_WITH_MUX_GATE(tsdm_clk, "tsdm",
			tsdm_parents, 0x1850,
			0, 5,  /* M */
			24, 3,  /* mux */
			BIT(31),	/* gate */
			0);

static SUNXI_CCU_GATE(tsdm_ahb_bus_clk, "tsdm-ahb-bus",
			"dcxo24M",
			0x1854, BIT(0), 0);

static SUNXI_CCU_GATE(tv_msi_lite_ahb_bus_clk, "tv-msi-lite-ahb-bus",
			"dcxo24M",
			0x1864, BIT(0), 0);

static SUNXI_CCU_GATE(video_in_ahb_bus_clk, "video-in-ahb-bus",
			"dcxo24M",
			0x1884, BIT(0), 0);

static SUNXI_CCU_GATE(pll_output_bypass_bus_clk, "pll-output-bypass-bus",
			"dcxo24M",
			0x1A20, BIT(0), 0);

static SUNXI_CCU_GATE(gmac0_aximon_bus_clk, "gmac0-aximon-bus",
			"dcxo24M",
			0x1C00, BIT(3), 0);

static SUNXI_CCU_GATE(hsi_aximon_bus_clk, "hsi-aximon-bus",
			"dcxo24M",
			0x1C00, BIT(2), 0);

static SUNXI_CCU_GATE(ce_sys_aximon_bus_clk, "ce-sys-aximon-bus",
			"dcxo24M",
			0x1C00, BIT(1), 0);

static SUNXI_CCU_GATE(gpu_aximon_bus_clk, "gpu-aximon-bus",
			"dcxo24M",
			0x1C00, BIT(0), 0);

static SUNXI_CCU_GATE(dcu_ahbmon_bus_clk, "dcu-ahbmon-bus",
			"dcxo24M",
			0x1C04, BIT(1), 0);

static SUNXI_CCU_GATE(cpu_sys_ahbmon_bus_clk, "cpu-sys-ahbmon-bus",
			"dcxo24M",
			0x1C04, BIT(0), 0);
/* ccu_des_end */

/* rst_def_start */
static struct ccu_reset_map sun55iw7_ccu_resets[] = {
	[RST_BUS_NSI]			= { 0x0580, BIT(30) },
	[RST_BUS_NSI_CFG]		= { 0x0584, BIT(16) },
	[RST_BUS_DMA0]			= { 0x0704, BIT(16) },
	[RST_BUS_SPINLOCK]		= { 0x0724, BIT(16) },
	[RST_BUS_MSGBOX_CPUX]		= { 0x0744, BIT(16) },
	[RST_BUS_MSGBOX_CPU]		= { 0x074c, BIT(16) },
	[RST_BUS_SMCARD]		= { 0x0770, BIT(16) },
	[RST_BUS_PWM0]			= { 0x0784, BIT(16) },
	[RST_BUS_DCU]			= { 0x07a4, BIT(16) },
	[RST_BUS_DAP]			= { 0x07ac, BIT(16) },
	[RST_BUS_TIMER0]		= { 0x0850, BIT(16) },
	[RST_BUS_DE0]			= { 0x0a04, BIT(16) },
	[RST_BUS_VEP]			= { 0x0a14, BIT(16) },
	[RST_BUS_VE0]			= { 0x0a8c, BIT(16) },
	[RST_BUS_CE_SY]			= { 0x0ac4, BIT(17) },
	[RST_BUS_CE_KEYSRAM]		= { 0x0ac4, BIT(16) },
	[RST_BUS_GPU]			= { 0x0b24, BIT(16) },
	[RST_BUS_DRAMC]			= { 0x0c0c, BIT(16) },
	[RST_BUS_SMHC0]			= { 0x0d0c, BIT(16) },
	[RST_BUS_SMHC1]			= { 0x0d1c, BIT(16) },
	[RST_BUS_SMHC2]			= { 0x0d2c, BIT(16) },
	[RST_BUS_UART0]			= { 0x0e00, BIT(16) },
	[RST_BUS_UART1]			= { 0x0e04, BIT(16) },
	[RST_BUS_UART2]			= { 0x0e08, BIT(16) },
	[RST_BUS_UART3]			= { 0x0e0c, BIT(16) },
	[RST_BUS_TWI0]			= { 0x0e80, BIT(16) },
	[RST_BUS_TWI1]			= { 0x0e84, BIT(16) },
	[RST_BUS_TWI2]			= { 0x0e88, BIT(16) },
	[RST_BUS_TWI3]			= { 0x0e8c, BIT(16) },
	[RST_BUS_TWI4]			= { 0x0e90, BIT(16) },
	[RST_BUS_SPI0]			= { 0x0f04, BIT(16) },
	[RST_BUS_SPI1]			= { 0x0f0c, BIT(16) },
	[RST_BUS_SPI2]			= { 0x0f14, BIT(16) },
	[RST_BUS_I2S0]			= { 0x120c, BIT(16) },
	[RST_BUS_I2S1]			= { 0x121c, BIT(16) },
	[RST_BUS_I2S2]			= { 0x122c, BIT(16) },
	[RST_BUS_I2S3]			= { 0x123c, BIT(16) },
	[RST_BUS_I2S4]			= { 0x124c, BIT(16) },
	[RST_BUS_OWA0]			= { 0x128c, BIT(16) },
	[RST_BUS_OWA1]			= { 0x12ac, BIT(16) },
	[RST_BUS_AUDIOCODEC0]		= { 0x12ec, BIT(16) },
	[RST_USB_0_DEV]			= { 0x1304, BIT(24) },
	[RST_USB_0_EHCI]		= { 0x1304, BIT(20) },
	[RST_USB_0_OHCI]		= { 0x1304, BIT(16) },
	[RST_USB_1_EHCI]		= { 0x130c, BIT(20) },
	[RST_USB_1_OHCI]		= { 0x130c, BIT(16) },
	[RST_USB_2P0_SY]		= { 0x1344, BIT(16) },
	[RST_USB_2]			= { 0x135c, BIT(16) },
	[RST_BUS_HSI_SY]		= { 0x13cc, BIT(16) },
	[RST_BUS_GMAC0_TOP_AHB]		= { 0x140c, BIT(18) },
	[RST_BUS_GMAC0_AXI]		= { 0x140c, BIT(17) },
	[RST_BUS_GMAC0_AHB]		= { 0x140c, BIT(16) },
	[RST_BUS_TCON_LCD0]		= { 0x1504, BIT(16) },
	[RST_BUS_LVDS0]			= { 0x1544, BIT(16) },
	[RST_BUS_MIPI_DSI0]		= { 0x1584, BIT(16) },
	[RST_BUS_TCON_TV0]		= { 0x1604, BIT(16) },
	[RST_BUS_TVE]			= { 0x1624, BIT(16) },
	[RST_BUS_TCON_TV1]		= { 0x1634, BIT(16) },
	[RST_BUS_HDMI_HDCP]		= { 0x1690, BIT(18) },
	[RST_BUS_HDMI_SUB]		= { 0x1690, BIT(17) },
	[RST_BUS_HDMI_MAIN]		= { 0x1690, BIT(16) },
	[RST_BUS_KSC]			= { 0x16a4, BIT(16) },
	[RST_BUS_VO0_REG]		= { 0x16c4, BIT(16) },
	[RST_BUS_VO1_REG]		= { 0x16d4, BIT(16) },
	[RST_BUS_VIDEO_OUT0]		= { 0x16e4, BIT(16) },
	[RST_BUS_LEDC]			= { 0x1704, BIT(16) },
	[RST_BUS_HRC]			= { 0x1824, BIT(16) },
	[RST_BUS_HDMI_RX]		= { 0x1834, BIT(16) },
	[RST_BUS_TVD]			= { 0x1844, BIT(16) },
	[RST_BUS_TSDM]			= { 0x1854, BIT(16) },
	[RST_BUS_TV_MSI_LITE]		= { 0x1864, BIT(16) },
	[RST_BUS_VIDEO_IN]		= { 0x1884, BIT(16) },
	[RST_BUS_GMAC0_AXIMON]		= { 0x1c00, BIT(19) },
	[RST_BUS_HSI_AXIMON]		= { 0x1c00, BIT(18) },
	[RST_BUS_CE_SYS_AXIMON]		= { 0x1c00, BIT(17) },
	[RST_BUS_GPU_AXIMON]		= { 0x1c00, BIT(16) },
	[RST_BUS_DCU_AHBMON]		= { 0x1c04, BIT(17) },
	[RST_BUS_CPU_SYS_AHBMON]	= { 0x1c04, BIT(16) },
};
/* rst_def_end */

/* ccu_def_start */
static struct clk_hw_onecell_data sun55iw7_hw_clks = {
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
		[CLK_PLL_GPU_VCO]			= &pll_gpu_vco_clk.common.hw,
		[CLK_PLL_GPU]				= &pll_gpu_clk.common.hw,
		[CLK_PLL_VIDEO0]			= &pll_video0_clk.common.hw,
		[CLK_PLL_VIDEO0_4X]			= &pll_video0_4x_clk.common.hw,
		[CLK_PLL_VIDEO0_3X]			= &pll_video0_3x_clk.common.hw,
		[CLK_PLL_VIDEO1]			= &pll_video1_clk.common.hw,
		[CLK_PLL_VIDEO1_4X]			= &pll_video1_4x_clk.common.hw,
		[CLK_PLL_VIDEO1_3X]			= &pll_video1_3x_clk.common.hw,
		[CLK_PLL_VE_VCO]			= &pll_ve_vco_clk.common.hw,
		[CLK_PLL_VE]				= &pll_ve_clk.common.hw,
		[CLK_PLL_ADC_VCO]			= &pll_adc_vco_clk.common.hw,
		[CLK_PLL_ADC_]				= &pll_adc_clk.common.hw,
		[CLK_PLL_AUDIO0]			= &pll_audio0_clk.common.hw,
		[CLK_AHB]				= &ahb_clk.common.hw,
		[CLK_APB0]				= &apb0_clk.common.hw,
		[CLK_APB1]				= &apb1_clk.common.hw,
		[CLK_APB_UART]				= &apb_uart_clk.common.hw,
		[CLK_CPUX_GIC]				= &cpux_gic_clk.common.hw,
		[CLK_NSI]				= &nsi_clk.common.hw,
		[CLK_BUS_NSI_CFG]			= &nsi_cfg_bus_clk.common.hw,
		[CLK_MBUS]				= &mbus_clk.common.hw,
		[CLK_BUS_IOMMU_APB]			= &iommu_apb_bus_clk.common.hw,
		[CLK_BUS_PLL_STBY_PERI0_CLK_SW_CFG]	= &pll_stby_peri0_clk_sw_cfg_bus_clk.common.hw,
		[CLK_BUS_SMHC2_AHB_SW_CFG]		= &smhc2_ahb_sw_cfg_bus_clk.common.hw,
		[CLK_BUS_SMHC1_AHB_SW_CFG]		= &smhc1_ahb_sw_cfg_bus_clk.common.hw,
		[CLK_BUS_SMHC0_AHB_SW_CFG]		= &smhc0_ahb_sw_cfg_bus_clk.common.hw,
		[CLK_BUS_HSI_AHB_SW_CFG]		= &hsi_ahb_sw_cfg_bus_clk.common.hw,
		[CLK_BUS_SECURE_SYS_AHB_SW_CFG]		= &secure_sys_ahb_sw_cfg_bus_clk.common.hw,
		[CLK_BUS_GPU_AHB_SW_CFG]		= &gpu_ahb_sw_cfg_bus_clk.common.hw,
		[CLK_BUS_VIDEO_OUT0_AHB_SW_CFG]		= &video_out0_ahb_sw_cfg_bus_clk.common.hw,
		[CLK_BUS_VIDEO_IN_AHB_SW_CFG]		= &video_in_ahb_sw_cfg_bus_clk.common.hw,
		[CLK_BUS_VE0_AHB_SW_CFG]		= &ve0_ahb_sw_cfg_bus_clk.common.hw,
		[CLK_BUS_PLL_PERI_APB0_SW_CFG]		= &pll_peri_apb0_sw_cfg_bus_clk.common.hw,
		[CLK_BUS_PLL_PERI_AHB_SW_CFG]		= &pll_peri_ahb_sw_cfg_bus_clk.common.hw,
		[CLK_MBUS_BUS_DMA0_SW_CFG]		= &dma0_mbus_sw_cfg_bus_clk.common.hw,
		[CLK_BUS_CE_SYS_AXI_SW_CFG]		= &ce_sys_axi_sw_cfg_bus_clk.common.hw,
		[CLK_BUS_GPU_AXI_SW_CFG]		= &gpu_axi_sw_cfg_bus_clk.common.hw,
		[CLK_MBUS_BUS_DE_SYS_SW_CFG]		= &de_sys_mbus_sw_cfg_bus_clk.common.hw,
		[CLK_MBUS_BUS_VIDEO_IN_SW_CFG]		= &video_in_mbus_sw_cfg_bus_clk.common.hw,
		[CLK_MBUS_BUS_VE0_SW_CFG]		= &ve0_mbus_sw_cfg_bus_clk.common.hw,
		[CLK_BUS_GMAC0_AXI]			= &gmac0_axi_bus_clk.common.hw,
		[CLK_MBUS_BUS_IOMMU]			= &iommu_mbus_bus_clk.common.hw,
		[CLK_MBUS_BUS_VIDEO_OUT]		= &video_out_mbus_bus_clk.common.hw,
		[CLK_BUS_CE_SYS_AXI]			= &ce_sys_axi_bus_clk.common.hw,
		[CLK_MBUS_BUS_VE0]			= &ve0_mbus_bus_clk.common.hw,
		[CLK_MBUS_BUS_DMA0]			= &dma0_mbus_bus_clk.common.hw,
		[CLK_BUS_SMHC2_AHB_AUTO_EN]		= &smhc2_ahb_auto_en_bus_clk.common.hw,
		[CLK_BUS_SMHC1_AHB_AUTO_EN]		= &smhc1_ahb_auto_en_bus_clk.common.hw,
		[CLK_BUS_SMHC0_AHB_AUTO_EN]		= &smhc0_ahb_auto_en_bus_clk.common.hw,
		[CLK_BUS_HSI_AHB_AUTO_EN]		= &hsi_ahb_auto_en_bus_clk.common.hw,
		[CLK_BUS_SECURE_SYS_AHB_AUTO_EN]	= &secure_sys_ahb_auto_en_bus_clk.common.hw,
		[CLK_BUS_GPU_AHB_AUTO_EN]		= &gpu_ahb_auto_en_bus_clk.common.hw,
		[CLK_BUS_VIDEO_OUT0_AHB_AUTO_EN]	= &video_out0_ahb_auto_en_bus_clk.common.hw,
		[CLK_BUS_VIDEO_IN_AHB_AUTO_EN]		= &video_in_ahb_auto_en_bus_clk.common.hw,
		[CLK_BUS_VE0_AHB_AUTO_EN]		= &ve0_ahb_auto_en_bus_clk.common.hw,
		[CLK_MBUS_BUS_DMA0_AUTO_EN]		= &dma0_mbus_auto_en_bus_clk.common.hw,
		[CLK_BUS_CE_SYS_AXI_AUTO_EN]		= &ce_sys_axi_auto_en_bus_clk.common.hw,
		[CLK_BUS_GPU_AXI_AUTO_EN]		= &gpu_axi_auto_en_bus_clk.common.hw,
		[CLK_MBUS_BUS_DE_SYS_AUTO_EN]		= &de_sys_mbus_auto_en_bus_clk.common.hw,
		[CLK_MBUS_BUS_VIDEO_IN_AUTO_EN]		= &video_in_mbus_auto_en_bus_clk.common.hw,
		[CLK_MBUS_BUS_VE0_AUTO_EN]		= &ve0_mbus_auto_en_bus_clk.common.hw,
		[CLK_BUS_DMA0_AHB]			= &dma0_ahb_bus_clk.common.hw,
		[CLK_BUS_SPINLOCK_AHB]			= &spinlock_ahb_bus_clk.common.hw,
		[CLK_BUS_MSGBOX_CPUX_AHB]		= &msgbox_cpux_ahb_bus_clk.common.hw,
		[CLK_BUS_MSGBOX_CPUS_AHB]		= &msgbox_cpus_ahb_bus_clk.common.hw,
		[CLK_BUS_SMCARD_APB]			= &smcard_apb_bus_clk.common.hw,
		[CLK_BUS_PWM0_APB]			= &pwm0_apb_bus_clk.common.hw,
		[CLK_BUS_DCU]				= &dcu_bus_clk.common.hw,
		[CLK_BUS_DAP_AHB]			= &dap_ahb_bus_clk.common.hw,
		[CLK_BUS_PLL_CEC_PERI]			= &pll_cec_peri_bus_clk.common.hw,
		[CLK_TIMER0_0_CLK]			= &timer0_0_clk_clk.common.hw,
		[CLK_TIMER0_1_CLK]			= &timer0_1_clk_clk.common.hw,
		[CLK_TIMER0_2_CLK]			= &timer0_2_clk_clk.common.hw,
		[CLK_BUS_TIMER0_AHB]			= &timer0_ahb_bus_clk.common.hw,
		[CLK_DE]				= &de_clk.common.hw,
		[CLK_BUS_DE0_AHB]			= &de0_ahb_bus_clk.common.hw,
		[CLK_VEP]				= &vep_clk.common.hw,
		[CLK_BUS_VEP_AHB]			= &vep_ahb_bus_clk.common.hw,
		[CLK_VE0]				= &ve0_clk.common.hw,
		[CLK_BUS_VE0_AHB]			= &ve0_ahb_bus_clk.common.hw,
		[CLK_CE_SYS]				= &ce_sys_clk.common.hw,
		[CLK_BUS_CE_KEYSRAM]			= &ce_keysram_bus_clk.common.hw,
		[CLK_BUS_CE_SYS_IP_AHB]			= &ce_sys_ip_ahb_bus_clk.common.hw,
		[CLK_BUS_GPU_AHB]			= &gpu_ahb_bus_clk.common.hw,
		[CLK_BUS_DRAMC_AHB]			= &dramc_ahb_bus_clk.common.hw,
		[CLK_SMHC0]				= &smhc0_clk.common.hw,
		[CLK_BUS_SMHC0_AHB]			= &smhc0_ahb_bus_clk.common.hw,
		[CLK_SMHC1]				= &smhc1_clk.common.hw,
		[CLK_BUS_SMHC1_AHB]			= &smhc1_ahb_bus_clk.common.hw,
		[CLK_SMHC2]				= &smhc2_clk.common.hw,
		[CLK_BUS_SMHC2_AHB]			= &smhc2_ahb_bus_clk.common.hw,
		[CLK_BUS_UART0_APB]			= &uart0_apb_bus_clk.common.hw,
		[CLK_BUS_UART1_APB]			= &uart1_apb_bus_clk.common.hw,
		[CLK_BUS_UART2_APB]			= &uart2_apb_bus_clk.common.hw,
		[CLK_BUS_UART3_APB]			= &uart3_apb_bus_clk.common.hw,
		[CLK_BUS_TWI0_APB]			= &twi0_apb_bus_clk.common.hw,
		[CLK_BUS_TWI1_APB]			= &twi1_apb_bus_clk.common.hw,
		[CLK_BUS_TWI2_APB]			= &twi2_apb_bus_clk.common.hw,
		[CLK_BUS_TWI3_APB]			= &twi3_apb_bus_clk.common.hw,
		[CLK_BUS_TWI4_APB]			= &twi4_apb_bus_clk.common.hw,
		[CLK_SPI0]				= &spi0_clk.common.hw,
		[CLK_BUS_SPI0_AHB]			= &spi0_ahb_bus_clk.common.hw,
		[CLK_SPI1]				= &spi1_clk.common.hw,
		[CLK_BUS_SPI1_AHB]			= &spi1_ahb_bus_clk.common.hw,
		[CLK_SPI2]				= &spi2_clk.common.hw,
		[CLK_BUS_SPI2_AHB]			= &spi2_ahb_bus_clk.common.hw,
		[CLK_I2S0]				= &i2s0_clk.common.hw,
		[CLK_BUS_I2S0_APB]			= &i2s0_apb_bus_clk.common.hw,
		[CLK_I2S1]				= &i2s1_clk.common.hw,
		[CLK_BUS_I2S1_APB]			= &i2s1_apb_bus_clk.common.hw,
		[CLK_I2S2]				= &i2s2_clk.common.hw,
		[CLK_BUS_I2S2_APB]			= &i2s2_apb_bus_clk.common.hw,
		[CLK_I2S3]				= &i2s3_clk.common.hw,
		[CLK_BUS_I2S3_APB]			= &i2s3_apb_bus_clk.common.hw,
		[CLK_I2S4]				= &i2s4_clk.common.hw,
		[CLK_BUS_I2S4_APB]			= &i2s4_apb_bus_clk.common.hw,
		[CLK_OWA0_TX]				= &owa0_tx_clk.common.hw,
		[CLK_OWA0_RX]				= &owa0_rx_clk.common.hw,
		[CLK_BUS_OWA0_APB]			= &owa0_apb_bus_clk.common.hw,
		[CLK_OWA1_TX]				= &owa1_tx_clk.common.hw,
		[CLK_OWA1_RX]				= &owa1_rx_clk.common.hw,
		[CLK_BUS_OWA1_APB]			= &owa1_apb_bus_clk.common.hw,
		[CLK_AUDIOCODEC0_DAC]			= &audiocodec0_dac_clk.common.hw,
		[CLK_AUDIOCODEC0_ADC]			= &audiocodec0_adc_clk.common.hw,
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
		[CLK_BUS_USB2_U2_PHY_REF]		= &usb2_u2_phy_ref_bus_clk.common.hw,
		[CLK_USB2_SUSPEND]			= &usb2_suspend_clk.common.hw,
		[CLK_USB2_REF]				= &usb2_ref_clk.common.hw,
		[CLK_USB2_U3_ONLY_UTMI]			= &usb2_u3_only_utmi_clk.common.hw,
		[CLK_USB2_U2_ONLY_PIPE]			= &usb2_u2_only_pipe_clk.common.hw,
		[CLK_HSI_COMB0_PHY_CFG]			= &hsi_comb0_phy_cfg_clk.common.hw,
		[CLK_HSI_COMB0_PHY_REF]			= &hsi_comb0_phy_ref_clk.common.hw,
		[CLK_BUS_HSI_AXI]			= &hsi_axi_bus_clk.common.hw,
		[CLK_BUS_HSI_AHB]			= &hsi_ahb_bus_clk.common.hw,
		[CLK_HSI_AXI]				= &hsi_axi_clk.common.hw,
		[CLK_GMAC0_PHY]				= &gmac0_phy_clk.common.hw,
		[CLK_BUS_GMAC0_AHB]			= &gmac0_ahb_bus_clk.common.hw,
		[CLK_TCON_LCD0]				= &tcon_lcd0_clk.common.hw,
		[CLK_BUS_TCON_LCD0_AHB]			= &tcon_lcd0_ahb_bus_clk.common.hw,
		[CLK_MIPI_DSI0]				= &mipi_dsi0_clk.common.hw,
		[CLK_BUS_MIPI_DSI0_AHB]			= &mipi_dsi0_ahb_bus_clk.common.hw,
		[CLK_COMBOPHY0]				= &combophy0_clk.common.hw,
		[CLK_TCON_TV0]				= &tcon_tv0_clk.common.hw,
		[CLK_BUS_TCON_TV0_AHB]			= &tcon_tv0_ahb_bus_clk.common.hw,
		[CLK_TVE]				= &tve_clk.common.hw,
		[CLK_BUS_TVE_AHB]			= &tve_ahb_bus_clk.common.hw,
		[CLK_TCON_TV1]				= &tcon_tv1_clk.common.hw,
		[CLK_BUS_TCON_TV1_AHB]			= &tcon_tv1_ahb_bus_clk.common.hw,
		[CLK_BUS_HDMI_REF]			= &hdmi_ref_bus_clk.common.hw,
		[CLK_HDMI_PREP]				= &hdmi_prep_clk.common.hw,
		[CLK_HDMI_LINKQPCLK]			= &hdmi_linkqpclk_clk.common.hw,
		[CLK_HDMI_PIXEL]			= &hdmi_pixel_clk.common.hw,
		[CLK_BUS_HDMI_AHB]			= &hdmi_ahb_bus_clk.common.hw,
		[CLK_BUS_VO0_REG_AHB]			= &vo0_reg_ahb_bus_clk.common.hw,
		[CLK_BUS_VO1_REG_AHB]			= &vo1_reg_ahb_bus_clk.common.hw,
		[CLK_BUS_VIDEO_OUT0_AHB]		= &video_out0_ahb_bus_clk.common.hw,
		[CLK_LEDC]				= &ledc_clk.common.hw,
		[CLK_BUS_LEDC_APB]			= &ledc_apb_bus_clk.common.hw,
		[CLK_CSI_MASTER0]			= &csi_master0_clk.common.hw,
		[CLK_CSI]				= &csi_clk.common.hw,
		[CLK_HRC]				= &hrc_clk.common.hw,
		[CLK_BUS_HRC]				= &hrc_bus_clk.common.hw,
		[CLK_HDMI_TCON_PCLK]			= &hdmi_tcon_pclk_clk.common.hw,
		[CLK_BUS_HDMI_RX]			= &hdmi_rx_bus_clk.common.hw,
		[CLK_TVD]				= &tvd_clk.common.hw,
		[CLK_BUS_TVD]				= &tvd_bus_clk.common.hw,
		[CLK_TSDM]				= &tsdm_clk.common.hw,
		[CLK_BUS_TSDM_AHB]			= &tsdm_ahb_bus_clk.common.hw,
		[CLK_BUS_TV_MSI_LITE_AHB]		= &tv_msi_lite_ahb_bus_clk.common.hw,
		[CLK_BUS_VIDEO_IN_AHB]			= &video_in_ahb_bus_clk.common.hw,
		[CLK_BUS_PLL_OUTPUT_BYPASS]		= &pll_output_bypass_bus_clk.common.hw,
		[CLK_BUS_GMAC0_AXIMON]			= &gmac0_aximon_bus_clk.common.hw,
		[CLK_BUS_HSI_AXIMON]			= &hsi_aximon_bus_clk.common.hw,
		[CLK_BUS_CE_SYS_AXIMON]			= &ce_sys_aximon_bus_clk.common.hw,
		[CLK_BUS_GPU_AXIMON]			= &gpu_aximon_bus_clk.common.hw,
		[CLK_BUS_DCU_AHBMON]			= &dcu_ahbmon_bus_clk.common.hw,
		[CLK_BUS_CPU_SYS_AHBMON]		= &cpu_sys_ahbmon_bus_clk.common.hw,
	},
	.num = CLK_NUMBER,
};
/* ccu_def_end */

static struct ccu_common *sun55iw7_ccu_clks[] = {
	&pll_peri0_clk.common,
	&pll_peri0_2x_clk.common,
	&pll_peri0_800m_clk.common,
	&pll_peri0_480m_clk.common,
	&pll_peri1_clk.common,
	&pll_peri1_2x_clk.common,
	&pll_peri1_800m_clk.common,
	&pll_peri1_480m_clk.common,
	&pll_gpu_vco_clk.common,
	&pll_gpu_clk.common,
	&pll_video0_clk.common,
	&pll_video0_4x_clk.common,
	&pll_video0_3x_clk.common,
	&pll_video1_clk.common,
	&pll_video1_4x_clk.common,
	&pll_video1_3x_clk.common,
	&pll_ve_vco_clk.common,
	&pll_ve_clk.common,
	&pll_adc_vco_clk.common,
	&pll_adc_clk.common,
	&pll_audio0_clk.common,
	&ahb_clk.common,
	&apb0_clk.common,
	&apb1_clk.common,
	&apb_uart_clk.common,
	&cpux_gic_clk.common,
	&nsi_clk.common,
	&nsi_cfg_bus_clk.common,
	&mbus_clk.common,
	&iommu_apb_bus_clk.common,
	&pll_stby_peri0_clk_sw_cfg_bus_clk.common,
	&smhc2_ahb_sw_cfg_bus_clk.common,
	&smhc1_ahb_sw_cfg_bus_clk.common,
	&smhc0_ahb_sw_cfg_bus_clk.common,
	&hsi_ahb_sw_cfg_bus_clk.common,
	&secure_sys_ahb_sw_cfg_bus_clk.common,
	&gpu_ahb_sw_cfg_bus_clk.common,
	&video_out0_ahb_sw_cfg_bus_clk.common,
	&video_in_ahb_sw_cfg_bus_clk.common,
	&ve0_ahb_sw_cfg_bus_clk.common,
	&pll_peri_apb0_sw_cfg_bus_clk.common,
	&pll_peri_ahb_sw_cfg_bus_clk.common,
	&dma0_mbus_sw_cfg_bus_clk.common,
	&ce_sys_axi_sw_cfg_bus_clk.common,
	&gpu_axi_sw_cfg_bus_clk.common,
	&de_sys_mbus_sw_cfg_bus_clk.common,
	&video_in_mbus_sw_cfg_bus_clk.common,
	&ve0_mbus_sw_cfg_bus_clk.common,
	&gmac0_axi_bus_clk.common,
	&iommu_mbus_bus_clk.common,
	&video_out_mbus_bus_clk.common,
	&ce_sys_axi_bus_clk.common,
	&ve0_mbus_bus_clk.common,
	&dma0_mbus_bus_clk.common,
	&smhc2_ahb_auto_en_bus_clk.common,
	&smhc1_ahb_auto_en_bus_clk.common,
	&smhc0_ahb_auto_en_bus_clk.common,
	&hsi_ahb_auto_en_bus_clk.common,
	&secure_sys_ahb_auto_en_bus_clk.common,
	&gpu_ahb_auto_en_bus_clk.common,
	&video_out0_ahb_auto_en_bus_clk.common,
	&video_in_ahb_auto_en_bus_clk.common,
	&ve0_ahb_auto_en_bus_clk.common,
	&dma0_mbus_auto_en_bus_clk.common,
	&ce_sys_axi_auto_en_bus_clk.common,
	&gpu_axi_auto_en_bus_clk.common,
	&de_sys_mbus_auto_en_bus_clk.common,
	&video_in_mbus_auto_en_bus_clk.common,
	&ve0_mbus_auto_en_bus_clk.common,
	&dma0_ahb_bus_clk.common,
	&spinlock_ahb_bus_clk.common,
	&msgbox_cpux_ahb_bus_clk.common,
	&msgbox_cpus_ahb_bus_clk.common,
	&smcard_apb_bus_clk.common,
	&pwm0_apb_bus_clk.common,
	&dcu_bus_clk.common,
	&dap_ahb_bus_clk.common,
	&pll_cec_peri_bus_clk.common,
	&timer0_0_clk_clk.common,
	&timer0_1_clk_clk.common,
	&timer0_2_clk_clk.common,
	&timer0_ahb_bus_clk.common,
	&de_clk.common,
	&de0_ahb_bus_clk.common,
	&vep_clk.common,
	&vep_ahb_bus_clk.common,
	&ve0_clk.common,
	&ve0_ahb_bus_clk.common,
	&ce_sys_clk.common,
	&ce_keysram_bus_clk.common,
	&ce_sys_ip_ahb_bus_clk.common,
	&gpu_ahb_bus_clk.common,
	&dramc_ahb_bus_clk.common,
	&smhc0_clk.common,
	&smhc0_ahb_bus_clk.common,
	&smhc1_clk.common,
	&smhc1_ahb_bus_clk.common,
	&smhc2_clk.common,
	&smhc2_ahb_bus_clk.common,
	&uart0_apb_bus_clk.common,
	&uart1_apb_bus_clk.common,
	&uart2_apb_bus_clk.common,
	&uart3_apb_bus_clk.common,
	&twi0_apb_bus_clk.common,
	&twi1_apb_bus_clk.common,
	&twi2_apb_bus_clk.common,
	&twi3_apb_bus_clk.common,
	&twi4_apb_bus_clk.common,
	&spi0_clk.common,
	&spi0_ahb_bus_clk.common,
	&spi1_clk.common,
	&spi1_ahb_bus_clk.common,
	&spi2_clk.common,
	&spi2_ahb_bus_clk.common,
	&i2s0_clk.common,
	&i2s0_apb_bus_clk.common,
	&i2s1_clk.common,
	&i2s1_apb_bus_clk.common,
	&i2s2_clk.common,
	&i2s2_apb_bus_clk.common,
	&i2s3_clk.common,
	&i2s3_apb_bus_clk.common,
	&i2s4_clk.common,
	&i2s4_apb_bus_clk.common,
	&owa0_tx_clk.common,
	&owa0_rx_clk.common,
	&owa0_apb_bus_clk.common,
	&owa1_tx_clk.common,
	&owa1_rx_clk.common,
	&owa1_apb_bus_clk.common,
	&audiocodec0_dac_clk.common,
	&audiocodec0_adc_clk.common,
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
	&usb2_u2_phy_ref_bus_clk.common,
	&usb2_suspend_clk.common,
	&usb2_ref_clk.common,
	&usb2_u3_only_utmi_clk.common,
	&usb2_u2_only_pipe_clk.common,
	&hsi_comb0_phy_cfg_clk.common,
	&hsi_comb0_phy_ref_clk.common,
	&hsi_axi_bus_clk.common,
	&hsi_ahb_bus_clk.common,
	&hsi_axi_clk.common,
	&gmac0_phy_clk.common,
	&gmac0_ahb_bus_clk.common,
	&tcon_lcd0_clk.common,
	&tcon_lcd0_ahb_bus_clk.common,
	&mipi_dsi0_clk.common,
	&mipi_dsi0_ahb_bus_clk.common,
	&combophy0_clk.common,
	&tcon_tv0_clk.common,
	&tcon_tv0_ahb_bus_clk.common,
	&tve_clk.common,
	&tve_ahb_bus_clk.common,
	&tcon_tv1_clk.common,
	&tcon_tv1_ahb_bus_clk.common,
	&hdmi_ref_bus_clk.common,
	&hdmi_prep_clk.common,
	&hdmi_linkqpclk_clk.common,
	&hdmi_pixel_clk.common,
	&hdmi_ahb_bus_clk.common,
	&vo0_reg_ahb_bus_clk.common,
	&vo1_reg_ahb_bus_clk.common,
	&video_out0_ahb_bus_clk.common,
	&ledc_clk.common,
	&ledc_apb_bus_clk.common,
	&csi_master0_clk.common,
	&csi_clk.common,
	&hrc_clk.common,
	&hrc_bus_clk.common,
	&hdmi_tcon_pclk_clk.common,
	&hdmi_rx_bus_clk.common,
	&tvd_clk.common,
	&tvd_bus_clk.common,
	&tsdm_clk.common,
	&tsdm_ahb_bus_clk.common,
	&tv_msi_lite_ahb_bus_clk.common,
	&video_in_ahb_bus_clk.common,
	&pll_output_bypass_bus_clk.common,
	&gmac0_aximon_bus_clk.common,
	&hsi_aximon_bus_clk.common,
	&ce_sys_aximon_bus_clk.common,
	&gpu_aximon_bus_clk.common,
	&dcu_ahbmon_bus_clk.common,
	&cpu_sys_ahbmon_bus_clk.common,
};

static struct ccu_reg_dump sun55iw7_distbus_restore_clks[] = {
	{0x0588, UPD_KEY_VALUE},
};

static const struct sunxi_ccu_desc sun55iw7_ccu_desc = {
	.ccu_clks	= sun55iw7_ccu_clks,
	.num_ccu_clks	= ARRAY_SIZE(sun55iw7_ccu_clks),

	.hw_clks	= &sun55iw7_hw_clks,

	.resets		= sun55iw7_ccu_resets,
	.num_resets	= ARRAY_SIZE(sun55iw7_ccu_resets),
};

static int sun55iw7_ccu_really_probe(struct device_node *node)
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
	set_reg(reg + SUN55IW7_AHB_GATE_EN_REG, 0x1, 1, SUN55IW7_AHB_MONITOR_ENABLE);
	set_reg(reg + SUN55IW7_AHB_GATE_EN_REG, 0x1, 1, SUN55IW7_SD_MONITOR_ENABLE);

	/*
	 * 1. enable pll clk gate
	 * 2. enable pll clk auto gate
	 */
	set_reg(reg + SUN55IW7_PLL_PERI0_GATE_EN_REG, 0x8fff0fff, 32, 0);
	set_reg(reg + SUN55IW7_PLL_PERI1_GATE_EN_REG, 0x9fff1fff, 32, 0);
	set_reg(reg + SUN55IW7_PLL_VIDEO_GATE_EN_REG, 0x330033, 32, 0);
	set_reg(reg + SUN55IW7_PLL_GPU_GATE_EN_REG, 0x10001, 32, 0);
	set_reg(reg + SUN55IW7_PLL_VE_GATE_EN_REG, 0x10001, 32, 0);
	set_reg(reg + SUN55IW7_PLL_AUDIO_GATE_EN_REG, 0x10001, 32, 0);
	set_reg(reg + SUN55IW7_PLL_ADC_GATE_EN_REG, 0x10001, 32, 0);

	ret = sunxi_ccu_probe(node, reg, &sun55iw7_ccu_desc);
	if (ret)
		return ret;

	sunxi_ccu_sleep_init(reg, sun55iw7_ccu_clks,
			ARRAY_SIZE(sun55iw7_ccu_clks),
			sun55iw7_distbus_restore_clks,
			ARRAY_SIZE(sun55iw7_distbus_restore_clks));

	return 0;
}

#if IS_ENABLED(CONFIG_AW_KERNEL_ORIGIN)
static void __init of_sun55iw7_ccu_init(struct device_node *node)
{
	sun55iw7_ccu_really_probe(node);
}

CLK_OF_DECLARE(sun55iw7_ccu_init, "allwinner,sun55iw7-ccu", of_sun55iw7_ccu_init);
#else
static int sun55iw7_ccu_probe(struct platform_device *pdev)
{
	struct device_node *node = pdev->dev.of_node;

	return sun55iw7_ccu_really_probe(node);
}

static const struct of_device_id sun55iw7_ccu_ids[] = {
	{ .compatible = "allwinner,sun55iw7-ccu" },
	{ }
};

static struct platform_driver sun55iw7_ccu_driver = {
	.probe	= sun55iw7_ccu_probe,
	.driver	= {
		.name	= "sun55iw7-ccu",
		.of_match_table	= sun55iw7_ccu_ids,
	},
};

static int __init sun55iw7_ccu_init(void)
{
	int err;

	err = platform_driver_register(&sun55iw7_ccu_driver);
	if (err)
		pr_err("register ccu sun55iw7 failed\n");

	return err;
}

core_initcall(sun55iw7_ccu_init);

static void __exit sun55iw7_ccu_exit(void)
{
	platform_driver_unregister(&sun55iw7_ccu_driver);
}
module_exit(sun55iw7_ccu_exit);
#endif

MODULE_DESCRIPTION("Allwinner sun55iw7 clk driver");
MODULE_AUTHOR("haili");
MODULE_LICENSE("GPL v2");
MODULE_VERSION(SUNXI_CCU_VERSION);
