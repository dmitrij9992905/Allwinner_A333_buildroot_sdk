/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Copyright(c) 2020 - 2023 Allwinner Technology Co.,Ltd. All rights reserved. */
/*
 * Copyright (c) 2007-2019 Allwinnertech Co., Ltd.
 * Author: zhengxiaobin <zhengxiaobin@allwinnertech.com>
 *
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#ifndef __G2D_DEBUG_H__
#define __G2D_DEBUG_H__

#include "g2d_driver_i.h"
#include <linux/types.h>
#include <sunxi-log.h>

#define REG_INTERVAL 0x04
#define HEXADECIMAL  0x10
#define REG_CL       0x0c

extern unsigned int loglevel;
extern unsigned int loglevel;
#define G2D_DRV_DBG(format, args...) \
	do { \
		if (loglevel & 0x1) { \
			sunxi_info(NULL, "[G2D]: " format, ## args); \
		} else { \
			sunxi_debug(NULL, "[G2D]: " format, ## args); \
		} \
	} while (0)

#define G2D_PARA_DBG(format, args...) \
	do { \
		if (loglevel & 0x2) { \
			sunxi_info(NULL, "[G2D]: " format, ## args); \
		} else { \
			sunxi_debug(NULL, "[G2D]: " format, ## args); \
		} \
	} while (0)
#define G2D_REG_DBG(format, args...) \
	do { \
		if (loglevel & 0x4) { \
			sunxi_info(NULL, "[G2D]: " format, ## args); \
		} else { \
			sunxi_debug(NULL, "[G2D]: " format, ## args); \
		} \
	} while (0)

#define G2D_ERR(format, args...) \
	sunxi_err(NULL, "[G2D]: " format, ## args)
#define G2D_WARN(format, args...) \
	sunxi_warn(NULL, "[G2D]: " format, ## args)
#define G2D_INFO(format, args...) \
	sunxi_info(NULL, "[G2D]: " format, ## args)

struct g2d_time_info {
	__u8 dump_time_info_en;
	struct timespec64 ctr_start_ts;
	struct timespec64 acq_lock_ts;
	struct timespec64 dma_map_start_ts;
	struct timespec64 dma_map_end_ts;
	struct timespec64 dma_unmap_start_ts;
	struct timespec64 dma_unmap_end_ts;
	struct timespec64 hw_proc_start_ts;
	struct timespec64 hw_proc_end_ts;
	struct timespec64 ctr_end_ts;
};

unsigned int __get_ts_diff(struct timespec64 ts_start, struct timespec64 ts_end);
void g2d_dump_reg(phys_addr_t phys_base, void __iomem *io_addr);
void dump_g2d_bld_info(const g2d_bld *g2d_bld_para);
void dump_mixer_para_info(const mixer_para *mixer_paras);
void dump_g2d_blt_h_info(const g2d_blt_h *g2d_blt_h_para);
void dump_g2d_lbc_rot_info(const g2d_lbc_rot *g2d_lbc_rot_para);
void dump_g2d_fillrect_h_info(const g2d_fillrect_h *g2d_fillrect_h_para);
void dump_g2d_maskblt_info(const g2d_maskblt *g2d_maskblt_para);

#endif  /* __G2D_DEBUG_H__ */
