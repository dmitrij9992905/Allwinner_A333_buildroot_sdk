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
#include "g2d_thread.h"
#include "g2d_platform.h"

static volatile struct g2d_top_reg *g2d_top;
static volatile struct g2d_hyper_thread_rcq_reg *g2d_ht[4];

void g2d_thread_set_base(unsigned long base)
{
	g2d_top = (struct g2d_top_reg *)(base);
	g2d_ht[0] = (struct g2d_hyper_thread_rcq_reg *)(base + G2D_HYPER_THREAD_RCQ);
}

__u32 g2d_ip_version(void)
{
	__u32 reg_val;
	reg_val = g2d_top->ver_ctl.dwval;
	return reg_val;
}

void g2d_version_time_query(int *year, int *month, int *date)
{
	*year = g2d_top->ver_tim.bits.year;
	*month = g2d_top->ver_tim.bits.month;
	*date = g2d_top->ver_tim.bits.date;
}

void g2d_thread_open(void)
{
	g2d_top->clk_gate.bits.core_clk_gate = 1;
	g2d_top->mclk_gate.bits.mclk_gate = 1;
	g2d_top->mclk_gate.bits.mbus_rst = 1;
	g2d_top->reset.bits.core_rst = 1;
}

void g2d_thread_close(void)
{
	g2d_top->clk_gate.bits.core_clk_gate = 0;
	g2d_top->mclk_gate.bits.mclk_gate = 0;
	g2d_top->mclk_gate.bits.mbus_rst = 0;
	g2d_top->reset.bits.core_rst = 0;
}

void g2d_thread_reset(void)
{
	g2d_top->reset.bits.core_rst = 0;
	g2d_top->reset.bits.core_rst = 1;
}

int g2d_threadn_open(int thread_id)
{
	switch (thread_id) {
	case 0:
		g2d_top->clk_gate.bits.thread0_clk_gate = 1;
		g2d_top->reset.bits.thread0_rst = 1;
		break;
	case 1:
		g2d_top->clk_gate.bits.thread1_clk_gate = 1;
		g2d_top->reset.bits.thread1_rst = 1;
		break;
	case 2:
		g2d_top->clk_gate.bits.thread2_clk_gate = 1;
		g2d_top->reset.bits.thread2_rst = 1;
		break;
	case 3:
		g2d_top->clk_gate.bits.thread3_clk_gate = 1;
		g2d_top->reset.bits.thread3_rst = 1;
		break;
	default:
		G2D_WARN("illegal thread_id\n");
		return -1;
	}
	return 0;
}

int g2d_threadn_close(int thread_id)
{
	switch (thread_id) {
	case 0:
		g2d_top->clk_gate.bits.thread0_clk_gate = 0;
		g2d_top->reset.bits.thread0_rst = 0;
		break;
	case 1:
		g2d_top->clk_gate.bits.thread1_clk_gate = 0;
		g2d_top->reset.bits.thread1_rst = 0;
		break;
	case 2:
		g2d_top->clk_gate.bits.thread2_clk_gate = 0;
		g2d_top->reset.bits.thread2_rst = 0;
		break;
	case 3:
		g2d_top->clk_gate.bits.thread3_clk_gate = 0;
		g2d_top->reset.bits.thread3_rst = 0;
		break;
	default:
		G2D_WARN("illegal thread_id\n");
		return -1;
	}
	return 0;
}

int g2d_threadn_reset(int thread_id)
{
	switch (thread_id) {
	case 0:
		g2d_top->reset.bits.thread0_rst = 0;
		g2d_top->reset.bits.thread0_rst = 1;
		break;
	case 1:
		g2d_top->reset.bits.thread1_rst = 0;
		g2d_top->reset.bits.thread1_rst = 1;
		break;
	case 2:
		g2d_top->reset.bits.thread2_rst = 0;
		g2d_top->reset.bits.thread2_rst = 1;
		break;
	case 3:
		g2d_top->reset.bits.thread3_rst = 0;
		g2d_top->reset.bits.thread3_rst = 1;
		break;
	default:
		G2D_WARN("illegal thread_id\n");
		return -1;
	}
	return 0;
}

void g2d_threadn_set_rcq_head(int thread_id, __u64 addr, __u32 len)
{
	__u32 haddr = (__u32)(addr >> 32);

	g2d_ht[thread_id]->tr_rcq_h_len.bits.rcq_header_haddr = haddr & 0xff;
	g2d_ht[thread_id]->tr_rcq_h_laddr.dwval = (__u32) (addr & 0xffffffff);
	g2d_ht[thread_id]->tr_rcq_h_len.bits.rcq_header_len = len & 0xffff;
}

void g2d_threadn_set_rcq_update_en(int thread_id, int en)
{
	g2d_ht[thread_id]->tr_rcq_update.bits.rcq_update = (en > 0) ? 1 : 0;
}

void g2d_thread_start(void)
{
	g2d_top->core_start.bits.g2d_core_start = 1;
}

void g2d_threadn_set_rcq_accept_irq_en(int thread_id, int en)
{
	g2d_ht[thread_id]->tr_irq_en.bits.rcq_accept_irq_en = en;
}

void g2d_threadn_set_timeout_irq_en(int thread_id, int en)
{
	g2d_ht[thread_id]->tr_irq_en.bits.timeout_irq_en = en;
}

void g2d_threadn_set_task_end_irq_en(int thread_id, int en)
{
	g2d_ht[thread_id]->tr_irq_en.bits.task_end_irq_en = en;
}

void g2d_thread_enable_timeout_mode(int en)
{
	g2d_top->mode_ctl.bits.timeout_mode = (en > 0) ? 1 : 0;
}

void g2d_thread_enable_master_mode(int en)
{
	g2d_top->mode_ctl.bits.opr_mode = (en > 0) ? 1 : 0;
}

void g2d_thread_set_timeout_num(unsigned int timeout_num)
{
	g2d_top->tot_num.dwval = timeout_num;
}

void g2d_thread_set_rst_num(unsigned int rst_num)
{
	g2d_top->core_rst_num.bits.g2d_core_rst_num = rst_num;
}

bool g2d_thread_status_query(int *thread_id, int *task_id, int *cmd_id)
{
	if (g2d_top->core_st.bits.busy == 1) {
		*thread_id = g2d_top->core_st.bits.exe_thread_id;
		*task_id = g2d_top->core_st.bits.exe_task_id;
		*cmd_id = g2d_top->core_st.bits.exe_cmd_id;
		return true;
	} else {
		return false;
	}
}

void g2d_thread_loading_stas_en(int en)
{
	g2d_top->load_st.bits.g2d_loading_stat_en = (en > 0) ? 1 : 0;
}

void g2d_thread_set_loading_cycle(__u32 num)
{
	g2d_top->load_sta_num.dwval = num;
}

unsigned int g2d_thread_get_valid_proc_cycle(void)
{
	return (__u32)(g2d_top->vld_proc_num.dwval);
}

int g2d_threadn_timeout_irq_query(int thread_id)
{
	G2D_DRV_DBG("g2d timeout%d irq\n", thread_id);
	if (g2d_ht[thread_id]->tr_irq.bits.timeout_irq & 0x1) {
		g2d_ht[thread_id]->tr_irq.bits.timeout_irq = 1;
		return 1;
	}
	return 0;
}

int g2d_threadn_rcq_accept_irq_query(int thread_id)
{
	G2D_DRV_DBG("g2d rcq%d irq\n", thread_id);
	if (g2d_ht[thread_id]->tr_irq.bits.rcq_accept_irq & 0x1) {
		g2d_ht[thread_id]->tr_irq.bits.rcq_accept_irq = 1;
		return 1;
	}
	return 0;
}

int g2d_threadn_task_end_irq_query(int thread_id)
{
	G2D_DRV_DBG("g2d task_end irq%d\n", thread_id);
	if (g2d_ht[thread_id]->tr_irq.bits.task_end_irq & 0x1) {
		g2d_ht[thread_id]->tr_irq.bits.task_end_irq = 1;
		return 1;
	}
	return 0;
}

int g2d_threadn_get_current_task_id(int thread_id)
{
	return g2d_ht[thread_id]->tr_tsk_id.bits.current_task_id;
}

int g2d_threadn_get_last_finished_task_id(int thread_id)
{
	return g2d_ht[thread_id]->tr_tsk_id.bits.finish_task_id;
}

int g2d_threadn_get_timeout_task_id(int thread_id)
{
	return g2d_ht[thread_id]->tr_to_info.bits.tout_task_id;
}

int g2d_threadn_get_timeout_cmd_id(int thread_id)
{
	return g2d_ht[thread_id]->tr_to_info.bits.tout_cmd_id;
}

void g2d_threadn_set_rcq_task_end_irq_gen(int thread_id, int en)
{
	g2d_ht[thread_id]->tr_rcq_attr.bits.task_end_irq_gen = (en > 0) ? 1 : 0;
}

void g2d_threadn_set_rcq_task_cmd_num(int thread_id, unsigned int num)
{
	g2d_ht[thread_id]->tr_rcq_attr.bits.task_cmd_num = num;
}
