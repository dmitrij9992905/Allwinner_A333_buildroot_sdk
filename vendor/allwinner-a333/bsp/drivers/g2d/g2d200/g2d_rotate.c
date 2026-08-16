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
#include "g2d_driver_i.h"
#include "g2d_rotate.h"
#include "g2d_top.h"
#include "g2d_platform.h"

void cal_uv_plane_rect(g2d_image_enh *img, int *cw, int *ch,
	int *cx, int *cy)
{
	if ((img->format >= G2D_FORMAT_YUV422UVC_V1U1V0U0)
		&& (img->format <= G2D_FORMAT_YUV422_PLANAR)) {
		*cw = img->width >> 1;
		*ch = img->height;
		*cx = img->clip_rect.x >> 1;
		*cy = img->clip_rect.y;
	} else if ((img->format >= G2D_FORMAT_YUV420UVC_V1U1V0U0)
		&& (img->format <= G2D_FORMAT_YUV420_PLANAR)) {
		*cw = img->width >> 1;
		*ch = img->height >> 1;
		*cx = img->clip_rect.x >> 1;
		*cy = img->clip_rect.y >> 1;
	} else if ((img->format >= G2D_FORMAT_YUV411UVC_V1U1V0U0)
		 && (img->format <= G2D_FORMAT_YUV411_PLANAR)) {
		*cw = img->width >> 2;
		*ch = img->height;
		*cx = img->clip_rect.x >> 2;
		*cy = img->clip_rect.y;
	} else {
		*cw = 0;
		*ch = 0;
		*cx = 0;
		*cy = 0;
	}
}

__s32 lbc_calc_ctrl(struct rot_submodule *p_rot, u32 flag, g2d_fmt_enh src_fmt,
	g2d_fmt_enh dst_fmt, u32 frm_width, u32 frm_height, u32 cmp_ratio,
	u32 enc_is_lossy, u32 dec_is_lossy)
{
	u32 seg_width = 16, seg_height = 4;
	u32 bit_depth = 8;
	u32 enc_c_ratio = 333;
	u32 ALIGN = 128;
	u32 seg_tar_bits, seg_tar_bits_y, seg_tar_bits_c, dec_segline_tar_bits, enc_segline_tar_bits;
	u32 y_mode_bits, c_mode_bits, y_data_bits, c_data_bits;
	struct g2d_rot_reg *p_reg = NULL;
	if (p_rot) {
		p_reg = p_rot->get_reg(p_rot);
		if (!p_reg)
			return -1;
	}

	if ((src_fmt != G2D_FORMAT_YUV420_PLANAR)
		|| (dst_fmt != G2D_FORMAT_YUV420_PLANAR)) {
		G2D_WARN("LBC only support YUV420 plannar fmt\n");
		return -1;
	}
	if (((flag & 0xff00) == G2D_ROT_180) || ((flag & 0xff00) == G2D_ROT_V)) {
		G2D_WARN("LBC not support 180 and V flip\n");
		return -1;
	}
	if (enc_is_lossy) {
		seg_tar_bits = ((seg_width * seg_height * bit_depth * cmp_ratio * 3 / 2000) / ALIGN) * ALIGN;
		seg_tar_bits_y = seg_tar_bits * (1024 - enc_c_ratio) / 1024;
		seg_tar_bits_c = seg_tar_bits - seg_tar_bits_y;
		dec_segline_tar_bits = ((frm_width + seg_width - 1) / seg_width) * seg_tar_bits;
		if (((flag & 0xf00) == G2D_ROT_90) || ((flag & 0xf00) == G2D_ROT_270))
			enc_segline_tar_bits = ((frm_height + seg_width - 1) / seg_width) * seg_tar_bits;
		else
			enc_segline_tar_bits = dec_segline_tar_bits;
	} else {
		y_mode_bits = seg_width / 8 * (3 * 2 + 2);
		c_mode_bits = 2 * (seg_width / 2 / 8 * 2);
		y_data_bits = seg_width * seg_height * bit_depth;
		c_data_bits = seg_width * seg_height * bit_depth / 2 + 2 * (seg_width / 2 / 8) * 4;
		seg_tar_bits = (y_data_bits + c_data_bits + y_mode_bits + c_mode_bits + ALIGN - 1) / ALIGN * ALIGN;
		seg_tar_bits_y = seg_tar_bits;
		seg_tar_bits_c = 0;
		dec_segline_tar_bits = ((frm_width + seg_width - 1) / seg_width) * seg_tar_bits;
		if (((flag & 0xf00) == G2D_ROT_90) || ((flag & 0xf00) == G2D_ROT_270))
			enc_segline_tar_bits = ((frm_height + seg_width - 1) / seg_width) * seg_tar_bits;
		else
			enc_segline_tar_bits = dec_segline_tar_bits;
	}
	if ((flag & 0xf00) == G2D_ROT_0)
		p_reg->lbc_ctl.bits.lbc_rot_angle = 0x0;
	if ((flag & 0xf00) == G2D_ROT_90)
		p_reg->lbc_ctl.bits.lbc_rot_angle = 0x1;
	if ((flag & 0xf00) == G2D_ROT_270)
		p_reg->lbc_ctl.bits.lbc_rot_angle = 0x2;
	if (flag & G2D_ROT_H)
		p_reg->lbc_ctl.bits.lbc_rot_angle = 0x3;
	p_reg->lbc_enc_ctl.bits.enc_is_lossy = enc_is_lossy;
	p_reg->lbc_enc_ctl.bits.enc_seg_rc_en = 0x1; /* only 1 frame not care use even */
	p_reg->lbc_enc_ctl.bits.enc_c_ratio = enc_c_ratio & 0x3ff;
	p_reg->lbc_enc_ctl.bits.enc_segline_tar_bits = enc_segline_tar_bits & 0x1ffff;
	p_reg->lbc_enc_ctl.bits.g2d_lbc_en = 0x1; /* only 1 frame not care use even */
	p_reg->lbc_ctl.bits.seg_tar_bits_c = seg_tar_bits_c & 0x7ff;
	p_reg->lbc_ctl.bits.seg_tar_bits_y = seg_tar_bits_y & 0x7ff;
	p_reg->lbc_dec_ctl.bits.dec_is_lossy = dec_is_lossy;
	p_reg->lbc_dec_ctl.bits.dec_segline_tar_bits = dec_segline_tar_bits & 0x1ffff;

	return 0;
}

__s32 g2d_rot_set(struct rot_submodule *p_rot, g2d_image_enh *src, g2d_image_enh *dst, __u32 flag,
	bool is_lbc, __u32 lbc_cmp_ratio, bool enc_is_lossy, bool dec_is_lossy)
{
	__u32 ch, cw, cy, cx;
	__u32 ycnt, ucnt, vcnt;
	__s32 ret = 0;
	__u32 pitch0, pitch1, pitch2;
	__u64 addr0, addr1, addr2;
	struct g2d_rot_reg *p_reg = NULL;
	if (p_rot) {
		p_reg = p_rot->get_reg(p_rot);
		if (p_reg == NULL)
			return -1;
	} else
		return -1;

	/* rotate use same format */
	dst->format = src->format;
	p_reg->rot_ctrl.bits.mode_sel = 1;
	if (!is_lbc) {
		if (flag & G2D_ROT_H)
			p_reg->rot_ctrl.bits.hflip_en = 1;
		if (flag & G2D_ROT_V)
			p_reg->rot_ctrl.bits.vflip_en = 1;
		if ((flag & 0xf00) == G2D_ROT_0)
			p_reg->rot_ctrl.bits.degree = 0;
		if ((flag & 0xf00) == G2D_ROT_90)
			p_reg->rot_ctrl.bits.degree = 1;
		if ((flag & 0xf00) == G2D_ROT_180)
			p_reg->rot_ctrl.bits.degree = 2;
		if ((flag & 0xf00) == G2D_ROT_270)
			p_reg->rot_ctrl.bits.degree = 3;
	}

	p_reg->in_fmt.bits.fmt = src->format & 0x3F;
	p_reg->in_size.bits.width = (src->clip_rect.w - 1) & 0x1fff;
	p_reg->in_size.bits.height = (src->clip_rect.h - 1) & 0x1fff;
	cal_uv_plane_rect(src, &cw, &ch, &cx, &cy);
	g2d_byte_cal(src->format, &ycnt, &ucnt, &vcnt);
	pitch0 = cal_align(ycnt * src->width, src->align[0]);
	pitch1 = cal_align(ucnt * cw, src->align[1]);
	pitch2 = cal_align(vcnt * cw, src->align[2]);
	addr0 = src->laddr[0] + ((__u64)src->haddr[0] << 32) +
		pitch0 * src->clip_rect.y + ycnt * src->clip_rect.x;
	addr1 = src->laddr[1] + ((__u64)src->haddr[1] << 32) + pitch1 * cy +
		ucnt * cx;
	addr2 = src->laddr[2] + ((__u64)src->haddr[2] << 32) + pitch2 * cy +
		vcnt * cx;
	if (addr0 % 4 != 0 || addr1 % 4 != 0 || addr2 % 4 != 0) {
		G2D_ERR("rotate output addr should be 4 bytes align\n");
		return -1;
	}
	p_reg->in_pitch0 = pitch0;
	p_reg->in_pitch1 = pitch1;
	p_reg->in_pitch2 = pitch2;
	p_reg->in_laddr0 = addr0 & 0xffffffff;
	p_reg->in_haddr0 = (addr0 >> 32) & 0xff;
	p_reg->in_laddr1 = addr1 & 0xffffffff;
	p_reg->in_haddr1 = (addr1 >> 32) & 0xff;
	p_reg->in_laddr2 = addr2 & 0xffffffff;
	p_reg->in_haddr2 = (addr2 >> 32) & 0xff;
	G2D_PARA_DBG("ROT input info: ----------------------------\n");
	G2D_PARA_DBG("ROT_InPITCH: %d, %d, %d\n",
			pitch0, pitch1, pitch2);
	G2D_PARA_DBG("SRC_ADDR0: 0x%llx\n", addr0);
	G2D_PARA_DBG("SRC_ADDR1: 0x%llx\n", addr1);
	G2D_PARA_DBG("SRC_ADDR2: 0x%llx\n", addr2);

	if (((flag & 0xf00) == G2D_ROT_90) || ((flag & 0xf00) == G2D_ROT_270)) {
		dst->clip_rect.w = src->clip_rect.h;
		dst->clip_rect.h = src->clip_rect.w;
	} else {
		dst->clip_rect.w = src->clip_rect.w;
		dst->clip_rect.h = src->clip_rect.h;
	}
	p_reg->out_size.bits.width = (dst->clip_rect.w - 1) & 0x1fff;
	p_reg->out_size.bits.height = (dst->clip_rect.h - 1) & 0x1fff;

	cal_uv_plane_rect(dst, &cw, &ch, &cx, &cy);
	g2d_byte_cal(dst->format, &ycnt, &ucnt, &vcnt);
	pitch0 = cal_align(ycnt * dst->width, dst->align[0]);
	pitch1 = cal_align(ucnt * cw, dst->align[1]);
	pitch2 = cal_align(vcnt * cw, dst->align[2]);
	addr0 = dst->laddr[0] + ((__u64)dst->haddr[0] << 32) +
		pitch0 * dst->clip_rect.y + ycnt * dst->clip_rect.x;
	addr1 = dst->laddr[1] + ((__u64)dst->haddr[1] << 32) + pitch1 * cy +
		ucnt * cx;
	addr2 = dst->laddr[2] + ((__u64)dst->haddr[2] << 32) + pitch2 * cy +
		vcnt * cx;
	if (addr0 % 4 != 0 || addr1 % 4 != 0 || addr2 % 4 != 0) {
		G2D_ERR("rotate output addr should be 4 bytes align\n");
		return -1;
	}
	if (pitch0 % 8 != 0 || pitch1 % 8 != 0 || pitch2 % 8 != 0) {
		G2D_ERR("rotate output pitch should be 8 bytes align\n");
		return -1;
	}
	p_reg->out_pitch0 = pitch0;
	p_reg->out_pitch1 = pitch1;
	p_reg->out_pitch2 = pitch2;
	p_reg->out_laddr0 = addr0 & 0xffffffff;
	p_reg->out_haddr0 = (addr0 >> 32) & 0xff;
	p_reg->out_laddr1 = addr1 & 0xffffffff;
	p_reg->out_haddr1 = (addr1 >> 32) & 0xff;
	p_reg->out_laddr2 = addr2 & 0xffffffff;
	p_reg->out_haddr2 = (addr2 >> 32) & 0xff;
	G2D_PARA_DBG("ROT output info: ----------------------------\n");
	G2D_PARA_DBG("ROT_OutPITCH: %d, %d, %d\n",
			pitch0, pitch1, pitch2);
	G2D_PARA_DBG("DST_ADDR0: 0x%llx\n", addr0);
	G2D_PARA_DBG("DST_ADDR1: 0x%llx\n", addr1);
	G2D_PARA_DBG("DST_ADDR2: 0x%llx\n", addr2);

	/* lbc */
	if (is_lbc) {
		ret = lbc_calc_ctrl(p_rot, flag, src->format, dst->format, src->width, src->height,
			lbc_cmp_ratio, (u32) enc_is_lossy, (u32) dec_is_lossy);
	}
	p_rot->set_block_dirty(p_rot, 0, 1);
	return ret;
}

static int rot_rcq_setup(struct rot_submodule *p_rot, u8 __iomem *base,
			struct g2d_rcq_mem_info *p_rcq_info)
{
	u8 __iomem *reg_base = base + G2D_ROT;
	int ret = 0;

	if (!p_rot) {
		G2D_WARN("Null pointer\n");
		goto OUT;
	}

	p_rot->reg_info->size = sizeof(struct g2d_rot_reg);
	p_rot->reg_info->vir_addr = (u8 *)g2d_top_reg_memory_alloc(
	    p_rot->reg_info->size, (void *)&(p_rot->reg_info->phy_addr),
	    p_rcq_info);

	if (!p_rot->reg_info->vir_addr) {
		G2D_WARN("Malloc writeback reg rcq memory fail\n");
		ret = -1;
		goto OUT;
	}

	p_rot->reg_blks->vir_addr = p_rot->reg_info->vir_addr;
	p_rot->reg_blks->phy_addr = p_rot->reg_info->phy_addr;
	p_rot->reg_blks->size = p_rot->reg_info->size;
	p_rot->reg_blks->reg_addr = reg_base;

OUT:
	return ret;
}

static __u32 rot_get_reg_block_num(struct rot_submodule *p_rot)
{
	if (p_rot)
		return p_rot->reg_blk_num;
	return 0;
}

static __s32 rot_get_reg_block(struct rot_submodule *p_rot,
	struct g2d_reg_block **blks)
{
	__s32 i = 0, ret = -1;

	if (p_rot) {
		for (i = 0; i < p_rot->reg_blk_num; ++i)
			blks[i] = p_rot->reg_blks + i;
		ret = 0;
	}

	return ret;
}

static struct g2d_rot_reg *rot_get_reg(struct rot_submodule *p_rot)
{
#ifdef G2D_ROT_RCQ_USED
	return (struct g2d_rot_reg *)(p_rot->reg_blks->vir_addr);
#else
	return (struct g2d_rot_reg *)(p_rot->reg_blks->reg_addr);
#endif
	return NULL;
}


static void rot_set_block_dirty(struct rot_submodule *p_rot, __u32 blk_id, __u32 dirty)
{
#ifdef G2D_ROT_RCQ_USED
	if (p_rot && p_rot->reg_blks->rcq_hd)
		p_rot->reg_blks->rcq_hd->dirty.bits.dirty = dirty;
	else
		G2D_WARN("Null pointer\n");
#else
	if (p_rot)
		p_rot->reg_blks->dirty = dirty;
	else
		G2D_WARN("Null pointer\n");
#endif
}

static __u32 rot_get_rcq_mem_size(struct rot_submodule *p_rot)
{
	return G2D_RCQ_BYTE_ALIGN(sizeof(struct g2d_rot_reg));
}

static __s32 rot_destory(struct rot_submodule *p_rot)
{
	__s32 ret = -1;

	if (p_rot) {
		kfree(p_rot->reg_blks);
		p_rot->reg_blks = NULL;
		kfree(p_rot->reg_info);
		p_rot->reg_info = NULL;
		ret = 0;
		kfree(p_rot);
	}

	return ret;
}

struct rot_submodule *g2d_rot_submodule_setup(struct g2d_frame *p_frame)
{
	struct rot_submodule *p_rot = NULL;

	p_rot = kzalloc(sizeof(*p_rot), GFP_KERNEL | __GFP_ZERO);
	if (!p_rot) {
		G2D_ERR("Kmalloc wb submodule fail\n");
		return NULL;
	}

	p_rot->rcq_setup = rot_rcq_setup;
	p_rot->reg_blk_num = 1;
	p_rot->get_reg_block_num = rot_get_reg_block_num;
	p_rot->get_reg_block = rot_get_reg_block;
	p_rot->get_reg = rot_get_reg;
	p_rot->set_block_dirty = rot_set_block_dirty;
	p_rot->get_rcq_mem_size = rot_get_rcq_mem_size;
	p_rot->destory = rot_destory;

	p_rot->reg_blks =
	    kzalloc(sizeof(*(p_rot->reg_blks)) * p_rot->reg_blk_num,
		    GFP_KERNEL | __GFP_ZERO);
	p_rot->reg_info =
	    kzalloc(sizeof(*(p_rot->reg_info)), GFP_KERNEL | __GFP_ZERO);

	if (!p_rot->reg_blks || !p_rot->reg_info) {
		G2D_ERR("Kmalloc wb reg info fail\n");
		goto FREE_ROT;
	}

	return p_rot;
FREE_ROT:
	kfree(p_rot->reg_blks);
	kfree(p_rot->reg_info);
	kfree(p_rot);

	return NULL;
}
