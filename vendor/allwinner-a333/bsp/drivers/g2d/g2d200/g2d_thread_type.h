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
#ifndef _G2D_THREAD_TYPE_H
#define _G2D_THREAD_TYPE_H

union g2d_ver_ctl {
	unsigned int dwval;
	struct {
		unsigned int rxpx:16;
		unsigned int ip_version:16;
	} bits;
};

union g2d_ver_tim {
	unsigned int dwval;
	struct {
		unsigned int date:8;
		unsigned int month:8;
		unsigned int year:16;
	} bits;
};

union g2d_reset {
	unsigned int dwval;
	struct {
		unsigned int core_rst:1;
		unsigned int res0:15;
		unsigned int thread0_rst:1;
		unsigned int res1:3;
		unsigned int thread1_rst:1;
		unsigned int res2:3;
		unsigned int thread2_rst:1;
		unsigned int res3:3;
		unsigned int thread3_rst:1;
		unsigned int res4:3;
	} bits;
};

union g2d_clk_gate {
	unsigned int dwval;
	struct {
		unsigned int core_clk_gate:1;
		unsigned int res0:15;
		unsigned int thread0_clk_gate:1;
		unsigned int res1:3;
		unsigned int thread1_clk_gate:1;
		unsigned int res2:3;
		unsigned int thread2_clk_gate:1;
		unsigned int res3:3;
		unsigned int thread3_clk_gate:1;
		unsigned int res4:3;
	} bits;
};

union g2d_mclk_gate {
	unsigned int dwval;
	struct {
		unsigned int mclk_gate:1;
		unsigned int res0:15;
		unsigned int mbus_rst:1;
		unsigned int res1:15;
	} bits;
};

union g2d_mode_ctl {
	unsigned int dwval;
	struct {
		unsigned int opr_mode:1;
		unsigned int res0:3;
		unsigned int timeout_mode:1;
		unsigned int res1:28;
	} bits;
};

union g2d_tot_num {
	unsigned int dwval;
	struct {
		unsigned int timeout_cycle_num:32;
	} bits;
};

union g2d_core_st {
	unsigned int dwval;
	struct {
		unsigned int exe_cmd_id:8;
		unsigned int exe_task_id:16;
		unsigned int exe_thread_id:2;
		unsigned int res0:2;
		unsigned int busy:1;
		unsigned int res1:3;
	} bits;
};

union g2d_core_start {
	unsigned int dwval;
	struct {
		unsigned int g2d_core_start:1;
		unsigned int res0:31;
	} bits;
};

union g2d_core_rst_num {
	unsigned int dwval;
	struct {
		unsigned int g2d_core_rst_num:16;
		unsigned int res0:16;
	} bits;
};

union g2d_load_sta {
	unsigned int dwval;
	struct {
		unsigned int g2d_loading_stat_en:1;
		unsigned int res0:31;
	} bits;
};

union g2d_load_sta_num {
	unsigned int dwval;
	struct {
		unsigned int g2d_loading_stat_num:32;
	} bits;
};

union g2d_vld_proc_num {
	unsigned int dwval;
	struct {
		unsigned int g2d_valid_proc_cycle:32;
	} bits;
};

union g2d_rdat_tot_ctl {
	unsigned int dwval;
	struct {
		unsigned int rdat_tot_mode:1;
		unsigned int res0:3;
		unsigned int rdat_tot_flag:1;
		unsigned int res1:11;
		unsigned int rdat_tot_num:12;
		unsigned int res2:4;
	} bits;
};

union g2d_tr_irq_en {
	unsigned int dwval;
	struct {
		unsigned int task_end_irq_en:1;
		unsigned int res0:3;
		unsigned int rcq_accept_irq_en:1;
		unsigned int res1:3;
		unsigned int timeout_irq_en:1;
		unsigned int res2:23;
	} bits;
};

union g2d_tr_irq {
	unsigned int dwval;
	struct {
		unsigned int task_end_irq:1;
		unsigned int res0:3;
		unsigned int rcq_accept_irq:1;
		unsigned int res1:3;
		unsigned int timeout_irq:1;
		unsigned int res2:23;
	} bits;
};

union g2d_tr_tsk_id {
	unsigned int dwval;
	struct {
		unsigned int finish_task_id:16;
		unsigned int current_task_id:16;
	} bits;
};

union g2d_tr_to_info {
	unsigned int dwval;
	struct {
		unsigned int tout_cmd_id:8;
		unsigned int tout_task_id:16;
	} bits;
};

union g2d_tr_rcq_h_laddr {
	unsigned int dwval;
	struct {
		unsigned int rcq_header_laddr:32;
	} bits;
};

union g2d_tr_rcq_h_len {
	unsigned int dwval;
	struct {
		unsigned int rcq_header_haddr:8;
		unsigned int res0:8;
		unsigned int rcq_header_len:16;
	} bits;
};

union g2d_tr_rcq_attr {
	unsigned int dwval;
	struct {
		unsigned int task_cmd_num:8;
		unsigned int res0:8;
		unsigned int task_end_irq_gen:1;
		unsigned int res1:15;
	} bits;
};

union g2d_tr_rcq_update {
	unsigned int dwval;
	struct {
		unsigned int rcq_update:1;
		unsigned int res0:31;
	} bits;
};

struct g2d_top_reg {
	/* 0x00 */
	union g2d_ver_ctl ver_ctl;
	union g2d_ver_tim ver_tim;
	unsigned int res0[2];
	/* 0x10 */
	union g2d_reset reset;
	union g2d_clk_gate clk_gate;
	union g2d_mclk_gate mclk_gate;
	unsigned int res1;
	/* 0x20 */
	union g2d_mode_ctl mode_ctl;
	union g2d_tot_num tot_num;
	union g2d_core_st core_st;
	unsigned int res2;
	/* 0x30 */
	union g2d_core_start core_start;
	union g2d_core_rst_num core_rst_num;
	unsigned int res3[2];
	/* 0x40 */
	union g2d_load_sta load_st;
	union g2d_load_sta_num load_sta_num;
	union g2d_vld_proc_num vld_proc_num;
	unsigned int res4;
	/* 0x40 */
	union g2d_rdat_tot_ctl rdat_tot_ctl;
};

struct g2d_hyper_thread_rcq_reg {
	union g2d_tr_irq_en tr_irq_en;
	union g2d_tr_irq tr_irq;
	union g2d_tr_tsk_id tr_tsk_id;
	union g2d_tr_to_info tr_to_info;
	union g2d_tr_rcq_h_laddr tr_rcq_h_laddr;
	union g2d_tr_rcq_h_len tr_rcq_h_len;
	union g2d_tr_rcq_attr tr_rcq_attr;
	union g2d_tr_rcq_update tr_rcq_update;
	unsigned int res[8];
};

#endif /* End of file */
