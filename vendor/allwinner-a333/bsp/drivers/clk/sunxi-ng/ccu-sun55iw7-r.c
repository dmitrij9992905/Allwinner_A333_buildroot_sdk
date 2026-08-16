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

#include "ccu-sun55iw7-r.h"

#define SUNXI_R_CCU_VERSION	"0.0.1"

/* ccu_des_start */
static const char * const ahbs_parents[] = { "dcxo24M", "rtc32k", "rc_16m", "pll-peri-div", "pll-peri0-300m" };

static SUNXI_CCU_M_WITH_MUX(ahbs_clk, "ahbs", ahbs_parents,
			0x0000, 0, 5, 24, 3, 0);

static const char * const apbs0_parents[] = { "dcxo24M", "rtc32k", "rc_16m", "pll-peri-div" };

static SUNXI_CCU_M_WITH_MUX(apbs0_clk, "apbs0", apbs0_parents,
			0x000C, 0, 5, 24, 3, 0);

static const char * const apbs1_parents[] = { "dcxo24M", "rtc32k", "rc_16m", "pll-peri-div" };

static SUNXI_CCU_M_WITH_MUX(apbs1_clk, "apbs1", apbs1_parents,
			0x0010, 0, 5, 24, 3, 0);

static const char * const s_timer0_0_parents[] = { "dcxo24M", "rtc32k", "rc_16m", "pll-peri-div" };

static SUNXI_CCU_M_WITH_MUX_GATE(s_timer0_0_clk, "s-timer0-0",
			s_timer0_0_parents, 0x0100,
			1, 3,  /* M */
			4, 3,  /* mux */
			BIT(0),	/* gate */
			0);

static const char * const s_timer0_1_parents[] = { "dcxo24M", "rtc32k", "rc_16m", "pll-peri-div" };

static SUNXI_CCU_M_WITH_MUX_GATE(s_timer0_1_clk, "s-timer0-1",
			s_timer0_1_parents, 0x0104,
			1, 3,  /* M */
			4, 3,  /* mux */
			BIT(0),	/* gate */
			0);

static const char * const s_timer0_2_parents[] = { "dcxo24M", "rtc32k", "rc_16m", "pll-peri-div" };

static SUNXI_CCU_M_WITH_MUX_GATE(s_timer0_2_clk, "s-timer0-2",
			s_timer0_2_parents, 0x0108,
			1, 3,  /* M */
			4, 3,  /* mux */
			BIT(0),	/* gate */
			0);

static SUNXI_CCU_GATE(s_timer0_bus_clk, "s-timer0-bus",
			"dcxo24M",
			0x011C, BIT(0), 0);

static const char * const s_edid_clk_parents[] = { "dcxo24M", "rtc32k", "rc_16m", "pll-peri-div" };

static SUNXI_CCU_M_WITH_MUX_GATE(s_edid_clk, "s-edid-clk",
			s_edid_clk_parents, 0x0124,
			0, 5,  /* M */
			24, 3,  /* mux */
			BIT(31),	/* gate */
			0);

static const char * const s_cec_clk_parents[] = { "rtc32k", "hdmi-cec-clk32k", "hdmi-cec-clk" };

static SUNXI_CCU_MUX_WITH_GATE(s_cec_clk, "s-cec-clk",
			s_cec_clk_parents, 0x0128,
			24, 2,  /* mux */
			BIT(31), 0);

static SUNXI_CCU_GATE(twd_bus_clk, "twd-bus",
			"dcxo24M",
			0x012C, BIT(0), 0);

static const char * const s_pwm_parents[] = { "dcxo24M", "rtc32k", "rc_16m" };

static SUNXI_CCU_MUX_WITH_GATE(s_pwm_clk, "s-pwm",
			s_pwm_parents, 0x0130,
			24, 2,  /* mux */
			BIT(31), 0);

static SUNXI_CCU_GATE(s_pwm_bus_clk, "s-pwm-bus",
			"dcxo24M",
			0x013C, BIT(0), 0);

static SUNXI_CCU_GATE(s_uart0_bus_clk, "s-uart0-bus",
			"apbs1",
			0x018C, BIT(0), 0);

static SUNXI_CCU_GATE(s_twi1_bus_clk, "s-twi1-bus",
			"apbs1",
			0x019C, BIT(1), 0);

static SUNXI_CCU_GATE(s_twi0_bus_clk, "s-twi0-bus",
			"apbs1",
			0x019C, BIT(0), 0);

static SUNXI_CCU_GATE(ppu_vo0_bus_clk, "ppu-vo0-bus",
			"dcxo24M",
			0x01AC, BIT(2), 0);

static SUNXI_CCU_GATE(ppu_gpu_bus_clk, "ppu-gpu-bus",
			"dcxo24M",
			0x01AC, BIT(1), 0);

static SUNXI_CCU_GATE(ppu_ve_bus_clk, "ppu-ve-bus",
			"dcxo24M",
			0x01AC, BIT(0), 0);

static SUNXI_CCU_GATE(tzma_bus_clk, "tzma-bus",
			"dcxo24M",
			0x01B0, BIT(0), 0);

static const char * const s_ir_rx_parents[] = { "rtc32k", "dcxo24M"};

static SUNXI_CCU_M_WITH_MUX_GATE(s_ir_rx_clk, "s-ir-rx",
			s_ir_rx_parents, 0x01C0,
			0, 5,  /* M */
			24, 2,  /* mux */
			BIT(31),	/* gate */
			0);

static SUNXI_CCU_GATE(s_ir_rx_bus_clk, "s-ir-rx-bus",
			"dcxo24M",
			0x01CC, BIT(0), 0);

static const char * const s_gpadc_parents[] = { "dcxo24M", "clk48m" };

static SUNXI_CCU_M_WITH_MUX_GATE(s_gpadc_clk, "s-gpadc",
			s_gpadc_parents, 0x01D0,
			0, 5,  /* M */
			24, 2,  /* mux */
			BIT(31),	/* gate */
			0);

static SUNXI_CCU_GATE(s_gpadc_bus_clk, "s-gpadc-bus",
			"dcxo24M",
			0x01DC, BIT(0), 0);

static SUNXI_CCU_GATE(s_ths_bus_clk, "s-ths-bus",
			"dcxo24M",
			0x01EC, BIT(0), 0);

static SUNXI_CCU_GATE(rtc_bus_clk, "rtc-bus",
			"dcxo24M",
			0x020C, BIT(0), 0);

static const char * const cpus_24m_parents[] = { "dcxo24M", "rtc32k", "rc_16m" };

static SUNXI_CCU_MUX_WITH_GATE(cpus_24m_clk, "cpus-24m",
			cpus_24m_parents, 0x0210,
			24, 2,  /* mux */
			BIT(31), 0);

static SUNXI_CCU_GATE(cpus_cfg_bus_clk, "cpus-cfg-bus",
			"dcxo24M",
			0x021C, BIT(1), 0);

static SUNXI_CCU_GATE(cpus_core_bus_clk, "cpus-core-bus",
			"dcxo24M",
			0x021C, BIT(0), 0);

static SUNXI_CCU_GATE(cpuidle_bus_clk, "cpuidle-bus",
			"dcxo24M",
			0x022C, BIT(0), 0);
/* ccu_des_end */

/* rst_def_start */
static struct ccu_reset_map sun55iw7_r_ccu_resets[] = {
	[RST_BUS_S_TIMER0]		= { 0x011c, BIT(16) },
	[RST_BUS_S_EDID]		= { 0x0120, BIT(16) },
	[RST_BUS_S_PWM]			= { 0x013c, BIT(16) },
	[RST_BUS_S_UART0]		= { 0x018c, BIT(16) },
	[RST_BUS_S_TWI1]		= { 0x019c, BIT(17) },
	[RST_BUS_S_TWI0]		= { 0x019c, BIT(16) },
	[RST_BUS_S_IR_RX]		= { 0x01cc, BIT(16) },
	[RST_BUS_S_GPADC]		= { 0x01dc, BIT(16) },
	[RST_BUS_S_TH]			= { 0x01ec, BIT(16) },
	[RST_BUS_RTC]			= { 0x020c, BIT(16) },
	[RST_BUS_CPUS_CFG]		= { 0x021c, BIT(16) },
	[RST_BUS_CPUIDLE]		= { 0x022c, BIT(16) },
};
/* rst_def_end */

/* ccu_def_start */
static struct clk_hw_onecell_data sun55iw7_r_hw_clks = {
	.hws    = {
		[CLK_AHBS]			= &ahbs_clk.common.hw,
		[CLK_APBS0]			= &apbs0_clk.common.hw,
		[CLK_APBS1]			= &apbs1_clk.common.hw,
		[CLK_S_TIMER0_0]		= &s_timer0_0_clk.common.hw,
		[CLK_S_TIMER0_1]		= &s_timer0_1_clk.common.hw,
		[CLK_S_TIMER0_2]		= &s_timer0_2_clk.common.hw,
		[CLK_BUS_S_TIMER0]		= &s_timer0_bus_clk.common.hw,
		[CLK_S_EDID]			= &s_edid_clk.common.hw,
		[CLK_S_CEC]			= &s_cec_clk.common.hw,
		[CLK_BUS_TWD]			= &twd_bus_clk.common.hw,
		[CLK_S_PWM]			= &s_pwm_clk.common.hw,
		[CLK_BUS_S_PWM]			= &s_pwm_bus_clk.common.hw,
		[CLK_BUS_S_UART0]		= &s_uart0_bus_clk.common.hw,
		[CLK_BUS_S_TWI1]		= &s_twi1_bus_clk.common.hw,
		[CLK_BUS_S_TWI0]		= &s_twi0_bus_clk.common.hw,
		[CLK_BUS_PPU_VO0]		= &ppu_vo0_bus_clk.common.hw,
		[CLK_BUS_PPU_GPU]		= &ppu_gpu_bus_clk.common.hw,
		[CLK_BUS_PPU_VE]		= &ppu_ve_bus_clk.common.hw,
		[CLK_BUS_TZMA]			= &tzma_bus_clk.common.hw,
		[CLK_S_IR_RX]			= &s_ir_rx_clk.common.hw,
		[CLK_BUS_S_IR_RX]		= &s_ir_rx_bus_clk.common.hw,
		[CLK_S_GPADC]			= &s_gpadc_clk.common.hw,
		[CLK_BUS_S_GPADC]		= &s_gpadc_bus_clk.common.hw,
		[CLK_BUS_S_THS]			= &s_ths_bus_clk.common.hw,
		[CLK_BUS_RTC]			= &rtc_bus_clk.common.hw,
		[CLK_CPUS_24M]			= &cpus_24m_clk.common.hw,
		[CLK_BUS_CPUS_CFG]		= &cpus_cfg_bus_clk.common.hw,
		[CLK_BUS_CPUS_CORE]		= &cpus_core_bus_clk.common.hw,
		[CLK_BUS_CPUIDLE]		= &cpuidle_bus_clk.common.hw,
	},
	.num = CLK_NUMBER,
};
/* ccu_def_end */

static struct ccu_common *sun55iw7_r_ccu_clks[] = {
	&ahbs_clk.common,
	&apbs0_clk.common,
	&apbs1_clk.common,
	&s_timer0_0_clk.common,
	&s_timer0_1_clk.common,
	&s_timer0_2_clk.common,
	&s_timer0_bus_clk.common,
	&s_edid_clk.common,
	&s_cec_clk.common,
	&twd_bus_clk.common,
	&s_pwm_clk.common,
	&s_pwm_bus_clk.common,
	&s_uart0_bus_clk.common,
	&s_twi1_bus_clk.common,
	&s_twi0_bus_clk.common,
	&ppu_vo0_bus_clk.common,
	&ppu_gpu_bus_clk.common,
	&ppu_ve_bus_clk.common,
	&tzma_bus_clk.common,
	&s_ir_rx_clk.common,
	&s_ir_rx_bus_clk.common,
	&s_gpadc_clk.common,
	&s_gpadc_bus_clk.common,
	&s_ths_bus_clk.common,
	&rtc_bus_clk.common,
	&cpus_24m_clk.common,
	&cpus_cfg_bus_clk.common,
	&cpus_core_bus_clk.common,
	&cpuidle_bus_clk.common,
};

static const struct sunxi_ccu_desc sun55iw7_r_ccu_desc = {
	.ccu_clks	= sun55iw7_r_ccu_clks,
	.num_ccu_clks	= ARRAY_SIZE(sun55iw7_r_ccu_clks),

	.hw_clks	= &sun55iw7_r_hw_clks,

	.resets		= sun55iw7_r_ccu_resets,
	.num_resets	= ARRAY_SIZE(sun55iw7_r_ccu_resets),
};

static int sun55iw7_r_ccu_probe(struct platform_device *pdev)
{
	void __iomem *reg;
	int ret;

	reg = devm_platform_ioremap_resource(pdev, 0);
	if (IS_ERR(reg))
		return PTR_ERR(reg);

	ret = sunxi_ccu_probe(pdev->dev.of_node, reg, &sun55iw7_r_ccu_desc);
	if (ret)
		return ret;

	sunxi_ccu_sleep_init(reg, sun55iw7_r_ccu_clks,
			ARRAY_SIZE(sun55iw7_r_ccu_clks),
			NULL, 0);

	return 0;
}

static const struct of_device_id sun55iw7_r_ccu_ids[] = {
	{ .compatible = "allwinner,sun55iw7-r-ccu" },
	{ }
};

static struct platform_driver sun55iw7_r_ccu_driver = {
	.probe	= sun55iw7_r_ccu_probe,
	.driver	= {
		.name	= "sun55iw7-r-ccu",
		.of_match_table	= sun55iw7_r_ccu_ids,
	},
};

static int __init sunxi_ccu_sun55iw7_r_init(void)
{
	int ret;

	ret = platform_driver_register(&sun55iw7_r_ccu_driver);
	if (ret)
		pr_err("register ccu sun55iw7-r failed\n");

	return ret;
}
core_initcall(sunxi_ccu_sun55iw7_r_init);

static void __exit sunxi_ccu_sun55iw7_r_exit(void)
{
	return platform_driver_unregister(&sun55iw7_r_ccu_driver);
}
module_exit(sunxi_ccu_sun55iw7_r_exit);

MODULE_DESCRIPTION("Allwinner sun55iw7-r clk driver");
MODULE_AUTHOR("haili");
MODULE_LICENSE("GPL v2");
MODULE_VERSION(SUNXI_R_CCU_VERSION);
