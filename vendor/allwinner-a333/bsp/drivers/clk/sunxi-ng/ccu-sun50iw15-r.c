// SPDX-License-Identifier: GPL-2.0

#include <linux/clk-provider.h>
#include <linux/module.h>
#include <linux/of_address.h>
#include <linux/platform_device.h>

#include "ccu_common.h"
#include "ccu_reset.h"

#include "ccu_div.h"
#include "ccu_gate.h"
#include "ccu_mp.h"
#include "ccu_nm.h"

#include "ccu-sun50iw15-r.h"

#define SUNXI_CCU_VERSION	"0.0.2"

/* ccu_des_start */
static const char * const ahbs_parents[] = { "dcxo24M", "ext-32k",
	"rc-16m", "pll-peri0-200m" };

static SUNXI_CCU_MP_WITH_MUX(r_ahb_clk, "r-ahb",
		ahbs_parents, 0x000,
		0, 5,
		8, 2,
		24, 3,
		CLK_IGNORE_UNUSED);

static SUNXI_CCU_M_WITH_MUX(r_apbs0_clk, "r-apbs0",
		ahbs_parents, 0x00c,
		0, 5,
		24, 3,
		CLK_IGNORE_UNUSED);

static SUNXI_CCU_M_WITH_MUX(r_apbs1_clk, "r-apbs1",
		ahbs_parents, 0x010,
		0, 5,
		24, 3,
		CLK_IGNORE_UNUSED);

static const char * const r_timer_parents[] = { "dcxo24M", "ext-32k",
	"rc-16m", "pll-peri0-200m" };

static struct ccu_div r_timer0_clk = {
	.enable		= BIT(0),
	.div		= _SUNXI_CCU_DIV_FLAGS(1, 4, CLK_DIVIDER_POWER_OF_TWO),
	.mux		= _SUNXI_CCU_MUX(4, 2),
	.common		= {
		.reg		= 0x0110,
		.hw.init	= CLK_HW_INIT_PARENTS("r-timer0",
				r_timer_parents,
				&ccu_div_ops, CLK_IGNORE_UNUSED),
	},
};

static struct ccu_div r_timer1_clk = {
	.enable		= BIT(0),
	.div		= _SUNXI_CCU_DIV_FLAGS(1, 4, CLK_DIVIDER_POWER_OF_TWO),
	.mux		= _SUNXI_CCU_MUX(4, 2),
	.common		= {
		.reg		= 0x0114,
		.hw.init	= CLK_HW_INIT_PARENTS("r-timer1",
				r_timer_parents,
				&ccu_div_ops, CLK_IGNORE_UNUSED),
	},
};

static struct ccu_div r_timer2_clk = {
	.enable		= BIT(0),
	.div		= _SUNXI_CCU_DIV_FLAGS(1, 4, CLK_DIVIDER_POWER_OF_TWO),
	.mux		= _SUNXI_CCU_MUX(4, 2),
	.common		= {
		.reg		= 0x0118,
		.hw.init	= CLK_HW_INIT_PARENTS("r-timer2",
				r_timer_parents,
				&ccu_div_ops, CLK_IGNORE_UNUSED),
	},
};

static SUNXI_CCU_GATE(r_bus_timer_clk, "r-timer-gating",
		"dcxo24M",
		0x011c, BIT(0), CLK_IGNORE_UNUSED);

static const char * const r_edid_parents[] = { "dcxo24M", "ext-32k", "rc-16m", "pll-peri0-200m" };

static SUNXI_CCU_M_WITH_MUX_GATE(r_edid_clk, "r-edid",
		r_edid_parents, 0x0124,
		0, 5,	/* M */
		24, 2,	/* mux */
		BIT(31),	/* gate */
		CLK_SET_RATE_PARENT | CLK_IGNORE_UNUSED);

static const char * const r_cec_parents[] = { "ext-32k", "pll-peri0-2x", "dcxo-24M" };

static SUNXI_CCU_MUX_WITH_GATE(r_cec_clk, "r-cec",
		r_cec_parents, 0x0128,
		24, 2,
		BIT(31), CLK_IGNORE_UNUSED);

static SUNXI_CCU_GATE(r_hdmi_cec_clk, "r-hdmi-cec-gating",
		"dcxo24M",
		0x0128, BIT(20), CLK_IGNORE_UNUSED);

static SUNXI_CCU_GATE(r_bus_wdt1_clk, "r-wdt1-gating",
		"dcxo24M",
		0x012c, BIT(0), CLK_IGNORE_UNUSED);

static const char * const r_pwm_parents[] = { "dcxo24M", "ext-32k", "rc-16m" };

static SUNXI_CCU_MUX_WITH_GATE(r_pwm_clk, "r-pwm",
		r_pwm_parents, 0x0130,
		24, 2,
		BIT(31), CLK_IGNORE_UNUSED);

static SUNXI_CCU_GATE(r_bus_pwm_clk, "r-pwm-gating",
		"dcxo24M",
		0x013c, BIT(0), CLK_IGNORE_UNUSED);

static SUNXI_CCU_GATE(r_bus_uart_clk, "r-uart0-gating",
		"dcxo24M",
		0x018c, BIT(0), CLK_IGNORE_UNUSED);

static SUNXI_CCU_GATE(r_bus_twi1_clk, "r-twi1-gating",
		"dcxo24M",
		0x019c, BIT(1), CLK_IGNORE_UNUSED);

static SUNXI_CCU_GATE(r_bus_twi0_clk, "r-twi0-gating",
		"dcxo24M",
		0x019c, BIT(0), CLK_IGNORE_UNUSED);

static SUNXI_CCU_GATE(r_bus_ppu_clk, "r-ppu-gating",
		"dcxo24M",
		0x01ac, BIT(0), CLK_IGNORE_UNUSED);

static SUNXI_CCU_GATE(r_bus_tzma_clk, "r-tzma-gating",
		"dcxo24M",
		0x01b0, BIT(0), CLK_IGNORE_UNUSED);

static SUNXI_CCU_GATE(r_cpus_bus_bist_clk, "r-cpus-bist-gating",
		"dcxo24M",
		0x01bc, BIT(0), CLK_IGNORE_UNUSED);

static const char * const r_irrx_parents[] = { "ext-32k", "dcxo24M" };

static SUNXI_CCU_M_WITH_MUX_GATE(r_irrx_clk, "r-irrx",
		r_irrx_parents, 0x01c0,
		0, 5,	/* M */
		24, 2,	/* mux */
		BIT(31),	/* gate */
		CLK_SET_RATE_PARENT | CLK_IGNORE_UNUSED);

static SUNXI_CCU_GATE(r_bus_irrx_clk, "r-irrx-gating",
		"dcxo24M",
		0x01cc, BIT(0), CLK_IGNORE_UNUSED);

static const char * const r_gpadc_parents[] = { "dcxo24M", /*"gpadc-48m"*/ };

static SUNXI_CCU_M_WITH_MUX_GATE(r_gpadc_clk, "r-gpadc",
		r_gpadc_parents, 0x01d0,
		0, 5,	/* M */
		24, 2,	/* mux */
		BIT(31),	/* gate */
		CLK_SET_RATE_PARENT | CLK_IGNORE_UNUSED);

static SUNXI_CCU_GATE(r_bus_gpadc_clk, "r-gpadc-gating",
		"dcxo24M",
		0x01dc, BIT(0), CLK_IGNORE_UNUSED);

static SUNXI_CCU_GATE(r_bus_ths_clk, "r-ths-gating",
		"dcxo24M",
		0x01ec, BIT(0), CLK_IGNORE_UNUSED);

static SUNXI_CCU_GATE(r_bus_rtc_clk, "r-rtc-gating",
		"dcxo24M",
		0x020c, BIT(0), CLK_IGNORE_UNUSED);

static SUNXI_CCU_GATE(r_bus_cpucfg_clk, "r-cpucfg-gating",
		"dcxo24M",
		0x022c, BIT(0), CLK_IGNORE_UNUSED);
/* ccu_des_end */

/* rst_def_start */
static struct ccu_reset_map sun50iw15_r_ccu_resets[] = {
	[RST_R_TIMER]		=  { 0x11c, BIT(16) },
	[RST_R_EDID]		=  { 0x120, BIT(16) },
	[RST_R_PWM]		=  { 0x13c, BIT(16) },
	[RST_R_UART]		=  { 0x18c, BIT(16) },
	[RST_R_TWI2]		=  { 0x19c, BIT(18) },
	[RST_R_TWI1]		=  { 0x19c, BIT(17) },
	[RST_R_TWI0]		=  { 0x19c, BIT(16) },
	[RST_R_PPU]		=  { 0x1ac, BIT(16) },
	[RST_R_IRRX]		=  { 0x1cc, BIT(16) },
	[RST_R_GPADC]		=  { 0x1dc, BIT(16) },
	[RST_R_THS]		=  { 0x1ec, BIT(16) },
	[RST_R_RTC]		=  { 0x20c, BIT(16) },
	[RST_R_CPUCFG]		=  { 0x22c, BIT(16) },
};
/* rst_def_end */

/* ccu_def_start */
static struct clk_hw_onecell_data sun50iw15_r_hw_clks = {
	.hws	= {
		[CLK_R_AHB]		= &r_ahb_clk.common.hw,
		[CLK_R_APBS0]		= &r_apbs0_clk.common.hw,
		[CLK_R_APBS1]		= &r_apbs1_clk.common.hw,
		[CLK_R_TIMER0]		= &r_timer0_clk.common.hw,
		[CLK_R_TIMER1]		= &r_timer1_clk.common.hw,
		[CLK_R_TIMER2]		= &r_timer2_clk.common.hw,
		[CLK_BUS_R_TIMER]	= &r_bus_timer_clk.common.hw,
		[CLK_R_EDID]		= &r_edid_clk.common.hw,
		[CLK_R_CEC]		= &r_cec_clk.common.hw,
		[CLK_R_HDMI_CEC]	= &r_hdmi_cec_clk.common.hw,
		[CLK_BUS_R_WDT1]	= &r_bus_wdt1_clk.common.hw,
		[CLK_R_PWM]		= &r_pwm_clk.common.hw,
		[CLK_BUS_R_PWM]		= &r_bus_pwm_clk.common.hw,
		[CLK_BUS_R_UART]	= &r_bus_uart_clk.common.hw,
		[CLK_BUS_R_TWI1]	= &r_bus_twi1_clk.common.hw,
		[CLK_BUS_R_TWI0]	= &r_bus_twi0_clk.common.hw,
		[CLK_R_PPU]		= &r_bus_ppu_clk.common.hw,
		[CLK_BUS_R_TZMA]	= &r_bus_tzma_clk.common.hw,
		[CLK_BUS_R_BIST]	= &r_cpus_bus_bist_clk.common.hw,
		[CLK_R_IRRX]		= &r_irrx_clk.common.hw,
		[CLK_BUS_R_IRRX]	= &r_bus_irrx_clk.common.hw,
		[CLK_R_GPADC]		= &r_gpadc_clk.common.hw,
		[CLK_BUS_R_GPADC]	= &r_bus_gpadc_clk.common.hw,
		[CLK_BUS_R_THS]		= &r_bus_ths_clk.common.hw,
		[CLK_BUS_R_RTC]		= &r_bus_rtc_clk.common.hw,
		[CLK_BUS_R_CPUCFG]	= &r_bus_cpucfg_clk.common.hw,
	},
	.num	= CLK_R_NUMBER,
};

/* ccu_def_end */

static struct ccu_common *sun50iw15_r_ccu_clks[] = {
	&r_ahb_clk.common,
	&r_apbs0_clk.common,
	&r_apbs1_clk.common,
	&r_timer0_clk.common,
	&r_timer1_clk.common,
	&r_timer2_clk.common,
	&r_bus_timer_clk.common,
	&r_edid_clk.common,
	&r_cec_clk.common,
	&r_hdmi_cec_clk.common,
	&r_bus_wdt1_clk.common,
	&r_pwm_clk.common,
	&r_bus_pwm_clk.common,
	&r_bus_uart_clk.common,
	&r_bus_twi1_clk.common,
	&r_bus_twi0_clk.common,
	&r_bus_ppu_clk.common,
	&r_bus_tzma_clk.common,
	&r_cpus_bus_bist_clk.common,
	&r_irrx_clk.common,
	&r_bus_irrx_clk.common,
	&r_gpadc_clk.common,
	&r_bus_gpadc_clk.common,
	&r_bus_ths_clk.common,
	&r_bus_rtc_clk.common,
	&r_bus_cpucfg_clk.common,
};



static const struct sunxi_ccu_desc sun50iw15_r_ccu_desc = {
	.ccu_clks	= sun50iw15_r_ccu_clks,
	.num_ccu_clks	= ARRAY_SIZE(sun50iw15_r_ccu_clks),

	.hw_clks	= &sun50iw15_r_hw_clks,

	.resets		= sun50iw15_r_ccu_resets,
	.num_resets	= ARRAY_SIZE(sun50iw15_r_ccu_resets),
};

static int sun50iw15_r_ccu_probe(struct platform_device *pdev)
{
	void __iomem *reg;
	int ret;

	reg = devm_platform_ioremap_resource(pdev, 0);
	if (IS_ERR(reg))
		return PTR_ERR(reg);

	ret = sunxi_ccu_probe(pdev->dev.of_node, reg, &sun50iw15_r_ccu_desc);
	if (ret)
		return ret;

	sunxi_ccu_sleep_init(reg, sun50iw15_r_ccu_clks,
			ARRAY_SIZE(sun50iw15_r_ccu_clks),
			NULL, 0);

	return 0;
}

static const struct of_device_id sun50iw15_r_ccu_ids[] = {
	{ .compatible = "allwinner,sun50iw15-r-ccu" },
	{ }
};

static struct platform_driver sun50iw15_r_ccu_driver = {
	.probe	= sun50iw15_r_ccu_probe,
	.driver	= {
		.name	= "sun50iw15-r-ccu",
		.of_match_table	= sun50iw15_r_ccu_ids,
	},
};

static int __init sunxi_ccu_sun50iw15_r_init(void)
{
	int ret;

	ret = platform_driver_register(&sun50iw15_r_ccu_driver);
	if (ret)
		pr_err("register ccu sun50iw15-r failed\n");

	return ret;
}
core_initcall(sunxi_ccu_sun50iw15_r_init);

static void __exit sunxi_ccu_sun50iw15_r_exit(void)
{
	return platform_driver_unregister(&sun50iw15_r_ccu_driver);
}
module_exit(sunxi_ccu_sun50iw15_r_exit);

MODULE_DESCRIPTION("Allwinner sun50iw15-r clk driver");
MODULE_AUTHOR("xuefan");
MODULE_LICENSE("GPL v2");
MODULE_VERSION(SUNXI_CCU_VERSION);
