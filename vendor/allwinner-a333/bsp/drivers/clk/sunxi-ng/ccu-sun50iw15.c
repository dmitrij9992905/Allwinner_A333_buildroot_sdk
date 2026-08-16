// SPDX-License-Identifier: GPL-2.0-or-later

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

#include "ccu-sun50iw15.h"

#define SUNXI_CCU_VERSION	"0.0.2"

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

#define SUN50IW15_PLL_PERI0_CTRL_REG   0x0020
static struct ccu_nm pll_peri0_clk = {
	.output			= BIT(27),
	.lock			= BIT(28),
	.lock_enable		= BIT(29),
	.ldo_en 		= BIT(30),
	.enable			= BIT(31),
	.n			= _SUNXI_CCU_MULT_MIN_MAX(8, 8, 53, 105),
	.min_rate		= 1272000000,
	.max_rate		= 2520000000UL,
	.common			= {
		.reg		= 0x0020,
		.hw.init	= CLK_HW_INIT("pll-peri0", "dcxo24M",
					&ccu_nm_ops,
					CLK_SET_RATE_UNGATE |
					CLK_IS_CRITICAL),
	},
};

static SUNXI_CCU_M(pll_peri0_2x_clk, "pll-peri0-2x",
			"pll-peri0", 0x0020,
			20, 3, 0);	/* p0 */

static SUNXI_CCU_M(pll_peri0_800m_clk, "pll-peri0-800m",
			"pll-peri0", 0x0020,
			16, 3, 0);	/* p1 */

static SUNXI_CCU_M(pll_peri0_480m_clk, "pll-peri0-480m",
			"pll-peri0", 0x0020,
			2, 3, 0);	/* p2 */

static CLK_FIXED_FACTOR(pll_peri0_600m_clk, "pll-peri0-600m",
			"pll-peri0-2x", 2, 1, 0);

static CLK_FIXED_FACTOR(pll_peri0_400m_clk, "pll-peri0-400m",
			"pll-peri0-800m", 2, 1, 0);

static CLK_FIXED_FACTOR(pll_peri0_300m_clk, "pll-peri0-300m",
			"pll-peri0-600m", 2, 1, 0);

static CLK_FIXED_FACTOR(pll_peri0_200m_clk, "pll-peri0-200m",
			"pll-peri0-400m", 2, 1, 0);

static CLK_FIXED_FACTOR(pll_peri0_160m_clk, "pll-peri0-160m",
			"pll-peri0-480m", 3, 1, 0);

static CLK_FIXED_FACTOR(pll_peri0_150m_clk, "pll-peri0-150m",
			"pll-peri0-300m", 2, 1, 0);

#define SUN50IW15_PLL_PERI1_CTRL_REG   0x0028
static struct ccu_nm pll_peri1_clk = {
	.output			= BIT(27),
	.lock			= BIT(28),
	.lock_enable		= BIT(29),
	.ldo_en 		= BIT(30),
	.enable			= BIT(31),
	.n			= _SUNXI_CCU_MULT_MIN_MAX(8, 8, 53, 105),
	.min_rate		= 1272000000,
	.max_rate		= 2520000000UL,
	.common			= {
		.reg		= 0x0028,
		.hw.init	= CLK_HW_INIT("pll-peri1", "dcxo24M",
					&ccu_nm_ops,
					CLK_SET_RATE_UNGATE |
					CLK_IS_CRITICAL),
	},
};

static SUNXI_CCU_M(pll_peri1_2x_clk, "pll-peri1-2x",
			"pll-peri0", 0x0028,
			20, 3, 0);	/* p0 */

static SUNXI_CCU_M(pll_peri1_800m_clk, "pll-peri1-800m",
			"pll-peri0", 0x0028,
			16, 3, 0);	/* p1 */

static SUNXI_CCU_M(pll_peri1_480m_clk, "pll-peri1-480m",
			"pll-peri0", 0x0028,
			2, 3, 0);	/* p2 */

static CLK_FIXED_FACTOR(pll_peri1_600m_clk, "pll-peri1-600m",
			"pll-peri1-2x", 2, 1, 0);

static CLK_FIXED_FACTOR(pll_peri1_400m_clk, "pll-peri1-400m",
			"pll-peri1-800m", 2, 1, 0);

static CLK_FIXED_FACTOR(pll_peri1_300m_clk, "pll-peri1-300m",
			"pll-peri1-600m", 2, 1, 0);

static CLK_FIXED_FACTOR(pll_peri1_200m_clk, "pll-peri1-200m",
			"pll-peri1-400m", 2, 1, 0);

static CLK_FIXED_FACTOR(pll_peri1_160m_clk, "pll-peri1-160m",
			"pll-peri1-480m", 3, 1, 0);

static CLK_FIXED_FACTOR(pll_peri1_150m_clk, "pll-peri1-150m",
			"pll-peri1-300m", 2, 1, 0);

#define SUN50IW15_PLL_GPU_CTRL_REG   0x0030
static struct ccu_nm pll_gpu_vco_clk = {
	.output			= BIT(27),
	.lock			= BIT(28),
	.lock_enable		= BIT(29),
	.ldo_en 		= BIT(30),
	.enable			= BIT(31),
	.n			= _SUNXI_CCU_MULT_MIN_MAX(8, 8, 53, 105),
	.min_rate		= 1272000000,
	.max_rate		= 2520000000UL,
	.common			= {
		.reg		= 0x0030,
		.hw.init	= CLK_HW_INIT("pll-gpu-vco", "dcxo24M",
					&ccu_nm_ops,
					CLK_SET_RATE_UNGATE |
					CLK_IS_CRITICAL),
	},
};

static SUNXI_CCU_M(pll_gpu_clk, "pll-gpu",
			"pll-gpu-vco", 0x0030,
			20, 3, 0);	/* p0 */

#define SUN50IW15_PLL_VIDEO0_CTRL_REG   0x0040
static struct ccu_nm pll_video0_clk = {
	.output			= BIT(27),
	.lock			= BIT(28),
	.lock_enable		= BIT(29),
	.ldo_en 		= BIT(30),
	.enable			= BIT(31),
	.n			= _SUNXI_CCU_MULT_MIN_MAX(8, 8, 53, 105),
	.min_rate		= 1272000000,
	.max_rate		= 2520000000UL,
	.common			= {
		.reg		= 0x0040,
		.hw.init	= CLK_HW_INIT("pll-video0", "dcxo24M",
					&ccu_nm_ops,
					CLK_SET_RATE_UNGATE |
					CLK_IS_CRITICAL),
	},
};

static SUNXI_CCU_M(pll_video0_4x_clk, "pll-video0-4x",
			"pll-video0", 0x0040,
			20, 3, 0);	/* p0 */

static SUNXI_CCU_M(pll_video0_1x_clk, "pll-video0-1x",
			"pll-video0", 0x0040,
			16, 3, 0);	/* p1 */

#define SUN50IW15_PLL_VIDEO1_CTRL_REG   0x0050
static struct ccu_nm pll_video1_clk = {
	.output			= BIT(27),
	.lock			= BIT(28),
	.lock_enable		= BIT(29),
	.ldo_en 		= BIT(30),
	.enable			= BIT(31),
	.n			= _SUNXI_CCU_MULT_MIN_MAX(8, 8, 53, 105),
	.min_rate		= 1272000000,
	.max_rate		= 2520000000UL,
	.common			= {
		.reg		= 0x0050,
		.hw.init	= CLK_HW_INIT("pll-video1", "dcxo24M",
					&ccu_nm_ops,
					CLK_SET_RATE_UNGATE |
					CLK_IS_CRITICAL),
	},
};

static SUNXI_CCU_M(pll_video1_4x_clk, "pll-video1-4x",
			"pll-video1", 0x0050,
			20, 3, 0);	/* p0 */

static SUNXI_CCU_M(pll_video1_3x_clk, "pll-video1-3x",
			"pll-video1", 0x0050,
			16, 3, 0);	/* p1 */

#define SUN50IW15_PLL_VE_CTRL_REG   0x0058
static struct ccu_nm pll_ve_vco_clk = {
	.output			= BIT(27),
	.lock			= BIT(28),
	.lock_enable		= BIT(29),
	.ldo_en 		= BIT(30),
	.enable			= BIT(31),
	.n			= _SUNXI_CCU_MULT_MIN_MAX(8, 8, 53, 105),
	.min_rate		= 1272000000,
	.max_rate		= 2520000000UL,
	.common			= {
		.reg		= 0x0058,
		.hw.init	= CLK_HW_INIT("pll-ve-vco", "dcxo24M",
					&ccu_nm_ops,
					CLK_SET_RATE_UNGATE |
					CLK_IS_CRITICAL),
	},
};

static SUNXI_CCU_M(pll_ve_clk, "pll-ve",
			"pll-ve-vco", 0x0058,
			20, 3, 0);	/* p0 */

#define SUN50IW15_PLL_ADC_CTRL_REG   0x0060
static struct ccu_nm pll_adc_vco_clk = {
	.output			= BIT(27),
	.lock			= BIT(28),
	.lock_enable		= BIT(29),
	.ldo_en 		= BIT(30),
	.enable			= BIT(31),
	.n			= _SUNXI_CCU_MULT_MIN_MAX(8, 8, 53, 105),
	.min_rate		= 1272000000,
	.max_rate		= 2520000000UL,
	.common			= {
		.reg		= 0x0060,
		.hw.init	= CLK_HW_INIT("pll-adc-vco", "dcxo24M",
					&ccu_nm_ops,
					CLK_SET_RATE_UNGATE |
					CLK_IS_CRITICAL),
	},
};

static SUNXI_CCU_M(pll_adc_clk, "pll-adc",
			"pll-adc-vco", 0x0060,
			20, 3, 0);	/* p0 */

#define SUN50IW15_PLL_VIDEO2_CTRL_REG   0x0068
static struct ccu_nm pll_video2_clk = {
	.output			= BIT(27),
	.lock			= BIT(28),
	.lock_enable		= BIT(29),
	.ldo_en 		= BIT(30),
	.enable			= BIT(31),
	.n			= _SUNXI_CCU_MULT_MIN_MAX(8, 8, 53, 105),
	.min_rate		= 1272000000,
	.max_rate		= 2520000000UL,
	.common			= {
		.reg		= 0x0068,
		.hw.init	= CLK_HW_INIT("pll-video2", "dcxo24M",
					&ccu_nm_ops,
					CLK_SET_RATE_UNGATE |
					CLK_IS_CRITICAL),
	},
};

static SUNXI_CCU_M(pll_video2_4x_clk, "pll-video2-4x",
			"pll-video2", 0x0068,
			20, 3, 0);	/* p0 */

static SUNXI_CCU_M(pll_video2_3x_clk, "pll-video2-3x",
			"pll-video2", 0x0068,
			16, 3, 0);	/* p1 */

#define SUN50IW15_PLL_AUDIO_CTRL_REG   0x0078
static struct ccu_sdm_setting pll_audio_sdm_table[] = {
	{ .rate = 196608000,  .pattern = 0xC001EB85, .m = 5, .n = 40 },	/* 24.576 */
	{ .rate = 1083801600, .pattern = 0xA000A234, .m = 2, .n = 90 },	/* 22.5792 */
};

static struct ccu_nm pll_audio_4x_clk = {
	.output			= BIT(27),
	.lock			= BIT(28),
	.lock_enable		= BIT(29),
	.ldo_en 		= BIT(30),
	.enable			= BIT(31),
	.n			= _SUNXI_CCU_MULT_MIN(8, 8, 12),
	.m			= _SUNXI_CCU_DIV(16, 6),
	.sdm			= _SUNXI_CCU_SDM(pll_audio_sdm_table, BIT(24),
				0x0178, BIT(31)),
	.min_rate		= 196608000,
	.common			= {
		.reg		= 0x0078,
		.features	= CCU_FEATURE_SIGMA_DELTA_MOD,
		.hw.init	= CLK_HW_INIT("pll-audio-4x", "dcxo24M",
					&ccu_nm_ops,
					CLK_SET_RATE_UNGATE),
	},
};

#define SUN50IW15_PLL_CPU_CTRL_REG   0x0080
static struct ccu_nkmp pll_cpu_clk = {
	.output			= BIT(27),
	.lock			= BIT(28),
	.lock_enable		= BIT(29),
	.ldo_en 		= BIT(30),
	.enable			= BIT(31),
	.n			= _SUNXI_CCU_MULT_MIN(8, 8, 12),
	.m			= _SUNXI_CCU_DIV(20, 2), /* M */
	.p			= _SUNXI_CCU_DIV(16, 3), /* P */
	.common			= {
		.reg		= 0x0080,
		.hw.init	= CLK_HW_INIT("pll-cpu", "dcxo24M",
					&ccu_nkmp_ops,
					CLK_SET_RATE_UNGATE |
					CLK_IS_CRITICAL),
	},
};

static const char * const ahb0_parents[] = { "dcxo24M", "clk32k", "rc-16m", "pll-peri0-600m" };

static SUNXI_CCU_M_WITH_MUX(ahb0_clk, "ahb0", ahb0_parents,
			0x0510, 0, 5, 24, 2, 0);

static const char * const apb0_parents[] = { "dcxo24M", "clk32k", "rc-16m", "pll-peri0-600m" };

static SUNXI_CCU_M_WITH_MUX(apb0_clk, "apb0", apb0_parents,
			0x0520, 0, 5, 24, 2, 0);

static const char * const apb1_parents[] = { "dcxo24M", "clk32k", "rc-16m", "pll-peri0-600m" };

static SUNXI_CCU_M_WITH_MUX(apb1_clk, "apb1", apb1_parents,
			0x0524, 0, 5, 24, 2, 0);

static const char * const apb_uart_parents[] = { "dcxo24M", "clk32k", "rc-16m", "pll-peri0-600m", "pll-peri0-480m" };

static SUNXI_CCU_M_WITH_MUX(apb_uart_clk, "apb-uart", apb_uart_parents,
			0x0528, 0, 5, 24, 3, 0);

static const char * const mbus_parents[] = { "dcxo24M", "hdr-clk", "pll-peri0-600m", "pll-peri0-480m", "pll-peri0-400m" };

static SUNXI_CCU_M_WITH_MUX_GATE(mbus_clk, "mbus",
			mbus_parents, 0x0540,
			0, 5,	/* M */
			24, 3,	/* mux */
			BIT(31),	/* gate */
			CLK_IGNORE_UNUSED);

static const char * const nsi_parents[] = { "dcxo24M", "pll-video2-3x", "pll-peri0-600m", "pll-peri0-480m", "pll-peri0-400m", "hdr-clk" };

static SUNXI_CCU_M_WITH_MUX_GATE(nsi_clk, "nsi",
			nsi_parents, 0x0544,
			0, 5,	/* M */
			24, 3,	/* mux */
			BIT(31),	/* gate */
			0);

static SUNXI_CCU_GATE(ve_ahb_sw_cfg_bus_clk, "ve-ahb-sw-cfg-bus",
			"dcxo24M",
			0x05C0, BIT(9), CLK_IGNORE_UNUSED);

static SUNXI_CCU_GATE(tvdisp_ahb_sw_cfg_bus_clk, "tvdisp-ahb-sw-cfg-bus",
			"dcxo24M",
			0x05C0, BIT(8), CLK_IGNORE_UNUSED);

static SUNXI_CCU_GATE(tvcap_ahb_sw_cfg_bus_clk, "tvcap-ahb-sw-cfg-bus",
			"dcxo24M",
			0x05C0, BIT(7), CLK_IGNORE_UNUSED);

static SUNXI_CCU_GATE(tvfe_ahb_sw_cfg_bus_clk, "tvfe-ahb-sw-cfg-bus",
			"dcxo24M",
			0x05C0, BIT(6), CLK_IGNORE_UNUSED);

static SUNXI_CCU_GATE(store_sys_ahb_sw_cfg_bus_clk, "store-sys-ahb-sw-cfg-bus",
			"dcxo24M",
			0x05C0, BIT(5), CLK_IGNORE_UNUSED);

static SUNXI_CCU_GATE(smhc1_ahb_sw_cfg_bus_clk, "smhc1-ahb-sw-cfg-bus",
			"dcxo24M",
			0x05C0, BIT(4), CLK_IGNORE_UNUSED);

static SUNXI_CCU_GATE(smhc0_ahb_sw_cfg_bus_clk, "smhc0-ahb-sw-cfg-bus",
			"dcxo24M",
			0x05C0, BIT(3), CLK_IGNORE_UNUSED);

static SUNXI_CCU_GATE(usb2_ahb_sw_cfg_bus_clk, "usb2-ahb-sw-cfg-bus",
			"dcxo24M",
			0x05C0, BIT(2), CLK_IGNORE_UNUSED);

static SUNXI_CCU_GATE(gmac_ahb_sw_cfg_bus_clk, "gmac-ahb-sw-cfg-bus",
			"dcxo24M",
			0x05C0, BIT(1), CLK_IGNORE_UNUSED);

static SUNXI_CCU_GATE(gpu_ahb_sw_cfg_bus_clk, "gpu-ahb-sw-cfg-bus",
			"dcxo24M",
			0x05C0, BIT(0), CLK_IGNORE_UNUSED);

static SUNXI_CCU_GATE(ve_mbus_sw_cfg_bus_clk, "ve-mbus-sw-cfg-bus",
			"dcxo24M",
			0x05E0, BIT(4), CLK_IGNORE_UNUSED);

static SUNXI_CCU_GATE(tvdisp_mbus_sw_cfg_bus_clk, "tvdisp-mbus-sw-cfg-bus",
			"dcxo24M",
			0x05E0, BIT(3), CLK_IGNORE_UNUSED);

static SUNXI_CCU_GATE(tvcap_mbus_sw_cfg_bus_clk, "tvcap-mbus-sw-cfg-bus",
			"dcxo24M",
			0x05E0, BIT(2), CLK_IGNORE_UNUSED);

static SUNXI_CCU_GATE(tvfe_mbus_sw_cfg_bus_clk, "tvfe-mbus-sw-cfg-bus",
			"dcxo24M",
			0x05E0, BIT(1), CLK_IGNORE_UNUSED);

static SUNXI_CCU_GATE(gpu_mbus_sw_cfg_bus_clk, "gpu-mbus-sw-cfg-bus",
			"dcxo24M",
			0x05E0, BIT(0), CLK_IGNORE_UNUSED);
static const char * const gpu_core_parents[] = { "pll-gpu", "pll-peri0-800m", "pll-peri0-600m", "pll-peri0-400m", "pll-peri0-300m", "pll-peri0-200m" };

static SUNXI_CCU_M_WITH_MUX_GATE(gpu_core_clk, "gpu-core",
			gpu_core_parents, 0x0670,
			0, 5,	/* M */
			24, 3,	/* mux */
			BIT(31),	/* gate */
			CLK_IGNORE_UNUSED);

static SUNXI_CCU_GATE(gpu_bus_clk, "gpu-bus",
			"dcxo24M",
			0x067C, BIT(0), CLK_IGNORE_UNUSED);

static const char * const ce_parents[] = { "dcxo24M", "pll-peri0-600m", "pll-peri0-480m", "pll-peri0-400m" };

static SUNXI_CCU_M_WITH_MUX_GATE(ce_clk, "ce",
			ce_parents, 0x0680,
			0, 5,	/* M */
			24, 3,	/* mux */
			BIT(31), CLK_IGNORE_UNUSED);

static SUNXI_CCU_GATE(ce_sys_bus_clk, "ce-sys-bus",
			"dcxo24M",
			0x068C, BIT(1), CLK_IGNORE_UNUSED);

static SUNXI_CCU_GATE(ce_bus_clk, "ce-bus",
			"dcxo24M",
			0x068C, BIT(0), CLK_IGNORE_UNUSED);

static const char * const ve_parents[] = { "pll-ve", "pll-peri0-800m", "pll-peri0-600m", "pll-peri0-480m" };

static SUNXI_CCU_M_WITH_MUX_GATE(ve_clk, "ve",
			ve_parents, 0x0690,
			0, 5,	/* M */
			24, 3,	/* mux */
			BIT(31),	/* gate */
			0);

static SUNXI_CCU_GATE(ve_bus_clk, "ve-bus",
			"dcxo24M",
			0x069C, BIT(0), CLK_IGNORE_UNUSED);

static SUNXI_CCU_GATE(dma_bus_clk, "dma-bus",
			"dcxo24M",
			0x070C, BIT(0), CLK_IGNORE_UNUSED);

static SUNXI_CCU_GATE(msgbox_bus_clk, "msgbox-bus",
			"dcxo24M",
			0x071C, BIT(0), CLK_IGNORE_UNUSED);

static SUNXI_CCU_GATE(spinlock_bus_clk, "spinlock-bus",
			"dcxo24M",
			0x072C, BIT(0), CLK_IGNORE_UNUSED);

static const char * const timer_clk_parents[] = { "dcxo24M", "rc-16m", "clk32k", "pll-peri0-200m" };

static struct ccu_div timer00_clk_clk = {
	.enable			= BIT(0),
	.div			= _SUNXI_CCU_DIV_FLAGS(1, 3, CLK_DIVIDER_POWER_OF_TWO),
	.mux			= _SUNXI_CCU_MUX(4, 3), /* mux */
	.common			= {
		.reg		= 0x0730,
		.hw.init	= CLK_HW_INIT_PARENTS("timer00-clk",
					timer_clk_parents,
					&ccu_div_ops, 0),
	},
};

static struct ccu_div timer01_clk_clk = {
	.enable			= BIT(0),
	.div			= _SUNXI_CCU_DIV_FLAGS(1, 3, CLK_DIVIDER_POWER_OF_TWO),
	.mux			= _SUNXI_CCU_MUX(4, 3), /* mux */
	.common			= {
		.reg		= 0x0734,
		.hw.init	= CLK_HW_INIT_PARENTS("timer01-clk",
					timer_clk_parents,
					&ccu_div_ops, 0),
	},
};

static struct ccu_div timer02_clk_clk = {
	.enable			= BIT(0),
	.div			= _SUNXI_CCU_DIV_FLAGS(1, 3, CLK_DIVIDER_POWER_OF_TWO),
	.mux			= _SUNXI_CCU_MUX(4, 3), /* mux */
	.common			= {
		.reg		= 0x0738,
		.hw.init	= CLK_HW_INIT_PARENTS("timer02-clk",
					timer_clk_parents,
					&ccu_div_ops, 0),
	},
};

static SUNXI_CCU_GATE(timer0_bus_clk, "timer0-bus",
			"dcxo24M",
			0x0750, BIT(0), CLK_IGNORE_UNUSED);

static SUNXI_CCU_GATE(dbgsys_bus_clk, "dbgsys-bus",
			"dcxo24M",
			0x078C, BIT(0), 0);

static SUNXI_CCU_GATE(pwm_bus_clk, "pwm-bus",
			"dcxo24M",
			0x07AC, BIT(0), CLK_IGNORE_UNUSED);

static SUNXI_CCU_GATE(cec_peri_bus_clk, "cec-peri-bus",
			"dcxo24M",
			0x07B0, BIT(0), CLK_IGNORE_UNUSED);

static SUNXI_CCU_GATE(iommu_bus_clk, "iommu-bus",
			"dcxo24M",
			0x07BC, BIT(0), CLK_IGNORE_UNUSED);

static SUNXI_CCU_GATE(tvfe_mbus_clk, "tvfe-mbus",
			"dcxo24M",
			0x0804, BIT(3), CLK_IGNORE_UNUSED);

static SUNXI_CCU_GATE(ce_mbus_clk, "ce-mbus",
			"dcxo24M",
			0x0804, BIT(2), CLK_IGNORE_UNUSED);

static SUNXI_CCU_GATE(ve3_mbus_clk, "ve3-mbus",
			"dcxo24M",
			0x0804, BIT(1), CLK_IGNORE_UNUSED);

static SUNXI_CCU_GATE(dma_mbus_clk, "dma-mbus",
			"dcxo24M",
			0x0804, BIT(0), CLK_IGNORE_UNUSED);

static SUNXI_CCU_GATE(dram_bus_clk, "dram-bus",
			"dcxo24M",
			0x080C, BIT(0), CLK_IGNORE_UNUSED);

static const char * const smhc0_parents[] = { "dcxo24M", "pll-peri0-400m", "pll-peri0-300m", "pll-peri1-400m", "pll-peri1-300m" };

static SUNXI_CCU_MP_WITH_MUX_GATE_NO_INDEX(smhc0_clk, "smhc0",
			smhc0_parents, 0x0830,
			0, 5,	/* M */
			8, 5,	/* N */
			24, 3,	/* mux */
			BIT(31), 0);

static const char * const smhc1_parents[] = { "dcxo24M", "pll-peri0-400m", "pll-peri0-300m", "pll-peri1-400m", "pll-peri1-300m" };

static SUNXI_CCU_MP_WITH_MUX_GATE_NO_INDEX(smhc1_clk, "smhc1",
			smhc1_parents, 0x0834,
			0, 5,	/* M */
			8, 5,	/* N */
			24, 3,	/* mux */
			BIT(31), 0);

static const char * const smhc2_parents[] = { "dcxo24M", "pll-peri0-800m", "pll-peri0-600m", "pll-peri1-800m", "pll-peri1-600m" };

static SUNXI_CCU_MP_WITH_MUX_GATE_NO_INDEX(smhc2_clk, "smhc2",
			smhc2_parents, 0x0838,
			0, 5,	/* M */
			8, 5,	/* N */
			24, 3,	/* mux */
			BIT(31), 0);

static SUNXI_CCU_GATE(smhc2_bus_clk, "smhc2-bus",
			"dcxo24M",
			0x084C, BIT(2), CLK_IGNORE_UNUSED);

static SUNXI_CCU_GATE(smhc1_bus_clk, "smhc1-bus",
			"dcxo24M",
			0x084C, BIT(1), CLK_IGNORE_UNUSED);

static SUNXI_CCU_GATE(smhc0_bus_clk, "smhc0-bus",
			"dcxo24M",
			0x084C, BIT(0), CLK_IGNORE_UNUSED);

static SUNXI_CCU_GATE(uart3_bus_clk, "uart3-bus",
			"dcxo24M",
			0x090C, BIT(3), CLK_IGNORE_UNUSED);

static SUNXI_CCU_GATE(uart2_bus_clk, "uart2-bus",
			"dcxo24M",
			0x090C, BIT(2), CLK_IGNORE_UNUSED);

static SUNXI_CCU_GATE(uart1_bus_clk, "uart1-bus",
			"dcxo24M",
			0x090C, BIT(1), CLK_IGNORE_UNUSED);

static SUNXI_CCU_GATE(uart0_bus_clk, "uart0-bus",
			"dcxo24M",
			0x090C, BIT(0), CLK_IGNORE_UNUSED);

static SUNXI_CCU_GATE(twi4_bus_clk, "twi4-bus",
			"dcxo24M",
			0x091C, BIT(4), CLK_IGNORE_UNUSED);

static SUNXI_CCU_GATE(twi3_bus_clk, "twi3-bus",
			"dcxo24M",
			0x091C, BIT(3), CLK_IGNORE_UNUSED);

static SUNXI_CCU_GATE(twi2_bus_clk, "twi2-bus",
			"dcxo24M",
			0x091C, BIT(2), CLK_IGNORE_UNUSED);

static SUNXI_CCU_GATE(twi1_bus_clk, "twi1-bus",
			"dcxo24M",
			0x091C, BIT(1), CLK_IGNORE_UNUSED);

static SUNXI_CCU_GATE(twi0_bus_clk, "twi0-bus",
			"dcxo24M",
			0x091C, BIT(0), CLK_IGNORE_UNUSED);

static const char * const spi0_parents[] = { "dcxo24M", "pll-peri0-300m", "pll-peri0-200m", "pll-peri0-160m", "pll-peri1-300m", "pll-peri1-200m", "pll-peri1-160m" };

static SUNXI_CCU_MP_WITH_MUX_GATE_NO_INDEX(spi0_clk, "spi0",
			spi0_parents, 0x0940,
			0, 5,	/* M */
			8, 5,	/* N */
			24, 3,	/* mux */
			BIT(31), 0);

static const char * const spi1_parents[] = { "dcxo24M", "pll-peri0-300m", "pll-peri0-200m", "pll-peri0-160m", "pll-peri1-300m", "pll-peri1-200m", "pll-peri1-160m" };

static SUNXI_CCU_MP_WITH_MUX_GATE_NO_INDEX(spi1_clk, "spi1",
			spi1_parents, 0x0944,
			0, 5,	/* M */
			8, 5,	/* N */
			24, 3,	/* mux */
			BIT(31), 0);

static const char * const spi2_parents[] = { "dcxo24M", "pll-peri0-300m", "pll-peri0-200m", "pll-peri0-160m", "pll-peri1-300m", "pll-peri1-200m", "pll-peri1-160m" };

static SUNXI_CCU_MP_WITH_MUX_GATE_NO_INDEX(spi2_clk, "spi2",
			spi2_parents, 0x0948,
			0, 5,	/* M */
			8, 5,	/* N */
			24, 3,	/* mux */
			BIT(31), 0);

static SUNXI_CCU_GATE(spi2_bus_clk, "spi2-bus",
			"dcxo24M",
			0x096C, BIT(2), CLK_IGNORE_UNUSED);

static SUNXI_CCU_GATE(spi1_bus_clk, "spi1-bus",
			"dcxo24M",
			0x096C, BIT(1), CLK_IGNORE_UNUSED);

static SUNXI_CCU_GATE(spi0_bus_clk, "spi0-bus",
			"dcxo24M",
			0x096C, BIT(0), CLK_IGNORE_UNUSED);

static SUNXI_CCU_M_WITH_GATE(gmac0_phy_clk, "gmac0-phy",
			"pll-peri0-150m", 0x0970,
			0, 5,
			BIT(31), 0);

static SUNXI_CCU_GATE(gmac0_bus_clk, "gmac0-bus",
			"dcxo24M",
			0x097C, BIT(0), CLK_IGNORE_UNUSED);

static const char * const i2spcm0_parents[] = { "pll-audio-4x", "tvfe1296m", "pll-peri0-200m" };

static SUNXI_CCU_M_WITH_MUX_GATE(i2spcm0_clk, "i2spcm0",
			i2spcm0_parents, 0x0A10,
			0, 5,	/* M */
			24, 3,	/* mux */
			BIT(31),	/* gate */
			0);

static SUNXI_CCU_GATE(i2spcm0_bus_clk, "i2spcm0-bus",
			"dcxo24M",
			0x0A20, BIT(0), CLK_IGNORE_UNUSED);

static SUNXI_CCU_GATE(owa1_bus_clk, "owa1-bus",
			"dcxo24M",
			0x0A2C, BIT(1), CLK_IGNORE_UNUSED);

static SUNXI_CCU_GATE(owa0_bus_clk, "owa0-bus",
			"dcxo24M",
			0x0A2C, BIT(0), CLK_IGNORE_UNUSED);

static const char * const owa0_rx_parents[] = { "pll-peri0-400m", "pll-peri0-300m", "pll-audio-4x" };

static SUNXI_CCU_M_WITH_MUX_GATE(owa0_rx_clk, "owa0-rx",
			owa0_rx_parents, 0x0A30,
			0, 5,	/* M */
			24, 3,	/* mux */
			BIT(31),	/* gate */
			0);

static const char * const owa0_tx_parents[] = { "pll-audio-4x", "tvfe1296m" };

static SUNXI_CCU_M_WITH_MUX_GATE(owa0_tx_clk, "owa0-tx",
			owa0_tx_parents, 0x0A34,
			0, 5,	/* M */
			24, 1,	/* mux */
			BIT(31),	/* gate */
			0);

static const char * const owa1_rx_parents[] = { "pll-peri0-400m", "pll-peri0-300m", "pll-audio-4x" };

static SUNXI_CCU_M_WITH_MUX_GATE(owa1_rx_clk, "owa1-rx",
			owa1_rx_parents, 0x0A40,
			0, 5,	/* M */
			24, 3,	/* mux */
			BIT(31),	/* gate */
			0);

static const char * const owa1_tx_parents[] = { "pll-audio-4x", "tvfe1296m"};

static SUNXI_CCU_M_WITH_MUX_GATE(owa1_tx_clk, "owa1-tx",
			owa1_tx_parents, 0x0A44,
			0, 5,	/* M */
			24, 1,	/* mux */
			BIT(31),	/* gate */
			0);

static const char * const audio_codec_dac_parents[] = { "pll-audio-4x", "tvfe1296m" };

static SUNXI_CCU_M_WITH_MUX_GATE(audio_codec_dac_clk, "audio-codec-dac",
			audio_codec_dac_parents, 0x0A60,
			0, 5,	/* M */
			24, 1,	/* mux */
			BIT(31),	/* gate */
			0);

static const char * const audio_codec_adc_parents[] = { "pll-audio-4x", "tvfe1296m" };

static SUNXI_CCU_M_WITH_MUX_GATE(audio_codec_adc_clk, "audio-codec-adc",
			audio_codec_adc_parents, 0x0A64,
			0, 5,	/* M */
			24, 1,	/* mux */
			BIT(31),	/* gate */
			0);

static SUNXI_CCU_GATE(audio_codec_bus_clk, "audio-codec-bus",
			"dcxo24M",
			0x0A6C, BIT(0), CLK_IGNORE_UNUSED);

static SUNXI_CCU_GATE(usb0_bus_clk, "usb0-bus",
			"dcxo24M",
			0x0A70, BIT(31), CLK_IGNORE_UNUSED);

static SUNXI_CCU_GATE(usb1_bus_clk, "usb1-bus",
			"dcxo24M",
			0x0A78, BIT(31), CLK_IGNORE_UNUSED);

static SUNXI_CCU_GATE(usb2_bus_clk, "usb2-bus",
			"dcxo24M",
			0x0A80, BIT(31), CLK_IGNORE_UNUSED);

static SUNXI_CCU_GATE(usbotg_bus_clk, "usbotg-bus",
			"dcxo24M",
			0x0A8C, BIT(8), CLK_IGNORE_UNUSED);

static SUNXI_CCU_GATE(usbehci2_bus_clk, "usbehci2-bus",
			"dcxo24M",
			0x0A8C, BIT(6), CLK_IGNORE_UNUSED);

static SUNXI_CCU_GATE(usbehci1_bus_clk, "usbehci1-bus",
			"dcxo24M",
			0x0A8C, BIT(5), CLK_IGNORE_UNUSED);

static SUNXI_CCU_GATE(usbehci0_bus_clk, "usbehci0-bus",
			"dcxo24M",
			0x0A8C, BIT(4), CLK_IGNORE_UNUSED);

static SUNXI_CCU_GATE(usbohci2_bus_clk, "usbohci2-bus",
			"dcxo24M",
			0x0A8C, BIT(2), CLK_IGNORE_UNUSED);

static SUNXI_CCU_GATE(usbohci1_bus_clk, "usbohci1-bus",
			"dcxo24M",
			0x0A8C, BIT(1), CLK_IGNORE_UNUSED);

static SUNXI_CCU_GATE(usbohci0_bus_clk, "usbohci0-bus",
			"dcxo24M",
			0x0A8C, BIT(0), CLK_IGNORE_UNUSED);

static const char * const tvfe_axi_parents[] = { "dcxo24M", "clk32k", "rc-16m", "pll-peri0-600m" };

static SUNXI_CCU_M_WITH_MUX_GATE(tvfe_axi_clk, "tvfe-axi",
			tvfe_axi_parents, 0x0D00,
			0, 5,	/* M */
			24, 2,	/* mux */
			BIT(31),	/* gate */
			0);

static SUNXI_CCU_GATE(tvfe_axi_bus_clk, "tvfe-axi-bus",
			"dcxo24M",
			0x0D0C, BIT(0), CLK_IGNORE_UNUSED);

static const char * const adc_parents[] = { "pll-adc", "pll-video0-1x" };

static SUNXI_CCU_MP_WITH_MUX_GATE(adc_clk, "adc",
			adc_parents, 0x0D10,
			0, 5,	/* M */
			8, 2,	/* P */
			24, 1,	/* mux */
			BIT(31),	/* gate */
			0);

static SUNXI_CCU_M_WITH_GATE(tsdm_ts_clk, "tsdm-ts",
			"pll-peri0-600m", 0x0D14,
			0, 5,	/* M */
			BIT(31),	/* gate */
			0);

static const char * const tvfe1296m_parents[] = { "pll-video0-4x", "pll-adc" };

static SUNXI_CCU_M_WITH_MUX_GATE(tvfe1296m_clk, "tvfe1296m",
			tvfe1296m_parents, 0x0D20,
			0, 5,	/* M */
			24, 1,	/* mux */
			BIT(31),	/* gate */
			0);

static const char * const i2h_parents[] = { "pll-peri0-2x", "pll-peri1-2x", "pll-video0-4x" };

static SUNXI_CCU_M_WITH_MUX_GATE(i2h_clk, "i2h",
			i2h_parents, 0x0D24,
			0, 5,	/* M */
			24, 3,	/* mux */
			BIT(31),	/* gate */
			0);

static const char * const audio_cpu_parents[] = { "pll-peri0-800m", "pll-peri0-600m", "pll-peri0-480m", "pll-peri1-800m", "pll-peri1-600m", "pll-peri1-480m" };

static SUNXI_CCU_M_WITH_MUX_GATE(audio_cpu_clk, "audio-cpu",
			audio_cpu_parents, 0x0D48,
			0, 5,	/* M */
			24, 3,	/* mux */
			BIT(31),	/* gate */
			0);

static const char * const mcsi_parents[] = { "pll-peri0-600m", "pll-peri0-480m", "pll-peri0-400m", "pll-peri0-150m", "pll-video1-3x", "pll-video2-3x", "pll-peri0-2x" };

static SUNXI_CCU_M_WITH_MUX_GATE(mcsi_clk, "mcsi",
			mcsi_parents, 0x0D4C,
			0, 5,	/* M */
			24, 3,	/* mux */
			BIT(31),	/* gate */
			0);

static const char * const csi_master_parents[] = { "dcxo24M", "pll-video0-4x", "pll-video0-1x", "pll-video1-4x", "pll-video1-3x" };

static SUNXI_CCU_M_WITH_MUX_GATE(csi_master_clk, "csi-master",
			csi_master_parents, 0x0D50,
			0, 5,	/* M */
			24, 3,	/* mux */
			BIT(31),	/* gate */
			0);

static SUNXI_CCU_GATE(mcsi_bus_clk, "mcsi-bus",
			"dcxo24M",
			0x0D64, BIT(7), CLK_IGNORE_UNUSED);

static const char * const tcd3_parents[] = { "pll-video0-4x", "pll-video0-1x", "pll-adc" };

static SUNXI_CCU_M_WITH_MUX_GATE(tcd3_clk, "tcd3",
			tcd3_parents, 0x0D6C,
			0, 5,	/* M */
			24, 3,	/* mux */
			BIT(31), 0);

static const char * const vincap_dma_parents[] = { "pll-peri0-400m", "pll-peri0-480m", "pll-peri0-600m", "pll-video0-4x", "pll-video1-4x", "pll-video2-4x" };

static SUNXI_CCU_M_WITH_MUX_GATE(vincap_dma_clk, "vincap-dma",
			vincap_dma_parents, 0x0D74,
			0, 5,	/* M */
			24, 3,	/* mux */
			BIT(31),	/* gate */
			0);

static SUNXI_CCU_GATE(hdmi_audio_clk, "hdmi-audio",
			"dcxo24M",
			0x0D80, BIT(31), 0);

static SUNXI_CCU_GATE(cap_300m_clk, "cap-300m",
			"dcxo24M",
			0x0D80, BIT(30), 0);

static SUNXI_CCU_GATE(tvcap_bus_clk, "tvcap-bus",
			"dcxo24M",
			0x0D88, BIT(0), CLK_IGNORE_UNUSED);

static const char * const deint_parents[] = { "pll-video1-4x", "lvds-clk" };

static SUNXI_CCU_M_WITH_MUX_GATE(deint_clk, "deint",
			deint_parents, 0x0DB0,
			0, 5,	/* M */
			24, 1,	/* mux */
			BIT(31),	/* gate */
			0);

static const char * const panel_parents[] = { "pll-video1-4x", "lvds-clk" };

static SUNXI_CCU_M_WITH_MUX_GATE(panel_clk, "panel",
			panel_parents, 0x0DB4,
			0, 5,	/* M */
			24, 1,	/* mux */
			BIT(31),	/* gate */
			0);

static const char * const svp_dtl_parents[] = { "pll-peri0-2x", "pll-video0-4x", "pll-peri1-2x" };

static SUNXI_CCU_M_WITH_MUX_GATE(svp_dtl_clk, "svp-dtl",
			svp_dtl_parents, 0x0DB8,
			0, 5,	/* M */
			24, 3,	/* mux */
			BIT(31),	/* gate */
			0);

static const char * const ksc_parents[] = { "pll-peri0-800m", "pll-peri0-600m", "pll-peri0-480m", "pll-video0-4x", "pll-video1-4x", "pll-video2-4x" };

static SUNXI_CCU_M_WITH_MUX_GATE(ksc_clk, "ksc",
			ksc_parents, 0x0DBC,
			0, 5,	/* M */
			24, 3,	/* mux */
			BIT(31),	/* gate */
			0);

static const char * const afbd_parents[] = { "pll-peri0-800m", "pll-peri0-600m", "pll-peri0-480m", "pll-video0-4x", "pll-adc" };

static SUNXI_CCU_M_WITH_MUX_GATE(afbd_clk, "afbd",
			afbd_parents, 0x0DC0,
			0, 5,	/* M */
			24, 3,	/* mux */
			BIT(31),	/* gate */
			0);

static const char * const dsi_parents[] = { "pll-peri0-2x", "pll-video0-4x", "pll-peri1-2x" };

static SUNXI_CCU_M_WITH_MUX_GATE(dsi_clk, "dsi",
			dsi_parents, 0x0DC4,
			0, 5,	/* M */
			24, 3,	/* mux */
			BIT(31),	/* gate */
			CLK_IGNORE_UNUSED);

static const char * const svp_pclk_deint_parents[] = { "pll-peri0-150m", "pll-peri0-400m", "pll-peri0-480m", "pll-video1-4x" };

static SUNXI_CCU_M_WITH_MUX_GATE(svp_pclk_deint_clk, "svp-pclk-deint",
			svp_pclk_deint_parents, 0x0DC8,
			0, 5,	/* M */
			24, 3,	/* mux */
			BIT(31),	/* gate */
			0);

static const char * const svp_pclk_nr_meter_parents[] = { "pll-peri0-200m", "pll-peri0-150m", "pll-peri0-400m", "pll-peri0-480m", "pll-video1-4x" };

static SUNXI_CCU_M_WITH_MUX_GATE(svp_pclk_nr_meter_clk, "svp-pclk-nr-meter",
			svp_pclk_nr_meter_parents, 0x0DCC,
			0, 5,	/* M */
			24, 3,	/* mux */
			BIT(31),	/* gate */
			0);

static const char * const combophy_parents[] = { "pll-video1-4x", "pll-video1-3x" };

static SUNXI_CCU_M_WITH_MUX_GATE(combophy_clk, "combophy",
			combophy_parents, 0x0DD0,
			0, 5,	/* M */
			24, 3,	/* mux */
			BIT(31),	/* gate */
			CLK_IGNORE_UNUSED);

static SUNXI_CCU_GATE(disp_bus_clk, "disp-bus",
			"dcxo24M",
			0x0DD8, BIT(0), CLK_IGNORE_UNUSED);
/* ccu_des_end */

/* rst_def_start */
static struct ccu_reset_map sun50iw15_ccu_resets[] = {
	[RST_BUS_PLL_SSC_RSTN]		= { 0x0404, BIT(30) },
	[RST_MBUS]			= { 0x0540, BIT(30) },
	[RST_BUS_NSI]			= { 0x0544, BIT(30) },
	[RST_BUS_GPU]			= { 0x067c, BIT(16) },
	[RST_BUS_CE_SY]			= { 0x068c, BIT(17) },
	[RST_BUS_CE]			= { 0x068c, BIT(16) },
	[RST_BUS_VE]			= { 0x069c, BIT(16) },
	[RST_BUS_DMA]			= { 0x070c, BIT(16) },
	[RST_BUS_MSGBOX]		= { 0x071c, BIT(16) },
	[RST_BUS_SPINLOCK]		= { 0x072c, BIT(16) },
	[RST_BUS_TIMER0]		= { 0x0750, BIT(16) },
	[RST_BUS_DBGSY]			= { 0x078c, BIT(16) },
	[RST_BUS_PWM]			= { 0x07ac, BIT(16) },
	[RST_BUS_DRAM]			= { 0x080c, BIT(16) },
	[RST_BUS_SMHC2]			= { 0x084c, BIT(18) },
	[RST_BUS_SMHC1]			= { 0x084c, BIT(17) },
	[RST_BUS_SMHC0]			= { 0x084c, BIT(16) },
	[RST_BUS_UART3]			= { 0x090c, BIT(19) },
	[RST_BUS_UART2]			= { 0x090c, BIT(18) },
	[RST_BUS_UART1]			= { 0x090c, BIT(17) },
	[RST_BUS_UART0]			= { 0x090c, BIT(16) },
	[RST_BUS_TWI4]			= { 0x091c, BIT(20) },
	[RST_BUS_TWI3]			= { 0x091c, BIT(19) },
	[RST_BUS_TWI2]			= { 0x091c, BIT(18) },
	[RST_BUS_TWI1]			= { 0x091c, BIT(17) },
	[RST_BUS_TWI0]			= { 0x091c, BIT(16) },
	[RST_BUS_SPI2]			= { 0x096c, BIT(18) },
	[RST_BUS_SPI1]			= { 0x096c, BIT(17) },
	[RST_BUS_SPI0]			= { 0x096c, BIT(16) },
	[RST_BUS_GMAC0]			= { 0x097c, BIT(16) },
	[RST_BUS_I2SPCM0]		= { 0x0a20, BIT(16) },
	[RST_BUS_OWA1]			= { 0x0a2c, BIT(17) },
	[RST_BUS_OWA0]			= { 0x0a2c, BIT(16) },
	[RST_BUS_AUDIO_CODEC]		= { 0x0a6c, BIT(16) },
	[RST_USB_PHY0_RSTN]		= { 0x0a70, BIT(30) },
	[RST_USB_PHY1_RSTN]		= { 0x0a78, BIT(30) },
	[RST_USB_PHY2_RSTN]		= { 0x0a80, BIT(30) },
	[RST_USB_OTG]			= { 0x0a8c, BIT(24) },
	[RST_USB_EHCI2]			= { 0x0a8c, BIT(22) },
	[RST_USB_EHCI1]			= { 0x0a8c, BIT(21) },
	[RST_USB_EHCI0]			= { 0x0a8c, BIT(20) },
	[RST_USB_OHCI2]			= { 0x0a8c, BIT(18) },
	[RST_USB_OHCI1]			= { 0x0a8c, BIT(17) },
	[RST_USB_OHCI0]			= { 0x0a8c, BIT(16) },
	[RST_BUS_LVD]			= { 0x0bac, BIT(16) },
	[RST_BUS_TVFE_AXI]		= { 0x0d0c, BIT(16) },
	[RST_BUS_MCSI]			= { 0x0d64, BIT(23) },
	[RST_BUS_TVFE_AHB]		= { 0x0d64, BIT(22) },
	[RST_BUS_TVFE_MBU]		= { 0x0d64, BIT(21) },
	[RST_BUS_CGU_FA]		= { 0x0d64, BIT(20) },
	[RST_BUS_AUDIO_AHB]		= { 0x0d64, BIT(19) },
	[RST_BUS_AUDIO_AXI]		= { 0x0d64, BIT(18) },
	[RST_BUS_AUDIO_CORE]		= { 0x0d64, BIT(17) },
	[RST_BUS_DEMOD]			= { 0x0d64, BIT(16) },
	[RST_BUS_TVCAP]			= { 0x0d88, BIT(16) },
	[RST_BUS_DISP]			= { 0x0dd8, BIT(16) },
};
/* rst_def_end */

/* ccu_def_start */
static struct clk_hw_onecell_data sun50iw15_hw_clks = {
	.hws    = {
		[CLK_PLL_PERI0]			= &pll_peri0_clk.common.hw,
		[CLK_PLL_PERI0_2X]		= &pll_peri0_2x_clk.common.hw,
		[CLK_PLL_PERI0_800M]		= &pll_peri0_800m_clk.common.hw,
		[CLK_PLL_PERI0_480M]		= &pll_peri0_480m_clk.common.hw,
		[CLK_PLL_PERI0_600M]		= &pll_peri0_600m_clk.hw,
		[CLK_PLL_PERI0_400M]		= &pll_peri0_400m_clk.hw,
		[CLK_PLL_PERI0_300M]		= &pll_peri0_300m_clk.hw,
		[CLK_PLL_PERI0_200M]		= &pll_peri0_200m_clk.hw,
		[CLK_PLL_PERI0_160M]		= &pll_peri0_160m_clk.hw,
		[CLK_PLL_PERI0_150M]		= &pll_peri0_150m_clk.hw,
		[CLK_PLL_PERI1]			= &pll_peri1_clk.common.hw,
		[CLK_PLL_PERI1_2X]		= &pll_peri1_2x_clk.common.hw,
		[CLK_PLL_PERI1_800M]		= &pll_peri1_800m_clk.common.hw,
		[CLK_PLL_PERI1_480M]		= &pll_peri1_480m_clk.common.hw,
		[CLK_PLL_PERI1_600M]		= &pll_peri1_600m_clk.hw,
		[CLK_PLL_PERI1_400M]		= &pll_peri1_400m_clk.hw,
		[CLK_PLL_PERI1_300M]		= &pll_peri1_300m_clk.hw,
		[CLK_PLL_PERI1_200M]		= &pll_peri1_200m_clk.hw,
		[CLK_PLL_PERI1_160M]		= &pll_peri1_160m_clk.hw,
		[CLK_PLL_PERI1_150M]		= &pll_peri1_150m_clk.hw,
		[CLK_PLL_GPU_VCO]		= &pll_gpu_vco_clk.common.hw,
		[CLK_PLL_GPU]			= &pll_gpu_clk.common.hw,
		[CLK_PLL_VIDEO0]		= &pll_video0_clk.common.hw,
		[CLK_PLL_VIDEO0_4X]		= &pll_video0_4x_clk.common.hw,
		[CLK_PLL_VIDEO0_1X]		= &pll_video0_1x_clk.common.hw,
		[CLK_PLL_VIDEO1]		= &pll_video1_clk.common.hw,
		[CLK_PLL_VIDEO1_4X]		= &pll_video1_4x_clk.common.hw,
		[CLK_PLL_VIDEO1_3X]		= &pll_video1_3x_clk.common.hw,
		[CLK_PLL_VE_VCO]		= &pll_ve_vco_clk.common.hw,
		[CLK_PLL_VE]			= &pll_ve_clk.common.hw,
		[CLK_PLL_ADC_VCO]		= &pll_adc_vco_clk.common.hw,
		[CLK_PLL_ADC]			= &pll_adc_clk.common.hw,
		[CLK_PLL_VIDEO2]		= &pll_video2_clk.common.hw,
		[CLK_PLL_VIDEO2_4X]		= &pll_video2_4x_clk.common.hw,
		[CLK_PLL_VIDEO2_3X]		= &pll_video2_3x_clk.common.hw,
		[CLK_PLL_AUDIO_4X]		= &pll_audio_4x_clk.common.hw,
		[CLK_PLL_CPU]			= &pll_cpu_clk.common.hw,
		[CLK_AHB0]			= &ahb0_clk.common.hw,
		[CLK_APB0]			= &apb0_clk.common.hw,
		[CLK_APB1]			= &apb1_clk.common.hw,
		[CLK_APB_UART]			= &apb_uart_clk.common.hw,
		[CLK_MBUS]			= &mbus_clk.common.hw,
		[CLK_NSI]			= &nsi_clk.common.hw,
		[CLK_BUS_VE_AHB_SW_CFG]		= &ve_ahb_sw_cfg_bus_clk.common.hw,
		[CLK_BUS_TVDISP_AHB_SW_CFG]		= &tvdisp_ahb_sw_cfg_bus_clk.common.hw,
		[CLK_BUS_TVCAP_AHB_SW_CFG]		= &tvcap_ahb_sw_cfg_bus_clk.common.hw,
		[CLK_BUS_TVFE_AHB_SW_CFG]		= &tvfe_ahb_sw_cfg_bus_clk.common.hw,
		[CLK_BUS_STORE_SYS_AHB_SW_CFG]		= &store_sys_ahb_sw_cfg_bus_clk.common.hw,
		[CLK_BUS_SMHC1_AHB_SW_CFG]		= &smhc1_ahb_sw_cfg_bus_clk.common.hw,
		[CLK_BUS_SMHC0_AHB_SW_CFG]		= &smhc0_ahb_sw_cfg_bus_clk.common.hw,
		[CLK_BUS_USB2_AHB_SW_CFG]		= &usb2_ahb_sw_cfg_bus_clk.common.hw,
		[CLK_BUS_GMAC_AHB_SW_CFG]		= &gmac_ahb_sw_cfg_bus_clk.common.hw,
		[CLK_BUS_GPU_AHB_SW_CFG]		= &gpu_ahb_sw_cfg_bus_clk.common.hw,
		[CLK_MBUS_BUS_VE_SW_CFG]		= &ve_mbus_sw_cfg_bus_clk.common.hw,
		[CLK_MBUS_BUS_TVDISP_SW_CFG]		= &tvdisp_mbus_sw_cfg_bus_clk.common.hw,
		[CLK_MBUS_BUS_TVCAP_SW_CFG]		= &tvcap_mbus_sw_cfg_bus_clk.common.hw,
		[CLK_MBUS_BUS_TVFE_SW_CFG]		= &tvfe_mbus_sw_cfg_bus_clk.common.hw,
		[CLK_MBUS_BUS_GPU_SW_CFG]		= &gpu_mbus_sw_cfg_bus_clk.common.hw,
		[CLK_GPU_CORE]			= &gpu_core_clk.common.hw,
		[CLK_BUS_GPU]			= &gpu_bus_clk.common.hw,
		[CLK_CE]			= &ce_clk.common.hw,
		[CLK_BUS_CE_SYS]		= &ce_sys_bus_clk.common.hw,
		[CLK_BUS_CE]			= &ce_bus_clk.common.hw,
		[CLK_VE]			= &ve_clk.common.hw,
		[CLK_BUS_VE]			= &ve_bus_clk.common.hw,
		[CLK_BUS_DMA]			= &dma_bus_clk.common.hw,
		[CLK_BUS_MSGBOX]		= &msgbox_bus_clk.common.hw,
		[CLK_BUS_SPINLOCK]		= &spinlock_bus_clk.common.hw,
		[CLK_TIMER00_CLK]		= &timer00_clk_clk.common.hw,
		[CLK_TIMER01_CLK]		= &timer01_clk_clk.common.hw,
		[CLK_TIMER02_CLK]		= &timer02_clk_clk.common.hw,
		[CLK_BUS_TIMER0]		= &timer0_bus_clk.common.hw,
		[CLK_BUS_DBGSYS]		= &dbgsys_bus_clk.common.hw,
		[CLK_BUS_PWM]			= &pwm_bus_clk.common.hw,
		[CLK_BUS_CEC_PERI]		= &cec_peri_bus_clk.common.hw,
		[CLK_BUS_IOMMU]			= &iommu_bus_clk.common.hw,
		[CLK_MBUS_TVFE]			= &tvfe_mbus_clk.common.hw,
		[CLK_MBUS_CE]			= &ce_mbus_clk.common.hw,
		[CLK_MBUS_VE3]			= &ve3_mbus_clk.common.hw,
		[CLK_MBUS_DMA]			= &dma_mbus_clk.common.hw,
		[CLK_BUS_DRAM]			= &dram_bus_clk.common.hw,
		[CLK_SMHC0]			= &smhc0_clk.common.hw,
		[CLK_SMHC1]			= &smhc1_clk.common.hw,
		[CLK_SMHC2]			= &smhc2_clk.common.hw,
		[CLK_BUS_SMHC2]			= &smhc2_bus_clk.common.hw,
		[CLK_BUS_SMHC1]			= &smhc1_bus_clk.common.hw,
		[CLK_BUS_SMHC0]			= &smhc0_bus_clk.common.hw,
		[CLK_BUS_UART3]			= &uart3_bus_clk.common.hw,
		[CLK_BUS_UART2]			= &uart2_bus_clk.common.hw,
		[CLK_BUS_UART1]			= &uart1_bus_clk.common.hw,
		[CLK_BUS_UART0]			= &uart0_bus_clk.common.hw,
		[CLK_BUS_TWI4]			= &twi4_bus_clk.common.hw,
		[CLK_BUS_TWI3]			= &twi3_bus_clk.common.hw,
		[CLK_BUS_TWI2]			= &twi2_bus_clk.common.hw,
		[CLK_BUS_TWI1]			= &twi1_bus_clk.common.hw,
		[CLK_BUS_TWI0]			= &twi0_bus_clk.common.hw,
		[CLK_SPI0]			= &spi0_clk.common.hw,
		[CLK_SPI1]			= &spi1_clk.common.hw,
		[CLK_SPI2]			= &spi2_clk.common.hw,
		[CLK_BUS_SPI2]			= &spi2_bus_clk.common.hw,
		[CLK_BUS_SPI1]			= &spi1_bus_clk.common.hw,
		[CLK_BUS_SPI0]			= &spi0_bus_clk.common.hw,
		[CLK_GMAC0_PHY]			= &gmac0_phy_clk.common.hw,
		[CLK_BUS_GMAC0]			= &gmac0_bus_clk.common.hw,
		[CLK_I2SPCM0]			= &i2spcm0_clk.common.hw,
		[CLK_BUS_I2SPCM0]		= &i2spcm0_bus_clk.common.hw,
		[CLK_BUS_OWA1]			= &owa1_bus_clk.common.hw,
		[CLK_BUS_OWA0]			= &owa0_bus_clk.common.hw,
		[CLK_OWA0_RX]			= &owa0_rx_clk.common.hw,
		[CLK_OWA0_TX]			= &owa0_tx_clk.common.hw,
		[CLK_OWA1_RX]			= &owa1_rx_clk.common.hw,
		[CLK_OWA1_TX]			= &owa1_tx_clk.common.hw,
		[CLK_AUDIO_CODEC_DAC]		= &audio_codec_dac_clk.common.hw,
		[CLK_AUDIO_CODEC_ADC]		= &audio_codec_adc_clk.common.hw,
		[CLK_BUS_AUDIO_CODEC]		= &audio_codec_bus_clk.common.hw,
		[CLK_BUS_USB0]			= &usb0_bus_clk.common.hw,
		[CLK_BUS_USB1]			= &usb1_bus_clk.common.hw,
		[CLK_BUS_USB2]			= &usb2_bus_clk.common.hw,
		[CLK_BUS_USBOTG]		= &usbotg_bus_clk.common.hw,
		[CLK_BUS_USBEHCI2]		= &usbehci2_bus_clk.common.hw,
		[CLK_BUS_USBEHCI1]		= &usbehci1_bus_clk.common.hw,
		[CLK_BUS_USBEHCI0]		= &usbehci0_bus_clk.common.hw,
		[CLK_BUS_USBOHCI2]		= &usbohci2_bus_clk.common.hw,
		[CLK_BUS_USBOHCI1]		= &usbohci1_bus_clk.common.hw,
		[CLK_BUS_USBOHCI0]		= &usbohci0_bus_clk.common.hw,
		[CLK_TVFE_AXI]			= &tvfe_axi_clk.common.hw,
		[CLK_BUS_TVFE_AXI]		= &tvfe_axi_bus_clk.common.hw,
		[CLK_ADC]			= &adc_clk.common.hw,
		[CLK_TSDM_TS]			= &tsdm_ts_clk.common.hw,
		[CLK_TVFE1296M]			= &tvfe1296m_clk.common.hw,
		[CLK_I2H]			= &i2h_clk.common.hw,
		[CLK_AUDIO_CPU]			= &audio_cpu_clk.common.hw,
		[CLK_MCSI]			= &mcsi_clk.common.hw,
		[CLK_CSI_MASTER]		= &csi_master_clk.common.hw,
		[CLK_BUS_MCSI]			= &mcsi_bus_clk.common.hw,
		[CLK_TCD3]			= &tcd3_clk.common.hw,
		[CLK_VINCAP_DMA]		= &vincap_dma_clk.common.hw,
		[CLK_TEST_HDMI_AUDIO]		= &hdmi_audio_clk.common.hw,
		[CLK_TEST_CAP_300M]		= &cap_300m_clk.common.hw,
		[CLK_BUS_TVCAP]			= &tvcap_bus_clk.common.hw,
		[CLK_DEINT]			= &deint_clk.common.hw,
		[CLK_PANEL]			= &panel_clk.common.hw,
		[CLK_SVP_DTL]			= &svp_dtl_clk.common.hw,
		[CLK_KSC]			= &ksc_clk.common.hw,
		[CLK_AFBD]			= &afbd_clk.common.hw,
		[CLK_DSI]			= &dsi_clk.common.hw,
		[CLK_SVP_PCLK_DEINT]		= &svp_pclk_deint_clk.common.hw,
		[CLK_SVP_PCLK_NR_METER]		= &svp_pclk_nr_meter_clk.common.hw,
		[CLK_COMBOPHY]			= &combophy_clk.common.hw,
		[CLK_BUS_DISP]			= &disp_bus_clk.common.hw,
	},
	.num = CLK_NUMBER,
};
/* ccu_def_end */

static struct ccu_common *sun50iw15_ccu_clks[] = {
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
	&pll_video0_1x_clk.common,
	&pll_video1_clk.common,
	&pll_video1_4x_clk.common,
	&pll_video1_3x_clk.common,
	&pll_ve_vco_clk.common,
	&pll_ve_clk.common,
	&pll_adc_vco_clk.common,
	&pll_adc_clk.common,
	&pll_video2_clk.common,
	&pll_video2_4x_clk.common,
	&pll_video2_3x_clk.common,
	&pll_audio_4x_clk.common,
	&pll_cpu_clk.common,
	&ahb0_clk.common,
	&apb0_clk.common,
	&apb1_clk.common,
	&apb_uart_clk.common,
	&mbus_clk.common,
	&nsi_clk.common,
	&ve_ahb_sw_cfg_bus_clk.common,
	&tvdisp_ahb_sw_cfg_bus_clk.common,
	&tvcap_ahb_sw_cfg_bus_clk.common,
	&tvfe_ahb_sw_cfg_bus_clk.common,
	&store_sys_ahb_sw_cfg_bus_clk.common,
	&smhc1_ahb_sw_cfg_bus_clk.common,
	&smhc0_ahb_sw_cfg_bus_clk.common,
	&usb2_ahb_sw_cfg_bus_clk.common,
	&gmac_ahb_sw_cfg_bus_clk.common,
	&gpu_ahb_sw_cfg_bus_clk.common,
	&ve_mbus_sw_cfg_bus_clk.common,
	&tvdisp_mbus_sw_cfg_bus_clk.common,
	&tvcap_mbus_sw_cfg_bus_clk.common,
	&tvfe_mbus_sw_cfg_bus_clk.common,
	&gpu_mbus_sw_cfg_bus_clk.common,
	&gpu_core_clk.common,
	&gpu_bus_clk.common,
	&ce_clk.common,
	&ce_sys_bus_clk.common,
	&ce_bus_clk.common,
	&ve_clk.common,
	&ve_bus_clk.common,
	&dma_bus_clk.common,
	&msgbox_bus_clk.common,
	&spinlock_bus_clk.common,
	&timer00_clk_clk.common,
	&timer01_clk_clk.common,
	&timer02_clk_clk.common,
	&timer0_bus_clk.common,
	&dbgsys_bus_clk.common,
	&pwm_bus_clk.common,
	&cec_peri_bus_clk.common,
	&iommu_bus_clk.common,
	&tvfe_mbus_clk.common,
	&ce_mbus_clk.common,
	&ve3_mbus_clk.common,
	&dma_mbus_clk.common,
	&dram_bus_clk.common,
	&smhc0_clk.common,
	&smhc1_clk.common,
	&smhc2_clk.common,
	&smhc2_bus_clk.common,
	&smhc1_bus_clk.common,
	&smhc0_bus_clk.common,
	&uart3_bus_clk.common,
	&uart2_bus_clk.common,
	&uart1_bus_clk.common,
	&uart0_bus_clk.common,
	&twi4_bus_clk.common,
	&twi3_bus_clk.common,
	&twi2_bus_clk.common,
	&twi1_bus_clk.common,
	&twi0_bus_clk.common,
	&spi0_clk.common,
	&spi1_clk.common,
	&spi2_clk.common,
	&spi2_bus_clk.common,
	&spi1_bus_clk.common,
	&spi0_bus_clk.common,
	&gmac0_phy_clk.common,
	&gmac0_bus_clk.common,
	&i2spcm0_clk.common,
	&i2spcm0_bus_clk.common,
	&owa1_bus_clk.common,
	&owa0_bus_clk.common,
	&owa0_rx_clk.common,
	&owa0_tx_clk.common,
	&owa1_rx_clk.common,
	&owa1_tx_clk.common,
	&audio_codec_dac_clk.common,
	&audio_codec_adc_clk.common,
	&audio_codec_bus_clk.common,
	&usb0_bus_clk.common,
	&usb1_bus_clk.common,
	&usb2_bus_clk.common,
	&usbotg_bus_clk.common,
	&usbehci2_bus_clk.common,
	&usbehci1_bus_clk.common,
	&usbehci0_bus_clk.common,
	&usbohci2_bus_clk.common,
	&usbohci1_bus_clk.common,
	&usbohci0_bus_clk.common,
	&tvfe_axi_clk.common,
	&tvfe_axi_bus_clk.common,
	&adc_clk.common,
	&tsdm_ts_clk.common,
	&tvfe1296m_clk.common,
	&i2h_clk.common,
	&audio_cpu_clk.common,
	&mcsi_clk.common,
	&csi_master_clk.common,
	&mcsi_bus_clk.common,
	&tcd3_clk.common,
	&vincap_dma_clk.common,
	&hdmi_audio_clk.common,
	&cap_300m_clk.common,
	&tvcap_bus_clk.common,
	&deint_clk.common,
	&panel_clk.common,
	&svp_dtl_clk.common,
	&ksc_clk.common,
	&afbd_clk.common,
	&dsi_clk.common,
	&svp_pclk_deint_clk.common,
	&svp_pclk_nr_meter_clk.common,
	&combophy_clk.common,
	&disp_bus_clk.common,
};

static const struct sunxi_ccu_desc sun50iw15_ccu_desc = {
	.ccu_clks	= sun50iw15_ccu_clks,
	.num_ccu_clks	= ARRAY_SIZE(sun50iw15_ccu_clks),

	.hw_clks	= &sun50iw15_hw_clks,

	.resets		= sun50iw15_ccu_resets,
	.num_resets	= ARRAY_SIZE(sun50iw15_ccu_resets),
};

static const u32 sun50iw15_pll_regs[] = {
	SUN50IW15_PLL_PERI0_CTRL_REG,
	SUN50IW15_PLL_PERI1_CTRL_REG,
	SUN50IW15_PLL_GPU_CTRL_REG,
	SUN50IW15_PLL_VIDEO0_CTRL_REG,
	SUN50IW15_PLL_VIDEO1_CTRL_REG,
	SUN50IW15_PLL_VE_CTRL_REG,
	SUN50IW15_PLL_ADC_CTRL_REG,
	SUN50IW15_PLL_VIDEO2_CTRL_REG,
	SUN50IW15_PLL_AUDIO_CTRL_REG,
	SUN50IW15_PLL_CPU_CTRL_REG,
};

static const u32 sun50iw15_pll_video_regs[] = {
	SUN50IW15_PLL_VIDEO0_CTRL_REG,
	SUN50IW15_PLL_VIDEO1_CTRL_REG,
	SUN50IW15_PLL_VIDEO2_CTRL_REG,
};

static int sun50iw15_ccu_probe(struct platform_device *pdev)
{
	struct resource *res;
	struct device *dev = &pdev->dev;
	void __iomem *reg;
	int i, ret;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		dev_err(dev, "Fail to get IORESOURCE_MEM\n");
		return -EINVAL;
	}

	reg = devm_ioremap(dev, res->start, resource_size(res));
	if (IS_ERR(reg)) {
		dev_err(dev, "Fail to map IO resource\n");
		return PTR_ERR(reg);
	}

	/* Enable the pll_en/ldo_en/lock_en bits on all PLLs */
	for (i = 0; i < ARRAY_SIZE(sun50iw15_pll_regs); i++) {
		set_reg(reg + sun50iw15_pll_regs[i], 0x7, 3, 29);
	}

	/*
	 * Force the output divider of video PLLs to 0.
	 *
	 * See the comment before pll-video0 definition for the reason.
	 */
	for (i = 0; i < ARRAY_SIZE(sun50iw15_pll_video_regs); i++) {
		set_reg(reg + sun50iw15_pll_video_regs[i], 0x0, 1, 0);
	}

	/* Enforce m1 = 0, m0 = 1 for Audio PLL */
	//set_reg(reg + SUN50IW15_PLL_AUDIO_CTRL_REG, 0x1, 0, 0);

	ret = sunxi_ccu_probe(pdev->dev.of_node, reg, &sun50iw15_ccu_desc);
	if (ret)
		return ret;

	sunxi_ccu_sleep_init(reg, sun50iw15_ccu_clks,
			ARRAY_SIZE(sun50iw15_ccu_clks),
			NULL, 0);

	return 0;
}

static const struct of_device_id sun50iw15_ccu_ids[] = {
	{ .compatible = "allwinner,sun50iw15-ccu" },
	{ }
};

static struct platform_driver sun50iw15_ccu_driver = {
	.probe	= sun50iw15_ccu_probe,
	.driver	= {
		.name	= "sun50iw15-ccu",
		.of_match_table	= sun50iw15_ccu_ids,
	},
};

static int __init sun50iw15_ccu_init(void)
{
	int err;

	err = platform_driver_register(&sun50iw15_ccu_driver);
	if (err)
		pr_err("register ccu sun50iw15 failed\n");

	return err;
}

core_initcall(sun50iw15_ccu_init);

static void __exit sun50iw15_ccu_exit(void)
{
	platform_driver_unregister(&sun50iw15_ccu_driver);
}
module_exit(sun50iw15_ccu_exit);

MODULE_DESCRIPTION("Allwinner sun50iw15 clk driver");
MODULE_AUTHOR("xuefan");
MODULE_LICENSE("GPL v2");
MODULE_VERSION(SUNXI_CCU_VERSION);
