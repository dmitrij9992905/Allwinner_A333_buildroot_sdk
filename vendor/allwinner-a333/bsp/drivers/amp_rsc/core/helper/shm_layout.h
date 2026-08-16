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

#ifndef __ASRM_SHM_LAYOUT_H__
#define __ASRM_SHM_LAYOUT_H__

/* mem layout
 * struct shared_mem {
 * 	struct rsc_allocation_table_header header;
 *
 * 	struct shm_user_group user_group[header.user_group_cnt];
 *
 * 	struct shm_peripheral_rsc_controller peri[header.peri_rsc_controller_cnt];
 *
 * 	struct shm_gpio_rsc_controller gpio[header.gpio_rsc_controller_cnt];
 *
 * 	struct shm_dma_rsc_controller dma[header.dma_rsc_controller_cnt];
 * };
 */

struct rsc_allocation_table_header {
	uint32_t magic;
	uint32_t version;
	uint32_t table_size;

	uint32_t user_group_offset;
	uint32_t user_group_cnt;

	uint32_t peri_rsc_controller_offset;
	uint32_t peri_rsc_controller_cnt;

	uint32_t gpio_rsc_controller_offset;
	uint32_t gpio_rsc_controller_cnt;

	uint32_t dma_rsc_controller_offset;
	uint32_t dma_rsc_controller_cnt;
} __attribute__((packed));

struct shm_rsc_controller_dev {
	/* provided by dts */
	char compatible[64];
	uint64_t base_addr;
	uint32_t len;
	uint32_t hw_user_group_cnt;
} __attribute__((packed));

struct shm_user_group {
	uint32_t id;
	uint32_t user_cnt;
	uint32_t users_id[];
} __attribute__((packed));

struct shm_peripheral_rsc {
	uint64_t base_addr;
	uint32_t len;
	uint32_t owner_id;
} __attribute__((packed));

struct shm_peripheral_rsc_controller {
	struct shm_rsc_controller_dev hw_dev;
	uint32_t flags;

	uint32_t peri_cnt;
	struct shm_peripheral_rsc rsc[];
} __attribute__((packed));

struct shm_gpio_rsc {
	uint32_t gpio_id;
	uint32_t owner_id;
} __attribute__((packed));

struct shm_gpio_rsc_controller {
	struct shm_rsc_controller_dev hw_dev;
	uint32_t flags;

	uint32_t rsc_cnt;
	struct shm_gpio_rsc rsc[];
} __attribute__((packed));

struct shm_dma_channel_rsc {
	uint32_t channel_id;
	uint32_t owner_id;
} __attribute__((packed));

struct shm_dma_rsc_controller {
	struct shm_rsc_controller_dev hw_dev;
	uint32_t flags;

	uint32_t rsc_cnt;
	struct shm_dma_channel_rsc rsc[];
} __attribute__((packed));

#endif /* __ASRM_SHM_LAYOUT_H__ */
