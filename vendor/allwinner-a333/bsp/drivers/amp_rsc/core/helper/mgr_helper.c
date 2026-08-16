// SPDX-License-Identifier: GPL-2.0-only
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

#include "../asrm_core.h"

#include "mgr_helper.h"
#include "fmt_helper.h"
#include "user_id_helper.h"
#include "gpio_helper.h"

#include "helper_osal.h"
static struct sunxi_osal_ops *os = &g_sunxi_os_ops;

#define mgr_print		(os->print)

#if 1 /* print error info */
#define mgr_assert(_cond, _ret, fmt, ...) \
	do { \
		if (!(_cond)) { \
			asrm_err(fmt, ##__VA_ARGS__); \
			return _ret; \
		} \
	} while (0)
#endif

#define mgr_show_struct(_depth, _title, _member_func, _para1) \
	show_struct((_depth), (_title), mgr_print, (_member_func), (_para1))

int insert_peri_rsc(peripheral_resource_manager_t *peri_manager,
		    amp_rsc_owner_id_t owner_id, struct reg_resource *res)
{
	peri_rsc_t *peri_res = os->zalloc(sizeof(*peri_res));

	mgr_assert(peri_res, -1, "alloc for peri_res failed");

	peri_res->amp_rsc.type = SUNXI_AMP_RSC_HW_PERI;
	peri_res->amp_rsc.owner_id = owner_id;
	peri_res->start_addr = res->start;
	peri_res->end_addr = res->end;
	INIT_LIST_HEAD(&peri_res->node);

	list_add_tail(&peri_res->node, &peri_manager->rsc_list);
	return 0;
}

int insert_gpio_rsc(struct gpio_rsc_manager *gpio_manager,
		    amp_rsc_owner_id_t owner_id, uint32_t gpio_id)
{
	gpio_resource_t *gpio_res = os->zalloc(sizeof(*gpio_res));

	mgr_assert(gpio_res, -1, "alloc for gpio_res failed");

	gpio_res->amp_rsc.type = SUNXI_AMP_RSC_HW_GPIO;
	gpio_res->amp_rsc.owner_id = owner_id;
	gpio_res->gpio_id = gpio_id;
	gpio_res->manager = gpio_manager;
	INIT_LIST_HEAD(&gpio_res->node);

	list_add_tail(&gpio_res->node, &gpio_manager->rsc_list);
	return 0;
}

int insert_dma_ch_rsc(struct dma_channel_rsc_manager *dma_manager,
		      amp_rsc_owner_id_t owner_id, uint32_t channel_id)
{
	dma_channel_rsc_t *dma_res = os->zalloc(sizeof(*dma_res));

	mgr_assert(dma_res, -1, "alloc for dma_res failed");

	dma_res->amp_rsc.type = SUNXI_AMP_RSC_HW_DMA_CHANNEL;
	dma_res->amp_rsc.owner_id = owner_id;
	dma_res->channel_id = channel_id;
	dma_res->manager = dma_manager;
	INIT_LIST_HEAD(&dma_res->node);

	list_add_tail(&dma_res->node, &dma_manager->rsc_list);
	return 0;
}

static void show_hw_isolator_info(unsigned int depth, hw_isolator_t *hw_iso)
{
	mgr_print("%shw_user_group_cnt = %u, addr = 0x%lx(+0x%lx), compatible = %s\n",
		  depth2str(depth),
		  (unsigned int)hw_iso->hw_user_group_cnt,
		  (unsigned long)hw_iso->base_addr,
		  (unsigned long)hw_iso->len, hw_iso->compatible);
}

static void show_user_group_info(unsigned int depth, amp_rsc_user_group_t *user_group)
{
	int i;

	mgr_print("%sid       = %u\n", depth2str(depth), (unsigned int)user_group->id);
	mgr_print("%suser_cnt = %u\n", depth2str(depth), (unsigned int)user_group->user_cnt);
	mgr_print("%suser_id  = ", depth2str(depth));
	for (i = 0; i < user_group->user_cnt; i++)
		mgr_print("%u(%s)%s",
			  (unsigned int)user_group->user_id[i],
			  userid2str(user_group->user_id[i]),
			  (i == (user_group->user_cnt - 1)) ? "\n" : ", ");
}

static void show_user_info(unsigned int depth, amp_rsc_user_t *user)
{
	// TODO
}

static void show_peri_res_list(unsigned int depth, struct list_head *list)
{
	peri_rsc_t *res, *tmp;

	list_for_each_entry_safe(res, tmp, list, node) {
		mgr_print("%s{ type = %u, owner_id = %u, start_addr = 0x%lx, end_addr = 0x%lx }\n",
			  depth2str(depth),
			  (unsigned int)res->amp_rsc.type,
			  (unsigned int)res->amp_rsc.owner_id,
			  (unsigned long)res->start_addr,
			  (unsigned long)res->end_addr);
	}
}

static void show_peri_res_info(unsigned int depth, struct peri_rsc_manager *peri_manager)
{
	mgr_print("%sis_hw_isolation = %u\n", depth2str(depth), peri_manager->is_hw_isolation);

	if (peri_manager->hw_iso)
		mgr_show_struct(depth, "hw_isolator", show_hw_isolator_info, peri_manager->hw_iso);

	if (!list_empty(&peri_manager->rsc_list))
		mgr_show_struct(depth, "peri_rsc", show_peri_res_list, &peri_manager->rsc_list);
}

static void show_gpio_rsc_list(unsigned int depth, struct list_head *list)
{
	gpio_resource_t *res, *tmp;
	char pin[8];

	list_for_each_entry_safe(res, tmp, list, node) {
		gpio_id_to_pin_name(res->gpio_id, pin);
		mgr_print("%s{ type = %u, owner_id = %u, gpio_id = 0x%x(%s) }\n",
			  depth2str(depth),
			  (unsigned int)res->amp_rsc.type,
			  (unsigned int)res->amp_rsc.owner_id,
			  (unsigned int)res->gpio_id,
			  pin);
	}
}

static void show_gpio_res_info(unsigned int depth, struct gpio_rsc_manager *gpio_manager)
{
	mgr_print("%shw_mem_region_start = 0x%lx\n", depth2str(depth), gpio_manager->hw_mem_region_start);
	mgr_print("%shw_mem_region_len = 0x%x\n", depth2str(depth), gpio_manager->hw_mem_region_len);

	mgr_print("%sis_hw_isolation = %u\n", depth2str(depth), gpio_manager->is_hw_isolation);

	if (gpio_manager->hw_iso)
		mgr_show_struct(depth, "hw_isolator", show_hw_isolator_info, gpio_manager->hw_iso);

	if (!list_empty(&gpio_manager->rsc_list))
		mgr_show_struct(depth, "gpio_rsc", show_gpio_rsc_list, &gpio_manager->rsc_list);
}

static void show_dma_rsc_list(unsigned int depth, struct list_head *list)
{
	dma_channel_rsc_t *res, *tmp;

	list_for_each_entry_safe(res, tmp, list, node) {
		mgr_print("%s{ type = %u, owner_id = %u, channel_id = %u }\n",
			  depth2str(depth),
			  (unsigned int)res->amp_rsc.type,
			  (unsigned int)res->amp_rsc.owner_id,
			  (unsigned int)res->channel_id);
	}
}

static void show_dma_res_info(unsigned int depth, struct dma_channel_rsc_manager *dma_manager)
{
	mgr_print("%shw_mem_region_start = 0x%lx\n", depth2str(depth), dma_manager->hw_mem_region_start);
	mgr_print("%shw_mem_region_len = 0x%x\n", depth2str(depth), dma_manager->hw_mem_region_len);

	mgr_print("%sis_hw_isolation = %u\n", depth2str(depth), dma_manager->is_hw_isolation);

	if (dma_manager->hw_iso)
		mgr_show_struct(depth, "hw_isolator", show_hw_isolator_info, dma_manager->hw_iso);

	if (!list_empty(&dma_manager->rsc_list))
		mgr_show_struct(depth, "dma_channel_rsc", show_dma_rsc_list,
				&dma_manager->rsc_list);
}

void sunxi_amp_rsc_show_config(unsigned int depth, amp_sys_rsc_manager_t *mgr)
{
	int i;

	for (i = 0; i < mgr->user_cnt; i++)
		mgr_show_struct(depth, "user", show_user_info, &mgr->user[i]);
	for (i = 0; i < mgr->user_group_cnt; i++)
		mgr_show_struct(depth, "user_group", show_user_group_info, &mgr->user_group[i]);
	for (i = 0; i < mgr->peri_manager_cnt; i++)
		mgr_show_struct(depth, "peri_manager", show_peri_res_info, &mgr->peri_manager[i]);
	for (i = 0; i < mgr->gpio_manager_cnt; i++)
		mgr_show_struct(depth, "gpio_manager", show_gpio_res_info, &mgr->gpio_manager[i]);
	for (i = 0; i < mgr->dma_manager_cnt; i++)
		mgr_show_struct(depth, "dma_manager", show_dma_res_info, &mgr->dma_manager[i]);
}
