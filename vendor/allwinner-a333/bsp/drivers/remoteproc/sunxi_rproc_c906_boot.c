/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright  2020-2025, Allwinnertech
 *
 * This file is provided under a dual BSD/GPL license.  When using or
 * redistributing this file, you may do so under either license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

/* #define DEBUG */
#include <linux/clk.h>
#include <linux/clk-provider.h>
#include <linux/dma-mapping.h>
#include <linux/err.h>
#include <linux/kernel.h>
#include <linux/mfd/syscon.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/regmap.h>
#include <linux/reset.h>
#include <asm/io.h>
#include "sunxi_rproc_boot.h"

/*
 * Register define
 */
#define RISCV_VER_REG (0x0000) /* RISCV Reset Control Register */
#define RISCV_STA_ADD0_REG (0x0004)
#define RISCV_STA_ADD1_REG (0x0008)
#define RF1P_CFG_REG (0x0010)	  /* RISCV Control Register0 */
#define TS_TMODE_SEL_REG (0x0040) /* RISCV PRID Register */

#define CCMU_RISCV_CLK_REG (0x0d00)
#define RISCV_CLK_MASK (0x7 << 24)
#define RISCV_CLK_HOSC (0x0 << 24)
#define RISCV_CLK_32K (0x1 << 24)
#define RISCV_CLK_16M (0x2 << 24)
#define RISCV_CLK_PERI_800M (0x3 << 24)
#define RISCV_CLK_PERI_1X (0x4 << 24)
#define RISCV_CLK_CPUPLL (0x5 << 24)
/* x must be 1 - 4 */
#define RISCV_AXI_FACTOR_N(x) (((x)-1) << 0)
/* x must be 1 - 32 */
#define RISCV_CLK_FACTOR_M(x) (((x)-1) << 0)
#define RISCV_CLK_M_MASK (0x1f << 0)

#define RISCV_GATING_RST_REG (0x0d04)
#define RISCV_GATING_RST_FIELD (0x16aa << 0)
#define RISCV_CLK_GATING (0x1 << 31)

#define RISCV_CFG_BGR_REG (0x0d0c)
#define RISCV_CFG_RST (0x1 << 16)
#define RISCV_CFG_GATING (0x1 << 0)

#define RISCV_RST_REG (0x0f20)
#define RISCV_RST_FIELD (0x16aa << 16)
#define RISCV_SOFT_RSTN (0x1 << 0)

extern int simulator_debug;
static int sunxi_rproc_c906_assert(struct sunxi_rproc_priv *rproc_priv);
static int sunxi_rproc_c906_deassert(struct sunxi_rproc_priv *rproc_priv);

static int devm_sunxi_rproc_c906_resource_get(struct sunxi_rproc_priv *rproc_priv, struct platform_device *pdev)
{
	struct sunxi_rproc_c906_cfg *c906_cfg;
	struct device *dev = &pdev->dev;
	struct device_node *np = dev->of_node;
	struct resource *res;
	u32 *map_array;
	int ret, i;

	rproc_priv->dev = dev;

	c906_cfg = devm_kzalloc(dev, sizeof(*c906_cfg), GFP_KERNEL);
	if (!c906_cfg) {
		dev_err(dev, "alloc c906 cfg error\n");
		return -ENOMEM;
	}

	c906_cfg->pll_clk = devm_clk_get(dev, "pll");
	if (IS_ERR_OR_NULL(c906_cfg->pll_clk)) {
		dev_err(dev, "no find pll in dts\n");
		return -ENXIO;
	}

	c906_cfg->mod_clk = devm_clk_get(dev, "mod");
	if (IS_ERR_OR_NULL(c906_cfg->mod_clk)) {
		dev_err(dev, "no find mod in dts\n");
		return -ENXIO;
	}

	c906_cfg->cfg_clk = devm_clk_get(dev, "cfg");
	if (IS_ERR_OR_NULL(c906_cfg->cfg_clk)) {
		dev_err(dev, "no find cfg in dts\n");
		return -ENXIO;
	}
	c906_cfg->rst_clk = devm_clk_get(dev, "riscv-rst");
	if (IS_ERR_OR_NULL(c906_cfg->rst_clk)) {
		dev_err(dev, "no find rst in dts\n");
		return -ENXIO;
	}

	c906_cfg->gate_clk = devm_clk_get(dev, "riscv-gate");
	if (IS_ERR_OR_NULL(c906_cfg->gate_clk)) {
		dev_err(dev, "no find gate in dts\n");
		return -ENXIO;
	}

	c906_cfg->cfg_rst = devm_reset_control_get(dev, "cfg-rst");
	if (IS_ERR_OR_NULL(c906_cfg->cfg_rst)) {
		dev_err(dev, "can't find cfg-rst in dts\n");
		return -ENXIO;
	}

	c906_cfg->msg_rst = devm_reset_control_get(dev, "msg-rst");
	if (IS_ERR_OR_NULL(c906_cfg->cfg_rst)) {
		dev_err(dev, "no find msg-rst in dts\n");
		return -ENXIO;
	}

	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "c906-cfg");
	if (IS_ERR_OR_NULL(res)) {
		dev_err(dev, "no find c906-cfg in dts\n");
		return -ENXIO;
	}

	c906_cfg->c906_cfg = devm_ioremap_resource(dev, res);
	if (IS_ERR_OR_NULL(c906_cfg->c906_cfg)) {
		dev_err(dev, "fail to ioremap c906-cfg\n");
		return -ENXIO;
	}

	ret = of_property_count_elems_of_size(np, "memory-mappings", sizeof(u32) * 3);
	if (ret <= 0) {
		dev_err(dev, "fail to get memory-mappings\n");
		return -ENXIO;
	}
	rproc_priv->mem_maps_cnt = ret;
	rproc_priv->mem_maps = devm_kcalloc(dev, rproc_priv->mem_maps_cnt,
										sizeof(struct sunxi_rproc_memory_mapping),
										GFP_KERNEL);
	if (!rproc_priv->mem_maps)
		return -ENOMEM;

	map_array = devm_kcalloc(dev, rproc_priv->mem_maps_cnt * 3, sizeof(u32), GFP_KERNEL);
	if (!map_array)
		return -ENOMEM;

	ret = of_property_read_u32_array(np, "memory-mappings", map_array,
									 rproc_priv->mem_maps_cnt * 3);
	if (ret) {
		dev_err(dev, "fail to read memory-mappings\n");
		return -ENXIO;
	}

	for (i = 0; i < rproc_priv->mem_maps_cnt; i++) {
		rproc_priv->mem_maps[i].da = map_array[i * 3];
		rproc_priv->mem_maps[i].len = map_array[i * 3 + 1];
		rproc_priv->mem_maps[i].pa = map_array[i * 3 + 2];
		dev_dbg(dev, "memory-mappings[%d]: da: 0x%llx, len: 0x%llx, pa: 0x%llx\n",
				i, rproc_priv->mem_maps[i].da, rproc_priv->mem_maps[i].len,
				rproc_priv->mem_maps[i].pa);
	}

	devm_kfree(dev, map_array);

	rproc_priv->rproc_cfg = c906_cfg;

	return 0;
}

static void devm_sunxi_rproc_c906_resource_put(struct sunxi_rproc_priv *rproc_priv, struct platform_device *pdev)
{
	return;
}

static int sunxi_rproc_c906_attach(struct sunxi_rproc_priv *rproc_priv)
{
	struct sunxi_rproc_c906_cfg *c906_cfg = rproc_priv->rproc_cfg;
	struct device *dev = rproc_priv->dev;
	int ret;

	dev_dbg(dev, "%s,%d\n", __func__, __LINE__);

	ret = clk_prepare_enable(c906_cfg->gate_clk);
	if (ret) {
		dev_err(dev, "gate clk enable err\n");
		return ret;
	}

	ret = clk_prepare_enable(c906_cfg->cfg_clk);
	if (ret) {
		dev_err(dev, "cfg clk enable err\n");
		return ret;
	}

	ret = sunxi_rproc_c906_deassert(rproc_priv);
	if (ret) {
		dev_err(dev, "rproc dessert err\n");
		return ret;
	}

	ret = clk_prepare_enable(c906_cfg->pll_clk);
	if (ret) {
		dev_err(dev, "pll clk enable err\n");
		return ret;
	}

	ret = clk_set_parent(c906_cfg->mod_clk, c906_cfg->pll_clk);
	if (ret) {
		dev_err(dev, "set mod clk parent to pll_clk err\n");
		return ret;
	}

	ret = clk_prepare_enable(c906_cfg->rst_clk);
	if (ret) {
		dev_err(dev, "cfg clk enable err\n");
		return ret;
	}

	return 0;
}

static int sunxi_rproc_c906_start(struct sunxi_rproc_priv *rproc_priv)
{
	struct sunxi_rproc_c906_cfg *c906_cfg = rproc_priv->rproc_cfg;
	struct device *dev = rproc_priv->dev;
	int ret;

	dev_dbg(dev, "%s,%d\n", __func__, __LINE__);

	if (simulator_debug) {
		dev_dbg(dev, "%s,%d c906 does not need to reset clk\n",
				__func__, __LINE__);
		return 0;
	}

	ret = clk_prepare_enable(c906_cfg->gate_clk);
	if (ret) {
		dev_err(dev, "gate clk enable err\n");
		return ret;
	}

	ret = clk_prepare_enable(c906_cfg->cfg_clk);
	if (ret) {
		dev_err(dev, "cfg clk enable err\n");
		return ret;
	}

	ret = sunxi_rproc_c906_assert(rproc_priv);
	if (ret) {
		dev_err(dev, "rproc assert err\n");
		return ret;
	}

	ret = sunxi_rproc_c906_deassert(rproc_priv);
	if (ret) {
		dev_err(dev, "rproc dessert err\n");
		return ret;
	}

	/* set vector */
	writel(rproc_priv->pc_entry, (c906_cfg->c906_cfg + RISCV_STA_ADD0_REG));
	writel(0, (c906_cfg->c906_cfg + RISCV_STA_ADD1_REG));

	ret = clk_prepare_enable(c906_cfg->pll_clk);
	if (ret) {
		dev_err(dev, "pll clk enable err\n");
		return ret;
	}

	ret = clk_set_parent(c906_cfg->mod_clk, c906_cfg->pll_clk);
	if (ret) {
		dev_err(dev, "set mod clk parent to pll_clk err\n");
		return ret;
	}

	ret = clk_prepare_enable(c906_cfg->rst_clk);
	if (ret) {
		dev_err(dev, "cfg clk enable err\n");
		return ret;
	}

	return 0;
}

static int sunxi_rproc_c906_stop(struct sunxi_rproc_priv *rproc_priv)
{
	struct sunxi_rproc_c906_cfg *c906_cfg = rproc_priv->rproc_cfg;
	int ret;

	dev_dbg(rproc_priv->dev, "%s,%d\n", __func__, __LINE__);

	if (simulator_debug) {
		dev_dbg(rproc_priv->dev, "%s,%d c906 does not need to close clk\n",
				__func__, __LINE__);
		return 0;
	}

	clk_disable(c906_cfg->rst_clk);
	clk_disable(c906_cfg->cfg_clk);
	clk_disable(c906_cfg->gate_clk);
	ret = sunxi_rproc_c906_assert(rproc_priv);
	if (ret) {
		dev_err(rproc_priv->dev, "rproc assert err\n");
		return ret;
	}

	return 0;
}

static int sunxi_rproc_c906_assert(struct sunxi_rproc_priv *rproc_priv)
{
	struct sunxi_rproc_c906_cfg *c906_cfg = rproc_priv->rproc_cfg;
	int ret;

	ret = reset_control_assert(c906_cfg->cfg_rst);
	if (ret) {
		dev_err(rproc_priv->dev, "cfg rst assert err\n");
		return -ENXIO;
	}

	ret = reset_control_assert(c906_cfg->msg_rst);
	if (ret) {
		dev_err(rproc_priv->dev, "msg rst assert err\n");
		return -ENXIO;
	}

	return ret;
}

static int sunxi_rproc_c906_deassert(struct sunxi_rproc_priv *rproc_priv)
{
	struct sunxi_rproc_c906_cfg *c906_cfg = rproc_priv->rproc_cfg;
	int ret;

	ret = reset_control_deassert(c906_cfg->cfg_rst);
	if (ret) {
		dev_err(rproc_priv->dev, "cfg rst de-assert err\n");
		return -ENXIO;
	}

	ret = reset_control_deassert(c906_cfg->msg_rst);
	if (ret) {
		dev_err(rproc_priv->dev, "msg rst de-assert err\n");
		return -ENXIO;
	}

	return ret;
}

static int sunxi_rproc_c906_reset(struct sunxi_rproc_priv *rproc_priv)
{
	int ret;

	ret = sunxi_rproc_c906_assert(rproc_priv);
	if (ret)
		return -ENXIO;

	ret = sunxi_rproc_c906_deassert(rproc_priv);
	if (ret)
		return -ENXIO;

	return ret;
}

static int sunxi_rproc_c906_enable_sram(struct sunxi_rproc_priv *rproc_priv, u32 value)
{
	/* c906 not use sram enable*/
	return 0;
}

static int sunxi_rproc_c906_set_runstall(struct sunxi_rproc_priv *rproc_priv, u32 value)
{
	/* c906 do not have runstall reg bit */
	return 0;
}

static bool sunxi_rproc_c906_is_booted(struct sunxi_rproc_priv *rproc_priv)
{
	struct sunxi_rproc_c906_cfg *c906_cfg = rproc_priv->rproc_cfg;

	if (__clk_is_enabled(c906_cfg->cfg_clk) && __clk_is_enabled(c906_cfg->rst_clk))
		return true;
	else
		return false;
}

static struct sunxi_rproc_ops sunxi_rproc_c906_ops = {
	.resource_get = devm_sunxi_rproc_c906_resource_get,
	.resource_put = devm_sunxi_rproc_c906_resource_put,
	.start = sunxi_rproc_c906_start,
	.stop = sunxi_rproc_c906_stop,
	.attach = sunxi_rproc_c906_attach,
	.reset = sunxi_rproc_c906_reset,
	.set_localram = sunxi_rproc_c906_enable_sram,
	.set_runstall = sunxi_rproc_c906_set_runstall,
	.is_booted = sunxi_rproc_c906_is_booted,
};

/* c906_boot_init must run before sunxi_rproc probe */
static int __init sunxi_rproc_c906_boot_init(void)
{
	int ret;

	ret = sunxi_rproc_priv_ops_register("c906", &sunxi_rproc_c906_ops, NULL);
	if (ret) {
		pr_err("c906 register ops failed\n");
		return ret;
	}

	return 0;
}
subsys_initcall(sunxi_rproc_c906_boot_init);

static void __exit sunxi_rproc_c906_boot_exit(void)
{
	int ret;

	ret = sunxi_rproc_priv_ops_unregister("c906");
	if (ret)
		pr_err("c906 unregister ops failed\n");
}
module_exit(sunxi_rproc_c906_boot_exit)

MODULE_DESCRIPTION("Allwinner sunxi rproc c906 boot driver");
MODULE_AUTHOR("caoyangguo <caoyangguo@allwinnertech.com>");
MODULE_AUTHOR("wujiayi <wujiayi@allwinnertech.com>");
MODULE_LICENSE("GPL v2");
MODULE_VERSION("1.0.2");
