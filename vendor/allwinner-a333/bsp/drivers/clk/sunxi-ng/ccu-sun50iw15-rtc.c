// SPDX-License-Identifier: GPL-2.0-only
/*
 * sunxi RTC ccu driver
 */

#include <linux/clk-provider.h>
#include <linux/io.h>
#include <linux/of_address.h>
#include <linux/platform_device.h>
#include <linux/module.h>

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
#include "ccu_phase.h"

#include "ccu-sun50iw15-rtc.h"

/* enable dxco24M for wake up */
static SUNXI_CCU_GATE(dcxo24M_out_clk, "dcxo24M-out", "dcxo24M", 0x160, BIT(31), 0);

/* rtc_src_sel: osc32k(losc) */
static SUNXI_CCU_GATE(iosc_clk, "iosc", "rc-16m", 0x160, BIT(0), 0);
static CLK_FIXED_FACTOR(iosc_div32k_clk, "iosc-div32k", "iosc", 500, 1, 0);
static const char * const osc32k_parents[] = { "iosc-div32k", "" };
static SUNXI_CCU_MUX_WITH_GATE_KEY(osc32k_clk, "osc32k", osc32k_parents,
				   0x0, 0, 1,
				   KEY_FIELD_MAGIC_NUM_RTC, 0, 0);

/* rtc_src_sel: dcxo24M-div32k */
static SUNXI_CCU_GATE_WITH_FIXED_RATE(dcxo24M_div32k_clk, "dcxo24M-div32k",
				      "dcxo24M", 0x60,
				      32768, BIT(16));

/* rtc-32k clock */
static const char * const rtc32k_clk_parents[] = { "osc32k", "dcxo24M-div32k"};
static SUNXI_CCU_MUX_WITH_GATE_KEY(rtc32k_clk, "rtc32k", rtc32k_clk_parents,
				   0x0, 1, 1,
				   KEY_FIELD_MAGIC_NUM_RTC, 0, 0);
/* rtc-1k clock */
static CLK_FIXED_FACTOR(rtc_1k_clk, "rtc-1k", "rtc32k", 32, 1, 0);

/* rtc-32k-fanout: only for debug */
static const char * const rtc_32k_fanout_clk_parents[] = { "osc32k", "",
							   "dcxo24M-div32k"};
static SUNXI_CCU_MUX_WITH_GATE(osc32k_out_clk, "osc32k-out",
			       rtc_32k_fanout_clk_parents, 0x60, 1,
			       2, BIT(0), 0);

/* rtc_spi_clk_gate */
static SUNXI_CCU_GATE(rtc_spi_clk, "rtc-spi", "r-ahb", 0x310, BIT(31), 0);

static struct ccu_common *sun50iw15_rtc_ccu_clks[] = {
	&dcxo24M_out_clk.common,
	&iosc_clk.common,
	&osc32k_clk.common,
	&dcxo24M_div32k_clk.common,
	&rtc32k_clk.common,
	&osc32k_out_clk.common,
	&rtc_spi_clk.common,
};

static struct clk_hw_onecell_data sun50iw15_rtc_ccu_hw_clks = {
	.hws	= {
		[CLK_DCXO24M_OUT]		= &dcxo24M_out_clk.common.hw,
		[CLK_IOSC]			= &iosc_clk.common.hw,
		[CLK_IOSC_DIV32K]		= &iosc_div32k_clk.hw,
		[CLK_OSC32K]			= &osc32k_clk.common.hw,
		[CLK_DCXO24M_DIV32K]		= &dcxo24M_div32k_clk.common.hw,
		[CLK_RTC32K]			= &rtc32k_clk.common.hw,
		[CLK_RTC_1K]			= &rtc_1k_clk.hw,
		[CLK_OSC32K_OUT]		= &osc32k_out_clk.common.hw,
		[CLK_RTC_SPI]			= &rtc_spi_clk.common.hw,
	},
	.num	= CLK_NUMBER,
};

static const struct sunxi_ccu_desc sun50iw15_rtc_ccu_desc = {
	.ccu_clks	= sun50iw15_rtc_ccu_clks,
	.num_ccu_clks	= ARRAY_SIZE(sun50iw15_rtc_ccu_clks),

	.hw_clks	= &sun50iw15_rtc_ccu_hw_clks,
};

static void clock_source_init(char __iomem *base)
{
	/* select losc(rc-16m div 32k) as the clock source for rtc */
	set_reg(base + LOSC_CTRL_REG, 0x0, 1, 0);
	set_reg(base + LOSC_CTRL_REG, 0x0, 1, 1);
}

static int sun50iw15_rtc_ccu_probe(struct platform_device *pdev)
{
	struct resource *res;
	struct device *dev = &pdev->dev;
	void __iomem *reg;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		dev_err(dev, "Fail to get IORESOURCE_MEM\n");
		return -EINVAL;
	}

	/*
	 * Don't use devm_ioremap_resource() here! Or else the RTC driver will
	 * not able to get the same resource later in rtc-sunxi.c.
	 */
	reg = devm_ioremap(dev, res->start, resource_size(res));
	if (IS_ERR(reg)) {
		dev_err(dev, "Fail to map IO resource\n");
		return PTR_ERR(reg);
	}

	clock_source_init(reg);

	return sunxi_ccu_probe(pdev->dev.of_node, reg, &sun50iw15_rtc_ccu_desc);
}

static const struct of_device_id sun50iw15_rtc_ccu_ids[] = {
	{ .compatible = "allwinner,sun50iw15-rtc-ccu" },
	{ }
};

static struct platform_driver sun50iw15_rtc_ccu_driver = {
	.probe	= sun50iw15_rtc_ccu_probe,
	.driver	= {
		.name	= "sun50iw15-rtc-ccu",
		.of_match_table	= sun50iw15_rtc_ccu_ids,
	},
};

static int __init sun50iw15_rtc_ccu_init(void)
{
	int err;

	err = platform_driver_register(&sun50iw15_rtc_ccu_driver);
	if (err)
		pr_err("Fail to register sunxi_rtc_ccu as platform device\n");

	return err;
}
core_initcall(sun50iw15_rtc_ccu_init);

static void __exit sun50iw15_rtc_ccu_exit(void)
{
	platform_driver_unregister(&sun50iw15_rtc_ccu_driver);
}
module_exit(sun50iw15_rtc_ccu_exit);

MODULE_DESCRIPTION("sunxi RTC CCU driver");
MODULE_AUTHOR("xuefan");
MODULE_LICENSE("GPL v2");
MODULE_VERSION("0.0.1");
