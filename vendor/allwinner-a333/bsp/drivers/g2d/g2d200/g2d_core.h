/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Copyright(c) 2020 - 2023 Allwinner Technology Co.,Ltd. All rights reserved. */
/*
 * Copyright (c) 2007-2019 Allwinnertech Co., Ltd.
 * Author: zhengxiaobin <zhengxiaobin@allwinnertech.com>
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
#ifndef _G2D_CORE_TYPE_H
#define _G2D_CORE_TYPE_H

#include "g2d_platform.h"
#include "g2d_rcq.h"

union g2d_core_ctl {
	unsigned int dwval;
	struct {
		unsigned int mixer_en:1;
		unsigned int res0:3;
		unsigned int rotate_en:1;
		unsigned int res1:3;
		unsigned int scan_order:1;
		unsigned int res2:23;
	} bits;
};

struct g2d_core_ctl_reg {
	union g2d_core_ctl core_ctl;
};

struct g2d_frame;
struct core_submodule {
	struct g2d_reg_block *reg_blks;
	__u32 reg_blk_num;
	struct g2d_reg_mem_info *reg_info;
	__s32 (*destory)(struct core_submodule *p_core);
	__s32 (*apply)(struct core_submodule *p_core, g2d_image_enh *p_image);
	__s32 (*rcq_setup)(struct core_submodule *p_core, u8 __iomem *base,
			   struct g2d_rcq_mem_info *p_rcq_info);
	__u32 (*get_reg_block_num)(struct core_submodule *p_core);
	__u32 (*get_rcq_mem_size)(struct core_submodule *p_core);
	__s32 (*get_reg_block)(struct core_submodule *p_core, struct g2d_reg_block **blks);
	struct g2d_core_ctl_reg  *(*get_reg)(struct core_submodule *p_core);
	void (*set_block_dirty)(struct core_submodule *p_core, __u32 blk_id, __u32 dirty);
};

void core_ctl_set_mixer_en(struct core_submodule *p_core, int en);
void core_ctl_set_rotate_en(struct core_submodule *p_core, int en);
void core_ctl_set_scan_order(struct core_submodule *p_core,
	unsigned int scan_order);
struct core_submodule *
g2d_core_submodule_setup(struct g2d_frame *p_frame);

#endif /* End of file */
