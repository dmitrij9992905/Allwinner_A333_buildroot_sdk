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
#include "g2d_mixer.h"
#include "g2d_platform.h"
#include <linux/idr.h>
#include <linux/version.h>
MODULE_IMPORT_NS(VFS_internal_I_am_really_a_filesystem_and_am_NOT_a_driver);

__s32 g2d_bsp_maskblt(struct g2d_frame *p_frame,
			     g2d_image_enh *src, g2d_image_enh *ptn,
			     g2d_image_enh *mask, g2d_image_enh *dst,
			     __u32 back_flag, __u32 fore_flag)
{
	g2d_rect rect0;
	__s32 ret = -1;

	if (!dst) {
		G2D_WARN("Null pointer\n");
		goto OUT;
	}

	if (dst->format > G2D_FORMAT_BGRA1010102) {
		G2D_WARN("Un support out format:%d\n", dst->format);
		goto OUT;
	}
	g2d_vlayer_set(p_frame->ovl_v, 0, dst);

	if (src) {
		src->clip_rect.w = dst->clip_rect.w;
		src->clip_rect.h = dst->clip_rect.h;
		g2d_uilayer_set(p_frame->ovl_u, 0, src);
	}
	if (ptn) {
		ptn->clip_rect.w = dst->clip_rect.w;
		ptn->clip_rect.h = dst->clip_rect.h;
		g2d_uilayer_set(p_frame->ovl_u, 1, ptn);
	}

	if (mask != NULL) {
		mask->clip_rect.w = dst->clip_rect.w;
		mask->clip_rect.h = dst->clip_rect.h;
		g2d_uilayer_set(p_frame->ovl_u, 2, mask);

		/* set the ROP4 */
		bld_set_rop_ctrl(p_frame->bld, 0x1);
		bld_rop3_set(p_frame->bld, 0, back_flag & 0xff);
		bld_rop3_set(p_frame->bld, 1, fore_flag & 0xff);
	} else {
		bld_set_rop_ctrl(p_frame->bld, 0x0);
		bld_rop3_set(p_frame->bld, 0, back_flag);
	}

	rect0.x = 0;
	rect0.y = 0;
	rect0.w = dst->clip_rect.w;
	rect0.h = dst->clip_rect.h;
	bld_in_set(p_frame->bld, 0, rect0, dst->bpremul);
	bld_out_setting(p_frame->bld, dst);
	g2d_wb_set(p_frame->wb, dst);
	ret = 0;
OUT:
	return ret;

}

__s32 g2d_fillrectangle(struct g2d_frame *p_frame,
				   g2d_image_enh *dst, __u32 color_value)
{
	g2d_rect rect0;
	__s32 ret = -1;

	if (!dst || !p_frame) {
		G2D_WARN("Null pointer\n");
		goto OUT;
	}

	/* set the input layer */
	g2d_vlayer_set(p_frame->ovl_v, 0, dst);
	/* set the fill color value */
	g2d_ovl_v_fc_set(p_frame->ovl_v, color_value);

	if (dst->format >= G2D_FORMAT_IYUV422_V0Y1U0Y0) {
		g2d_vsu_para_set(p_frame->scal, dst->format, dst->clip_rect.w,
				  dst->clip_rect.h, dst->clip_rect.w,
				  dst->clip_rect.h, 0xff);
		if (dst->gamut == G2D_BT601)
			bld_csc_reg_set(p_frame->bld, 1, G2D_RGB2YUV_601, 0, dst->color_range);
		else
			bld_csc_reg_set(p_frame->bld, 1, G2D_RGB2YUV_709, 0, dst->color_range);
	}

	/* for interleaved test */
	if ((dst->format >= G2D_FORMAT_IYUV422_V0Y1U0Y0)
			&& (dst->format <= G2D_FORMAT_IYUV422_Y1U0Y0V0)) {
		if (dst->gamut == G2D_BT601) {
			bld_csc_reg_set(p_frame->bld, 0, G2D_RGB2YUV_601, 0, dst->color_range);
			bld_csc_reg_set(p_frame->bld, 1, G2D_RGB2YUV_601, 0, dst->color_range);
			bld_csc_reg_set(p_frame->bld, 2, G2D_RGB2YUV_601, 0, dst->color_range);
		} else {
			bld_csc_reg_set(p_frame->bld, 0, G2D_RGB2YUV_709, 0, dst->color_range);
			bld_csc_reg_set(p_frame->bld, 1, G2D_RGB2YUV_709, 0, dst->color_range);
			bld_csc_reg_set(p_frame->bld, 2, G2D_RGB2YUV_709, 0, dst->color_range);
		}

		bld_bk_set(p_frame->bld, 0xff123456);
		bld_porter_duff(p_frame->bld, G2D_BLD_SRCOVER);

		g2d_ovl_u_fc_set(p_frame->ovl_u, 0, 0xffffffff);
		g2d_ovl_u_fc_set(p_frame->ovl_u, 1, 0xffffffff);
	}

	rect0.x = 0;
	rect0.y = 0;
	rect0.w = dst->clip_rect.w;
	rect0.h = dst->clip_rect.h;
	bld_in_set(p_frame->bld, 0, rect0, dst->bpremul);
	bld_cs_set(p_frame->bld, dst->format);

	/* ROP sel ch0 pass */
	bld_set_rop_ctrl(p_frame->bld, 0xf0);
	bld_out_setting(p_frame->bld, dst);
	g2d_wb_set(p_frame->wb, dst);

	ret = 0;
OUT:
	return ret;
}

bool g2d_bld_out_size_check(g2d_image_enh *src,
		g2d_image_enh *src2, g2d_image_enh *dst)
{
	if (dst->clip_rect.w < src->resize.w
		|| dst->clip_rect.w < src2->clip_rect.w
		|| dst->clip_rect.h < src->resize.h
		|| dst->clip_rect.h < src2->clip_rect.h)
		return false;

	return true;
}

__s32 g2d_bsp_bld(struct g2d_frame *p_frame, g2d_image_enh *src,
				g2d_image_enh *src2, g2d_image_enh *dst,
				__u32 flag, g2d_ck *ck_para)
{
	g2d_rect rect0, rect1;
	g2d_coor coor;
	__s32 ret = -1;
	__u32 midw, midh;
	g2d_image_enh param_image;

	if (!dst || !src || !p_frame || !ck_para) {
		G2D_WARN("Null pointer\n");
		goto OUT;
	}

	if (g2d_bld_out_size_check(src, src2, dst) == false) {
		G2D_WARN("[BLD] size is not suitable\n");
		goto OUT;
	}

	g2d_vlayer_set(p_frame->ovl_v, 0, src);
	/**
	 * Do not use the channel's coor, channel.
	 * There is only one layer, which is meaningless and will increase the burden of scale.
	*/
	coor.x = 0;
	coor.y = 0;
	g2d_vlayer_overlay_set(p_frame->ovl_v, 0, &coor,
			src->clip_rect.w, src->clip_rect.h);

	g2d_uilayer_set(p_frame->ovl_u, 2, src2);
	g2d_uilayer_overlay_set(p_frame->ovl_u, 2, &coor,
			src2->clip_rect.w, src2->clip_rect.h);

	if (src->bbuff == 0)
		g2d_ovl_v_fc_set(p_frame->ovl_v, src->color);
	if (src2->bbuff == 0)
		g2d_ovl_u_fc_set(p_frame->ovl_u, 2, src2->color);

	if (src->format > G2D_FORMAT_BGRA1010102) {
		if (src2->format > G2D_FORMAT_BGRA1010102) {
			G2D_WARN("[BLD] not support two yuv layer\n");
			goto OUT;
		} else {
			if (src->resize.w == 0 || src->resize.h) {
				G2D_WARN("The resize of the VI channel should not be set to 0 \n");
				goto OUT;
			}
			g2d_ovl_v_calc_coarse(p_frame->ovl_v, src->format, src->clip_rect.w,
						src->clip_rect.h, src->resize.w,
						src->resize.h, &midw, &midh);
			g2d_vsu_para_set(p_frame->scal, src->format, midw, midh,
					src->resize.w, src->resize.h, dst->alpha);
			/**
			 * Only use csc0.
			 */
			switch (dst->gamut) {
			case G2D_BT601: {
				bld_csc_reg_set(p_frame->bld, 0, G2D_YUV2RGB_601, src->color_range, dst->color_range);
				break;
			}
			case G2D_BT709: {
				bld_csc_reg_set(p_frame->bld, 0, G2D_YUV2RGB_709, src->color_range, dst->color_range);
				break;
			}
			case G2D_BT2020: {
				bld_csc_reg_set(p_frame->bld, 0, G2D_YUV2RGB_2020, src->color_range, dst->color_range);
				break;
			}
			}

		}
	} else {
		if (src2->format > G2D_FORMAT_BGRA1010102) {
			G2D_WARN("[BLD] please use ch0(src0) to set YUV layer\n");
			goto OUT;
		}

		/* do scale */
		if (src->resize.w != 0 && src->resize.h != 0) {
			g2d_ovl_v_calc_coarse(p_frame->ovl_v, src->format, src->clip_rect.w,
						src->clip_rect.h, src->resize.w,
						src->resize.h, &midw, &midh);

			g2d_vsu_para_set(p_frame->scal, src->format, midw, midh,
						src->resize.w, src->resize.h, src->alpha);
		}
	}

	if (dst->format > G2D_FORMAT_BGRA1010102) {
		switch (dst->gamut) {
		case G2D_BT601: {
			bld_csc_reg_set(p_frame->bld, 2, G2D_RGB2YUV_601, src->color_range, dst->color_range);
			break;
		}
		case G2D_BT709: {
			bld_csc_reg_set(p_frame->bld, 2, G2D_RGB2YUV_709, src->color_range, dst->color_range);
			break;
		}
		case G2D_BT2020: {
			bld_csc_reg_set(p_frame->bld, 2, G2D_RGB2YUV_2020, src->color_range, dst->color_range);
			break;
		}
		}
	}

	bld_set_rop_ctrl(p_frame->bld, 0xf0);
	rect0.x = src->coor.x;
	rect0.y = src->coor.y;
	rect0.w = src->resize.w;
	rect0.h = src->resize.h;

	rect1.x = src2->coor.x;
	rect1.y = src2->coor.y;
	rect1.w = src2->clip_rect.w;
	rect1.h = src2->clip_rect.h;

	bld_in_set(p_frame->bld, 0, rect0, src->bpremul);
	bld_in_set(p_frame->bld, 1, rect1, src2->bpremul);

	if (flag == 0) {
		/* flag not set use default */
		flag = G2D_BLD_SRCOVER;
	}

	bld_porter_duff(p_frame->bld, flag & 0xFFF);

	bld_ck_para_set(p_frame->bld, ck_para, flag);

	bld_cs_set(p_frame->bld, G2D_FORMAT_BGRA1010102);

	// calculating output size after bld.
	param_image.bpremul = dst->bpremul;
	param_image.clip_rect.w = (rect0.x + rect0.w) > (rect1.x + rect1.w)
					? (rect0.x + rect0.w) : (rect1.x + rect1.w);
	param_image.clip_rect.h = (rect0.y + rect0.h) > (rect1.y + rect1.h)
					? (rect0.y + rect0.h) : (rect1.y + rect1.h);
	bld_out_setting(p_frame->bld, &param_image);

	g2d_wb_set(p_frame->wb, dst);

	ret = 0;
OUT:
	return ret;
}

__s32 g2d_bsp_bitblt(struct g2d_frame *p_frame, g2d_image_enh *src,
			   g2d_image_enh *dst, __u32 flag)
{
	g2d_rect rect0, rect1;
	bool bpre;
	__u32 midw, midh;
	__s32 ret = -1;

	if (!p_frame || !src || !dst)
		goto OUT;

	if ((flag & 0x0fffffff) == G2D_BLT_NONE) {
		g2d_vlayer_set(p_frame->ovl_v, 0, src);
		/* need abp process */
		if (src->mode)
			g2d_uilayer_set(p_frame->ovl_u, 2,
					dst);
		if ((src->format >= G2D_FORMAT_IYUV422_V0Y1U0Y0) ||
		    (src->clip_rect.w != dst->clip_rect.w) ||
		    (src->clip_rect.h != dst->clip_rect.h)) {
			g2d_ovl_v_calc_coarse(
			    p_frame->ovl_v, src->format, src->clip_rect.w,
			    src->clip_rect.h, dst->clip_rect.w,
			    dst->clip_rect.h, &midw, &midh);
			g2d_vsu_para_set(p_frame->scal, src->format, midw, midh,
					 dst->clip_rect.w, dst->clip_rect.h,
					 0xff);
		}
		bld_porter_duff(p_frame->bld, G2D_BLD_SRCOVER);
		/* Default value */
		bld_set_rop_ctrl(p_frame->bld, 0xf0);
		rect0.x = 0;
		rect0.y = 0;
		rect0.w = dst->clip_rect.w;
		rect0.h = dst->clip_rect.h;
		bld_in_set(p_frame->bld, 0, rect0, dst->bpremul);
		bld_cs_set(p_frame->bld, src->format);
		if (src->mode) {
			/* need abp process */
			rect1.x = 0;
			rect1.y = 0;
			rect1.w = dst->clip_rect.w;
			rect1.h = dst->clip_rect.h;
			bld_in_set(p_frame->bld, 1, rect1, dst->bpremul);
		}
		if ((src->format <= G2D_FORMAT_BGRA1010102) &&
				(dst->format > G2D_FORMAT_BGRA1010102)) {
			if (dst->gamut == G2D_BT601) {
				bld_csc_reg_set(p_frame->bld, 2, G2D_RGB2YUV_601,
						src->color_range, dst->color_range);
			} else {
				bld_csc_reg_set(p_frame->bld, 2, G2D_RGB2YUV_709,
						src->color_range, dst->color_range);
			}
		}
		if ((src->format > G2D_FORMAT_BGRA1010102) &&
				(dst->format <= G2D_FORMAT_BGRA1010102)) {
			if (dst->gamut == G2D_BT601) {
				bld_csc_reg_set(p_frame->bld, 2, G2D_YUV2RGB_601,
						src->color_range, dst->color_range);
			} else {
				bld_csc_reg_set(p_frame->bld, 2, G2D_YUV2RGB_709,
						src->color_range, dst->color_range);
			}
		}

		bld_out_setting(p_frame->bld, dst);
		g2d_wb_set(p_frame->wb, dst);
	} else if ((flag & 0x0fffffff) == G2D_BLT_EXTRACT) {
		g2d_vlayer_set(p_frame->ovl_v, 0, src);
		/* need abp process */
		if (src->mode)
			g2d_uilayer_set(p_frame->ovl_u, 2,
					dst);
		if ((src->format >= G2D_FORMAT_IYUV422_V0Y1U0Y0) ||
		    (src->clip_rect.w != dst->clip_rect.w) ||
		    (src->clip_rect.h != dst->clip_rect.h)) {
			g2d_ovl_v_set_extract(
			    p_frame->ovl_v, src->format, src->clip_rect.w,
			    src->clip_rect.h, dst->clip_rect.w,
			    dst->clip_rect.h);
			g2d_vsu_para_set(p_frame->scal, src->format, dst->clip_rect.w, dst->clip_rect.h,
					 dst->clip_rect.w, dst->clip_rect.h,
					 0xff);
		}
		bld_porter_duff(p_frame->bld, G2D_BLD_SRCOVER);
		/* Default value */
		bld_set_rop_ctrl(p_frame->bld, 0xf0);
		rect0.x = 0;
		rect0.y = 0;
		rect0.w = dst->clip_rect.w;
		rect0.h = dst->clip_rect.h;
		bld_in_set(p_frame->bld, 0, rect0, dst->bpremul);
		bld_cs_set(p_frame->bld, src->format);
		if (src->mode) {
			/* need abp process */
			rect1.x = 0;
			rect1.y = 0;
			rect1.w = dst->clip_rect.w;
			rect1.h = dst->clip_rect.h;
			bld_in_set(p_frame->bld, 1, rect1, dst->bpremul);
		}
		if ((src->format <= G2D_FORMAT_BGRA1010102) &&
				(dst->format > G2D_FORMAT_BGRA1010102)) {
			if (dst->gamut == G2D_BT601) {
				bld_csc_reg_set(p_frame->bld, 2, G2D_RGB2YUV_601,
						src->color_range, dst->color_range);
			} else {
				bld_csc_reg_set(p_frame->bld, 2, G2D_RGB2YUV_709,
						src->color_range, dst->color_range);
			}
		}
		if ((src->format > G2D_FORMAT_BGRA1010102) &&
				(dst->format <= G2D_FORMAT_BGRA1010102)) {
			if (dst->gamut == G2D_BT601) {
				bld_csc_reg_set(p_frame->bld, 2, G2D_YUV2RGB_601,
						src->color_range, dst->color_range);
			} else {
				bld_csc_reg_set(p_frame->bld, 2, G2D_YUV2RGB_709,
						src->color_range, dst->color_range);
			}
		}

		bld_out_setting(p_frame->bld, dst);
		g2d_wb_set(p_frame->wb, dst);
	} else if (flag & G2D_BLT_WHITENESS) {
		if ((src->format > G2D_FORMAT_BGRA1010102) |
		    (dst->format > G2D_FORMAT_BGRA1010102)) {
			G2D_WARN("Only support rgb format\n");
			goto OUT;
		}
		g2d_uilayer_set(p_frame->ovl_u, 0, dst);
		g2d_vlayer_set(p_frame->ovl_v, 0, src);
		bpre = false;
		if (src->bpremul || dst->bpremul)
			bpre = true;
		if ((src->clip_rect.w != dst->clip_rect.w)
		    || (src->clip_rect.h != dst->clip_rect.h)) {
			g2d_ovl_v_calc_coarse(
			    p_frame->ovl_v, src->format, src->clip_rect.w,
			    src->clip_rect.h, dst->clip_rect.w,
			    dst->clip_rect.h, &midw, &midh);
			g2d_vsu_para_set(p_frame->scal, src->format, midw, midh,
					 dst->clip_rect.w, dst->clip_rect.h,
					 0xff);
		}
		/* Default value */
		bld_porter_duff(p_frame->bld, G2D_BLD_SRCOVER);
		bld_set_rop_ctrl(p_frame->bld, 0x00);
		bld_rop2_set(p_frame->bld, flag & 0xff);

		/* set bld para */
		rect0.x = 0;
		rect0.y = 0;
		rect0.w = dst->clip_rect.w;
		rect0.h = dst->clip_rect.h;
		bld_in_set(p_frame->bld, 0, rect0, bpre);
		bld_out_setting(p_frame->bld, dst);
		g2d_wb_set(p_frame->wb, dst);
	}

	ret = 0;
OUT:
	return ret;

}
