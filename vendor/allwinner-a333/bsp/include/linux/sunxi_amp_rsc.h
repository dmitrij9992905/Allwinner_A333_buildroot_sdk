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

#ifndef __AMP_RSC_H__
#define __AMP_RSC_H__

#ifdef __KERNEL__
#include "../../drivers/amp_rsc/core/include/asrm_common.h"
#else
#include "../../hal/source/amp_rsc/core/include/asrm_common.h"
#endif

typedef struct sunxi_peri_rsc_desc {
	unsigned long start_addr;
	uint32_t len;
} sunxi_peri_rsc_desc_t;

typedef struct sunxi_gpio_rsc_desc {
	sunxi_peri_rsc_desc_t peri;

	uint32_t gpio_id;
} sunxi_gpio_rsc_desc_t;

typedef struct sunxi_dma_rsc_desc {
	sunxi_peri_rsc_desc_t peri;

	uint32_t channel_id;
} sunxi_dma_rsc_desc_t;

typedef struct sunxi_amp_rsc_request_info {
	/* The type of user in AMP system, it need to be provided when the type of requested resource
	   is memory and the type of user is not CPU */
	sunxi_amp_rsc_user_type_t user_type;
	/* The ID of user in AMP system, it need to be provided when the type of requested resource
	   is memory and the type of user is not CPU */
	sunxi_amp_rsc_user_id_t user_id;

	/* Software module ID string, it's used to be record which software module has requested this resource */
	const char *sw_module_id_str;

	sunxi_amp_rsc_type_t rsc_type;
	union {
		sunxi_peri_rsc_desc_t peri;
		sunxi_gpio_rsc_desc_t gpio;
		sunxi_dma_rsc_desc_t dma;
	};
} sunxi_amp_rsc_req_info_t;

#ifdef CONFIG_AW_AMP_SYS_RSC_MANAGER

int sunxi_init_amp_sys_rsc_manager(void);

int sunxi_amp_rsc_request(const sunxi_amp_rsc_req_info_t *rsc_req_info, sunxi_amp_rsc_t *resource);

int sunxi_amp_rsc_free(sunxi_amp_rsc_t resource);

int sunxi_amp_rsc_has_permission(sunxi_amp_rsc_t resource);

#ifdef AMP_SYS_RSC_MANAGER_ON_LINUX_KERNEL
#include <linux/platform_device.h>
#include <linux/of.h>

int sunxi_pdev_request_peri_rsc_by_index(struct platform_device *pdev,
	unsigned int pdev_rsc_index, const char *sw_module_id_str, sunxi_amp_rsc_t *amp_resource);

int sunxi_pdev_request_peri_rsc_by_name(struct platform_device *pdev,
	const char *pdev_rsc_name, const char *sw_module_id_str, sunxi_amp_rsc_t *amp_resource);

static inline int sunxi_pdev_request_peri_rsc(struct platform_device *pdev,
	const char *sw_module_id_str, sunxi_amp_rsc_t *amp_resource)
{
	return sunxi_pdev_request_peri_rsc_by_index(pdev, 0, sw_module_id_str, amp_resource);
}

#ifdef CONFIG_AW_ASRM_PROVIDE_PERI_CLK_INFO
int sunxi_of_is_rproc_peri_clk(struct device_node *node, uint32_t clk_id);
#else
static inline int sunxi_of_is_rproc_peri_clk(struct device_node *node, uint32_t clk_id)
{
	return 0;
}
#endif
#endif /* AMP_SYS_RSC_MANAGER_ON_LINUX_KERNEL */

#else

static inline int sunxi_init_amp_sys_rsc_manager(void)
{
	return 0;
}

static inline int sunxi_amp_rsc_request(const sunxi_amp_rsc_req_info_t *rsc_req_info, sunxi_amp_rsc_t *resource)
{
	return 0;
}

static inline int sunxi_amp_rsc_free(sunxi_amp_rsc_t resource)
{
	return 0;
}

static inline int sunxi_amp_rsc_has_permission(sunxi_amp_rsc_t resource)
{
	return 0;
}

#ifdef AMP_SYS_RSC_MANAGER_ON_LINUX_KERNEL
#include <linux/platform_device.h>
#include <linux/of.h>

static inline int sunxi_pdev_request_peri_rsc_by_index(struct platform_device *pdev,
	unsigned int pdev_rsc_index, const char *sw_module_id_str, sunxi_amp_rsc_t *amp_resource)
{
	return 0;
}

static inline int sunxi_pdev_request_peri_rsc_by_name(struct platform_device *pdev,
	const char *pdev_rsc_name, const char *sw_module_id_str, sunxi_amp_rsc_t *amp_resource)
{
	return 0;
}

static inline int sunxi_pdev_request_peri_rsc(struct platform_device *pdev,
	const char *sw_module_id_str, sunxi_amp_rsc_t *amp_resource)
{
	return 0;
}

static inline int sunxi_of_is_rproc_peri_clk(struct device_node *node, uint32_t clk_id)
{
	return 0;
}
#endif /* AMP_SYS_RSC_MANAGER_ON_LINUX_KERNEL */

#endif /* CONFIG_AW_AMP_SYS_RSC_MANAGER */

#endif /* __AMP_RSC_H__ */
