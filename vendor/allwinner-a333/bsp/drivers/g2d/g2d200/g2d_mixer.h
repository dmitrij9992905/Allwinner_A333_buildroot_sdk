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
#ifndef _G2D_MIXER_H
#define _G2D_MIXER_H

#include "g2d_driver_i.h"
#include "g2d_platform.h"
#include "g2d_ovl_v.h"
#include "g2d_ovl_u.h"
#include "g2d_wb.h"
#include "g2d_bld.h"
#include "g2d_scal.h"
#include "g2d_top.h"

__s32 g2d_bsp_maskblt(struct g2d_frame *p_frame,
		g2d_image_enh *src, g2d_image_enh *ptn,
		g2d_image_enh *mask, g2d_image_enh *dst,
		__u32 back_flag, __u32 fore_flag);
__s32 g2d_fillrectangle(struct g2d_frame *p_frame,
		g2d_image_enh *dst, __u32 color_value);
bool g2d_bld_out_size_check(g2d_image_enh *src,
		g2d_image_enh *src2, g2d_image_enh *dst);
__s32 g2d_bsp_bld(struct g2d_frame *p_frame, g2d_image_enh *src,
		g2d_image_enh *src2, g2d_image_enh *dst,
		__u32 flag, g2d_ck *ck_para);
__s32 g2d_bsp_bitblt(struct g2d_frame *p_frame, g2d_image_enh *src,
		g2d_image_enh *dst, __u32 flag);

#endif /* End of file */
