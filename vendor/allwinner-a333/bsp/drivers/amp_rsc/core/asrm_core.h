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

#ifndef __ASRM_CORE_H__
#define __ASRM_CORE_H__

#include "include/asrm_common.h"
#include "include/asrm_port.h"
#include "include/asrm_lowlevel.h"

struct amp_rsc_user;
typedef struct amp_rsc_user amp_rsc_user_t;

struct amp_rsc_user_group;
typedef struct amp_rsc_user_group amp_rsc_user_group_t;

typedef struct amp_sys_rsc_manager {
	/* user info */
	uint32_t user_cnt;
	amp_rsc_user_t *user;

	/* user group info */
	uint32_t user_group_cnt;
	amp_rsc_user_group_t *user_group;

	struct list_head hw_iso_drv_list;

	/* the allocation info of resource */
	//asrm_port_mutex_t peri_manager_lock;
	//struct list_head peri_manager_list;

	uint32_t peri_manager_cnt;
	struct peri_rsc_manager *peri_manager;

	uint32_t gpio_manager_cnt;
	struct gpio_rsc_manager *gpio_manager;

	uint32_t dma_manager_cnt;
	struct dma_channel_rsc_manager *dma_manager;
} amp_sys_rsc_manager_t;

typedef struct amp_rsc_user {
	amp_rsc_user_type_t user_type;
	amp_rsc_user_id_t user_id;
} amp_rsc_user_t;

typedef uint32_t amp_rsc_user_group_id_t;

#define MAX_USER_NUM_IN_USER_GROUP 15
typedef struct amp_rsc_user_group {
	amp_rsc_user_group_id_t id;
	uint32_t user_cnt;
	amp_rsc_user_id_t user_id[MAX_USER_NUM_IN_USER_GROUP];
} amp_rsc_user_group_t;

typedef amp_rsc_user_group_id_t amp_rsc_owner_id_t;

#define ASRM_MAX_SW_MODULE_ID_STR_LEN 15
typedef struct amp_sys_rsc {
	amp_rsc_type_t type;
	/* the ID of resource owner */
	amp_rsc_owner_id_t owner_id;

	uint32_t ref_cnt;
	/* the current software module which is using this resource */
	char current_sw_module[ASRM_MAX_SW_MODULE_ID_STR_LEN + 1];
	/* implicit allocation(not allocate explicitly on dts) */
	int is_implicit_alloc;
} amp_sys_rsc_t;

#define ASRM_SUPPORT_MAX_HW_USER_GROUP_NUM 16
typedef struct hw_isolator {
	/* provided by dts */
	char compatible[64];
	unsigned long base_addr;
	uint32_t len;

	uint32_t hw_user_group_cnt;

	amp_rsc_user_group_id_t user_group_map[ASRM_SUPPORT_MAX_HW_USER_GROUP_NUM];

	hw_isolator_dev_t hw_iso_dev;
} hw_isolator_t;

/*
typedef struct rsc_manager_base {
	int is_hw_isolation;
	asrm_port_mutex_t rsc_lock;
	struct list_head rsc_list;
	hw_rsc_controller_dev_t *hw_dev;
} rsc_manager_base_t;
*/

typedef struct peri_rsc {
	amp_sys_rsc_t amp_rsc;
	struct peri_rsc_manager *manager;
	unsigned long start_addr;
	unsigned long end_addr;
	struct list_head node;
} peri_rsc_t;

typedef struct peri_rsc_manager {
	//rsc_manager_base_t manager;
	int is_hw_isolation;
	asrm_port_mutex_t rsc_lock;
	struct list_head rsc_list;
	hw_isolator_t *hw_iso;
	/* IF the hardware implementtion let a peripheral resource manager
	 * manage some peripherals, and another peripheral resource manager
	 * manage some peripherals. We can get this peripherals's info from dts.
	 */
	//uint32_t managed_peri_cnt;
	//sunxi_peri_rsc_desc_t *managed_peri_info;

	struct list_head node;
} peri_rsc_manager_t;

typedef peri_rsc_manager_t peripheral_resource_manager_t;

typedef struct gpio_rsc {
	amp_sys_rsc_t amp_rsc;
	struct gpio_rsc_manager *manager;
	uint32_t gpio_id;
	struct list_head node;
} gpio_rsc_t;
typedef gpio_rsc_t gpio_resource_t;

typedef struct gpio_rsc_manager {
	//rsc_manager_base_t manager;
	unsigned long hw_mem_region_start;
	uint32_t hw_mem_region_len;

	int is_hw_isolation;
	asrm_port_mutex_t rsc_lock;
	struct list_head rsc_list;
	hw_isolator_t *hw_iso;

	//uint32_t max_gpio_id;
} gpio_rsc_manager_t;

typedef struct dma_channel_rsc {
	amp_sys_rsc_t amp_rsc;
	struct dma_channel_rsc_manager *manager;
	uint32_t channel_id;
	struct list_head node;
} dma_channel_rsc_t;
typedef dma_channel_rsc_t dma_rsc_t;

typedef struct dma_channel_rsc_manager {
	//rsc_manager_base_t manager;
	unsigned long hw_mem_region_start;
	uint32_t hw_mem_region_len;

	int is_hw_isolation;
	asrm_port_mutex_t rsc_lock;
	struct list_head rsc_list;
	hw_isolator_t *hw_iso;

	//uint32_t max_channel_id;
} dma_channel_rsc_manager_t;
typedef dma_channel_rsc_manager_t dma_rsc_manager_t;

static inline peri_rsc_t *asr_to_peri_rsc(amp_sys_rsc_t *asr)
{
	return container_of(asr, peri_rsc_t, amp_rsc);
}

static inline gpio_rsc_t *asr_to_gpio_rsc(amp_sys_rsc_t *asr)
{
	return container_of(asr, gpio_rsc_t, amp_rsc);
}

static inline dma_channel_rsc_t *asr_to_dma_channel_rsc(amp_sys_rsc_t *asr)
{
	return container_of(asr, dma_channel_rsc_t, amp_rsc);
}

#define ASRM_INVALID_RSC_USER_ID -1
#define ASRM_INVALID_RSC_USER_GROUP_ID -1

#define ASRM_IMPLICIT_ALLOC_RSC_OWNER_ID 0

#define ASRM_RSC_HANDLE_VALUE_NO_PERMISSION 0xA0DEAD00
#define ASRM_RSC_HANDLE_VALUE_USER_REASON_REQ_FAILED 0xA0DEAD66
#define ASRM_RSC_HANDLE_VALUE_INTERNAL_REASON_REQ_FAILED 0xA0DEAD55

#define ASRM_CURRENT_RSC_USER_ID CONFIG_AW_CURRENT_AMP_SYS_RSC_USER_ID

#endif /* __ASRM_CORE_H__ */
