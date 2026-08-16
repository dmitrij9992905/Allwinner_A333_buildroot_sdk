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

#include <fdt_support.h>
#include <fdtdec.h>

#undef strtoul
#include "helper_osal.h"
#include "fdt_helper.h"
#include "mgr_helper.h"
#include "fmt_helper.h"
#include "user_id_helper.h"
#include "gpio_helper.h"

static struct sunxi_osal_ops *os = &g_sunxi_os_ops;

static const char *prop_of_shm_info     = "asrm_shm";
static const char *g_asrm_node_path       = "/amp_system_resource_manager";
static const char *g_asrm_node_alias      = "asrm";
static const char *prop_of_user         = "user";
static const char *prop_of_user_group   = "user_group";
static const char *prop_of_peri_manager = "peripheral_resource_manager";
static const char *prop_of_gpio_manager = "gpio_resource_manager";
static const char *prop_of_dma_manager  = "dma_resource_manager";

static const void *used_fdt;

#if 1 /* print error info */
#define fdt_assert(_cond, _ret, fmt, ...) \
	do { \
		if (!(_cond)) { \
			asrm_err(fmt, ##__VA_ARGS__); \
			return _ret; \
		} \
	} while (0)
#endif

// xxxx;
static inline int fdt_getprop_bool(const void *fdt, int off, const char *prop)
{
	return fdt_getprop(fdt, off, prop, NULL) ? 1 : 0;
}

// xxx = <N>;
static inline int fdt_getprop_u32(const void *fdt, int off, const char *prop, uint32_t *pval)
{
	int len;
	const fdt32_t *val = fdt_getprop(fdt, off, prop, &len);

	if (val == NULL || len != sizeof(*val))
		return -1;

	if (pval)
		*pval = fdt32_to_cpu(*val);
	return 0;
}

// prefix@N { ... };
static inline int fdt_node_is_match(const void *fdt, int off, const char *prefix, u32 *id)
{
	const char *name = fdt_get_name(fdt, off, NULL);

	if (!name || strncmp(name, prefix, os->strlen(prefix)))
		return 0;
	if (id)
		*id = os->strtoul(name + os->strlen(prefix) + 1, NULL, 0);
	return 1;
}

static void fdt_node_debug(const void *fdt, int off, const char *title)
{
#ifdef ASRM_CORE_DEBUG
	int len;
	const char *str = fdt_get_name(fdt, off, &len);

	asrm_dbg("%s: into %s(%d)", title ? title : "", str ? str : "", len);
#endif
}

static hw_isolator_t *get_hw_isolator_info(int off, int is_hw_isolation)
{
	int len;
	int ret;
	const char *str;
	fdt32_t *phandle;
	hw_isolator_t *c;
	struct fdt_resource res;
	uint32_t hw_user_group_cnt = 0;

	if (is_hw_isolation) {
		ret = fdt_getprop_u32(used_fdt, off, "hw_user_group_num", &hw_user_group_cnt);
		fdt_assert((ret >= 0), NULL, "get hw_user_group_num failed!");
	}

	phandle = (fdt32_t *)fdt_getprop(used_fdt, off, "controller", NULL);
	if (!phandle)
		return NULL;

	off = fdt_node_offset_by_phandle(used_fdt, fdt32_to_cpu(*phandle));
	fdt_assert((off >= 0), NULL, "can not found controller node");

	str = (const char *)fdt_getprop(used_fdt, off, "compatible", &len);
	fdt_assert(str, NULL, "can not found controller compatible");

	if (len > sizeof(c->compatible))
		asrm_warn("controller compatible truncated");

	ret = fdt_get_resource(used_fdt, off, "reg", 0, &res);
	fdt_assert((ret == 0), NULL, "controller can not found reg");

	c = os->zalloc(sizeof(*c));
	fdt_assert(c, NULL, "alloc for controller failed!");

	os->strncpy(&c->compatible[0], str, sizeof(c->compatible));
	c->base_addr = res.start;
	c->len = res.end - res.start + 1;
	c->hw_user_group_cnt = hw_user_group_cnt;

	if (!fdt_get_resource(used_fdt, off, "reg", 1, &res))
		asrm_warn("controller has mutil regs");

	return c;
}

static int get_user_group_id(const char *prop, int node, const char *prefix)
{
	fdt32_t *phandle;
	int off;
	int ret;
	const char *str;

	phandle = (fdt32_t *)fdt_getprop(used_fdt, node, prop, NULL);
	fdt_assert(phandle, -1, "can not found item %s", prop);

	off = fdt_node_offset_by_phandle(used_fdt, fdt32_to_cpu(*phandle));
	fdt_assert((off >= 0), -1, "can not found node %s", prop);

	str = fdt_get_name(used_fdt, off, NULL);
	fdt_assert(str, -1, "get %s's name failed!", prop);

	ret = strncmp(str, prefix, os->strlen(prefix));
	fdt_assert((ret == 0), -1, "%s's name not match %s", prop, prefix);

	return os->strtoul(str + os->strlen(prefix) + 1, NULL, 0);
}

static int get_user_group_config(amp_rsc_user_group_t *user_group, int subnode)
{
	const char *str;
	int len;
	int i;
	const fdt32_t *user_id;
	u32 id;

	if (!fdt_node_is_match(used_fdt, subnode, prop_of_user_group, &id))
		return -1;
	if (!user_group)
		return 0; // not need to parser info
	user_group->id = id;

	str = "users";
	user_id = (const fdt32_t *)fdt_getprop(used_fdt, subnode, str, &len);
	fdt_assert(user_id, -1, "can not find key: %s", str);

	user_group->user_cnt = len / sizeof(user_id[0]);

	for (i = 0; i < user_group->user_cnt; i++)
		user_group->user_id[i] = fdt32_to_cpu(user_id[i]);

	return 0;
}

static int get_user_config(amp_rsc_user_t *user, int subnode)
{
	if (!fdt_node_is_match(used_fdt, subnode, prop_of_user, NULL))
		return -1;
	if (!user)
		return 0; // not need to parser info

	// TODO
	return 0;
}

static int get_devices_cnt(const char *prop, int node)
{
	int len;
	const void *item = fdt_getprop(used_fdt, node, prop, &len);
	fdt_assert(item, -1, "can not found item %s", prop);

	return len / sizeof(fdt32_t);
}

static int get_device_reg(const char *prop, int idx, int node, struct fdt_resource *res)
{
	fdt32_t *phandle;
	int off;
	const char *str;
	int len;
	int ret;

	phandle = (fdt32_t *)fdt_getprop(used_fdt, node, prop, &len);
	fdt_assert(phandle, -1, "can not found item %s", prop);
	fdt_assert((len / sizeof(*phandle) > idx), -1, "error idx%d for item %s", idx, prop);

	off = fdt_node_offset_by_phandle(used_fdt, fdt32_to_cpu(phandle[idx]));
	fdt_assert((off >= 0), -1, "can not found node %s", prop);

	fdt_node_debug(used_fdt, off, __func__);

	str = "reg";
	ret = fdt_get_resource(used_fdt, off, str, 0, res);
	fdt_assert((ret == 0), -1, "device can not found %s", str);

	return 0;
}

static int insert_each_peri_rsc(peri_rsc_manager_t *peri_manager, int node)
{
	int i, cnt;
	int owner_id;
	struct fdt_resource res;
	struct reg_resource reg_res;
	int ret;

	fdt_node_debug(used_fdt, node, __func__);

	owner_id = get_user_group_id("owner", node, prop_of_user_group);
	fdt_assert((owner_id >= 0), -1, "get owner_id failed!");

	cnt = get_devices_cnt("devices", node);
	fdt_assert((cnt > 0), -1, "get_devices_cnt failed!");

	for (i = 0; i < cnt ; i++) {
		ret = get_device_reg("devices", i, node, &res);
		fdt_assert((ret == 0), -1, "get_device_reg failed!");

		reg_res.start = res.start;
		reg_res.end = res.end;
		ret = insert_peri_rsc(peri_manager, owner_id, &reg_res);
		fdt_assert((ret == 0), -1, "insert_peri_rsc failed!");
	}

	return 0;
}

static int get_peri_res_config(struct peri_rsc_manager *peri_manager, int node)
{
	int subnode;
	int ret;

	if (!fdt_node_is_match(used_fdt, node, prop_of_peri_manager, NULL))
		return -1;
	if (!peri_manager)
		return 0; // not need to parser info

	peri_manager->is_hw_isolation = fdt_getprop_bool(used_fdt, node, "hw_isolation");
	if (peri_manager->is_hw_isolation)
		peri_manager->hw_iso = get_hw_isolator_info(node, 1);

	INIT_LIST_HEAD(&peri_manager->rsc_list);
	fdt_for_each_subnode(subnode, used_fdt, node) {
		ret = insert_each_peri_rsc(peri_manager, subnode);
		fdt_assert((ret == 0), -1, "insert_each_peri_rsc failed!");
	}

	return 0;
}

static int insert_each_gpio_rsc(struct gpio_rsc_manager *gpio_manager, int node)
{
	int ret, len, owner_id, i, cnt;
	const char *key;
	const char *pin;

	fdt_node_debug(used_fdt, node, __func__);

	owner_id = get_user_group_id("owner", node, prop_of_user_group);
	fdt_assert((owner_id >= 0), -1, "get owner_id failed!");

	key = "pins";
	cnt = fdt_stringlist_count(used_fdt, node, key);
	fdt_assert((cnt > 0), -1, "can not get item: %s", key);

	for (i = 0; i < cnt; i++) {
		pin = fdt_stringlist_get(used_fdt, node, key, i, &len);
		fdt_assert(pin, -1, "can not get item: %s idx %d", key, i);

		ret = pin_name_check(pin);
		fdt_assert((ret == 0), -1, "pin_name_check(%s) failed", pin);

		ret = insert_gpio_rsc(gpio_manager, owner_id, pin_name_to_gpio_id(pin));
		fdt_assert((ret == 0), -1, "insert_gpio_rsc failed");
	}

	return 0;
}

static int get_gpio_res_config(struct gpio_rsc_manager *gpio_manager, int node)
{
	int subnode;
	int ret;

	if (!fdt_node_is_match(used_fdt, node, prop_of_gpio_manager, NULL))
		return -1;
	if (!gpio_manager)
		return 0; // not need to parser info

	gpio_manager->is_hw_isolation = fdt_getprop_bool(used_fdt, node, "hw_isolation");
	gpio_manager->hw_iso = get_hw_isolator_info(node, gpio_manager->is_hw_isolation);

	INIT_LIST_HEAD(&gpio_manager->rsc_list);
	fdt_for_each_subnode(subnode, used_fdt, node) {
		ret = insert_each_gpio_rsc(gpio_manager, subnode);
		fdt_assert((ret == 0), -1, "insert_each_gpio_rsc failed");
	}

	return 0;
}

static int insert_each_dma_channel_rsc(struct dma_channel_rsc_manager *dma_manager, int node)
{
	int ret, len, owner_id, i, cnt;
	const char *key;
	const fdt32_t *channels;

	fdt_node_debug(used_fdt, node, __func__);

	owner_id = get_user_group_id("owner", node, prop_of_user_group);
	fdt_assert((owner_id >= 0), -1, "get owner_id failed!");

	key = "channels";
	channels = (const fdt32_t *)fdt_getprop(used_fdt, node, key, &len);
	fdt_assert(channels, -1, "can not find key: %s", key);

	cnt = len / sizeof(channels[0]);
	for (i = 0; i < cnt; i++) {
		ret = insert_dma_ch_rsc(dma_manager, owner_id, fdt32_to_cpu(channels[i]));
		fdt_assert((ret == 0), -1, "insert_dma_ch_rsc failed");
	}

	return 0;
}

static int get_dma_res_config(struct dma_channel_rsc_manager *dma_manager, int node)
{
	int subnode;
	int ret;

	if (!fdt_node_is_match(used_fdt, node, prop_of_dma_manager, NULL))
		return -1;
	if (!dma_manager)
		return 0; // not need to parser info

	dma_manager->is_hw_isolation = fdt_getprop_bool(used_fdt, node, "hw_isolation");
	dma_manager->hw_iso = get_hw_isolator_info(node, dma_manager->is_hw_isolation);

	INIT_LIST_HEAD(&dma_manager->rsc_list);
	fdt_for_each_subnode(subnode, used_fdt, node) {
		ret = insert_each_dma_channel_rsc(dma_manager, subnode);
		fdt_assert((ret == 0), -1, "insert_each_dma_channel_rsc failed");
	}

	return 0;
}

int sunxi_amp_rsc_get_config_from_dts(amp_sys_rsc_manager_t *mgr)
{
	int len, node, subnode;
	const char *str = NULL;
	void *mem = NULL;
	const char *node_path;

	used_fdt = (void *)working_fdt;
	node_path = g_asrm_node_path;
	node = fdt_path_offset(used_fdt, g_asrm_node_path);
	if (node < 0) {
		node_path = g_asrm_node_alias;
		node = fdt_path_offset(used_fdt, node_path);
	}
	fdt_assert((node >= 0), -1, "can not find fdt path: %s", node_path);

	str = (const char *)fdt_getprop(used_fdt, node, "status", &len);
	fdt_assert(str, -1, "can not find node status");

	if (strcmp(str, "okay")) {
		asrm_info("AMP System Resource Manager is disabled!");
		return -1;
	}

	mgr->user_cnt = 0;
	mgr->user_group_cnt = 0;
	mgr->peri_manager_cnt = 0;
	mgr->gpio_manager_cnt = 0;
	mgr->dma_manager_cnt = 0;
	fdt_for_each_subnode(subnode, used_fdt, node) {
		fdt_node_debug(used_fdt, subnode, __func__);

		if (!get_user_group_config(NULL, subnode)) {
			mgr->user_group_cnt++;
			continue;
		}
		if (!get_user_config(NULL, subnode)) {
			mgr->user_cnt++;
			continue;
		}
		if (!get_peri_res_config(NULL, subnode)) {
			mgr->peri_manager_cnt++;
			continue;
		}
		if (!get_gpio_res_config(NULL, subnode)) {
			mgr->gpio_manager_cnt++;
			continue;
		}
		if (!get_dma_res_config(NULL, subnode)) {
			mgr->dma_manager_cnt++;
			continue;
		}
	}
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
	fdt_assert((len > 0), -1, "len error, something is wrong!");

	mem = os->zalloc(len);
	fdt_assert(mem, -1, "alloc for ASRM failed!");

	mgr->user         = mem;
	mgr->user_group  = (typeof(mgr->user_group))&mgr->user[mgr->user_cnt];
	mgr->peri_manager = (typeof(mgr->peri_manager))&mgr->user_group[mgr->user_group_cnt];
	mgr->gpio_manager = (typeof(mgr->gpio_manager))&mgr->peri_manager[mgr->peri_manager_cnt];
	mgr->dma_manager  = (typeof(mgr->dma_manager))&mgr->gpio_manager[mgr->gpio_manager_cnt];

	mgr->user_cnt = 0;
	mgr->user_group_cnt = 0;
	mgr->peri_manager_cnt = 0;
	mgr->gpio_manager_cnt = 0;
	mgr->dma_manager_cnt = 0;
	fdt_for_each_subnode(subnode, used_fdt, node) {
		fdt_node_debug(used_fdt, subnode, __func__);

		if (!get_user_group_config(&mgr->user_group[mgr->user_group_cnt], subnode)) {
			mgr->user_group_cnt++;
			continue;
		}
		if (!get_user_config(&mgr->user[mgr->user_cnt], subnode)) {
			mgr->user_cnt++;
			continue;
		}
		if (!get_peri_res_config(&mgr->peri_manager[mgr->peri_manager_cnt], subnode)) {
			mgr->peri_manager_cnt++;
			continue;
		}
		if (!get_gpio_res_config(&mgr->gpio_manager[mgr->gpio_manager_cnt], subnode)) {
			mgr->gpio_manager_cnt++;
			continue;
		}
		if (!get_dma_res_config(&mgr->dma_manager[mgr->dma_manager_cnt], subnode)) {
			mgr->dma_manager_cnt++;
			continue;
		}
	}

	return 0;
}

int sunxi_amp_rsc_get_shm_info(unsigned long *paddr, unsigned long *plen)
{
	int ret, node, subnode;
	const char *str;
	struct fdt_resource res;

	used_fdt = (void *)working_fdt;
	str = "/reserved-memory";
	node = fdt_path_offset(used_fdt, str);
	fdt_assert((node >= 0), -1, "can not find fdt path: %s", str);

	fdt_for_each_subnode(subnode, used_fdt, node) {
		fdt_node_debug(used_fdt, subnode, __func__);

		str = fdt_get_name(used_fdt, subnode, NULL);
		if (!str || strncmp(str, prop_of_shm_info, os->strlen(prop_of_shm_info)))
			continue;

		str = "reg";
		ret = fdt_get_resource(used_fdt, subnode, str, 0, &res);
		fdt_assert((ret == 0), -1, "can not get %s", str);

		if (paddr)
			*paddr = res.start;
		if (plen)
			*plen = res.end - res.start + 1;
		return 0;
	}

	return -1;
}
