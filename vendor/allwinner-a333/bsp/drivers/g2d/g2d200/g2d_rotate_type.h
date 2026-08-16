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
#ifndef _G2D_ROTATE_TYPE_H
#define _G2D_ROTATE_TYPE_H

union g2d_rot_ctrl {
	unsigned int dwval;
	struct {
		unsigned int mode_sel:2;
		unsigned int res0:2;
		unsigned int degree:2;
		unsigned int vflip_en:1;
		unsigned int hflip_en:1;
		unsigned int res1:22;
		unsigned int bist_en:1;
		unsigned int start:1;
	} bits;
};

union g2d_rot_interrupt {
	unsigned int dwval;
	struct {
		unsigned int rot_irq:1;
		unsigned int res0:15;
		unsigned int finish_irq_en:1;
		unsigned int res1:15;
	} bits;
};

union g2d_rot_time_ctrl {
	unsigned int dwval;
	struct {
		unsigned int timeout_st:1;
		unsigned int res0:29;
		unsigned int timeout_rst_en:1;
		unsigned int timeout_rst:1;
	} bits;
};

union g2d_rot_in_fmt {
	unsigned int dwval;
	struct {
		unsigned int fmt:6;
		unsigned int res0:26;
	} bits;
};

union g2d_rot_size {
	unsigned int dwval;
	struct {
		unsigned int width:13;
		unsigned int res0:3;
		unsigned int height:13;
		unsigned int res1:3;
	} bits;
};

union g2d_rot_rand_ctrl {
	unsigned int dwval;
	struct {
		unsigned int rand_en:1;
		unsigned int res0:3;
		unsigned int mode:2;
		unsigned int res1:2;
		unsigned int seed:24;
	} bits;
};

union g2d_rot_rand_clk {
	unsigned int dwval;
	struct {
		unsigned int neg_num:16;
		unsigned int pos_num:16;
	} bits;
};

union g2d_lbc_enc_ctl {
	unsigned int dwval;
	struct {
		unsigned int res:1;
		unsigned int g2d_lbc_en:1;
		unsigned int enc_segline_tar_bits:17;
		unsigned int enc_c_ratio:10;
		unsigned int enc_round_offset:1;
		unsigned int enc_seg_rc_en:1;
		unsigned int enc_is_lossy:1;
	} bits;
};

union g2d_lbc_ctl {
	unsigned int dwval;
	struct {
		unsigned int res:7;
		unsigned int lbc_rot_angle:3;
		unsigned int seg_tar_bits_c:11;
		unsigned int seg_tar_bits_y:11;
	} bits;
};

union g2d_lbc_dec_ctl {
	unsigned int dwval;
	struct {
		unsigned int res0:2;
		unsigned int dec_segline_tar_bits:17;
		unsigned int res1:12;
		unsigned int dec_is_lossy:1;
	} bits;
};

union g2d_lbc_cmp_idle_ctl {
	unsigned int dwval;
	struct {
		unsigned int lbc_cmp_ratio:28;
		unsigned int res:4;
	} bits;
};

struct g2d_rot_reg {
	/* 0x00 */
	union g2d_rot_ctrl rot_ctrl;
	union g2d_rot_interrupt rot_int;
	union g2d_rot_time_ctrl time_ctrl;
	unsigned int res0[5];
	/* 0x20 */
	union g2d_rot_in_fmt in_fmt;
	union g2d_rot_size in_size;
	unsigned int res1[2];
	/* 0x30 */
	unsigned int in_pitch0;
	unsigned int in_pitch1;
	unsigned int in_pitch2;
	unsigned int res2;
	/* 0x40 */
	unsigned int in_laddr0;
	unsigned int in_haddr0;
	unsigned int in_laddr1;
	unsigned int in_haddr1;
	/* 0x50 */
	unsigned int in_laddr2;
	unsigned int in_haddr2;
	unsigned int res3[11];
	/* 0x84 */
	union g2d_rot_size out_size;
	unsigned int res4[2];
	/* 0x90 */
	unsigned int out_pitch0;
	unsigned int out_pitch1;
	unsigned int out_pitch2;
	unsigned int res5;
	/* 0xa0 */
	unsigned int out_laddr0;
	unsigned int out_haddr0;
	unsigned int out_laddr1;
	unsigned int out_haddr1;
	/* 0xb0 */
	unsigned int out_laddr2;
	unsigned int out_haddr2;
	union g2d_rot_rand_ctrl rand_in_ctrl;
	union g2d_rot_rand_clk rand_in_clk;
	/* 0xc0 */
	union g2d_rot_rand_ctrl rand_out_ctrl;
	union g2d_rot_rand_clk rand_out_clk;
	/* 0xc8 */
	union g2d_lbc_enc_ctl lbc_enc_ctl;
	union g2d_lbc_ctl lbc_ctl;
	union g2d_lbc_dec_ctl lbc_dec_ctl;
	union g2d_lbc_cmp_idle_ctl lbc_cmp_idle_ctl;
};

#endif /* End of file */
