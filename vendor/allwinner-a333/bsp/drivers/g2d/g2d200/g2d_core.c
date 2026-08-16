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
#include "g2d_core.h"

void core_ctl_set_mixer_en(struct core_submodule *p_core, int en)
{
	struct g2d_core_ctl_reg *p_reg = p_core->get_reg(p_core);
	p_reg->core_ctl.bits.mixer_en = (en > 0) ? 1 : 0;
	p_reg->core_ctl.bits.rotate_en = (en > 0) ? 0 : 1;
	p_core->set_block_dirty(p_core, 0, 1);
}

void core_ctl_set_rotate_en(struct core_submodule *p_core, int en)
{
	struct g2d_core_ctl_reg *p_reg = p_core->get_reg(p_core);
	p_reg->core_ctl.bits.rotate_en = (en > 0) ? 1 : 0;
	p_reg->core_ctl.bits.mixer_en = (en > 0) ? 0 : 1;
	p_core->set_block_dirty(p_core, 0, 1);
}

void core_ctl_set_scan_order(struct core_submodule *p_core,
	unsigned int scan_order)
{
	struct g2d_core_ctl_reg *p_reg = p_core->get_reg(p_core);
	p_reg->core_ctl.bits.scan_order = scan_order;
	p_core->set_block_dirty(p_core, 0, 1);
}

static int core_rcq_setup(struct core_submodule *p_core, u8 __iomem *base,
			   struct g2d_rcq_mem_info *p_rcq_info)
{
	u8 __iomem *reg_base = base + G2D_CORE;
	int ret = -1;

	if (!p_core) {
		G2D_WARN("Null pointer\n");
		goto OUT;
	}

	p_core->reg_info->size = sizeof(struct g2d_core_ctl_reg);
	p_core->reg_info->vir_addr = (u8 *)g2d_top_reg_memory_alloc(
	    p_core->reg_info->size, (void *)&(p_core->reg_info->phy_addr),
	    p_rcq_info);

	if (!p_core->reg_info->vir_addr) {
		G2D_WARN("Malloc writeback reg rcq memory fail\n");
		goto OUT;
	}

	p_core->reg_blks->vir_addr = p_core->reg_info->vir_addr;
	p_core->reg_blks->phy_addr = p_core->reg_info->phy_addr;
	p_core->reg_blks->size = p_core->reg_info->size;
	p_core->reg_blks->reg_addr = reg_base;
	ret = 0;

OUT:
	return ret;
}

static __u32 core_get_reg_block_num(struct core_submodule *p_core)
{
	if (p_core)
		return p_core->reg_blk_num;
	return 0;
}

static __s32 core_get_reg_block(struct core_submodule *p_core,
			    struct g2d_reg_block **blks)
{
	int i = 0;
	if (p_core) {
		for (i = 0; i < p_core->reg_blk_num; ++i)
			blks[i] = p_core->reg_blks + i;
	}
	return 0;
}

static struct g2d_core_ctl_reg *core_get_reg(struct core_submodule *p_core)
{
#if G2D_MIXER_RCQ_USED == 1
	return (struct g2d_core_ctl_reg *)(p_core->reg_blks
						   ->vir_addr);
#else
	return (struct g2d_core_ctl_reg *)(p_core->reg_blks
						   ->reg_addr);
#endif
	return NULL;
}

static void core_set_block_dirty(struct core_submodule *p_core, __u32 blk_id, __u32 dirty)
{
#if G2D_MIXER_RCQ_USED == 1
	if (p_core && p_core->reg_blks->rcq_hd)
		p_core->reg_blks->rcq_hd->dirty.bits.dirty = dirty;
	else
		G2D_WARN("Null pointer\n");
#else

	if (p_core)
		p_core->reg_blks->dirty = dirty;
	else
		G2D_WARN("Null pointer\n");
#endif
}


static __u32 core_get_rcq_mem_size(struct core_submodule *p_core)
{
	return G2D_RCQ_BYTE_ALIGN(sizeof(struct g2d_core_ctl_reg));
}

static __s32 core_destory(struct core_submodule *p_core)
{
	if (p_core) {
		kfree(p_core->reg_blks);
		p_core->reg_blks = NULL;

		kfree(p_core->reg_info);
		p_core->reg_info = NULL;
		kfree(p_core);
	}

	return 0;
}

struct core_submodule *
g2d_core_submodule_setup(struct g2d_frame *p_frame)
{
	struct core_submodule *p_core = NULL;

	p_core = kmalloc(sizeof(*p_core), GFP_KERNEL | __GFP_ZERO);

	if (!p_core) {
		G2D_WARN("Kmalloc wb submodule fail\n");
		return NULL;
	}

	p_core->rcq_setup = core_rcq_setup;
	p_core->reg_blk_num = 1;
	p_core->get_reg_block_num = core_get_reg_block_num;
	p_core->get_reg_block = core_get_reg_block;
	p_core->get_reg = core_get_reg;
	p_core->set_block_dirty = core_set_block_dirty;
	p_core->get_rcq_mem_size = core_get_rcq_mem_size;
	p_core->destory = core_destory;

	p_core->reg_blks =
	    kmalloc(sizeof(*(p_core->reg_blks)) * p_core->reg_blk_num,
		    GFP_KERNEL | __GFP_ZERO);
	p_core->reg_info =
	    kmalloc(sizeof(*(p_core->reg_info)), GFP_KERNEL | __GFP_ZERO);

	if (!p_core->reg_blks || !p_core->reg_info) {
		G2D_WARN("Kmalloc wb reg info fail\n");
		goto FREE_CORE;
	}

	return p_core;

FREE_CORE:
	kfree(p_core->reg_blks);
	kfree(p_core->reg_info);
	kfree(p_core);

	return NULL;
}
