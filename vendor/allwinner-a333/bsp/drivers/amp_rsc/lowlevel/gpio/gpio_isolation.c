/* SPDX-License-Identifier: GPL-2.0-only */
/* Copyright (c) 2019-2025 Allwinner Technology Co., Ltd. ALL rights reserved.
 *
 * Allwinner is a trademark of Allwinner Technology Co.,Ltd., registered in
 * the People's Republic of China and other countries.
 * All Allwinner Technology Co.,Ltd. trademarks are used with permission.
 *
 * DISCLAIMER
 * THIRD PARTY LICENCES MAY BE REQUIRED TO IMPLEMENT THE SOLUTION/PRODUCT.
 * IF YOU NEED TO INTEGRATE THIRD PARTY'S TECHNOLOGY (SONY, DTS, DOLBY, AVS OR MPEGLA, ETC.)
 * IN ALLWINNERS'SDK OR PRODUCTS, YOU SHALL BE SOLELY RESPONSIBLE TO OBTAIN
 * ALL APPROPRIATELY REQUIRED THIRD PARTY LICENCES.
 * ALLWINNER SHALL HAVE NO WARRANTY, INDEMNITY OR OTHER OBLIGATIONS WITH RESPECT TO MATTERS
 * COVERED UNDER ANY REQUIRED THIRD PARTY LICENSE.
 * YOU ARE SOLELY RESPONSIBLE FOR YOUR USAGE OF THIRD PARTY'S TECHNOLOGY.
 *
 *
 * THIS SOFTWARE IS PROVIDED BY ALLWINNER"AS IS" AND TO THE MAXIMUM EXTENT
 * PERMITTED BY LAW, ALLWINNER EXPRESSLY DISCLAIMS ALL WARRANTIES OF ANY KIND,
 * WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING WITHOUT LIMITATION REGARDING
 * THE TITLE, NON-INFRINGEMENT, ACCURACY, CONDITION, COMPLETENESS, PERFORMANCE
 * OR MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * IN NO EVENT SHALL ALLWINNER BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS, OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <asrm_lowlevel.h>

#include "gpio_isolation.h"

#define PORT_OS_SEL_REG_BASE		(0x0800)
#define PORT_OS_SEL_REG_OFFSET		(0x4)
#define PORT_OS_SEL_REG_WIDTH		(16)
#define PORT_OS_SEL_FIELD_MASK		(0x3)
#define PORT_OS_SEL_FIELD_OFFSET	(2)

#define GPIO_OS_ID_CFG_REG		(0x0860)
#define GPIO_OS_ID_CFG_FIELD_MASK	(0x1f)
#define GPIO_OS_ID_CFG_FIELD_OFFSET	(8)

#define GPIO_OS_ID_CFG_BYPASS_REG	(0x0864)
#define GPIO_OS_BYPASS_KEY		(0x23AA)

#define POW2(n)		(1 << n)

#ifndef SUNXI_GPIOS_PER_BANK
#define SUNXI_GPIOS_PER_BANK		(32)
#endif

struct gpio_isolator {
	uint32_t id_map;
	void __iomem *base_addr;
};

static int gpio_iso_enable(hw_isolator_dev_t *idev)
{
	struct gpio_isolator *gpio_iso;
	void __iomem *gpio_os_bypass_reg;
	uint32_t reg_val = 0;

	if (!idev)
		return -1;

	gpio_iso = hw_isolator_dev_get_drvdata(idev);
	gpio_os_bypass_reg = gpio_iso->base_addr + GPIO_OS_ID_CFG_BYPASS_REG;

	reg_val = readl(gpio_os_bypass_reg);
	reg_val |= (GPIO_OS_BYPASS_KEY << 16);
	writel(reg_val, gpio_os_bypass_reg);

	reg_val = readl(gpio_os_bypass_reg);
	reg_val &= ~(0x1 << 0);		/* no-bypass */
	writel(reg_val, gpio_os_bypass_reg);

	return 0;
}

static int gpio_iso_disable(hw_isolator_dev_t *idev)
{
	struct gpio_isolator *gpio_iso;
	void __iomem *gpio_os_bypass_reg;
	uint32_t reg_val = 0;

	if (!idev)

		return -1;
	gpio_iso = hw_isolator_dev_get_drvdata(idev);
	gpio_os_bypass_reg = gpio_iso->base_addr + GPIO_OS_ID_CFG_BYPASS_REG;

	reg_val = readl(gpio_os_bypass_reg);
	reg_val |= (GPIO_OS_BYPASS_KEY << 16);
	writel(reg_val, gpio_os_bypass_reg);

	reg_val = readl(gpio_os_bypass_reg);
	reg_val |= (0x1 << 0);		/* bypass */
	writel(reg_val, gpio_os_bypass_reg);

	return 0;
}

static int gpio_iso_set_user_group(hw_isolator_dev_t *idev, const hw_rsc_user_group_info_t *group_info)
{
	struct gpio_isolator *gpio_iso;
	void __iomem *gpio_os_id_cfg_reg;
	uint32_t reg_val = 0;
	uint32_t i = 0;

	if (!idev || !group_info)
		return -1;

	gpio_iso = hw_isolator_dev_get_drvdata(idev);
	gpio_os_id_cfg_reg = gpio_iso->base_addr + GPIO_OS_ID_CFG_REG;

	reg_val = readl(gpio_os_id_cfg_reg);
	reg_val &= ~(GPIO_OS_ID_CFG_FIELD_MASK << (group_info->id * GPIO_OS_ID_CFG_FIELD_OFFSET));
	writel(reg_val, gpio_os_id_cfg_reg);

	for (i = 0; i < group_info->user_cnt; i++) {
		reg_val = readl(gpio_os_id_cfg_reg);
		reg_val |= (POW2(group_info->user_id[i]) << (group_info->id * GPIO_OS_ID_CFG_FIELD_OFFSET));
		writel(reg_val, gpio_os_id_cfg_reg);
	}

	return 0;
}

static int gpio_iso_get_user_group(hw_isolator_dev_t *idev, hw_rsc_user_group_info_t *group_info)
{
	struct gpio_isolator *gpio_iso;
	void __iomem *gpio_os_id_cfg_reg;
	uint32_t reg_val = 0;

	gpio_iso = hw_isolator_dev_get_drvdata(idev);
	gpio_os_id_cfg_reg = gpio_iso->base_addr + GPIO_OS_ID_CFG_REG;

	reg_val = readl(gpio_os_id_cfg_reg);
	reg_val = (reg_val >> (group_info->id * GPIO_OS_ID_CFG_FIELD_OFFSET) & GPIO_OS_ID_CFG_FIELD_MASK);
	group_info->id = reg_val;

	return 0;
}

static int gpio_iso_set_resource_owner(hw_isolator_dev_t *idev, const hw_rsc_info_t *rsc_info, uint32_t user_group_id)
{
	struct gpio_isolator *gpio_iso;
	void __iomem *gpio_os_sel_reg_base;
	uint32_t gpio_id = 0;
	void __iomem *port_reg = 0;
	uint32_t reg_val = 0;

	if (rsc_info->type != SUNXI_AMP_RSC_HW_GPIO)
		return -1;

	gpio_iso = hw_isolator_dev_get_drvdata(idev);
	gpio_os_sel_reg_base = gpio_iso->base_addr + PORT_OS_SEL_REG_BASE;

	gpio_id = rsc_info->gpio.gpio_id;
	port_reg = gpio_os_sel_reg_base + ((gpio_id / PORT_OS_SEL_REG_WIDTH) * PORT_OS_SEL_REG_OFFSET);
	reg_val = readl(port_reg);
	reg_val &= ~(PORT_OS_SEL_FIELD_MASK << (gpio_id * PORT_OS_SEL_FIELD_OFFSET));
	reg_val |= (user_group_id << ((gpio_id % SUNXI_GPIOS_PER_BANK) * PORT_OS_SEL_FIELD_OFFSET));
	writel(reg_val, port_reg);

	return 0;
}

static int gpio_iso_get_resource_owner(hw_isolator_dev_t *idev, const hw_rsc_info_t *rsc_info, uint32_t *user_group_id)
{
	struct gpio_isolator *gpio_iso;
	void __iomem *gpio_os_sel_reg_base;
	uint32_t gpio_id = 0;
	void __iomem *port_reg = 0;
	uint32_t reg_val = 0;

	if (rsc_info->type != SUNXI_AMP_RSC_HW_GPIO)
		return -1;

	gpio_iso = hw_isolator_dev_get_drvdata(idev);
	gpio_os_sel_reg_base = gpio_iso->base_addr + PORT_OS_SEL_REG_BASE;

	gpio_id = rsc_info->gpio.gpio_id;
	port_reg = gpio_os_sel_reg_base + ((gpio_id / PORT_OS_SEL_REG_WIDTH) * PORT_OS_SEL_REG_OFFSET);
	reg_val = readl(port_reg);
	reg_val = ((reg_val >> (gpio_id % 16)) & 0x3);
	*user_group_id = reg_val;

	return 0;
}

static const hw_isolator_dev_ops_t g_gpio_iso_ops = {
	.enable = gpio_iso_enable,
	.disable = gpio_iso_disable,
	.set_user_group = gpio_iso_set_user_group,
	.get_user_group = gpio_iso_get_user_group,
	.set_resource_owner = gpio_iso_set_resource_owner,
	.get_resource_owner = gpio_iso_get_resource_owner,
};

static int gpio_isolation_probe(hw_isolator_dev_t *idev)
{
	struct gpio_isolator *gpio_iso;
	reg_addr_info_t addr_info;
	uint32_t hw_user_gourp_num;

	gpio_iso = asrm_port_malloc(sizeof(struct gpio_isolator));
	if (!gpio_iso) {
		asrm_err("mem allocation for GPIO isolator failed\n");
		return -1;
	}

	hw_isolator_dev_set_drvdata(idev, gpio_iso);
	hw_isolator_dev_set_ops(idev, &g_gpio_iso_ops);

	hw_isolator_dev_get_reg_addr_info(idev, &addr_info);
	gpio_iso->base_addr = (void __iomem *)addr_info.base_addr;

	hw_user_gourp_num = hw_isolator_dev_get_user_group_num(idev);
	asrm_info("GPIO isolator init! addr: %lx, hw_user_gourp_num: %u",
		addr_info.base_addr, hw_user_gourp_num);
	return 0;
}

static int gpio_isolation_remove(hw_isolator_dev_t *idev)
{
	return 0;
}

static const hw_isolator_dev_info_t g_gpio_iso_match[] = {
	{ .compatible = "allwinner,sun55iw6-pinctrl" },
	{ .compatible = "allwinner,sun8iw22-pinctrl" },
	{ /* sentinel */ }
};

hw_isolator_driver_t g_gpio_iso_drv = {
	.probe = gpio_isolation_probe,
	.remove = gpio_isolation_remove,
	.match_table = g_gpio_iso_match,
};
