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

#include "de_bld_top_platform.h"

#define DE_DISP_BASE_OFFSET(base, disp, disp_size)			    \
					((base) + (disp) * (disp_size))

#define DISP_BASE_V3XX			(0x280000)
#define DE_DISP_SIZE_V3XX		(0x20000)

#define DISP_BASE_V2XX			(0x1C0000)
#define DE_DISP_SIZE_V2XX		(0x100000)

struct de_version_bld_top {
	unsigned int version;
	unsigned int bld_cnt;
	struct de_bld_top_desc **bld_top;
};

static struct de_bld_top_desc de212_bld0_top = {
    .name = "bld0_top",
    .id = 0,
    .disp_base = DE_DISP_BASE_OFFSET(DISP_BASE_V2XX, 0, DE_DISP_SIZE_V2XX),
    .bld_top_offset = 0x0,
};

static struct de_bld_top_desc *de212_bld_top[] = {
	&de212_bld0_top,
};

static struct de_version_bld_top de212 = {
	.version = 0x212,
	.bld_cnt = ARRAY_SIZE(de212_bld_top),
	.bld_top = &de212_bld_top[0],
};

static struct de_version_bld_top *de_version[] = {
	&de212,
};

const struct de_bld_top_desc *get_bld_top_dsc(struct module_create_info *info)
{
	int i, j;
	for (i = 0; i < ARRAY_SIZE(de_version); i++) {
		if (de_version[i]->version == info->de_version) {
			for (j = 0; j < de_version[i]->bld_cnt; j++) {
				if (de_version[i]->bld_top[j]->id == info->id)
					return de_version[i]->bld_top[j];
			}
		}
	}
	return NULL;
}
