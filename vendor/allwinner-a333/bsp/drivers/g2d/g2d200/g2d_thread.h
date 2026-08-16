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
#ifndef _G2D_THREAD_H
#define _G2D_THREAD_H

#include <linux/types.h>
#include "g2d_thread_type.h"

#define G2D_TOP        (0x00000)
#define G2D_MIXER      (0x00100)
#define G2D_BLD        (0x00400)
#define G2D_V0         (0x00800)
#define G2D_UI0        (0x01000)
#define G2D_UI1        (0x01800)
#define G2D_UI2        (0x02000)
#define G2D_WB         (0x03000)
#define G2D_VSU        (0x08000)
#define G2D_ROT        (0x28000)
#define G2D_GSU        (0x30000)

void g2d_thread_set_base(unsigned long base);
__u32 g2d_ip_version(void);
void g2d_version_time_query(int *yeal, int *month, int *date);
void g2d_thread_open(void);
void g2d_thread_close(void);
void g2d_thread_reset(void);

void g2d_thread_enable_timeout_mode(int en);
void g2d_thread_enable_master_mode(int en);
void g2d_thread_start(void);

void g2d_thread_set_timeout_num(unsigned int timeout_num);
void g2d_thread_set_rst_num(unsigned int rst_num);
bool g2d_thread_status_query(int *thread_id, int *task_id, int *cmd_id);
void g2d_thread_loading_stas_en(int en);
void g2d_thread_set_loading_cycle(__u32 num);
unsigned int g2d_thread_get_valid_proc_cycle(void);

int g2d_threadn_open(int thread_id);
int g2d_threadn_close(int thread_id);
int g2d_threadn_reset(int thread_id);

void g2d_threadn_set_rcq_accept_irq_en(int thread_id, int en);
void g2d_threadn_set_timeout_irq_en(int thread_id, int en);
void g2d_threadn_set_task_end_irq_en(int thread_id, int en);

int g2d_threadn_timeout_irq_query(int thread_id);
int g2d_threadn_rcq_accept_irq_query(int thread_id);
int g2d_threadn_task_end_irq_query(int thread_id);

int g2d_threadn_get_current_task_id(int thread_id);
int g2d_threadn_get_last_finished_task_id(int thread_id);

int g2d_threadn_get_timeout_task_id(int thread_id);
int g2d_threadn_get_timeout_cmd_id(int thread_id);

void g2d_threadn_set_rcq_head(int thread_id, __u64 addr, __u32 len);
void g2d_threadn_set_rcq_task_end_irq_gen(int thread_id, int en);
void g2d_threadn_set_rcq_task_cmd_num(int thread_id, unsigned int num);
void g2d_threadn_set_rcq_update_en(int thread_id, int en);

#endif /* End of file */
