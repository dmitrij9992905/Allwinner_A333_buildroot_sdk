/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Copyright(c) 2020 - 2023 Allwinner Technology Co.,Ltd. All rights reserved. */
/*
 * Allwinner SoCs display driver.
 *
 * Copyright (C) 2017 Allwinner.
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2.  This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */

#include <linux/kernel.h>
#include "de_bld_top.h"
#include "de_bld_top_type.h"
#include "de_bld_top_platform.h"

enum {
	BLD_TOP_REG_BLK_CTL = 0,
	BLD_TOP_REG_BLK_NUM,
};

struct bld_top_debug_info {
	bool enable;
	bool vsu_mux;
	bool cross_en;
	bool abp_byp;
	struct drm_rect crtc_pos;
	unsigned int port_id;
	bool is_premult;
};

struct de_bld_top_private {
	struct de_reg_mem_info reg_mem_info;
	struct bld_top_debug_info debug[2];
	const struct de_bld_top_desc *dsc;
	u32 reg_blk_num;
	struct de_reg_block reg_blks[BLD_TOP_REG_BLK_NUM];
};

static inline struct bld_top_reg *get_bld_top_reg(struct de_bld_top_private *priv)
{
	return (struct bld_top_reg *)(priv->reg_blks[0].vir_addr);
}

static inline struct bld_top_reg *get_bld_top_hw_reg(struct de_bld_top_private *priv)
{
	return (struct bld_top_reg *)(priv->reg_blks[0].reg_addr);
}

static void bld_top_set_block_dirty(struct de_bld_top_private *priv, u32 blk_id,
				   u32 dirty)
{
	priv->reg_blks[blk_id].dirty = dirty;
	if (priv->reg_blks[blk_id].rcq_hd)
		priv->reg_blks[blk_id].rcq_hd->dirty.dwval = dirty;
}

int de_bld_top_set_vsu_mux(struct de_bld_top_handle *hdl, u8 mux)
{
	struct de_bld_top_private *priv = hdl->private;
	struct bld_top_reg *reg = get_bld_top_reg(priv);

	hdl->private->debug[0].enable = mux == 1 ? 1 : 0;
	hdl->private->debug[0].vsu_mux = mux == 1 ? 1 : 0;

	reg->ctl.bits.vsu_mux = mux == 1 ? 1 : 0;
	bld_top_set_block_dirty(priv, BLD_TOP_REG_BLK_CTL, 1);
	return 0;
}

int de_bld_top_set_cross(struct de_bld_top_handle *hdl, u8 en)
{
	struct de_bld_top_private *priv = hdl->private;
	struct bld_top_reg *reg = get_bld_top_reg(priv);

	hdl->private->debug[0].enable = en == 1 ? 1 : 0;
	hdl->private->debug[0].cross_en = (en == 1) ? 1 : 0;
	hdl->private->debug[0].abp_byp = (en == 1) ? 1 : 0;

	reg->ctl.bits.cross_en = (en == 1) ? 1 : 0;
	reg->ctl.bits.abp_byp = (en == 1) ? 1 : 0;
	bld_top_set_block_dirty(priv, BLD_TOP_REG_BLK_CTL, 1);
	return 0;
}

void dump_bld_top_state(struct drm_printer *p, struct de_bld_top_handle *hdl)
{
	struct bld_top_debug_info *info;
	unsigned long base = (unsigned long)hdl->private->reg_blks[0].reg_addr;
	unsigned long de_base = (unsigned long)hdl->cinfo.de_reg_base;

	drm_printf(p, "bld top@%8x: %sable\n", (unsigned int)(base - de_base), hdl->private->debug[0].enable ? "en" : "dis");

	info = &hdl->private->debug[0];
	if (hdl->private->debug[0].enable) {
		drm_printf(p, "\t vsu mux: %s, cross: %s, abp bypass: %s\n",
			info->vsu_mux == 1 ? "UCH" : "VCH", info->cross_en ? "enable" : "diabled", info->abp_byp ? "enable" : "disable");
	}

	if (hdl->private->debug[0].enable) {
		drm_printf(p, "\n\n");
	}
}

struct de_bld_top_handle *de_blender_top_create(struct module_create_info *info)
{
	int i;
	struct de_bld_top_handle *hdl;
	struct de_reg_block *block;
	struct de_reg_mem_info *reg_mem_info;
	struct de_bld_top_private *priv;
	u8 __iomem *reg_base;
	const struct de_bld_top_desc *dsc;

	dsc = get_bld_top_dsc(info);
	if (!dsc)
		return NULL;

	hdl = kmalloc(sizeof(*hdl), GFP_KERNEL | __GFP_ZERO);
	hdl->private = kmalloc(sizeof(*hdl->private), GFP_KERNEL | __GFP_ZERO);
	hdl->disp_reg_base = dsc->disp_base;
	memcpy(&hdl->cinfo, info, sizeof(*info));

	hdl->private->dsc = dsc;
	priv = hdl->private;

	reg_mem_info = &(priv->reg_mem_info);
	reg_mem_info->size = sizeof(struct bld_top_reg);
	reg_mem_info->vir_addr = (u8 *)sunxi_de_reg_buffer_alloc(hdl->cinfo.de,
		reg_mem_info->size, (void *)&(reg_mem_info->phy_addr),
		info->update_mode == RCQ_MODE);
	if (NULL == reg_mem_info->vir_addr) {
		DRM_ERROR("alloc bld top[%d] mm fail!size=0x%x\n",
		     info->id, reg_mem_info->size);
		return ERR_PTR(-ENOMEM);
	}

	reg_base = info->de_reg_base + dsc->disp_base + dsc->bld_top_offset;

	block = &(priv->reg_blks[BLD_TOP_REG_BLK_CTL]);
	block->phy_addr = reg_mem_info->phy_addr;
	block->vir_addr = reg_mem_info->vir_addr;
	block->size = 0x4;
	block->reg_addr = reg_base;

	priv->reg_blk_num = BLD_TOP_REG_BLK_NUM;

	hdl->block_num = priv->reg_blk_num;
	hdl->block = kmalloc(sizeof(block[0]) * hdl->block_num, GFP_KERNEL | __GFP_ZERO);
	for (i = 0; i < hdl->private->reg_blk_num; i++)
		hdl->block[i] = &priv->reg_blks[i];

	return hdl;
}
