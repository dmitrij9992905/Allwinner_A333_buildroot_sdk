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
#include "g2d_debug.h"
#include "g2d_platform.h"
#include "g2d_top.h"

static struct g2d_time_info g2d_time_inf;

void __dump_reg(phys_addr_t phys_base, void __iomem *base,
	__u32 start_offset, __u32 end_offset)
{
	__u32 i;
	__u8 buf[64], cnt = 0;

	for (i = start_offset; i < end_offset; i += REG_INTERVAL) {
		if ((i - start_offset) % HEXADECIMAL == 0)
			cnt += sprintf(buf + cnt, "0x%08x: ",
					   (__u32)(phys_base + i));

		cnt += sprintf(buf + cnt, "%08x ",
				   readl(base + i));

		if ((i - start_offset) % HEXADECIMAL == REG_CL) {
			G2D_REG_DBG("%s\n", buf);
			cnt = 0;
		}
	}

	if (cnt > 0) {
		G2D_REG_DBG("%s\n", buf);
	}
}

void g2d_dump_reg(phys_addr_t phys_base, void __iomem *io_addr)
{
	G2D_REG_DBG("G2D_TOP_REG:\n");
	__dump_reg(phys_base, io_addr, G2D_TOP, G2D_TOP + 0x60);
	__dump_reg(phys_base, io_addr, G2D_HYPER_THREAD_RCQ, G2D_HYPER_THREAD_RCQ + 0x150);
	__dump_reg(phys_base, io_addr, G2D_CORE, G2D_CORE + 0x10);
#if IS_ENABLED(CONFIG_G2D_ROTATE)
	G2D_REG_DBG("G2D_ROTATE_REG:\n");
	__dump_reg(phys_base, io_addr, G2D_ROT, G2D_ROT + 0xd0);
#endif
#if IS_ENABLED(CONFIG_G2D_MIXER)
	G2D_REG_DBG("G2D_MIXER_REG:\n");
	__dump_reg(phys_base, io_addr, G2D_BLD, G2D_BLD + 0x200);
	__dump_reg(phys_base, io_addr, G2D_V0, G2D_V0 + 0x40);
	__dump_reg(phys_base, io_addr, G2D_UI0, G2D_UI0 + 0x20);
	__dump_reg(phys_base, io_addr, G2D_UI1, G2D_UI1 + 0x20);
	__dump_reg(phys_base, io_addr, G2D_UI2, G2D_UI2 + 0x20);
	__dump_reg(phys_base, io_addr, G2D_WB, G2D_WB + 0x30);
	__dump_reg(phys_base, io_addr, G2D_VSU, G2D_VSU + 0x100);
#endif
}

struct g2d_time_info *get_g2d_time_inf(void)
{
	return &g2d_time_inf;
}

unsigned int __get_ts_diff(struct timespec64 ts_start, struct timespec64 ts_end)
{
	return ((ts_end.tv_sec - ts_start.tv_sec) * 1000000
		+ (ts_end.tv_nsec - ts_start.tv_nsec) / NSEC_PER_USEC);
}

void dump_g2d_image_enh_info(const g2d_image_enh *p_image)
{
	G2D_PARA_DBG("image.bbuff           :%d\n", p_image->bbuff);
	G2D_PARA_DBG("image.color           :0x%x\n", p_image->color);
	G2D_PARA_DBG("image.use_phy_addr    :%d\n", p_image->use_phy_addr);
	G2D_PARA_DBG("image.fd              :%d\n", p_image->fd);
	G2D_PARA_DBG("image.laddr[0]        :0x%x\n", p_image->laddr[0]);
	G2D_PARA_DBG("image.laddr[1]        :0x%x\n", p_image->laddr[1]);
	G2D_PARA_DBG("image.laddr[2]        :0x%x\n", p_image->laddr[2]);
	G2D_PARA_DBG("image.haddr[0]        :0x%x\n", p_image->haddr[0]);
	G2D_PARA_DBG("image.haddr[1]        :0x%x\n", p_image->haddr[1]);
	G2D_PARA_DBG("image.haddr[2]        :0x%x\n", p_image->haddr[2]);
	G2D_PARA_DBG("image.format          :0x%x\n", p_image->format);
	G2D_PARA_DBG("image.width           :%d\n", p_image->width);
	G2D_PARA_DBG("image.height          :%d\n", p_image->height);
	G2D_PARA_DBG("image.align[0]        :%d\n", p_image->align[0]);
	G2D_PARA_DBG("image.align[1]        :%d\n", p_image->align[1]);
	G2D_PARA_DBG("image.align[2]        :%d\n", p_image->align[2]);
	G2D_PARA_DBG("image.clip_rect.x     :%d\n", p_image->clip_rect.x);
	G2D_PARA_DBG("image.clip_rect.y     :%d\n", p_image->clip_rect.y);
	G2D_PARA_DBG("image.clip_rect.w     :%d\n", p_image->clip_rect.w);
	G2D_PARA_DBG("image.clip_rect.h     :%d\n", p_image->clip_rect.h);
	G2D_PARA_DBG("image.resize.w        :%d\n", p_image->resize.w);
	G2D_PARA_DBG("image.resize.h        :%d\n", p_image->resize.h);
	G2D_PARA_DBG("image.coor.x          :%d\n", p_image->coor.x);
	G2D_PARA_DBG("image.coor.y          :%d\n", p_image->coor.y);
	G2D_PARA_DBG("image.gamut           :%d\n", p_image->gamut);
	G2D_PARA_DBG("image.alpha           :%d\n", p_image->alpha);
	G2D_PARA_DBG("image.bpremul         :%d\n", p_image->bpremul);
	G2D_PARA_DBG("image.mode            :%d\n", p_image->mode);
	G2D_PARA_DBG("image.color_range     :%d\n", p_image->color_range);
}

void dump_g2d_ck_info(const g2d_ck *g2d_ck)
{
	G2D_PARA_DBG("g2d_ck.match_rule     :%d\n", ((g2d_ck->match_rule) ? 1 : 0));
	G2D_PARA_DBG("g2d_ck.max_color      :%d\n", g2d_ck->max_color);
	G2D_PARA_DBG("g2d_ck.min_color      :%d\n", g2d_ck->min_color);
}

void dump_g2d_bld_info(const g2d_bld *g2d_bld_para)
{
	G2D_PARA_DBG("bld_cmd_flag:         :0x%x\n", g2d_bld_para->bld_cmd);
	G2D_PARA_DBG("dst_image para: \n");
	dump_g2d_image_enh_info(&(g2d_bld_para->dst_image));
	G2D_PARA_DBG("src0_image para: \n");
	dump_g2d_image_enh_info(&(g2d_bld_para->src_image[0]));
	G2D_PARA_DBG("src1_image para: \n");
	dump_g2d_image_enh_info(&(g2d_bld_para->src_image[1]));
}

void dump_mixer_para_info(const mixer_para *mixer_paras)
{
	G2D_PARA_DBG("op_flag:              :0x%x\n", mixer_paras->op_flag);
	G2D_PARA_DBG("flag_h:               :0x%x\n", mixer_paras->flag_h);
	G2D_PARA_DBG("back_flag:            :0x%x\n", mixer_paras->back_flag);
	G2D_PARA_DBG("fore_flag:            :0x%x\n", mixer_paras->fore_flag);
	G2D_PARA_DBG("bld_cmd:              :0x%x\n", mixer_paras->bld_cmd);
	G2D_PARA_DBG("src_image para: \n");
	dump_g2d_image_enh_info(&(mixer_paras->src_image_h));
	G2D_PARA_DBG("ptn_image para: \n");
	dump_g2d_image_enh_info(&(mixer_paras->ptn_image_h));
	G2D_PARA_DBG("mask_image para: \n");
	dump_g2d_image_enh_info(&(mixer_paras->mask_image_h));
	G2D_PARA_DBG("dst_image para: \n");
	dump_g2d_image_enh_info(&(mixer_paras->dst_image_h));
	G2D_PARA_DBG("g2d_ck para: \n");
	dump_g2d_ck_info(&(mixer_paras->ck_para));
}

void dump_g2d_maskblt_info(const g2d_maskblt *g2d_maskblt_para)
{
	G2D_PARA_DBG("back_flag:            :0x%x\n", g2d_maskblt_para->back_flag);
	G2D_PARA_DBG("fore_flag:            :0x%x\n", g2d_maskblt_para->fore_flag);
	G2D_PARA_DBG("src_image para: \n");
	dump_g2d_image_enh_info(&(g2d_maskblt_para->src_image_h));
	G2D_PARA_DBG("ptn_image para: \n");
	dump_g2d_image_enh_info(&(g2d_maskblt_para->ptn_image_h));
	G2D_PARA_DBG("mask_image para: \n");
	dump_g2d_image_enh_info(&(g2d_maskblt_para->mask_image_h));
	G2D_PARA_DBG("dst_image para: \n");
	dump_g2d_image_enh_info(&(g2d_maskblt_para->dst_image_h));
}

void dump_g2d_blt_h_info(const g2d_blt_h *g2d_blt_h_para)
{
	G2D_PARA_DBG("flag_h:               :0x%x\n", g2d_blt_h_para->flag_h);
	G2D_PARA_DBG("src_image para: \n");
	dump_g2d_image_enh_info(&(g2d_blt_h_para->src_image_h));
	G2D_PARA_DBG("dst_image para: \n");
	dump_g2d_image_enh_info(&(g2d_blt_h_para->dst_image_h));
}

void dump_g2d_fillrect_h_info(const g2d_fillrect_h *g2d_fillrect_h_para)
{
	G2D_PARA_DBG("dst_image para: \n");
	dump_g2d_image_enh_info(&(g2d_fillrect_h_para->dst_image_h));
}

void dump_g2d_lbc_rot_info(const g2d_lbc_rot *g2d_lbc_rot_para)
{
	G2D_PARA_DBG("g2d_blt_h: \n");
	dump_g2d_blt_h_info(&(g2d_lbc_rot_para->blt));
	G2D_PARA_DBG("lbc_cmp_ratio         :%d\n", g2d_lbc_rot_para->lbc_cmp_ratio);
	G2D_PARA_DBG("enc_is_lossy          :%d\n", g2d_lbc_rot_para->enc_is_lossy);
	G2D_PARA_DBG("dec_is_lossy          :%d\n", g2d_lbc_rot_para->dec_is_lossy);
}

