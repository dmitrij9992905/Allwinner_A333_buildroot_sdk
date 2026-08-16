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

#include "shm_helper.h"
#include "mgr_helper.h"
#include "fmt_helper.h"
#include "user_id_helper.h"
#include "gpio_helper.h"
#include "shm_layout.h"

#include "helper_osal.h"
static struct sunxi_osal_ops *os = &g_sunxi_os_ops;

#define shm_print		(os->print)

#define shm_show_struct(_depth, _title, _member_func, _para1) \
	show_struct((_depth), (_title), shm_print, (_member_func), (_para1))

#if 1 /* print error info */
#define shm_assert(_cond, _ret, fmt, ...) \
	do { \
		if (!(_cond)) { \
			asrm_err(fmt, ##__VA_ARGS__); \
			return _ret; \
		} \
	} while (0)
#endif

static void show_hw_dev_info_from_shm(unsigned int depth,
					  struct shm_rsc_controller_dev *shm_dev)
{
	shm_print("%shw_user_group_cnt = %u, addr = 0x%lx(+0x%lx), compatible = %s\n",
		  depth2str(depth),
		  (unsigned int)shm_dev->hw_user_group_cnt,
		  (unsigned long)shm_dev->base_addr, (unsigned long)shm_dev->len,
		  shm_dev->compatible);
}

static void fill_hw_dev_info_to_shm(hw_isolator_t *hw_dev,
					struct shm_rsc_controller_dev *shm_dev)
{
	shm_dev->hw_user_group_cnt = hw_dev->hw_user_group_cnt;
	shm_dev->base_addr = hw_dev->base_addr;
	shm_dev->len = hw_dev->len;
	os->strncpy(&shm_dev->compatible[0], &hw_dev->compatible[0], sizeof(shm_dev->compatible));
}

static hw_isolator_t *create_hw_dev_info_from_shm(struct shm_rsc_controller_dev *shm_dev)
{
	hw_isolator_t *c;

	if (!shm_dev->base_addr && !shm_dev->len && shm_dev->compatible[0] == 0)
		return NULL;

	c = os->zalloc(sizeof(*c));
	shm_assert(c, NULL, "alloc for controller failed!");


	os->strncpy(&c->compatible[0], &shm_dev->compatible[0], sizeof(c->compatible));
	c->base_addr = shm_dev->base_addr;
	c->len = shm_dev->len;
	c->hw_user_group_cnt = shm_dev->hw_user_group_cnt;
	return c;
}

static uint32_t get_users_id_num(amp_rsc_user_group_t *user_group)
{
	return user_group->user_cnt;
}

static struct shm_user_group *next_user_group_entry(struct shm_user_group *ug_entry)
{
	return (struct shm_user_group *)&ug_entry->users_id[ug_entry->user_cnt];
}

static void show_shm_user_group_config(unsigned int depth, struct shm_user_group *ug_entry)
{
	int i;

	shm_print("%sid       = %u\n", depth2str(depth), ug_entry->id);
	shm_print("%suser_cnt = %u\n", depth2str(depth), ug_entry->user_cnt);
	shm_print("%suser_id  = ", depth2str(depth));
	for (i = 0; i < ug_entry->user_cnt; i++)
		shm_print("%u(%s)%s",
			  (unsigned int)ug_entry->users_id[i],
			  userid2str(ug_entry->users_id[i]),
			  (i == (ug_entry->user_cnt - 1)) ? "\n" : ", ");
}

static void fill_user_group_config_to_shm(amp_rsc_user_group_t *user_group,
					  struct shm_user_group *ug_entry)
{
	int i;

	ug_entry->id = user_group->id;
	if (ug_entry->user_cnt != user_group->user_cnt)
		asrm_err("user_cnt error");
	for (i = 0; i < user_group->user_cnt; i++)
		ug_entry->users_id[i] = user_group->user_id[i];
}

static int get_user_group_config_from_shm(amp_rsc_user_group_t *user_group,
					  struct shm_user_group *ug_entry)
{
	int i;

	user_group->id = ug_entry->id;
	user_group->user_cnt = ug_entry->user_cnt;
	for (i = 0; i < ug_entry->user_cnt; i++)
		user_group->user_id[i] = ug_entry->users_id[i];

	return 0;
}

static uint32_t get_peri_rsc_num(struct peri_rsc_manager *peri_manager)
{
	uint32_t i = 0;
	peri_rsc_t *rsc, *tmp;

	list_for_each_entry_safe(rsc, tmp, &peri_manager->rsc_list, node)
		i++;

	return i;
}

static struct shm_peripheral_rsc_controller *
next_peri_entry(struct shm_peripheral_rsc_controller *peri_entry)
{
	return (struct shm_peripheral_rsc_controller *)&peri_entry->rsc[peri_entry->peri_cnt];
}

static void show_shm_peri_rsc_member(unsigned int depth,
				     struct shm_peripheral_rsc_controller *peri_entry)
{
	int i;
	struct shm_peripheral_rsc *rsc_entry;

	for (i = 0; i < peri_entry->peri_cnt; i++) {
		rsc_entry = &peri_entry->rsc[i];
		shm_print("%s{ owner_id = %u, base_addr: %lx (+%lx) }\n",
			  depth2str(depth),
			  rsc_entry->owner_id,
			  (unsigned long)rsc_entry->base_addr,
			  (unsigned long)rsc_entry->len);
	}
}

static void show_shm_peri_config(unsigned int depth,
				 struct shm_peripheral_rsc_controller *peri_entry)
{
	shm_print("%sflags    = 0x%x\n", depth2str(depth), peri_entry->flags);
	if (peri_entry->flags)
		shm_show_struct(depth, "hw_dev", show_hw_dev_info_from_shm, &peri_entry->hw_dev);

	shm_print("%speri_cnt = %u\n", depth2str(depth), peri_entry->peri_cnt);
	shm_show_struct(depth, "peri_rsc", show_shm_peri_rsc_member, peri_entry);
}

static void fill_peri_config_to_shm(struct peri_rsc_manager *peri_manager,
				    struct shm_peripheral_rsc_controller *peri_entry)
{
	int i = 0;
	struct shm_peripheral_rsc *rsc_entry;
	peri_rsc_t *rsc, *tmp;

	list_for_each_entry_safe(rsc, tmp, &peri_manager->rsc_list, node) {
		rsc_entry = &peri_entry->rsc[i++];
		// rsc->amp_rsc.type;
		rsc_entry->owner_id = rsc->amp_rsc.owner_id;
		rsc_entry->base_addr = rsc->start_addr;
		rsc_entry->len = rsc->end_addr - rsc->start_addr + 1;
	}
	if (peri_entry->peri_cnt != i)
		asrm_err("peri_cnt error\n");
	peri_entry->flags = peri_manager->is_hw_isolation;
	if (peri_manager->hw_iso)
		fill_hw_dev_info_to_shm(peri_manager->hw_iso, &peri_entry->hw_dev);
}

static int get_peri_config_from_shm(struct peri_rsc_manager *peri_manager,
				    struct shm_peripheral_rsc_controller *peri_entry)
{
	int i;
	int ret;
	struct shm_peripheral_rsc *rsc_entry;
	struct reg_resource res;

	peri_manager->is_hw_isolation = peri_entry->flags;
	if (peri_manager->is_hw_isolation)
		peri_manager->hw_iso = create_hw_dev_info_from_shm(&peri_entry->hw_dev);

	INIT_LIST_HEAD(&peri_manager->rsc_list);
	for (i = 0; i < peri_entry->peri_cnt; i++) {
		rsc_entry = &peri_entry->rsc[i];
		res.start = rsc_entry->base_addr;
		res.end = rsc_entry->base_addr + rsc_entry->len - 1;
		ret = insert_peri_rsc(peri_manager, rsc_entry->owner_id, &res);
		shm_assert((ret == 0), -1, "insert_peri_rsc failed!");
	}

	return 0;
}

static uint32_t get_gpio_rsc_num(struct gpio_rsc_manager *gpio_manager)
{
	uint32_t i = 0;
	gpio_resource_t *rsc, *tmp;

	list_for_each_entry_safe(rsc, tmp, &gpio_manager->rsc_list, node)
		i++;

	return i;
}

static struct shm_gpio_rsc_controller *next_gpio_entry(struct shm_gpio_rsc_controller *gpio_entry)
{
	return (struct shm_gpio_rsc_controller *)&gpio_entry->rsc[gpio_entry->rsc_cnt];
}

static void show_shm_gpio_rsc_member(unsigned int depth, struct shm_gpio_rsc_controller *gpio_entry)
{
	int i;
	struct shm_gpio_rsc *rsc_entry;
	char pin[8];

	for (i = 0; i < gpio_entry->rsc_cnt; i++) {
		rsc_entry = &gpio_entry->rsc[i];
		gpio_id_to_pin_name(rsc_entry->gpio_id, pin);
		shm_print("%s{ owner_id = %u, gpio_id = 0x%x(%s) }\n", depth2str(depth),
			  (unsigned int)rsc_entry->owner_id,
			  (unsigned int)rsc_entry->gpio_id,
			  pin);
	}
}

static void show_shm_gpio_config(unsigned int depth, struct shm_gpio_rsc_controller *gpio_entry)
{
	shm_print("%sflags    = 0x%x\n", depth2str(depth), gpio_entry->flags);
	if (gpio_entry->flags)
		shm_show_struct(depth, "hw_dev", show_hw_dev_info_from_shm, &gpio_entry->hw_dev);

	shm_print("%srsc_cnt = %u\n", depth2str(depth), gpio_entry->rsc_cnt);
	shm_show_struct(depth, "peri_rsc", show_shm_gpio_rsc_member, gpio_entry);
}

static void fill_gpio_config_to_shm(struct gpio_rsc_manager *gpio_manager,
				    struct shm_gpio_rsc_controller *gpio_entry)
{
	uint32_t i = 0;
	gpio_resource_t *rsc, *tmp;
	struct shm_gpio_rsc *rsc_entry;

	list_for_each_entry_safe(rsc, tmp, &gpio_manager->rsc_list, node) {
		rsc_entry = &gpio_entry->rsc[i++];
		//rsc->amp_rsc.type;
		rsc_entry->owner_id = rsc->amp_rsc.owner_id;
		rsc_entry->gpio_id = rsc->gpio_id;
	}
	if (gpio_entry->rsc_cnt != i)
		asrm_err("gpio rsc_cnt error\n");

	gpio_entry->flags = gpio_manager->is_hw_isolation;
	if (gpio_manager->hw_iso)
		fill_hw_dev_info_to_shm(gpio_manager->hw_iso, &gpio_entry->hw_dev);
}

static int get_gpio_config_from_shm(struct gpio_rsc_manager *gpio_manager,
				    struct shm_gpio_rsc_controller *gpio_entry)
{
	int i;
	int ret;
	struct shm_gpio_rsc *rsc_entry;

	gpio_manager->hw_mem_region_start = (unsigned long)gpio_entry->hw_dev.base_addr;
	gpio_manager->hw_mem_region_len =  gpio_entry->hw_dev.len;

	gpio_manager->is_hw_isolation = gpio_entry->flags;
	if (gpio_manager->is_hw_isolation)
		gpio_manager->hw_iso = create_hw_dev_info_from_shm(&gpio_entry->hw_dev);

	INIT_LIST_HEAD(&gpio_manager->rsc_list);
	for (i = 0; i < gpio_entry->rsc_cnt; i++) {
		rsc_entry = &gpio_entry->rsc[i];
		ret = insert_gpio_rsc(gpio_manager, rsc_entry->owner_id, rsc_entry->gpio_id);
		shm_assert((ret == 0), -1, "insert_gpio_rsc failed!");
	}

	return 0;
}

static uint32_t get_dma_rsc_num(struct dma_channel_rsc_manager *dma_manager)
{
	uint32_t i = 0;
	dma_channel_rsc_t *rsc, *tmp;

	list_for_each_entry_safe(rsc, tmp, &dma_manager->rsc_list, node)
		i++;

	return i;
}

static struct shm_dma_rsc_controller *next_dma_entry(struct shm_dma_rsc_controller *dma_entry)
{
	return (struct shm_dma_rsc_controller *)&dma_entry->rsc[dma_entry->rsc_cnt];
}

static void show_shm_dma_rsc_member(unsigned int depth, struct shm_dma_rsc_controller *dma_entry)
{
	int i;
	struct shm_dma_channel_rsc *rsc_entry;

	for (i = 0; i < dma_entry->rsc_cnt; i++) {
		rsc_entry = &dma_entry->rsc[i];
		shm_print("%s{ owner_id = %u, channel_id = %u }\n", depth2str(depth),
			  (unsigned int)rsc_entry->owner_id,
			  (unsigned int)rsc_entry->channel_id);
	}
}

static void show_shm_dma_config(unsigned int depth, struct shm_dma_rsc_controller *dma_entry)
{
	shm_print("%sflags    = 0x%x\n", depth2str(depth), dma_entry->flags);
	if (dma_entry->flags)
		shm_show_struct(depth, "hw_dev", show_hw_dev_info_from_shm, &dma_entry->hw_dev);

	shm_print("%srsc_cnt = %u\n", depth2str(depth), dma_entry->rsc_cnt);
	shm_show_struct(depth, "peri_rsc", show_shm_dma_rsc_member, dma_entry);
}

static void fill_dma_config_to_shm(struct dma_channel_rsc_manager *dma_manager,
				   struct shm_dma_rsc_controller *dma_entry)
{
	uint32_t i = 0;
	dma_channel_rsc_t *rsc, *tmp;
	struct shm_dma_channel_rsc *rsc_entry;

	list_for_each_entry_safe(rsc, tmp, &dma_manager->rsc_list, node) {
		rsc_entry = &dma_entry->rsc[i++];
		//rsc->amp_rsc.type;
		rsc_entry->owner_id = rsc->amp_rsc.owner_id;
		rsc_entry->channel_id = rsc->channel_id;
	}
	if (dma_entry->rsc_cnt != i)
		asrm_err("dma rsc_cnt error\n");
	dma_entry->flags = dma_manager->is_hw_isolation;
	if (dma_manager->hw_iso)
		fill_hw_dev_info_to_shm(dma_manager->hw_iso, &dma_entry->hw_dev);
}

static int get_dma_config_from_shm(struct dma_channel_rsc_manager *dma_manager,
				   struct shm_dma_rsc_controller *dma_entry)
{
	int i;
	int ret;
	struct shm_dma_channel_rsc *rsc_entry;

	dma_manager->hw_mem_region_start = (unsigned long)dma_entry->hw_dev.base_addr;
	dma_manager->hw_mem_region_len = dma_entry->hw_dev.len;

	dma_manager->is_hw_isolation = dma_entry->flags;
	if (dma_manager->is_hw_isolation)
		dma_manager->hw_iso = create_hw_dev_info_from_shm(&dma_entry->hw_dev);

	INIT_LIST_HEAD(&dma_manager->rsc_list);
	for (i = 0; i < dma_entry->rsc_cnt; i++) {
		rsc_entry = &dma_entry->rsc[i];
		ret = insert_dma_ch_rsc(dma_manager, rsc_entry->owner_id, rsc_entry->channel_id);
		shm_assert((ret == 0), -1, "insert_dma_ch_rsc failed!");
	}

	return 0;
}

int sunxi_amp_rsc_config_to_shm(amp_sys_rsc_manager_t *mgr, void *shm, size_t shm_size)
{
	int i;
	size_t need_size = 0;
	struct rsc_allocation_table_header *header_entry;
	struct shm_user_group *ug_entry, *ug_entry_tmp;
	struct shm_peripheral_rsc_controller *peri_entry, *peri_entry_tmp;
	struct shm_gpio_rsc_controller *gpio_entry, *gpio_entry_tmp;
	struct shm_dma_rsc_controller *dma_entry, *dma_entry_tmp;

	if (!mgr || !shm || shm_size == 0) {
		asrm_err("para error!");
		return -1;
	}

	need_size =  sizeof(*header_entry);

	need_size += sizeof(*ug_entry)  * mgr->user_group_cnt;
	for (i = 0; i < mgr->user_group_cnt; i++)
		need_size += get_users_id_num(&mgr->user_group[i]) * sizeof(ug_entry->users_id[0]);

	need_size += sizeof(*peri_entry) * mgr->peri_manager_cnt;
	for (i = 0; i < mgr->peri_manager_cnt; i++)
		need_size += get_peri_rsc_num(&mgr->peri_manager[i]) * sizeof(peri_entry->rsc[0]);

	need_size += sizeof(*gpio_entry) * mgr->gpio_manager_cnt;
	for (i = 0; i < mgr->gpio_manager_cnt; i++)
		need_size += get_gpio_rsc_num(&mgr->gpio_manager[i]) * sizeof(gpio_entry->rsc[0]);

	need_size += sizeof(*dma_entry)  * mgr->dma_manager_cnt;
	for (i = 0; i < mgr->dma_manager_cnt; i++)
		need_size += get_dma_rsc_num(&mgr->dma_manager[i]) * sizeof(dma_entry->rsc[0]);

	if (need_size == 0) {
		asrm_err("need_size error, something wrong!");
		return -1;
	}

	if (need_size > shm_size) {
		asrm_err("need_size > shm_size, mem truncated!");
		return -1;
	}
	asrm_info("shm_size: %lx, need_size: %lx",
		  (unsigned long)shm_size,
		  (unsigned long)need_size);
	os->memset(shm, 0, need_size);

	header_entry = (typeof(header_entry))shm;

	ug_entry = (typeof(ug_entry))&header_entry[1];
	ug_entry_tmp = ug_entry;
	for (i = 0; i < mgr->user_group_cnt; i++) {
		ug_entry_tmp->user_cnt = get_users_id_num(&mgr->user_group[i]);
		fill_user_group_config_to_shm(&mgr->user_group[i], ug_entry_tmp);
		ug_entry_tmp = next_user_group_entry(ug_entry_tmp);
	}

	peri_entry = (typeof(peri_entry))ug_entry_tmp;
	peri_entry_tmp = peri_entry;
	for (i = 0; i < mgr->peri_manager_cnt; i++) {
		peri_entry_tmp->peri_cnt = get_peri_rsc_num(&mgr->peri_manager[i]);
		fill_peri_config_to_shm(&mgr->peri_manager[i], peri_entry_tmp);
		peri_entry_tmp = next_peri_entry(peri_entry_tmp);
	}

	gpio_entry = (typeof(gpio_entry))peri_entry_tmp;
	gpio_entry_tmp = gpio_entry;
	for (i = 0; i < mgr->gpio_manager_cnt; i++) {
		gpio_entry_tmp->rsc_cnt = get_gpio_rsc_num(&mgr->gpio_manager[i]);
		fill_gpio_config_to_shm(&mgr->gpio_manager[i], gpio_entry_tmp);
		gpio_entry_tmp = next_gpio_entry(gpio_entry_tmp);
	}

	dma_entry = (typeof(dma_entry))gpio_entry_tmp;
	dma_entry_tmp = dma_entry;
	for (i = 0; i < mgr->dma_manager_cnt; i++) {
		dma_entry_tmp->rsc_cnt = get_dma_rsc_num(&mgr->dma_manager[i]);
		fill_dma_config_to_shm(&mgr->dma_manager[i], dma_entry_tmp);
		dma_entry_tmp = next_dma_entry(dma_entry_tmp);
	}

	header_entry->user_group_offset = (void *)ug_entry - shm;
	header_entry->user_group_cnt = mgr->user_group_cnt;

	header_entry->peri_rsc_controller_offset = (void *)peri_entry - shm;
	header_entry->peri_rsc_controller_cnt = mgr->peri_manager_cnt;

	header_entry->gpio_rsc_controller_offset = (void *)gpio_entry - shm;
	header_entry->gpio_rsc_controller_cnt = mgr->gpio_manager_cnt;

	header_entry->dma_rsc_controller_offset = (void *)dma_entry - shm;
	header_entry->dma_rsc_controller_cnt = mgr->dma_manager_cnt;

	os->memcpy(&header_entry->magic, "ARSC", sizeof(header_entry->magic));
	header_entry->version = 1;
	header_entry->table_size = need_size;

	return 0;
}

void sunxi_amp_rsc_show_config_in_shm(unsigned int depth, const void *shm, size_t shm_size)
{
	int i;

	struct rsc_allocation_table_header *header_entry;
	struct shm_user_group *ug_entry, *ug_entry_tmp;
	struct shm_peripheral_rsc_controller *peri_entry, *peri_entry_tmp;
	struct shm_gpio_rsc_controller *gpio_entry, *gpio_entry_tmp;
	struct shm_dma_rsc_controller *dma_entry, *dma_entry_tmp;

	header_entry = (typeof(header_entry))shm;

	ug_entry = (typeof(ug_entry))(shm + header_entry->user_group_offset);
	ug_entry_tmp = ug_entry;
	for (i = 0; i < header_entry->user_group_cnt; i++) {
		shm_show_struct(depth, "shm_user_group", show_shm_user_group_config, ug_entry_tmp);
		ug_entry_tmp = next_user_group_entry(ug_entry_tmp);
	}

	peri_entry = (typeof(peri_entry))(shm + header_entry->peri_rsc_controller_offset);
	peri_entry_tmp = peri_entry;
	for (i = 0; i < header_entry->peri_rsc_controller_cnt; i++) {
		shm_show_struct(depth, "shm_peri", show_shm_peri_config, peri_entry_tmp);
		peri_entry_tmp = next_peri_entry(peri_entry_tmp);
	}

	gpio_entry = (typeof(gpio_entry))(shm + header_entry->gpio_rsc_controller_offset);
	gpio_entry_tmp = gpio_entry;
	for (i = 0; i < header_entry->gpio_rsc_controller_cnt; i++) {
		shm_show_struct(depth, "shm_gpio", show_shm_gpio_config, gpio_entry_tmp);
		gpio_entry_tmp = next_gpio_entry(gpio_entry_tmp);
	}

	dma_entry = (typeof(dma_entry))(shm + header_entry->dma_rsc_controller_offset);
	dma_entry_tmp = dma_entry;
	for (i = 0; i < header_entry->dma_rsc_controller_cnt; i++) {
		shm_show_struct(depth, "shm_dma", show_shm_dma_config, dma_entry_tmp);
		dma_entry_tmp = next_dma_entry(dma_entry_tmp);
	}
}

int sunxi_amp_rsc_get_config_from_shm(amp_sys_rsc_manager_t *mgr,
				      const void *shm, size_t shm_size)
{
	int i;
	int ret;
	int len;
	uint32_t magic;
	void *mem;
	struct rsc_allocation_table_header *header_entry;
	struct shm_user_group *ug_entry, *ug_entry_tmp;
	struct shm_peripheral_rsc_controller *peri_entry, *peri_entry_tmp;
	struct shm_gpio_rsc_controller *gpio_entry, *gpio_entry_tmp;
	struct shm_dma_rsc_controller *dma_entry, *dma_entry_tmp;

	header_entry = (typeof(header_entry))shm;

	os->memcpy(&magic, "ARSC", sizeof(magic));
	if (header_entry->magic != magic) {
		asrm_err("magic error: %x %x\n", header_entry->magic, magic);
		return -1;
	}

	if (header_entry->version != 1) {
		asrm_err("version error: %x\n", header_entry->version);
		return -1;
	}

	mgr->user_cnt = 0;
	mgr->user_group_cnt = 0;
	mgr->peri_manager_cnt = 0;
	mgr->gpio_manager_cnt = 0;
	mgr->dma_manager_cnt = 0;

	ug_entry = (typeof(ug_entry))(shm + header_entry->user_group_offset);
	peri_entry = (typeof(peri_entry))(shm + header_entry->peri_rsc_controller_offset);
	gpio_entry = (typeof(gpio_entry))(shm + header_entry->gpio_rsc_controller_offset);
	dma_entry = (typeof(dma_entry))(shm + header_entry->dma_rsc_controller_offset);

	mgr->user_group_cnt   = header_entry->user_group_cnt;
	mgr->peri_manager_cnt = header_entry->peri_rsc_controller_cnt;
	mgr->gpio_manager_cnt = header_entry->gpio_rsc_controller_cnt;
	mgr->dma_manager_cnt  = header_entry->dma_rsc_controller_cnt;

	asrm_info("user_cnt:         %u", mgr->user_cnt);
	asrm_info("user_group_cnt:   %u", mgr->user_group_cnt);
	asrm_info("peri_manager_cnt: %u", mgr->peri_manager_cnt);
	asrm_info("gpio_manager_cnt: %u", mgr->gpio_manager_cnt);
	asrm_info("dma_manager_cnt:  %u", mgr->dma_manager_cnt);

	len =  sizeof(mgr->user[0])         * mgr->user_cnt;
	len += sizeof(mgr->user_group[0])  * mgr->user_group_cnt;
	len += sizeof(mgr->peri_manager[0]) * mgr->peri_manager_cnt;
	len += sizeof(mgr->gpio_manager[0]) * mgr->gpio_manager_cnt;
	len += sizeof(mgr->dma_manager[0])  * mgr->dma_manager_cnt;
	shm_assert((len > 0), -1, "len error, something is wrong!");

	mem = os->zalloc(len);
	shm_assert(mem, -1, "alloc for ASRM failed!");

	mgr->user         = mem;
	mgr->user_group  = (typeof(mgr->user_group))&mgr->user[mgr->user_cnt];
	mgr->peri_manager = (typeof(mgr->peri_manager))&mgr->user_group[mgr->user_group_cnt];
	mgr->gpio_manager = (typeof(mgr->gpio_manager))&mgr->peri_manager[mgr->peri_manager_cnt];
	mgr->dma_manager  = (typeof(mgr->dma_manager))&mgr->gpio_manager[mgr->gpio_manager_cnt];

	ug_entry_tmp = ug_entry;
	for (i = 0; i < header_entry->user_group_cnt; i++) {
		ret = get_user_group_config_from_shm(&mgr->user_group[i], ug_entry_tmp);
		shm_assert((ret == 0), -1, "get_user_group_config_from_shm failed!");
		ug_entry_tmp = next_user_group_entry(ug_entry_tmp);
	}

	peri_entry_tmp = peri_entry;
	for (i = 0; i < header_entry->peri_rsc_controller_cnt; i++) {
		ret = get_peri_config_from_shm(&mgr->peri_manager[i], peri_entry_tmp);
		shm_assert((ret == 0), -1, "get_peri_config_from_shm failed!");
		peri_entry_tmp = next_peri_entry(peri_entry_tmp);
	}

	gpio_entry_tmp = gpio_entry;
	for (i = 0; i < header_entry->gpio_rsc_controller_cnt; i++) {
		ret = get_gpio_config_from_shm(&mgr->gpio_manager[i], gpio_entry_tmp);
		shm_assert((ret == 0), -1, "get_gpio_config_from_shm failed!");
		gpio_entry_tmp = next_gpio_entry(gpio_entry_tmp);
	}

	dma_entry_tmp = dma_entry;
	for (i = 0; i < header_entry->dma_rsc_controller_cnt; i++) {
		ret = get_dma_config_from_shm(&mgr->dma_manager[i], dma_entry_tmp);
		shm_assert((ret == 0), -1, "get_dma_config_from_shm failed!");
		dma_entry_tmp = next_dma_entry(dma_entry_tmp);
	}

	return 0;
}
