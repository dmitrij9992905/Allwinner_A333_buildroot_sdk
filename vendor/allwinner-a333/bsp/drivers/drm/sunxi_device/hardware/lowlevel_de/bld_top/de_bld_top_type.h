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

#ifndef _DE_BLD_TOP_TYPE_H_
#define _DE_BLD_TOP_TYPE_H_

#include <linux/types.h>

union bld_top_en_ctl_reg {
	u32 dwval;
	struct {
		u32 vsu_mux:1;
		u32 res1:3;
		u32 cross_en:1;
		u32 res2:3;
		u32 abp_byp:1;
		u32 res3:23;
	} bits;
};

struct bld_top_reg {
	union bld_top_en_ctl_reg ctl;
};

#endif /* #ifndef _DE_BLD_TOP_TYPE_H_ */
