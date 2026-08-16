/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Copyright(c) 2020 - 2023 Allwinner Technology Co.,Ltd. All rights reserved. */
/*
 * Allwinner SoCs display driver.
 *
 * Copyright (C) 2023 Allwinner.
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2.  This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */

#ifndef _DE_BLENDER_TOP_H_
#define _DE_BLENDER_TOP_H_

#include <linux/types.h>
#include "de_base.h"

struct de_bld_top_handle {
	struct module_create_info cinfo;
	u32 disp_reg_base;
	unsigned int block_num;
	struct de_reg_block **block;
	struct de_bld_top_private *private;
};

int de_bld_top_set_vsu_mux(struct de_bld_top_handle *hdl, u8 mux);

int de_bld_top_set_cross(struct de_bld_top_handle *hdl, u8 en);

void dump_bld_top_state(struct drm_printer *p, struct de_bld_top_handle *hdl);

struct de_bld_top_handle *de_blender_top_create(struct module_create_info *info);

#endif /* #ifndef _DE_BLENDER_TOP_H_ */
