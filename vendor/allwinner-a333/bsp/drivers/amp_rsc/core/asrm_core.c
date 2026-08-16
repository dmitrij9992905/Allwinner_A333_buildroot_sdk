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

#include "asrm_core.h"

#include "../lowlevel/lowlevel_common.h"

#ifdef AMP_SYS_RSC_MANAGER_ON_LINUX_KERNEL
#include <linux/module.h>
#include <linux/sunxi_amp_rsc.h>
#include <linux/of.h>
#include <linux/io.h>
#include <linux/of_reserved_mem.h>
#else
#include <sunxi_amp_rsc.h>
#endif

#ifdef AMP_SYS_RSC_MANAGER_ON_RTOS
#ifdef CONFIG_COMPONENTS_AMP_USER_RESOURCE
#include <amp_user_resource.h>
#endif
#endif

#ifdef AMP_SYS_RSC_MANAGER_ON_UBOOT
#include "helper/fdt_helper.h"
#endif

#include "helper/mgr_helper.h"
#include "helper/shm_helper.h"

typedef sunxi_amp_rsc_req_info_t amp_rsc_req_info_t;
typedef sunxi_amp_rsc_t amp_rsc_t;
typedef sunxi_peri_rsc_desc_t peri_rsc_desc_t;
typedef sunxi_gpio_rsc_desc_t gpio_rsc_desc_t;
typedef sunxi_dma_rsc_desc_t dma_rsc_desc_t;



#define ASRM_CORE_VERSION "1.2.2"


static amp_sys_rsc_manager_t g_amp_sys_rsc_mgr;
static int g_is_asrm_init;


#ifdef AMP_SYS_RSC_MANAGER_ON_LINUX_KERNEL
//static const void *shm;
static const char *prop_of_shm = "asrm_shm";

static void __iomem *shm_ioremap(unsigned long *psize)
{
	struct device_node *node;
	struct device_node *subnode;
	struct reserved_mem *rmem;
	void __iomem *va;

	node = of_find_node_by_path("/reserved-memory");
	if (!node)
		return NULL;

	for_each_child_of_node(node, subnode) {
		if (!of_node_name_eq(subnode, prop_of_shm))
			continue;

		rmem = of_reserved_mem_lookup(subnode);
		if (!rmem) {
			asrm_err("of_reserved_mem_lookup failed!");
			return NULL;
		}

		va = ioremap(rmem->base, rmem->size);
		if (IS_ERR_OR_NULL(va)) {
			asrm_err("ioremap failed!");
			return NULL;
		}

		if (psize)
			*psize = rmem->size;
		return va;
	}

	asrm_err("not found mem: %s", prop_of_shm);
	return NULL;
}
#endif

#define ASRM_SETUP_HW_ISOLATION

//#define ASRM_SETUP_HW_ISOLATION_ON_RTOS

#ifdef ASRM_SETUP_HW_ISOLATION
static int asrm_get_sw_user_group_id(const hw_isolator_t *hw_iso,
	hw_rsc_user_group_id_t hw_ug_id, amp_rsc_user_group_id_t *sw_ug_id)
{
	if (hw_ug_id >= hw_iso->hw_user_group_cnt)
		return -1;

	*sw_ug_id = hw_iso->user_group_map[hw_ug_id];
	return 0;
}

static int asrm_get_hw_user_group_id(const hw_isolator_t *hw_iso,
	amp_rsc_user_group_id_t sw_ug_id, hw_rsc_user_group_id_t *hw_ug_id)
{
	uint32_t i;

	for (i = 0; i < hw_iso->hw_user_group_cnt; i++) {
		if (hw_iso->user_group_map[i] == sw_ug_id) {
			*hw_ug_id = i;
			return 0;
		}
	}

	return -1;
}

static int asrm_lowlevel_find_hw_iso_drv(struct list_head *iso_drv_list, const char *compatible_str, hw_isolator_driver_t **hw_iso_drv)
{
	int is_iso_drv_found, find_cnt;

	hw_isolator_driver_t *pos;
	const hw_isolator_dev_info_t *iso_dev_info;

	uint32_t compatible_str_len, iso_dev_compatible_str_len;

	compatible_str_len = strlen(compatible_str);

	asrm_dbg("target compatible str: '%s'", compatible_str);

	is_iso_drv_found = 0;
	find_cnt = 0;
	list_for_each_entry(pos, iso_drv_list, node) {
		for (iso_dev_info = &pos->match_table[0]; iso_dev_info->compatible[0] != '\0'; iso_dev_info++) {
			find_cnt++;
			asrm_dbg("compatible str: '%s'", iso_dev_info->compatible);

			iso_dev_compatible_str_len = strlen(iso_dev_info->compatible);
			if (compatible_str_len != iso_dev_compatible_str_len)
				continue;

			if (strncmp(compatible_str, iso_dev_info->compatible, iso_dev_compatible_str_len))
				continue;

			is_iso_drv_found = 1;
			break;
		}
		if (is_iso_drv_found)
			break;
	}

	asrm_dbg("find_cnt: %d", find_cnt);
	if (is_iso_drv_found) {
		*hw_iso_drv = pos;
		return 0;
	}

	return -1;
}

static int grm_hw_iso_dev_setup_resource_owner(gpio_rsc_manager_t *grm, hw_isolator_t *hw_iso)
{
	int ret;
	gpio_rsc_t *pos;
	hw_rsc_info_t hw_rsc;
	hw_isolator_dev_t *idev;
	hw_rsc_user_group_id_t hw_ug_id;
	amp_rsc_user_group_id_t sw_ug_id;

	idev = &hw_iso->hw_iso_dev;
	hw_rsc.type = SUNXI_AMP_RSC_HW_GPIO;
	hw_rsc.gpio.peri.start_addr = hw_iso->base_addr;
	hw_rsc.gpio.peri.len = hw_iso->len;

	asrm_port_mutex_lock(&grm->rsc_lock);
	list_for_each_entry(pos, &grm->rsc_list, node) {
		hw_rsc.gpio.gpio_id = pos->gpio_id;

		sw_ug_id = pos->amp_rsc.owner_id;
		ret = asrm_get_hw_user_group_id(hw_iso, sw_ug_id, &hw_ug_id);
		if (ret) {
			asrm_err("asrm_get_hw_user_group_id failed, sw_ug_id: %u, ret: %d", sw_ug_id, ret);
			continue;
		}

		asrm_dbg("begin set hw resource owner, gpio_id: %u, hw_ug_id: %u", hw_rsc.gpio.gpio_id, hw_ug_id);
		ret = idev->ops->set_resource_owner(idev, &hw_rsc, hw_ug_id);
		if (ret) {
			asrm_err("hw isolator device set resource owner failed, sw_ug_id: %u, ret: %d", sw_ug_id, ret);
		}
	}
	asrm_port_mutex_unlock(&grm->rsc_lock);

	return 0;
}

static int grm_hw_iso_dev_setup_user_group0(hw_isolator_t *hw_iso)
{
	int ret;
	uint32_t i, current_hw_ug_id, current_sys_user_count;
	hw_rsc_user_group_info_t hw_ug_info;
	hw_isolator_dev_t *idev;

	idev = &hw_iso->hw_iso_dev;

	asrm_dbg("begin hw iso dev setup user group0");

	/* setup hw user group */
#if defined(CONFIG_MACH_SUN8IW22)
	current_sys_user_count = 5;
#elif defined(CONFIG_ARCH_SUN8IW22)
	current_sys_user_count = 5;
#else
#error "we need platform provide a API to get all user's id"
#endif

	current_hw_ug_id = 0;

	hw_ug_info.id = current_hw_ug_id;
	hw_ug_info.user_cnt = current_sys_user_count;
	hw_ug_info.user_id = asrm_port_malloc(hw_ug_info.user_cnt * sizeof(hw_rsc_user_id_t));

	for (i = 0; i < hw_ug_info.user_cnt; i++) {
		hw_ug_info.user_id[i] = i;
	}

	asrm_dbg("begin set hw user group, hw_ug_id: %u, user_cnt: %u", hw_ug_info.id, hw_ug_info.user_cnt);
	ret = idev->ops->set_user_group(idev, &hw_ug_info);
	if (ret) {
		asrm_err("hw isolator device set user group failed, ret: %d", ret);
	}

	asrm_port_free(hw_ug_info.user_id);
	hw_ug_info.user_id = NULL;
	return 0;
}

static int asrm_setup_hw_isolator(const amp_sys_rsc_manager_t *asrm, hw_isolator_t *hw_iso)
{
	int ret, is_sw_ug_found;
	uint32_t i, j, current_hw_ug_id, hw_user_group_cnt, sw_user_group_cnt;
	amp_rsc_user_group_t *sw_user_group;
	hw_rsc_user_group_info_t hw_ug_info;
	hw_isolator_dev_t *idev;
	amp_rsc_user_group_id_t sw_ug_id;

	idev = &hw_iso->hw_iso_dev;

	asrm_dbg("begin hw iso dev setup");

	/* setup hw user group */
	//user_group_cnt = asrm->user_group_cnt;
	hw_user_group_cnt = hw_iso->hw_user_group_cnt;
	sw_user_group_cnt = asrm->user_group_cnt;

	ret = grm_hw_iso_dev_setup_user_group0(hw_iso);
	if (ret) {
		asrm_err("grm_hw_iso_dev_setup_user_group0 failed, ret: %d", ret);
	}

	for (i = 1; i < hw_user_group_cnt; i++) {
		current_hw_ug_id = i;
		ret = asrm_get_sw_user_group_id(hw_iso, current_hw_ug_id, &sw_ug_id);
		if (ret) {
			asrm_err("asrm_get_sw_user_group_id failed, ret: %d", ret);
			continue;
		}

		hw_ug_info.id = current_hw_ug_id;
		if (sw_ug_id != ASRM_INVALID_RSC_USER_GROUP_ID) {
			is_sw_ug_found = 0;
			for (j = 0; j < sw_user_group_cnt; j++) {
				sw_user_group = &asrm->user_group[j];
				if (sw_user_group->id == sw_ug_id) {
					is_sw_ug_found = 1;
					break;
				}
			}

			if (!is_sw_ug_found) {
				asrm_err("sw user group not found, current hw ug id: %u", current_hw_ug_id);
				continue;
			}

			hw_ug_info.user_cnt = sw_user_group->user_cnt;
			hw_ug_info.user_id = asrm_port_malloc(hw_ug_info.user_cnt * sizeof(hw_rsc_user_id_t));
			for (j = 0; j < sw_user_group->user_cnt; j++) {
				hw_ug_info.user_id[j] = sw_user_group->user_id[j];
			}
		} else {
			hw_ug_info.user_cnt = 0;
			hw_ug_info.user_id = NULL;
		}

		asrm_dbg("begin set hw user group, hw_ug_id: %u, user_cnt: %u", hw_ug_info.id, hw_ug_info.user_cnt);
		ret = idev->ops->set_user_group(idev, &hw_ug_info);
		if (ret) {
			asrm_err("hw isolator device set user group failed, ret: %d", ret);
		}

		if (hw_ug_info.user_id) {
			asrm_port_free(hw_ug_info.user_id);
			hw_ug_info.user_id = NULL;
		}
	}

	/* setup hw resource owner */
	for (i = 0; i < asrm->gpio_manager_cnt; i++) {
		ret = grm_hw_iso_dev_setup_resource_owner(&asrm->gpio_manager[i], hw_iso);
		if (ret) {
			asrm_err("grm_hw_iso_dev_setup_resource_owner failed, ret: %d", ret);
		}
	}

	ret = idev->ops->enable(idev);
	if (ret) {
		asrm_err("hw isolator device enable failed, ret: %d", ret);
	}

	return 0;
}

static __attribute__((__unused__)) int asrm_setup_hw_isolation(amp_sys_rsc_manager_t *asrm)
{
	int ret;

	uint32_t i, j, sw_ug_index;
	gpio_rsc_manager_t *grm;
	const char *mgr_compatible_str;
	int mgr_compatible_str_len;
	hw_isolator_driver_t *iso_drv;
	hw_isolator_t *hw_iso;
	hw_isolator_dev_t *idev;

	asrm_dbg("begin setup hw isolation");

	for (i = 0; i < asrm->gpio_manager_cnt; i++) {
		asrm_dbg("begin setup gpio hw isolation");
		grm = &asrm->gpio_manager[i];
		if (!grm->is_hw_isolation)
			continue;

		hw_iso = grm->hw_iso;

		mgr_compatible_str = hw_iso->compatible;
		if (!mgr_compatible_str)
			continue;

		mgr_compatible_str_len = strlen(mgr_compatible_str);

		if (!mgr_compatible_str_len) {
			asrm_warn("compatible str is zero!");
			continue;
		}

		asrm_dbg("begin find hw iso driver");

		ret = asrm_lowlevel_find_hw_iso_drv(&asrm->hw_iso_drv_list, mgr_compatible_str, &iso_drv);
		if (ret) {
			asrm_warn("hw isolator driver not found, compatible: '%s', ret: %d",
				mgr_compatible_str, ret);
			continue;
		}

		asrm_dbg("begin probe iso driver");
		idev = &hw_iso->hw_iso_dev;
		ret = iso_drv->probe(idev);
		if (ret) {
			asrm_err("hw isolator driver probe failed, ret: %d", ret);
			continue;
		}

		asrm_dbg("begin map user group! hw_user_group_cnt: %u", hw_iso->hw_user_group_cnt);


		asrm_dbg("iso: %p, ug_map: %p, idev: %p", hw_iso, hw_iso->user_group_map, &hw_iso->hw_iso_dev);
		asrm_dbg("sw ug id %u is mapped to hw ug id %u", 0, 0);
		hw_iso->user_group_map[0] = 0;

		sw_ug_index = 0;
		for (j = 1; j < hw_iso->hw_user_group_cnt; j++) {

			if (sw_ug_index >= asrm->user_group_cnt) {
				asrm_dbg("sw ug id %u is mapped to hw ug id %u",
					ASRM_INVALID_RSC_USER_GROUP_ID, j);
				hw_iso->user_group_map[j] = ASRM_INVALID_RSC_USER_GROUP_ID;
			} else {
				asrm_dbg("sw ug id %u is mapped to hw ug id %u, sw_ug_index: %u",
					asrm->user_group[sw_ug_index].id, j, sw_ug_index);

				hw_iso->user_group_map[j] = asrm->user_group[sw_ug_index].id;
				sw_ug_index++;
			}
		}

		asrm_dbg("begin setup hw isolator");
		ret = asrm_setup_hw_isolator(asrm, hw_iso);
		if (ret) {
			asrm_err("hw isolator device setup failed, ret: %d", ret);
		}
	}

	return 0;
}
#endif

#ifdef AMP_SYS_RSC_MANAGER_ON_UBOOT
void flush_dcache_range(unsigned long start, unsigned long stop);
int sunxi_init_amp_sys_rsc_manager(void)
{
	int ret;
	unsigned long shm_base, shm_size;
	void *shm;
	amp_sys_rsc_manager_t *asrm;

	if (g_is_asrm_init)
		return 0;

	asrm = &g_amp_sys_rsc_mgr;
	ret = sunxi_amp_rsc_get_config_from_dts(asrm);
	if (ret) {
		asrm_err("sunxi_amp_rsc_get_config_from_dts failed!");
		return ret;
	}
	//sunxi_amp_rsc_show_config(0, asrm);

	ret = sunxi_amp_rsc_get_shm_info(&shm_base, &shm_size);
	if (ret) {
		asrm_err("sunxi_amp_rsc_get_shm_info failed!");
		return ret;
	}
	shm = (void *)shm_base;

	ret = sunxi_amp_rsc_config_to_shm(asrm, shm, shm_size);
	if (ret) {
		asrm_err("sunxi_amp_rsc_config_to_shm failed!");
		return ret;
	}
	flush_dcache_range((unsigned long)shm, (unsigned long)(shm + shm_size));
	//sunxi_amp_rsc_show_config_in_shm(0, shm, shm_size);

	INIT_LIST_HEAD(&asrm->hw_iso_drv_list);
	ret = asrm_lowlevel_init();
	if (ret) {
		return ret;
	}

	ret = asrm_setup_hw_isolation(asrm);
	if (ret) {
		asrm_err("asrm_setup_hw_isolation failed, ret: %d", ret);
	}

	g_is_asrm_init = 1;
	asrm_info("AMP System Resource Manager(ASRM) init success!");
	return 0;
}
#else
int sunxi_init_amp_sys_rsc_manager(void)
{
	int ret;

	const void *shm;
	unsigned long shm_size;
	amp_sys_rsc_manager_t *asrm;

#ifdef AMP_SYS_RSC_MANAGER_ON_RTOS
	amp_user_resource_t user_rsc;
	struct shared_mem_info *info;

	ret = get_amp_user_resource(3, &user_rsc);
	if (ret) {
		asrm_err("get_amp_user_resource failed!");
		return ret;
	}
	info = (struct shared_mem_info *)user_rsc.buf;
	if (info->addr == 0 || info->addr == (unsigned long)-1lu || info->len == 0) {
		asrm_err("shared_mem_info error!");
		return -1;
	}
	shm = (const void *)(unsigned long)info->addr;
	shm_size = info->len;
#endif

#ifdef AMP_SYS_RSC_MANAGER_ON_LINUX_KERNEL
	shm = shm_ioremap(&shm_size);
	if (!shm) {
		asrm_err("shm_ioremap failed!");
		return -1;
	}
#endif

	asrm_info("shm: 0x%lx, shm_size: 0x%lx", (unsigned long)shm, (unsigned long)shm_size);

	//sunxi_amp_rsc_show_config_in_shm(0, shm, shm_size);
	asrm = &g_amp_sys_rsc_mgr;
	ret = sunxi_amp_rsc_get_config_from_shm(asrm, shm, shm_size);
	if (ret) {
		asrm_err("sunxi_amp_rsc_get_config_from_shm failed!");
		return ret;
	}
	//sunxi_amp_rsc_show_config(0, asrm);

	INIT_LIST_HEAD(&asrm->hw_iso_drv_list);
	ret = asrm_lowlevel_init();
	if (ret) {
		asrm_err("asrm_lowlevel_init failed, ret: %d", ret);
		return ret;
	}

#if defined(AMP_SYS_RSC_MANAGER_ON_RTOS) && defined(ASRM_SETUP_HW_ISOLATION_ON_RTOS)
	ret = asrm_setup_hw_isolation(asrm);
	if (ret) {
		asrm_err("asrm_setup_hw_isolation failed, ret: %d", ret);
	}
#endif

	g_is_asrm_init = 1;
	asrm_info("AMP System Resource Manager(ASRM) init success!");
	return 0;
}
#endif

#ifdef AMP_SYS_RSC_MANAGER_ON_LINUX_KERNEL
postcore_initcall(sunxi_init_amp_sys_rsc_manager);
#endif

#ifdef AMP_SYS_RSC_MANAGER_ON_RTOS
static __attribute__((__unused__)) void dump_peri_desc(const peri_rsc_desc_t *desc)
{
	printf("start: 0x%08lx\n", desc->start_addr);
	printf("len: 0x%08x\n", desc->len);
}

static __attribute__((__unused__)) void dump_rsc_req_info(const amp_rsc_req_info_t *rsc_req_info)
{
	amp_rsc_type_t rsc_type;

	rsc_type = rsc_req_info->rsc_type;
	printf("RSC type: %d\n", rsc_type);
	if (rsc_type == SUNXI_AMP_RSC_HW_PERI) {
		printf("Peripheral resource\n");
		dump_peri_desc(&rsc_req_info->peri);
	} else if (rsc_type == SUNXI_AMP_RSC_HW_GPIO) {
		printf("GPIO resource\n");
		dump_peri_desc(&rsc_req_info->gpio.peri);
		printf("GPIO ID: %u\n", rsc_req_info->gpio.gpio_id);
	} else if (rsc_type == SUNXI_AMP_RSC_HW_DMA_CHANNEL) {
		printf("DMA channel resource\n");
		dump_peri_desc(&rsc_req_info->dma.peri);
		printf("Channel ID: %u\n", rsc_req_info->dma.channel_id);
	} else {
		printf("Unknown resource type\n");
	}
}
#endif

static inline int check_rsc_type(sunxi_amp_rsc_type_t rsc_type)
{
	switch (rsc_type) {
	case SUNXI_AMP_RSC_HW_PERI:
	case SUNXI_AMP_RSC_HW_GPIO:
	case SUNXI_AMP_RSC_HW_DMA_CHANNEL:
		return 0;
	default:
		return -1;
	}

	return 0;
}

#define MAX_PERI_ADDR_SPACE_LEN (1024 * 1024)
static inline int check_peri_rsc_info(const struct sunxi_peri_rsc_desc *info)
{
	if (info->start_addr == 0)
		return  ASRM_RET_INVALID_PERI_START_ADDR;

	if (info->len > MAX_PERI_ADDR_SPACE_LEN)
		return ASRM_RET_INVALID_PERI_MEM_REGION_LEN;

	return 0;
}

static inline int check_gpio_rsc_info(const struct sunxi_gpio_rsc_desc *info)
{
	return check_peri_rsc_info(&info->peri);
}

static inline int check_dma_rsc_info(const struct sunxi_dma_rsc_desc *info)
{
	return check_peri_rsc_info(&info->peri);
}

static int check_rsc_req_info(const amp_rsc_req_info_t *rsc_req_info)
{
	int ret;

	if (check_rsc_type(rsc_req_info->rsc_type))
		return ASRM_RET_INVALID_RSC_TYPE;

	switch (rsc_req_info->rsc_type) {
	case SUNXI_AMP_RSC_HW_PERI:
		ret = check_peri_rsc_info(&rsc_req_info->peri);
		break;
	case SUNXI_AMP_RSC_HW_GPIO:
		ret = check_gpio_rsc_info(&rsc_req_info->gpio);
		break;
	case SUNXI_AMP_RSC_HW_DMA_CHANNEL:
		ret = check_dma_rsc_info(&rsc_req_info->dma);
		break;
	default:
		ret = ASRM_RET_INVALID_RSC_TYPE;
	}

	return ret;
}

static int init_implicit_rsc(amp_sys_rsc_t *asr)
{
	asr->owner_id = ASRM_IMPLICIT_ALLOC_RSC_OWNER_ID;
	asr->is_implicit_alloc = 1;
	return 0;
}

static void asr_set_sw_module_id(amp_sys_rsc_t *rsc, const char *sw_module_id)
{
	strncpy(rsc->current_sw_module, sw_module_id, ASRM_MAX_SW_MODULE_ID_STR_LEN);
	rsc->current_sw_module[ASRM_MAX_SW_MODULE_ID_STR_LEN] = '\0';
}

static int manager_add_rsc(asrm_port_mutex_t *rsc_lock, struct list_head *rsc_list, struct list_head *rsc_node)
{
    INIT_LIST_HEAD(rsc_node);

	asrm_port_mutex_lock(rsc_lock);
	list_add_tail(rsc_node, rsc_list);
	asrm_port_mutex_unlock(rsc_lock);

	return 0;
}

static int manager_del_rsc(asrm_port_mutex_t *rsc_lock, struct list_head *rsc_node)
{
	asrm_port_mutex_lock(rsc_lock);
	if (!list_empty(rsc_node))
		list_del_init(rsc_node);
	asrm_port_mutex_unlock(rsc_lock);

	return 0;
}

static inline int is_peri_info_match(unsigned long start_addr, uint32_t len, const peri_rsc_desc_t *peri_info)
{
	if ((start_addr == peri_info->start_addr)
		&& (len == peri_info->len)) {
		return 1;
	}

	return 0;
}


static int prm_create_implicit_peri_rsc(peri_rsc_manager_t *peri_manager, const peri_rsc_desc_t *peri_info, peri_rsc_t **rsc)
{
	peri_rsc_t *pr;

	pr = asrm_port_zalloc(sizeof(peri_rsc_t));
	if (!pr)
		return -1;

	pr->start_addr = peri_info->start_addr;
	pr->end_addr = peri_info->start_addr + peri_info->len - 1;
	pr->manager = peri_manager;

	init_implicit_rsc(&pr->amp_rsc);
	pr->amp_rsc.type = SUNXI_AMP_RSC_HW_PERI;

	*rsc = pr;
	return 0;
}

static inline int prm_add_peri_rsc(peri_rsc_manager_t *peri_manager,
	peri_rsc_t *rsc)
{
	return manager_add_rsc(&peri_manager->rsc_lock, &peri_manager->rsc_list, &rsc->node);
}

static inline int prm_del_peri_rsc(peri_rsc_manager_t *peri_manager,
	peri_rsc_t *rsc)
{
	return manager_del_rsc(&peri_manager->rsc_lock, &rsc->node);
}

static int prm_find_peri_rsc(peri_rsc_manager_t *peri_manager, const peri_rsc_desc_t *peri, peri_rsc_t **rsc)
{
	peri_rsc_t *pos;

	asrm_port_mutex_lock(&peri_manager->rsc_lock);
	list_for_each_entry(pos, &peri_manager->rsc_list, node) {
		asrm_dbg("start: 0x%lx, len: 0x%lx", pos->start_addr, pos->end_addr);
		if ((pos->start_addr == peri->start_addr)
			&& (pos->end_addr == (peri->start_addr + peri->len - 1))) {
			*rsc = pos;
			asrm_port_mutex_unlock(&peri_manager->rsc_lock);
			return 0;
		}
	}
	asrm_port_mutex_unlock(&peri_manager->rsc_lock);

	return -1;
}


static int grm_create_implicit_gpio_rsc(gpio_rsc_manager_t *gpio_manager, uint32_t gpio_id, gpio_rsc_t **rsc)
{
	gpio_rsc_t *tmp_rsc;

	tmp_rsc = asrm_port_zalloc(sizeof(gpio_rsc_t));
	if (!tmp_rsc)
		return -1;

	tmp_rsc->manager = gpio_manager;
	tmp_rsc->gpio_id = gpio_id;

	init_implicit_rsc(&tmp_rsc->amp_rsc);
	tmp_rsc->amp_rsc.type = SUNXI_AMP_RSC_HW_GPIO;

	*rsc = tmp_rsc;
	return 0;
}

static inline int grm_add_gpio_rsc(gpio_rsc_manager_t *gpio_manager,
	gpio_rsc_t *rsc)
{
	return manager_add_rsc(&gpio_manager->rsc_lock, &gpio_manager->rsc_list, &rsc->node);
}

static inline int grm_del_gpio_rsc(gpio_rsc_manager_t *gpio_manager,
	gpio_rsc_t *rsc)
{
	return manager_del_rsc(&gpio_manager->rsc_lock, &rsc->node);
}

static int grm_find_gpio_rsc(gpio_rsc_manager_t *gpio_manager, const gpio_rsc_desc_t *gpio, gpio_rsc_t **rsc)
{
	gpio_rsc_t *pos;

	asrm_port_mutex_lock(&gpio_manager->rsc_lock);
	list_for_each_entry(pos, &gpio_manager->rsc_list, node) {
		if (pos->gpio_id == gpio->gpio_id) {
			*rsc = pos;
			asrm_port_mutex_unlock(&gpio_manager->rsc_lock);
			return 0;
		}
	}
	asrm_port_mutex_unlock(&gpio_manager->rsc_lock);

	return -1;
}

static int dcrm_create_implicit_dma_channel_rsc(dma_rsc_manager_t *dma_manager, uint32_t channel_id, dma_channel_rsc_t **rsc)
{
	dma_channel_rsc_t *tmp_rsc;

	tmp_rsc = asrm_port_zalloc(sizeof(dma_channel_rsc_t));
	if (!tmp_rsc)
		return -1;

	tmp_rsc->channel_id = channel_id;
	tmp_rsc->manager = dma_manager;

	init_implicit_rsc(&tmp_rsc->amp_rsc);
	tmp_rsc->amp_rsc.type = SUNXI_AMP_RSC_HW_GPIO;

	*rsc = tmp_rsc;
	return 0;
}

static inline int dcrm_add_dma_channel_rsc(dma_rsc_manager_t *dma_manager,
	dma_channel_rsc_t *rsc)
{
	return manager_add_rsc(&dma_manager->rsc_lock, &dma_manager->rsc_list, &rsc->node);
}

static inline int dcrm_del_dma_channel_rsc(dma_rsc_manager_t *dma_manager,
	dma_channel_rsc_t *rsc)
{
	return manager_del_rsc(&dma_manager->rsc_lock, &rsc->node);
}

static int dcrm_find_dma_channel_rsc(dma_rsc_manager_t *dma_manager, const dma_rsc_desc_t *dma_channel, dma_channel_rsc_t **rsc)
{
	dma_channel_rsc_t *pos;

	asrm_port_mutex_lock(&dma_manager->rsc_lock);
	list_for_each_entry(pos, &dma_manager->rsc_list, node) {
		if (pos->channel_id == dma_channel->channel_id) {
			*rsc = pos;
			asrm_port_mutex_unlock(&dma_manager->rsc_lock);
			return 0;
		}
	}
	asrm_port_mutex_unlock(&dma_manager->rsc_lock);

	return -1;
}



static int asrm_create_implicit_peri_rsc(const amp_sys_rsc_manager_t *asrm, const peri_rsc_desc_t *peri_info, peri_rsc_t **rsc)
{
	uint32_t i;
	peri_rsc_manager_t *prm;

	for (i = 0; i < asrm->peri_manager_cnt; i++) {
		//currently we put implicit peripheral resource into the first prm
		//perhaps these is more than one prm on hardware implemention in the future.
		prm = &asrm->peri_manager[i];
		return prm_create_implicit_peri_rsc(prm, peri_info, rsc);
	}

	asrm_err("explicit allocated Peripheral rsc manager(0x%lx,0x%x) not found!",
		peri_info->start_addr, peri_info->len);
	return ASRM_RET_EXPLICIT_MANAGER_NOT_FOUND;
}

static int asrm_find_peri_rsc(const amp_sys_rsc_manager_t *asrm, const peri_rsc_desc_t *peri_info, peri_rsc_t **rsc)
{
	uint32_t i;
	peri_rsc_manager_t *peri_manager;
	peri_rsc_t *peri_rsc;

	for (i = 0; i < asrm->peri_manager_cnt; i++) {
		peri_manager = &asrm->peri_manager[i];
		if (!prm_find_peri_rsc(peri_manager, peri_info, &peri_rsc)) {
			*rsc = peri_rsc;
			return 0;
		}
	}

	return ASRM_RET_RSC_NOT_FOUND;
}

static inline int asrm_add_peri_rsc(const amp_sys_rsc_manager_t *asrm, peri_rsc_t *rsc)
{
	return prm_add_peri_rsc(rsc->manager, rsc);
}

static inline int asrm_del_peri_rsc(const amp_sys_rsc_manager_t *asrm, peri_rsc_t *rsc)
{
	return prm_del_peri_rsc(rsc->manager, rsc);
}


static int asrm_create_implicit_gpio_rsc(const amp_sys_rsc_manager_t *asrm, const gpio_rsc_desc_t *gpio_info, gpio_rsc_t **rsc)
{
	uint32_t i;
	gpio_rsc_manager_t *grm;
	//gpio_rsc_t *gpio_rsc;

	for (i = 0; i < asrm->gpio_manager_cnt; i++) {
		grm = &asrm->gpio_manager[i];
		if (is_peri_info_match(grm->hw_mem_region_start, grm->hw_mem_region_len, &gpio_info->peri)) {
			return grm_create_implicit_gpio_rsc(grm, gpio_info->gpio_id, rsc);
		}
	}

	asrm_err("explicit allocated GPIO rsc manager(0x%lx,0x%x) not found!",
		gpio_info->peri.start_addr, gpio_info->peri.len);
	return ASRM_RET_EXPLICIT_MANAGER_NOT_FOUND;
}

static int asrm_find_gpio_rsc(const amp_sys_rsc_manager_t *asrm, const gpio_rsc_desc_t *gpio_info, gpio_rsc_t **rsc)
{
	uint32_t i;
	int is_grm_found;
	gpio_rsc_manager_t *grm;
	gpio_rsc_t *gpio_rsc;

	asrm_dbg("start: 0x%lx, len: 0x%x", gpio_info->peri.start_addr, gpio_info->peri.len);
	is_grm_found = 0;
	for (i = 0; i < asrm->gpio_manager_cnt; i++) {
		grm = &asrm->gpio_manager[i];

		asrm_dbg("grm start: 0x%lx, len: 0x%x", grm->hw_mem_region_start, grm->hw_mem_region_len);
		if (is_peri_info_match(grm->hw_mem_region_start, grm->hw_mem_region_len, &gpio_info->peri)) {
			is_grm_found = 1;
			break;
		}
	}

	if (!is_grm_found)
		return ASRM_RET_MANAGER_NOT_FOUND;

	if (!grm_find_gpio_rsc(grm, gpio_info, &gpio_rsc)) {
		*rsc = gpio_rsc;
		return 0;
	}

	return ASRM_RET_RSC_NOT_FOUND;
}

static inline int asrm_add_gpio_rsc(const amp_sys_rsc_manager_t *asrm, gpio_rsc_t *rsc)
{
	return grm_add_gpio_rsc(rsc->manager, rsc);
}

static inline int asrm_del_gpio_rsc(const amp_sys_rsc_manager_t *asrm, gpio_rsc_t *rsc)
{
	return grm_del_gpio_rsc(rsc->manager, rsc);
}

static int asrm_create_implicit_dma_channel_rsc(const amp_sys_rsc_manager_t *asrm, const dma_rsc_desc_t *dma_ch_info, dma_channel_rsc_t **rsc)
{
	uint32_t i;
	dma_channel_rsc_manager_t *dcrm;
	//gpio_rsc_t *gpio_rsc;

	for (i = 0; i < asrm->dma_manager_cnt; i++) {
		dcrm = &asrm->dma_manager[i];
		if (is_peri_info_match(dcrm->hw_mem_region_start, dcrm->hw_mem_region_len, &dma_ch_info->peri)) {
			return dcrm_create_implicit_dma_channel_rsc(dcrm, dma_ch_info->channel_id, rsc);
		}
	}

	asrm_err("explicit allocated DMA channel rsc manager(0x%lx,0x%x) not found!",
		dma_ch_info->peri.start_addr, dma_ch_info->peri.len);
	return ASRM_RET_EXPLICIT_MANAGER_NOT_FOUND;
}

static int asrm_find_dma_channel_rsc(const amp_sys_rsc_manager_t *asrm, const dma_rsc_desc_t *dma_channel_info, dma_channel_rsc_t **rsc)
{
	uint32_t i;
	int is_dcrm_found;
	dma_channel_rsc_manager_t *dcrm;
	dma_channel_rsc_t *dma_channel_rsc;

	is_dcrm_found = 0;
	for (i = 0; i < asrm->dma_manager_cnt; i++) {
		dcrm = &asrm->dma_manager[i];

		if (is_peri_info_match(dcrm->hw_mem_region_start, dcrm->hw_mem_region_len, &dma_channel_info->peri)) {
			is_dcrm_found = 1;
			break;
		}
	}

	if (!is_dcrm_found)
		return ASRM_RET_MANAGER_NOT_FOUND;

	if (!dcrm_find_dma_channel_rsc(dcrm, dma_channel_info, &dma_channel_rsc)) {
		*rsc = dma_channel_rsc;
		return 0;
	}

	return ASRM_RET_RSC_NOT_FOUND;

}

static inline int asrm_add_dma_channel_rsc(const amp_sys_rsc_manager_t *asrm, dma_channel_rsc_t *rsc)
{
	return dcrm_add_dma_channel_rsc(rsc->manager, rsc);
}

static inline int asrm_del_dma_channel_rsc(const amp_sys_rsc_manager_t *asrm, dma_channel_rsc_t *rsc)
{
	return dcrm_del_dma_channel_rsc(rsc->manager, rsc);
}

static int asrm_create_implicit_amp_sys_rsc(const amp_sys_rsc_manager_t *asrm, const amp_rsc_req_info_t *rsc_req_info, amp_sys_rsc_t **asr)
{
	int ret;
	amp_rsc_type_t rsc_type;

	rsc_type = rsc_req_info->rsc_type;
	if (rsc_type == SUNXI_AMP_RSC_HW_PERI) {
		peri_rsc_t *peri_rsc;
		ret = asrm_create_implicit_peri_rsc(asrm, &rsc_req_info->peri, &peri_rsc);
		if (!ret)
			*asr = &peri_rsc->amp_rsc;
	} else if (rsc_type == SUNXI_AMP_RSC_HW_GPIO) {
		gpio_rsc_t *gpio_rsc;
		ret = asrm_create_implicit_gpio_rsc(asrm, &rsc_req_info->gpio, &gpio_rsc);
		if (!ret)
			*asr = &gpio_rsc->amp_rsc;
	} else if (rsc_type == SUNXI_AMP_RSC_HW_DMA_CHANNEL) {
		dma_channel_rsc_t *dma_channel_rsc;
		ret = asrm_create_implicit_dma_channel_rsc(asrm, &rsc_req_info->dma, &dma_channel_rsc);
		if (!ret)
			*asr = &dma_channel_rsc->amp_rsc;
	} else {
		ret = ASRM_RET_INVALID_RSC_TYPE;
	}

	return ret;
}

static int asrm_find_amp_sys_rsc(const amp_sys_rsc_manager_t *asrm, const amp_rsc_req_info_t *rsc_req_info, amp_sys_rsc_t **asr)
{
	int ret;
	amp_rsc_type_t rsc_type;

	rsc_type = rsc_req_info->rsc_type;
	if (rsc_type == SUNXI_AMP_RSC_HW_PERI) {
		peri_rsc_t *peri_rsc;
		ret = asrm_find_peri_rsc(asrm, &rsc_req_info->peri, &peri_rsc);
		if (!ret) {
			*asr = &peri_rsc->amp_rsc;
			return 0;
		}
	} else if (rsc_type == SUNXI_AMP_RSC_HW_GPIO) {
		gpio_rsc_t *gpio_rsc;
		ret = asrm_find_gpio_rsc(asrm, &rsc_req_info->gpio, &gpio_rsc);
		if (!ret) {
			*asr = &gpio_rsc->amp_rsc;
			return 0;
		}
	} else if (rsc_type == SUNXI_AMP_RSC_HW_DMA_CHANNEL) {
		dma_channel_rsc_t *dma_channel_rsc;
		ret = asrm_find_dma_channel_rsc(asrm, &rsc_req_info->dma, &dma_channel_rsc);
		if (!ret) {
			*asr = &dma_channel_rsc->amp_rsc;
			return 0;
		}
	} else {
		ret = ASRM_RET_INVALID_RSC_TYPE;
	}

	asrm_dbg("find failed, ret: %d", ret);
	if (ret == ASRM_RET_MANAGER_NOT_FOUND)
		ret = ASRM_RET_RSC_NOT_FOUND;

	return ret;
}

static int asrm_add_amp_sys_rsc(const amp_sys_rsc_manager_t *asrm, amp_sys_rsc_t *asr)
{
	amp_rsc_type_t rsc_type;

	rsc_type = asr->type;

	if (rsc_type == SUNXI_AMP_RSC_HW_PERI) {
		peri_rsc_t *pr = asr_to_peri_rsc(asr);
		return asrm_add_peri_rsc(asrm, pr);
	} else if (rsc_type == SUNXI_AMP_RSC_HW_GPIO) {
		gpio_rsc_t *gr = asr_to_gpio_rsc(asr);
		return asrm_add_gpio_rsc(asrm, gr);
	} else if (rsc_type == SUNXI_AMP_RSC_HW_DMA_CHANNEL) {
		dma_channel_rsc_t *dcr = asr_to_dma_channel_rsc(asr);
		return asrm_add_dma_channel_rsc(asrm, dcr);
	} else {
		return ASRM_RET_INVALID_RSC_TYPE;
	}

	return 0;
}

static int asrm_del_amp_sys_rsc(amp_sys_rsc_manager_t *asrm, amp_sys_rsc_t *asr)
{
	int ret;
	amp_rsc_type_t rsc_type;
	void *rsc;

	if (!asr->is_implicit_alloc) {
		asrm_err("del explicit alloc rsc is not allowed!\n");
		return ASRM_RET_DEL_EXPLICIT_ALLOC_RSC;
	}

	rsc_type = asr->type;
	if (rsc_type == SUNXI_AMP_RSC_HW_PERI) {
		peri_rsc_t *pr = asr_to_peri_rsc(asr);
		rsc = pr;
		ret = asrm_del_peri_rsc(asrm, pr);
	} else if (rsc_type == SUNXI_AMP_RSC_HW_GPIO) {
		gpio_rsc_t *gr = asr_to_gpio_rsc(asr);
		rsc = gr;
		ret = asrm_del_gpio_rsc(asrm, gr);
	} else if (rsc_type == SUNXI_AMP_RSC_HW_DMA_CHANNEL) {
		dma_channel_rsc_t *dcr = asr_to_dma_channel_rsc(asr);
		rsc = dcr;
		ret = asrm_del_dma_channel_rsc(asrm, dcr);
	} else {
		return ASRM_RET_INVALID_RSC_TYPE;
	}

	if (!ret)
		asrm_port_free(rsc);

	return ret;
}

static int asrm_get_current_user_id(amp_rsc_user_id_t *user_id)
{
	*user_id = ASRM_CURRENT_RSC_USER_ID;
	return 0;
}

static int asrm_get_current_user_group_id(const amp_sys_rsc_manager_t *asrm, amp_rsc_user_group_id_t *user_group_id)
{
	int ret;
	uint32_t i, j;
	amp_rsc_user_id_t current_user_id = ASRM_INVALID_RSC_USER_ID;
	amp_rsc_user_group_t *ug;

	ret = asrm_get_current_user_id(&current_user_id);
	if (ret)
		return ret;

	for (i = 0; i < asrm->user_group_cnt; i++) {
		ug = &asrm->user_group[i];

		for (j = 0; j < ug->user_cnt; j++) {
			if (ug->user_id[j] == current_user_id) {
				*user_group_id = ug->id;
				return 0;
			}
		}
	}

	*user_group_id = ASRM_IMPLICIT_ALLOC_RSC_OWNER_ID;
	return 0;
}

int sunxi_amp_rsc_request(const amp_rsc_req_info_t *rsc_req_info, amp_rsc_t *resource)
{
	int ret;
	amp_sys_rsc_manager_t *asrm;
	amp_sys_rsc_t *asr;

	asrm_dbg("RSC Type: %d", rsc_req_info->rsc_type);

	//dump_rsc_req_info(rsc_req_info);

	ret = check_rsc_req_info(rsc_req_info);
	if (ret) {
		*resource = (amp_rsc_t)ASRM_RSC_HANDLE_VALUE_USER_REASON_REQ_FAILED;
		return ret;
	}

	if (!g_is_asrm_init) {
		*resource = (amp_rsc_t)ASRM_RSC_HANDLE_VALUE_INTERNAL_REASON_REQ_FAILED;
		return 0;
	}

	asrm = &g_amp_sys_rsc_mgr;
	ret = asrm_find_amp_sys_rsc(asrm, rsc_req_info, &asr);
	if (ret && (ret != ASRM_RET_RSC_NOT_FOUND))
		return ret;

	if (!ret) {
		amp_rsc_user_group_id_t ug_id;
		asrm_dbg("amp sys rsc found!");

		ret = asrm_get_current_user_group_id(asrm, &ug_id);
		if (ret)
			return ret;

		if (asr->owner_id != ug_id) {
			*resource = (amp_rsc_t)ASRM_RSC_HANDLE_VALUE_NO_PERMISSION;
			return ASRM_RET_CURRENT_USER_NO_PERMISSION;
		}

		asr->ref_cnt++;
		if (rsc_req_info->sw_module_id_str)
			asr_set_sw_module_id(asr, rsc_req_info->sw_module_id_str);

		*resource = asr;
		return 0;
	} else {
		asrm_dbg("amp sys rsc not found!");
	}

	ret = asrm_create_implicit_amp_sys_rsc(asrm, rsc_req_info, &asr);
	if (ret)
		return ret;

	ret =  asrm_add_amp_sys_rsc(asrm, asr);
	if (ret)
		return ret;

	asr->ref_cnt++;
	if (rsc_req_info->sw_module_id_str)
		asr_set_sw_module_id(asr, rsc_req_info->sw_module_id_str);

	*resource = asr;
	return 0;
}

int sunxi_amp_rsc_free(amp_rsc_t resource)
{
	amp_sys_rsc_t *asr;
	amp_sys_rsc_manager_t *asrm;

	asr = (amp_sys_rsc_t *)resource;

	if (!asr) {
		return ASRM_RET_INVALID_PARAMETER;
	}

	if ((asr == (amp_sys_rsc_t *)ASRM_RSC_HANDLE_VALUE_NO_PERMISSION) ||
		(asr == (amp_sys_rsc_t *)ASRM_RSC_HANDLE_VALUE_USER_REASON_REQ_FAILED) ||
		(asr == (amp_sys_rsc_t *)ASRM_RSC_HANDLE_VALUE_INTERNAL_REASON_REQ_FAILED))
		return 0;

	asr->ref_cnt--;

	if (asr->is_implicit_alloc) {
		if (asr->ref_cnt)
			return 0;

		asrm = &g_amp_sys_rsc_mgr;
		return asrm_del_amp_sys_rsc(asrm, asr);
	}

	return 0;
}

int sunxi_amp_rsc_has_permission(amp_rsc_t resource)
{
	amp_sys_rsc_t *asr;

	asr = (amp_sys_rsc_t *)resource;

	if ((!asr) ||
		(asr == (amp_sys_rsc_t *)ASRM_RSC_HANDLE_VALUE_NO_PERMISSION) ||
		(asr == (amp_sys_rsc_t *)ASRM_RSC_HANDLE_VALUE_USER_REASON_REQ_FAILED))
		return 0;

	if (asr == (amp_sys_rsc_t *)ASRM_RSC_HANDLE_VALUE_INTERNAL_REASON_REQ_FAILED)
		return 1;

	//if (rsc->is_implicit_alloc)
	//	return 1;

	return 1;
}

#ifdef AMP_SYS_RSC_MANAGER_ON_LINUX_KERNEL
static int request_peri_resource_by_rsc_desc(struct device *dev,
	const struct resource *pdev_res, const char *sw_module_id_str, amp_rsc_t *resource)
{
	int ret;
	amp_rsc_req_info_t rsc_info;

	rsc_info.sw_module_id_str = sw_module_id_str;
	rsc_info.rsc_type = SUNXI_AMP_RSC_HW_PERI;
	rsc_info.peri.start_addr = pdev_res->start;
	rsc_info.peri.len = pdev_res->end - pdev_res->start + 1;

	//TODO: temporary debug
	dev_err(dev, "AMP resource request, type: %d, start: %lx, len: %x",
		rsc_info.rsc_type, rsc_info.peri.start_addr, rsc_info.peri.len);

	ret = sunxi_amp_rsc_request(&rsc_info, resource);
	if (ret) {
		//sunxi_err(&pdev->dev, "AMP resource request failed, ret: %d\n", ret);
		return -EACCES;
	}

	return 0;

}

int sunxi_pdev_request_peri_rsc_by_index(struct platform_device *pdev,
	unsigned int pdev_rsc_index, const char *sw_module_id_str, amp_rsc_t *amp_resource)
{
	struct resource *pdev_res;

	pdev_res = platform_get_resource(pdev, IORESOURCE_MEM, pdev_rsc_index);
	if (pdev_res == NULL) {
		return -EIO;
	}

	return request_peri_resource_by_rsc_desc(&pdev->dev, pdev_res, sw_module_id_str, amp_resource);
}

int sunxi_pdev_request_peri_rsc_by_name(struct platform_device *pdev,
	const char *pdev_rsc_name, const char *sw_module_id_str, amp_rsc_t *amp_resource)
{
	struct resource *pdev_res;

	pdev_res = platform_get_resource_byname(pdev, IORESOURCE_MEM, pdev_rsc_name);
	if (pdev_res == NULL) {
		return -EIO;
	}

	return request_peri_resource_by_rsc_desc(&pdev->dev, pdev_res, sw_module_id_str, amp_resource);
}
#endif


int hw_isolator_driver_register(hw_isolator_driver_t *drv)
{
	amp_sys_rsc_manager_t *asrm = &g_amp_sys_rsc_mgr;

	list_add(&drv->node, &asrm->hw_iso_drv_list);
	return 0;
}



int hw_isolator_dev_get_reg_addr_info(const hw_isolator_dev_t *idev, reg_addr_info_t *addr_info)
{
	hw_isolator_t *hw_iso;

	if (!addr_info)
		return -1;

	hw_iso = container_of(idev, hw_isolator_t, hw_iso_dev);

	addr_info->base_addr = hw_iso->base_addr;
	addr_info->len = hw_iso->len;
	return 0;
}

uint32_t hw_isolator_dev_get_user_group_num(const hw_isolator_dev_t *idev)
{
	const hw_isolator_t *hw_iso;

	hw_iso = container_of(idev, hw_isolator_t, hw_iso_dev);
	return hw_iso->hw_user_group_cnt;
}

#ifdef AMP_SYS_RSC_MANAGER_ON_RTOS
#include <hal_cmd.h>
static int cmd_asrm_dump(int argc, char *argv[])
{
	sunxi_amp_rsc_show_config(0, &g_amp_sys_rsc_mgr);
	return 0;
}
FINSH_FUNCTION_EXPORT_CMD(cmd_asrm_dump, asrm_dump, dump current asrm info);
#endif

#ifdef AMP_SYS_RSC_MANAGER_ON_LINUX_KERNEL
MODULE_DESCRIPTION("Allwinnertech AMP System Resource Manager(ASRM)");
MODULE_AUTHOR("liusijun <liusijun@allwinnertech.com>");
MODULE_AUTHOR("shihongfu <shihongfu@allwinnertech.com>");
MODULE_LICENSE("GPL v2");
MODULE_VERSION(ASRM_CORE_VERSION);
#endif
