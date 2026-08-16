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


#ifndef __ASRM_LOWLEVEL_H__
#define __ASRM_LOWLEVEL_H__

#include "asrm_common.h"
#include "asrm_port.h"

typedef sunxi_amp_rsc_type_t amp_rsc_type_t;
typedef sunxi_amp_rsc_user_id_t amp_rsc_user_id_t;
typedef sunxi_amp_rsc_user_type_t amp_rsc_user_type_t;


typedef uint32_t hw_rsc_user_group_id_t;
typedef amp_rsc_user_id_t hw_rsc_user_id_t;

typedef struct hw_rsc_user_group_info {
	hw_rsc_user_group_id_t id;
	uint32_t user_cnt;
	hw_rsc_user_id_t *user_id;
} hw_rsc_user_group_info_t;

typedef struct hw_peri_rsc_info {
	unsigned long start_addr;
	unsigned long len;
} hw_peri_rsc_info_t;

typedef struct hw_gpio_rsc_info {
	hw_peri_rsc_info_t peri;

	uint32_t gpio_id;
} hw_gpio_rsc_info_t;

typedef struct hw_dma_rsc_info {
	hw_peri_rsc_info_t peri;

	uint32_t channel_id;
} hw_dma_rsc_info_t;

typedef struct hw_rsc_info {
	amp_rsc_type_t type;
	union {
		hw_peri_rsc_info_t peri;
		hw_gpio_rsc_info_t gpio;
		hw_dma_rsc_info_t dma;
	};
} hw_rsc_info_t;

typedef struct hw_isolator_dev {
	/* Driver data, set and get with hw_isolator_dev_set_drvdata/hw_isolator_dev_get_drvdata */
	void *driver_data;
	/* controller ops, set with hw_isolator_dev_set_ops */
	const struct hw_isolator_dev_ops *ops;
} hw_isolator_dev_t;

typedef struct hw_isolator_dev_ops {
	int (*enable)(hw_isolator_dev_t *idev);
	int (*disable)(hw_isolator_dev_t *idev);
	int (*set_user_group)(hw_isolator_dev_t *idev, const hw_rsc_user_group_info_t *group_info);
	int (*get_user_group)(hw_isolator_dev_t *idev, hw_rsc_user_group_info_t *group_info);
	int (*set_resource_owner)(hw_isolator_dev_t *idev, const hw_rsc_info_t *rsc_info, uint32_t user_group_id);
	int (*get_resource_owner)(hw_isolator_dev_t *idev, const hw_rsc_info_t *rsc_info, uint32_t *user_group_id);
} hw_isolator_dev_ops_t;


typedef struct hw_isolator_dev_info {
    char compatible[64];
    const void *data;
} hw_isolator_dev_info_t;

typedef struct hw_isolator_driver {
    int (*probe)(hw_isolator_dev_t *idev);
    int (*remove)(hw_isolator_dev_t *idev);

    const hw_isolator_dev_info_t *match_table;
    struct list_head node;
} hw_isolator_driver_t;


int hw_isolator_driver_register(hw_isolator_driver_t *drv);

static inline void hw_isolator_dev_set_ops(hw_isolator_dev_t *idev, const struct hw_isolator_dev_ops *ops)
{
	idev->ops = ops;
}

static inline void hw_isolator_dev_set_drvdata(hw_isolator_dev_t *idev, void *data)
{
	idev->driver_data = data;
}

static inline void *hw_isolator_dev_get_drvdata(const hw_isolator_dev_t *idev)
{
    return idev->driver_data;
}

typedef struct reg_addr_info {
	unsigned long base_addr;
	uint32_t len;
} reg_addr_info_t;

int hw_isolator_dev_get_reg_addr_info(const hw_isolator_dev_t *idev, reg_addr_info_t *addr_info);
uint32_t hw_isolator_dev_get_user_group_num(const hw_isolator_dev_t *idev);

//#define ASRM_CORE_DEBUG

#ifdef ASRM_CORE_DEBUG
#define asrm_dbg_without_newline(fmt, ...) \
			asrm_printf(ASRM_LOG_COLOR_BLUE "[ASRM_D][%s:%d] " fmt \
				ASRM_LOG_COLOR_NONE, __func__, __LINE__, ##__VA_ARGS__)

#define asrm_dbg(fmt, ...) asrm_dbg_without_newline(fmt"\n", ##__VA_ARGS__)
#else
#define asrm_dbg_without_newline(fmt, ...)
#define asrm_dbg(fmt, args...)
#endif /* ASRM_CORE_DEBUG */

#define asrm_info_without_newline(fmt, ...) \
			asrm_printf(ASRM_LOG_COLOR_GREEN "[ASRM_I][%s:%d] " fmt \
				ASRM_LOG_COLOR_NONE, __func__, __LINE__, ##__VA_ARGS__)

#define asrm_info(fmt, ...) asrm_info_without_newline(fmt"\n", ##__VA_ARGS__)

#define asrm_warn_without_newline(fmt, ...) \
			asrm_printf(ASRM_LOG_COLOR_YELLOW "[ASRM_W][%s:%d] " fmt \
				ASRM_LOG_COLOR_NONE, __func__, __LINE__, ##__VA_ARGS__)

#define asrm_warn(fmt, ...) asrm_warn_without_newline(fmt"\n", ##__VA_ARGS__)

#define asrm_err_without_newline(fmt, ...) \
			asrm_printf(ASRM_LOG_COLOR_RED "[ASRM_E][%s:%d] " fmt \
				ASRM_LOG_COLOR_NONE, __func__, __LINE__, ##__VA_ARGS__)

#define asrm_err(fmt, ...) asrm_err_without_newline(fmt"\n", ##__VA_ARGS__)

#endif /* __ASRM_LOWLEVEL_H__ */
