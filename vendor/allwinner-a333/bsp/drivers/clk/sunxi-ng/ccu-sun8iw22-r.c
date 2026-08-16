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

#include "ccu-sun8iw22-r.h"

#define SUNXI_R_CCU_VERSION	"0.0.2"

/* ccu_des_start */

static const char * const ahbs_clk_parents[] = { "dcxo24M", "rtc-32k", "rc_16m", "pll-peri-div", "pll-peri0-200m" };

static SUNXI_CCU_M_WITH_MUX(ahbs_clk, "ahbs-clk", ahbs_clk_parents,
			0x0000, 0, 5, 24, 3, 0);

static const char * const apbs0_parents[] = { "dcxo24M", "rtc-32k", "rc_16m", "pll-peri-div" };

static SUNXI_CCU_M_WITH_MUX(apbs0_clk, "apbs0", apbs0_parents,
			0x000C, 0, 5, 24, 3, 0);

static const char * const apbs1_parents[] = { "dcxo24M", "rtc-32k", "rc_16m", "pll-peri-div" };

static SUNXI_CCU_M_WITH_MUX(apbs1_clk, "apbs1", apbs1_parents,
			0x0010, 0, 5, 24, 3, 0);

static SUNXI_CCU_GATE(twd_bus_clk, "twd-bus",
			"dcxo24M",
			0x012C, BIT(0), 0);

static SUNXI_CCU_GATE(rtc_bus_clk, "rtc-bus",
			"dcxo24M",
			0x020C, BIT(0), 0);

static SUNXI_CCU_GATE(cpuidle_bus_clk, "cpuidle-bus",
			"dcxo24M",
			0x022C, BIT(0), 0);

/* ccu_des_end */

/* rst_def_start */
static struct ccu_reset_map sun8iw22_r_ccu_resets[] = {
	[RST_BUS_RTC]			= { 0x020c, BIT(16) },
	[RST_BUS_CPUIDLE]		= { 0x022c, BIT(16) },
};
/* rst_def_end */

/* ccu_def_start */
static struct clk_hw_onecell_data sun8iw22_r_hw_clks = {
	.hws    = {
		[CLK_AHBS]			= &ahbs_clk.common.hw,
		[CLK_APBS0]			= &apbs0_clk.common.hw,
		[CLK_APBS1]			= &apbs1_clk.common.hw,
		[CLK_BUS_TWD]			= &twd_bus_clk.common.hw,
		[CLK_BUS_RTC]			= &rtc_bus_clk.common.hw,
		[CLK_BUS_CPUIDLE]		= &cpuidle_bus_clk.common.hw,
	},
	.num = CLK_NUMBER,
};
/* ccu_def_end */

static struct ccu_common *sun8iw22_r_ccu_clks[] = {
	&ahbs_clk.common,
	&apbs0_clk.common,
	&apbs1_clk.common,
	&twd_bus_clk.common,
	&rtc_bus_clk.common,
	&cpuidle_bus_clk.common,
};

static const struct sunxi_ccu_desc sun8iw22_r_ccu_desc = {
	.ccu_clks	= sun8iw22_r_ccu_clks,
	.num_ccu_clks	= ARRAY_SIZE(sun8iw22_r_ccu_clks),

	.hw_clks	= &sun8iw22_r_hw_clks,

	.resets		= sun8iw22_r_ccu_resets,
	.num_resets	= ARRAY_SIZE(sun8iw22_r_ccu_resets),
};

static int sun8iw22_r_ccu_probe(struct platform_device *pdev)
{
	void __iomem *reg;
	int ret;

	reg = devm_platform_ioremap_resource(pdev, 0);
	if (IS_ERR(reg))
		return PTR_ERR(reg);

	ret = sunxi_ccu_probe(pdev->dev.of_node, reg, &sun8iw22_r_ccu_desc);
	if (ret)
		return ret;

	sunxi_ccu_sleep_init(reg, sun8iw22_r_ccu_clks,
			ARRAY_SIZE(sun8iw22_r_ccu_clks),
			NULL, 0);

	return 0;
}

static const struct of_device_id sun8iw22_r_ccu_ids[] = {
	{ .compatible = "allwinner,sun8iw22-r-ccu" },
	{ }
};

static struct platform_driver sun8iw22_r_ccu_driver = {
	.probe	= sun8iw22_r_ccu_probe,
	.driver	= {
		.name	= "sun8iw22-r-ccu",
		.of_match_table	= sun8iw22_r_ccu_ids,
	},
};

static int __init sunxi_ccu_sun8iw22_r_init(void)
{
	int ret;

	ret = platform_driver_register(&sun8iw22_r_ccu_driver);
	if (ret)
		pr_err("register ccu sun8iw22-r failed\n");

	return ret;
}
core_initcall(sunxi_ccu_sun8iw22_r_init);

static void __exit sunxi_ccu_sun8iw22_r_exit(void)
{
	return platform_driver_unregister(&sun8iw22_r_ccu_driver);
}
module_exit(sunxi_ccu_sun8iw22_r_exit);

MODULE_DESCRIPTION("Allwinner sun8iw22-r clk driver");
MODULE_AUTHOR("haili");
MODULE_LICENSE("GPL v2");
MODULE_VERSION(SUNXI_R_CCU_VERSION);
