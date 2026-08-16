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
#ifndef _G2D_CORE_H
#define _G2D_CORE_H
#include <linux/idr.h>
#include <linux/list.h>
#include "g2d_driver_i.h"
#include "g2d_ovl_v.h"
#include "g2d_ovl_u.h"
#include "g2d_wb.h"
#include "g2d_bld.h"
#include "g2d_scal.h"
#include "g2d_top.h"
#include "g2d_rotate.h"
#include "g2d_core.h"

#define REG_INTERVAL 0x04
#define HEXADECIMAL  0x10
#define REG_CL       0x0c

#define G2D_BYTE_ALIGN(x) (((x + (4*1024-1)) >> 12) << 12)

#define G2D_TOP                   (0x00000)
#define G2D_HYPER_THREAD_RCQ      (0x00100)
#define G2D_CORE                  (0x00300)
#define G2D_BLD                   (0x00400)
#define G2D_V0                    (0x00800)
#define G2D_UI0                   (0x01000)
#define G2D_UI1                   (0x01800)
#define G2D_UI2                   (0x02000)
#define G2D_WB                    (0x03000)
#define G2D_VSU                   (0x08000)
#define G2D_ROT                   (0x28000)
#define G2D_GSU                   (0x30000)


enum frame_type {
	ROTATE_FRAME     = 0x0,
	MIXER_FRAME      = 0x1,
};

struct g2d_frame;
struct g2d_task;
/*
 * frame
 */
struct g2d_frame {
	__u32 frame_id;
	u8 __iomem *g2d_base;
	enum frame_type type;
	struct core_submodule *core;
	struct ovl_v_submodule *ovl_v;
	struct ovl_u_submodule *ovl_u;
	struct blender_submodule *bld;
	struct scaler_submodule *scal;
	struct wb_submodule *wb;
	struct rot_submodule *rot;
	struct dmabuf_item *src_item;
	struct dmabuf_item *dst_item;
	struct dmabuf_item *ptn_item;
	struct dmabuf_item *mask_item;
	__s32 (*destory)(struct g2d_frame *p_frame);
	__s32 (*apply)(struct g2d_frame *p_frame,
		     struct mixer_para *p_para);
	__s32 (*frame_mem_setup)(struct g2d_frame *p_frame,
				 struct mixer_para *p_para,
				 struct g2d_task *p_task);
	__u32 (*frame_get_reg_block_num)(struct g2d_frame *p_frame);
	__u32 (*frame_get_rcq_mem_size)(struct g2d_frame *p_frame);
};

/*
 * task
 */
struct g2d_task {
	struct list_head list;
	__u32 task_id;
	__u32 frame_cnt;
	__u32 thread_id;
	struct g2d_frame *frame;
	struct g2d_rcq_mem_info *p_rcq_info;
	__g2d_info_t *p_g2d_info;
	__s32 (*mem_setup)(struct g2d_task *p_task,
				 struct mixer_para *p_para);
	__s32 (*apply)(struct g2d_task *p_task);
	__s32 (*destory)(struct g2d_task *p_task);
};

struct g2d_time_info *get_g2d_time_inf(void);
void g2d_bsp_set_base(unsigned long base);
__s32 g2d_bsp_open(int thread_id);
__s32 g2d_bsp_close(int thread_id);
__s32 g2d_bsp_reset(int thread_id);
void g2d_set_dmabuf_dev(struct device *dmabuf_dev);
int g2d_dma_map(int fd, struct dmabuf_item *item);
void g2d_dma_unmap(struct dmabuf_item *item);
__s32 g2d_set_info(g2d_image_enh *g2d_img, struct dmabuf_item *item);
int g2d_set_image_addr(struct dmabuf_item **p_item, g2d_image_enh *p_img);
__s32 g2d_byte_cal(__u32 format, __u32 *ycnt, __u32 *ucnt, __u32 *vcnt);
__u32 cal_align(__u32 width, __u32 align);
__s32 g2d_image_check(g2d_image_enh *p_image);
__s32 g2d_lbc_rot_set_para(__g2d_info_t *p_g2d_info, g2d_lbc_rot *para);
__s32 g2d_rotate_set_para(__g2d_info_t *p_g2d_info, g2d_image_enh *src, g2d_image_enh *dst, __u32 flag);
void g2d_bsp_handle_irq(__g2d_drv_t *g2d_ext_hd);
int g2d_wait_cmd_finish(unsigned int timeout);

/*
 * @name       :g2d_task_process
 * @brief      :mixer task process
 * @param[IN]  :p_g2d_info:pointer of hardware resource
 * @param[IN]  :p_para:mixer task parameter
 * @param[IN]  :frame_len:number of frame
 * @return     :0 if success, -1 else
 */
__s32 g2d_task_process(__g2d_info_t *p_g2d_info, struct mixer_para *p_para,
			 unsigned int frame_len);

/*
 * @name       :g2d_task_create
 * @brief      :create mixer task instance include memory allocate
 * @param[IN]  :p_g2d_info:pointer of hardware resource
 * @param[IN]  :p_para:mixer task parameter
 * @param[IN]  :frame_len:number of frame
 * @return     :task_id >= 1, else fail
 */
__u32 g2d_task_create(__g2d_info_t *p_g2d_info, struct mixer_para *p_para,
			 unsigned int frame_len);

/*
 * @name       :g2d_task_get_by_id
 * @brief      :get task instance of specified task id
 * @param[IN]  :id: task id
 * @return     :pointer of mixer task or NULL if fail
 */
struct g2d_task *g2d_task_get_by_id(__u32 id);

#endif /* End of file */
