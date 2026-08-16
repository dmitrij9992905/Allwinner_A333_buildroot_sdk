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
#include <linux/clk.h>
#include <linux/iopoll.h>
#include <linux/syscore_ops.h>
#include <linux/slab.h>

#include "ccu_common.h"
#include "ccu_reset.h"
#include "ccu_nm.h"
#include "ccu_nkmp.h"
#include <dt-bindings/clock/sun8iw22-cpupll-ccu.h>

#define SUNXI_CPUPLL_CCU_VERSION	"0.0.1"

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

/* ccu base */
#define SUN8IW22_PLL_CPU_REG		(0x0340)
#define SUN8IW22_PLL_CPU_PAT0_REG	(0x0344)
#define SUN8IW22_PLL_CPU_SSC_REG	(0x0354)

/* cpux pll cfg base */
#define SUN8IW22_CPU_CLK_REG			(0x0018)
#define SUN8IW22_CPU_CLK_DIV_REG		(0x0020)

#define SUN8IW22_CPU_PLL_MAX_RATE		(2016000000)

static struct ccu_nkmp pll_cpu_clk = {
	.output			= BIT(27),
	.lock			= BIT(28),
	.lock_enable		= BIT(29),
	.enable			= BIT(31),
	.n		= _SUNXI_CCU_MULT_OFFSET_MIN_MAX(8, 8, 0, 20, 84),
//	.m			= _SUNXI_CCU_DIV(0, 4), /* m will not be used */
	.max_rate	= SUN8IW22_CPU_PLL_MAX_RATE,
//	.p		= _SUNXI_CCU_DIV(16, 2), /* p in cpu_clk reg, it will not be used when freq below 480 */
//	.p_reg		= SUN8IW22_CPU_CLK_REG,
	.common			= {
		.reg		= SUN8IW22_PLL_CPU_REG,
		.ssc_reg	= SUN8IW22_PLL_CPU_SSC_REG,
		.clear		= BIT(26),
		.features	= CCU_FEATURE_CLEAR_MOD | CCU_FEATURE_CLAC_CACHED | CCU_FEATURE_TYPE_NKMP,
		.hw.init	= CLK_HW_INIT("pll-cpu", "dcxo24M",
					&ccu_nkmp_ops,
				CLK_GET_RATE_NOCACHE | CLK_IS_CRITICAL \
				| CLK_SET_RATE_UNGATE),
	},
};

static const char * const cpu_parents[] = { "dcxo24M", "osc32k", "iosc", "pll-cpu", "pll-peri0-2x", "pll-peri0-600m", "pll-peri1-2x" };
static SUNXI_CCU_MUX(cpu_clk, "cpu", cpu_parents,
		SUN8IW22_CPU_CLK_REG, 24, 3, CLK_SET_RATE_PARENT | CLK_IS_CRITICAL);


/* M or N only support 1 or 3 */
static SUNXI_CCU_M(cpu_apb_div_clk, "cpu-apb-div",
		"cpu", SUN8IW22_CPU_CLK_DIV_REG, 2, 2, 0);
static SUNXI_CCU_M(cpu_axi_div_clk, "cpu-axi-div",
		"cpu", SUN8IW22_CPU_CLK_DIV_REG, 0, 2, 0);

static struct ccu_common *sunxi_pll_cpu_clks[] = {
	&pll_cpu_clk.common,
	&cpu_clk.common,
	&cpu_apb_div_clk.common,
	&cpu_axi_div_clk.common,
};

static struct clk_hw_onecell_data sunxi_cpupll_hw_clks = {
	.hws	= {
		[CLK_PLL_CPU]		= &pll_cpu_clk.common.hw,
		[CLK_CPU]		= &cpu_clk.common.hw,
		[CLK_CPU_APB_DIV]	= &cpu_apb_div_clk.common.hw,
		[CLK_CPU_AXI_DIV]	= &cpu_axi_div_clk.common.hw,
	},
	.num = CLK_CPUPLL_MAX_NO,
};

static const struct sunxi_ccu_desc cpupll_desc = {
	.ccu_clks	= sunxi_pll_cpu_clks,
	.num_ccu_clks	= ARRAY_SIZE(sunxi_pll_cpu_clks),
	.hw_clks	= &sunxi_cpupll_hw_clks,
	.resets		= NULL,
	.num_resets	= 0,
};

static const u32 __maybe_unused sun8iw22_pll_cpu_regs[] = {
	SUN8IW22_PLL_CPU_REG,
};

static const u32 __maybe_unused sun8iw22_pll_cpu_ssc_regs[] = {
	SUN8IW22_PLL_CPU_SSC_REG,
};

static void ccupll_helper_wait_for_lock(void __iomem *addr, u32 lock)
{
	u32 reg;

	WARN_ON(readl_relaxed_poll_timeout(addr, reg, reg & lock, 100, 70000));
}

static void cpupll_helper_wait_for_clear(void __iomem *addr, u32 clear)
{
	u32 reg;

	reg = readl(addr);
	writel(reg | clear, addr);

	WARN_ON(readl_relaxed_poll_timeout_atomic(addr, reg, !(reg & clear), 100, 10000));
}

static int cpupll_notifier_cb(struct notifier_block *nb, unsigned long event, void *data)
{
	struct ccu_pll_nb *pll = to_ccu_pll_nb(nb);
	int ret = 0;

	if (event == PRE_RATE_CHANGE) {
		/* Enable ssc function */
		set_reg(pll->common->base + pll->common->ssc_reg, 1, 1, pll->enable);
	} else if (event == POST_RATE_CHANGE) {
		/* Disable ssc function */
		set_reg(pll->common->base + pll->common->ssc_reg, 0, 1, pll->enable);
		ccu_helper_wait_for_clear(pll->common, pll->common->clear);
	}

	return notifier_from_errno(ret);
}

static struct ccu_pll_nb cpupll_nb = {
	.common = &pll_cpu_clk.common,
	.enable = 31, /* switch ssc mode */
	.clk_nb = {
		.notifier_call = cpupll_notifier_cb,
	},
};

void __iomem *reg_pll_cpu_base;
void __iomem *reg_pll_cpu;
void __iomem *reg_pll_cpu_pat0;
void __iomem *reg_pll_cpu_ssc;

static const struct of_device_id sun8iw22_cpupll_ids[] = {
	{ .compatible = "allwinner,sun8iw22-cpupll" },
	{ }
};

static int sun8iw22_cpupll_probe(struct platform_device *pdev)
{
	int i;
	u32 val;
	struct resource *res;
	unsigned int step = 0, ssc = 0;
	struct device_node *np = pdev->dev.of_node;

	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "pll_cpu_base");
	if (!res) {
		sunxi_err(&pdev->dev, "find pll_cpu_base resource\n");
		return -ENOMEM;
	}

	reg_pll_cpu_base = ioremap(res->start, resource_size(res));
	if (!reg_pll_cpu_base) {
		sunxi_err(&pdev->dev, "reg_pll_cpu_base ioremap\n");
		return -ENOMEM;
	}

	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "pll_cpu");
	if (!res) {
		sunxi_err(&pdev->dev, "find pll_cpu resource\n");
		return -ENOMEM;
	}
	reg_pll_cpu = ioremap(res->start, resource_size(res));
	if (!reg_pll_cpu) {
		sunxi_err(&pdev->dev, "reg_pll_cpu ioremap\n");
		return -ENOMEM;
	}

	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "pll_cpu_pat0");
	if (!res) {
		sunxi_err(&pdev->dev, "find pll_cpu_pat0 resource\n");
		return -ENOMEM;
	}
	reg_pll_cpu_pat0 = ioremap(res->start, resource_size(res));
	if (!reg_pll_cpu_pat0) {
		sunxi_err(&pdev->dev, "reg_pll_cpu_pat0 ioremap\n");
		return -ENOMEM;
	}

	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "pll_cpu_ssc");
	if (!res) {
		sunxi_err(&pdev->dev, "find pll_cpu_ssc resource\n");
		return -ENOMEM;
	}
	reg_pll_cpu_ssc = ioremap(res->start, resource_size(res));
	if (!reg_pll_cpu_ssc) {
		sunxi_err(&pdev->dev, "reg_pll_cpu_ssc ioremap\n");
		return -ENOMEM;
	}

	if (of_property_read_u32(np, "pll_step", &step))
		step = 0x8;

	if (of_property_read_u32(np, "pll_ssc", &ssc))
		ssc = 0x3300;

	/* TODO: assume boot use the cpupll */
	for (i = 0; i < ARRAY_SIZE(sun8iw22_pll_cpu_ssc_regs); i++) {
		/*
		 * 1. Config n,m1,m0,p: default:480M
		 * 2. Enable pll_en pll_ldo_en lock_en pll_output
		 * 3. wait for update and lock
		 */
		val = readl(reg_pll_cpu);
		val |= BIT(27) | BIT(29) | BIT(31);
		writel(val, reg_pll_cpu);

		cpupll_helper_wait_for_clear(reg_pll_cpu, BIT(26));
		ccupll_helper_wait_for_lock(reg_pll_cpu, BIT(28));

		/*
		 * set pat0_ctrl_reg
		 */
		val = readl(reg_pll_cpu_pat0);
		val |= GENMASK(30, 29);  /* ues Triangular(3bit) */
		writel(val, reg_pll_cpu_pat0);

		/*
		 * set ssc/step in ssc reg
		 */
		val = readl(reg_pll_cpu_ssc);
		val &= ~GENMASK(28, 12);
		val &= ~GENMASK(3, 0);
		val |= (ssc << 12 | step << 0);
		writel(val, reg_pll_cpu_ssc);

		/*
		 * enable ssc mode
		 */
		val = readl(reg_pll_cpu_ssc);
		val |= BIT(31);
		writel(val, reg_pll_cpu_ssc);

		cpupll_helper_wait_for_clear(reg_pll_cpu, BIT(26));

		/*
		 * disable ssc mode
		 */
		val = readl(reg_pll_cpu_ssc);
		val &= ~BIT(31);
		writel(val, reg_pll_cpu_ssc);

		cpupll_helper_wait_for_clear(reg_pll_cpu, BIT(26));
	}

	sunxi_ccu_probe(pdev->dev.of_node, reg_pll_cpu_base, &cpupll_desc);

	ccu_pll_notifier_register(&cpupll_nb);

	sunxi_info(NULL, "sunxi cpupll driver version: %s\n", SUNXI_CPUPLL_CCU_VERSION);

	return 0;

}

static struct platform_driver sun8iw22_cpupll_driver = {
	.probe	= sun8iw22_cpupll_probe,
	.driver	= {
		.name	= "sun8iw22-cpupll",
		.of_match_table	= sun8iw22_cpupll_ids,
	},
};

static int __init sun8iw22_ccu_cpupll_init(void)
{
	int err;

	err = platform_driver_register(&sun8iw22_cpupll_driver);
	if (err)
		pr_err("register ccu sun8iw22 failed\n");

	return err;
}
core_initcall(sun8iw22_ccu_cpupll_init);

static void __exit sun8iw22_ccu_exit(void)
{

	iounmap(reg_pll_cpu_base);
	iounmap(reg_pll_cpu);
	iounmap(reg_pll_cpu_pat0);
	iounmap(reg_pll_cpu_ssc);

	platform_driver_unregister(&sun8iw22_cpupll_driver);
}
module_exit(sun8iw22_ccu_exit);

MODULE_DESCRIPTION("Allwinner sun8iw22 clk driver");
MODULE_AUTHOR("liufeng");
MODULE_LICENSE("GPL v2");
MODULE_VERSION(SUNXI_CPUPLL_CCU_VERSION);
