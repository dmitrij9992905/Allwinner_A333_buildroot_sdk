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
#ifndef _G2D_ROTATE_H
#define _G2D_ROTATE_H
#include <linux/types.h>
#include <uapi/linux/sunxi-g2d.h>
#include "g2d_rotate_type.h"
#include "g2d_platform.h"
#include "g2d_rcq.h"

struct g2d_frame;
struct rot_submodule {
	struct g2d_reg_block *reg_blks;
	__u32 reg_blk_num;
	struct g2d_reg_mem_info *reg_info;
	__s32 (*destory)(struct rot_submodule *p_rot);
	__s32 (*apply)(struct rot_submodule *p_rot, g2d_image_enh *src, g2d_image_enh *dst, __u32 flag);
	__s32 (*rcq_setup)(struct rot_submodule *p_rot, u8 __iomem *base,
			struct g2d_rcq_mem_info *p_rcq_info);
	__u32 (*get_reg_block_num)(struct rot_submodule *p_rot);
	__u32 (*get_rcq_mem_size)(struct rot_submodule *p_rot);
	__s32 (*get_reg_block)(struct rot_submodule *p_rot, struct g2d_reg_block **blks);
	struct g2d_rot_reg *(*get_reg)(struct rot_submodule *p_rot);
	void (*set_block_dirty)(struct rot_submodule *p_rot, __u32 blk_id, __u32 dirty);
};

struct rot_submodule *g2d_rot_submodule_setup(struct g2d_frame *p_frame);

__s32 g2d_rot_set(struct rot_submodule *p_rot, g2d_image_enh *src, g2d_image_enh *dst, __u32 flag,
				  bool is_lbc, __u32 lbc_cmp_ratio, bool enc_is_lossy, bool dec_is_lossy);

#endif /* End of file */
