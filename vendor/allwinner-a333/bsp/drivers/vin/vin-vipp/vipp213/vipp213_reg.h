/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Copyright(c) 2020 - 2023 Allwinner Technology Co.,Ltd. All rights reserved. */
/*
 *
 * Copyright (c) 2007-2021 Allwinnertech Co., Ltd.
 *
 * Authors:  Zheng ZeQun <zequnzheng@allwinnertech.com>
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

#ifndef __VIPP213__REG___H__
#define __VIPP213__REG___H__

#include <linux/types.h>
#include "../../platform/platform_cfg.h"

#if IS_ENABLED(CONFIG_ARCH_SUN252IW1)
#define SCALER_DOWN_0 8
#define SCALER_DOWN_1 4
#define SCALER_DOWN_2 2
#else
#define SCALER_DOWN_0 16
#define SCALER_DOWN_1 8
#define SCALER_DOWN_2 4
#endif

enum vipp_mode_select {
	VIPP_SCALER = 1 << 0,
	VIPP_BILINEAR = 1 << 1,
	VIPP_NEARNEST = 1 << 2,
	VIPP_ALL_NOT = 1 << 3,
};
#if IS_ENABLED(CONFIG_ARCH_SUN8IW22)
#define SCALER0_MODE_SUPPORT (VIPP_BILINEAR | VIPP_NEARNEST)
#define SCALER1_MODE_SUPPORT (VIPP_BILINEAR | VIPP_NEARNEST)
#define SCALER2_MODE_SUPPORT (VIPP_ALL_NOT)
#define SCALER3_MODE_SUPPORT (VIPP_ALL_NOT)
#else
#define SCALER0_MODE_SUPPORT (VIPP_ALL_NOT)
#define SCALER1_MODE_SUPPORT (VIPP_SCALER)
#define SCALER2_MODE_SUPPORT (VIPP_SCALER | VIPP_BILINEAR | VIPP_NEARNEST)
#define SCALER3_MODE_SUPPORT (VIPP_BILINEAR | VIPP_NEARNEST)
#endif

#define MAX_OVERLAY_NUM 0
#define MAX_COVER_NUM 0
#define MAX_ORL_NUM 0
 //decide by which vipp not support scaler/bilinear/nearest
#if IS_ENABLED(CONFIG_ARCH_SUN8IW22)
#define MAX_OSD_NUM 0
#else
#define MAX_OSD_NUM 4
#endif

#define VIPP_REG_SIZE 0x200
#define VIPP_VIRT_NUM 4
#define VIPP_CHN_INT_AMONG_OFFSET 6

#if IS_ENABLED(CONFIG_ARCH_SUN8IW22)
extern int vipp_virtual_find_ch[9];
extern int vipp_virtual_find_logic[9];
extern int vipp_virtual_find_sel[9];
#else
extern int vipp_virtual_find_ch[18];
extern int vipp_virtual_find_logic[18];
extern int vipp_virtual_find_sel[18];
#endif

/*register value*/
enum vipp_ready_flag {
	NOT_READY = 0,
	HAS_READY = 1,
};

enum vipp_embed_ispbe_cfg {
	ENCPP_CFG_FROM_ISPFE = 0,
	ENCPP_CFG_FROM_VIPP = 1,
};

enum vipp_en_sel {
	ID_LOST_EN = (1<<0),
	AHB_MBUS_W_EN = (1<<1),

	CHN0_YUV2RGB_FMT_ERR_EN = (1<<4),
	CHN1_YUV2RGB_FMT_ERR_EN = (1<<5),
	CHN2_YUV2RGB_FMT_ERR_EN = (1<<6),
	CHN3_YUV2RGB_FMT_ERR_EN = (1<<7),

	CHN0_REG_LOAD_EN = (1<<8),
	CHN0_FRAME_LOST_EN = (1<<9),
	CHN0_HBLANK_SHORT_EN = (1<<10),
	CHN0_PARA_NOT_READY_EN = (1<<11),
	CHN0_HSHORT_INT_EN = (1<<12),
	CHN0_VSHORT_INT_EN = (1<<13),

	CHN1_REG_LOAD_EN = (1<<14),
	CHN1_FRAME_LOST_EN = (1<<15),
	CHN1_HBLANK_SHORT_EN = (1<<16),
	CHN1_PARA_NOT_READY_EN = (1<<17),
	CHN1_HSHORT_INT_EN = (1<<18),
	CHN1_VSHORT_INT_EN = (1<<19),

	CHN2_REG_LOAD_EN = (1<<20),
	CHN2_FRAME_LOST_EN = (1<<21),
	CHN2_HBLANK_SHORT_EN = (1<<22),
	CHN2_PARA_NOT_READY_EN = (1<<23),
	CHN2_HSHORT_INT_EN = (1<<24),
	CHN2_VSHORT_INT_EN = (1<<25),

	CHN3_REG_LOAD_EN = (1<<26),
	CHN3_FRAME_LOST_EN = (1<<27),
	CHN3_HBLANK_SHORT_EN = (1<<28),
	CHN3_PARA_NOT_READY_EN = (1<<29),
	CHN3_HSHORT_INT_EN = (1<<30),
	CHN3_VSHORT_INT_EN = (1<<31),

	VIPP_EN_ALL = 0xffffff03,
};

enum vipp_status_sel {
	ID_LOST_PD = (1<<0),
	AHB_MBUS_W_PD = (1<<1),

	CHN0_YUV2RGB_FMT_ERR_PD = (1<<4),
	CHN1_YUV2RGB_FMT_ERR_PD = (1<<5),
	CHN2_YUV2RGB_FMT_ERR_PD = (1<<6),
	CHN3_YUV2RGB_FMT_ERR_PD = (1<<7),

	CHN0_REG_LOAD_PD = (1<<8),
	CHN0_FRAME_LOST_PD = (1<<9),
	CHN0_HBLANK_SHORT_PD = (1<<10),
	CHN0_PARA_NOT_READY_PD = (1<<11),
	CHN0_HSHORT_INT_PD = (1<<12),
	CHN0_VSHORT_INT_PD = (1<<13),

	CHN1_REG_LOAD_PD = (1<<14),
	CHN1_FRAME_LOST_PD = (1<<15),
	CHN1_HBLANK_SHORT_PD = (1<<16),
	CHN1_PARA_NOT_READY_PD = (1<<17),
	CHN1_HSHORT_INT_PD = (1<<18),
	CHN1_VSHORT_INT_PD = (1<<19),

	CHN2_REG_LOAD_PD = (1<<20),
	CHN2_FRAME_LOST_PD = (1<<21),
	CHN2_HBLANK_SHORT_PD = (1<<22),
	CHN2_PARA_NOT_READY_PD = (1<<23),
	CHN2_HSHORT_INT_PD = (1<<24),
	CHN2_VSHORT_INT_PD = (1<<25),

	CHN3_REG_LOAD_PD = (1<<26),
	CHN3_FRAME_LOST_PD = (1<<27),
	CHN3_HBLANK_SHORT_PD = (1<<28),
	CHN3_PARA_NOT_READY_PD = (1<<29),
	CHN3_HSHORT_INT_PD = (1<<30),
	CHN3_VSHORT_INT_PD = (1<<31),

	VIPP_STATUS_ALL = 0xffffff03,
};

enum vipp_en_sel1 {
	CHN0_FRM_START_EN = (1<<0),
	CHN0_FRM_DONE_EN = (1<<1),

	CHN1_FRM_START_EN = (1<<8),
	CHN1_FRM_DONE_EN = (1<<9),

	CHN2_FRM_START_EN = (1<<16),
	CHN2_FRM_DONE_EN = (1<<17),

	CHN3_FRM_START_EN = (1<<24),
	CHN3_FRM_DONE_EN = (1<<25),
};

enum vipp_status_sel1 {
	CHN0_FRM_START_PD = (1<<0),
	CHN0_FRM_DONE_PD = (1<<1),

	CHN1_FRM_START_PD = (1<<8),
	CHN1_FRM_DONE_PD = (1<<9),

	CHN2_FRM_START_PD = (1<<16),
	CHN2_FRM_DONE_PD = (1<<17),

	CHN3_FRM_START_PD = (1<<24),
	CHN3_FRM_DONE_PD = (1<<25),
};

/*register data struct for vipp100*/
enum vipp_update_flag {
	NOT_UPDATED = 0,
	HAS_UPDATED = 1,
};

/*register data struct*/
enum vipp_format {
	YUV420 = 0,
	YUV422 = 1,
	YUV2RGB888 = 2,
	YUV2RGB565 = 3,
};

struct vipp_version {
	unsigned int ver_big;
	unsigned int ver_small;
};

struct vipp_feature_list {
	unsigned int yuv422to420;
	unsigned int orl_exit;
};

struct vipp_status {
	unsigned char id_lost_pd;
	unsigned char ahb_mbus_w_pd;

	unsigned char chn0_yuv2rgb_fmt_err_pd;
	unsigned char chn1_yuv2rgb_fmt_err_pd;
	unsigned char chn2_yuv2rgb_fmt_err_pd;
	unsigned char chn3_yuv2rgb_fmt_err_pd;

	unsigned char chn0_reg_load_pd;
	unsigned char chn0_frame_lost_pd;
	unsigned char chn0_hblank_short_pd;
	unsigned char chn0_para_not_ready_pd;
	unsigned char chn0_hshort_pd;
	unsigned char chn0_vshort_pd;

	unsigned char chn1_reg_load_pd;
	unsigned char chn1_frame_lost_pd;
	unsigned char chn1_hblank_short_pd;
	unsigned char chn1_para_not_ready_pd;
	unsigned char chn1_hshort_pd;
	unsigned char chn1_vshort_pd;

	unsigned char chn2_reg_load_pd;
	unsigned char chn2_frame_lost_pd;
	unsigned char chn2_hblank_short_pd;
	unsigned char chn2_para_not_ready_pd;
	unsigned char chn2_hshort_pd;
	unsigned char chn2_vshort_pd;

	unsigned char chn3_reg_load_pd;
	unsigned char chn3_frame_lost_pd;
	unsigned char chn3_hblank_short_pd;
	unsigned char chn3_para_not_ready_pd;
	unsigned char chn3_hshort_pd;
	unsigned char chn3_vshort_pd;
};

struct vipp_status1 {
	unsigned char chn0_frm_start;
	unsigned char chn0_frm_done;

	unsigned char chn1_frm_start;
	unsigned char chn1_frm_done;

	unsigned char chn2_frm_start;
	unsigned char chn2_frm_done;

	unsigned char chn3_frm_start;
	unsigned char chn3_frm_done;
};

struct vipp_scaler_config {
	enum vipp_format sc_out_fmt;
	unsigned int sc_x_ratio;
	unsigned int sc_y_ratio;
	unsigned int sc_w_shift_c;
	unsigned int sc_w_shift_y;
	unsigned int sc_ratio_precision;
};

struct vipp_scaler_size {
	unsigned int sc_width;
	unsigned int sc_height;
};

struct vipp_bilinear_config {
	enum vipp_format nearest_out_fmt;
	enum vipp_format bilinear_out_fmt;
	unsigned int bilinear_ratio_x;
	unsigned int bilinear_ratio_y;
	unsigned int bilinear_phase_x;
	unsigned int bilinear_phase_y;
};

struct vipp_ds_config {
	unsigned int ds_phase;
	unsigned int ds_h_num;
	unsigned int ds_w_num;
};

struct vipp_ds_size {
	unsigned int ds_width;
	unsigned int ds_height;
};

struct vipp_crop {
	unsigned int hor;
	unsigned int ver;
	unsigned int width;
	unsigned int height;
};

struct ispfe_interp_cfg {
	unsigned char coord_x;
	unsigned char coord_y;
	unsigned char roi_w_num;
	unsigned char roi_h_num;

	short interp_init_x_phase;
	short interp_init_y_phase;

	unsigned short interp_x_step;
	unsigned short interp_y_step;
};

/* osd cfg, not use */
enum vipp_osd_argb {
	ARGB1555 = 0,
	ARGB4444 = 1,
	ARGB8888 = 2,
};

struct vipp_osd_config {
	unsigned char osd_ov_en;
	unsigned char osd_cv_en;
	unsigned char osd_orl_en;
	enum vipp_osd_argb osd_argb_mode;
	unsigned char osd_stat_en;
	unsigned int osd_orl_width;
	short osd_ov_num;
	short osd_cv_num;
	short osd_orl_num;
};

struct vipp_rgb2yuv_factor {
	unsigned int jc0;
	unsigned int jc1;
	unsigned int jc2;
	unsigned int jc3;
	unsigned int jc4;
	unsigned int jc5;
	unsigned int jc6;
	unsigned int jc7;
	unsigned int jc8;
	unsigned int jc9;
	unsigned int jc10;
	unsigned int jc11;
};

struct vipp_osd_overlay_cfg {
	unsigned int h_start;
	unsigned int h_end;
	unsigned int v_start;
	unsigned int v_end;
	unsigned int alpha;
	unsigned int inv_en;
	unsigned int inv_th;
	unsigned int inv_w_rgn;
	unsigned int inv_h_rgn;
};

struct vipp_osd_cover_cfg {
	unsigned int h_start;
	unsigned int h_end;
	unsigned int v_start;
	unsigned int v_end;
};

struct vipp_osd_cover_data {
	unsigned int y;
	unsigned int u;
	unsigned int v;
};

struct vipp_osd_para_config {
	struct vipp_osd_overlay_cfg overlay_cfg[MAX_OVERLAY_NUM + 1];
	struct vipp_osd_cover_cfg cover_cfg[MAX_COVER_NUM + 1];
	struct vipp_osd_cover_data cover_data[MAX_COVER_NUM + 1];
	struct vipp_osd_cover_cfg orl_cfg[MAX_ORL_NUM + 1];
	struct vipp_osd_cover_data orl_data[MAX_ORL_NUM + 1];
};

/*
 * Detail information of top function
 */
int vipp_set_base_addr(unsigned int id, vin_dma_addr_t addr);
void vipp_top_clk_en(unsigned int id, unsigned int en);
void vipp_cap_enable(unsigned int id);
void vipp_cap_disable(unsigned int id);
void vipp_ver_en(unsigned int id, unsigned int en);
void vipp_version_get(unsigned int id, struct vipp_version *v);
void vipp_work_mode(unsigned int id, unsigned int mode);
void vipp_feature_list_get(unsigned int id, struct vipp_feature_list *fl);
void vipp_irq_enable(unsigned int id, unsigned int irq_flag);
void vipp_irq_disable(unsigned int id, unsigned int irq_flag);
unsigned int vipp_get_irq_en(unsigned int id, unsigned int irq_flag);
void vipp_get_status(unsigned int id, struct vipp_status *status);
void vipp_clear_status(unsigned int id, enum vipp_status_sel sel);
void vipp_irq1_enable(unsigned int id, unsigned int irq_flag);
void vipp_irq1_disable(unsigned int id, unsigned int irq_flag);
void vipp_get_status1(unsigned int id, struct vipp_status1 *status);
void vipp_clear_status1(unsigned int id, enum vipp_status_sel1 sel);
/*
 * Detail information of chn function
 */
void vipp_chn_cap_enable(unsigned int id);
void vipp_chn_cap_disable(unsigned int id);
void vipp_set_para_ready(unsigned int id, enum vipp_ready_flag flag);
void vipp_chn_cap_disable_para_notready(unsigned int id);
void vipp_chn_bypass_mode(unsigned int id, unsigned int mode);
void vipp_chn_embed_top_en(unsigned int id, unsigned int en);
void vipp_chn_embed_ispbe_info_en(unsigned int id, unsigned int en);
void vipp_chn_select_embed_ispbe_cfg_sel(unsigned int id, enum vipp_embed_ispbe_cfg cfg);
void vipp_set_reg_load_addr(unsigned int id, vin_dma_addr_t dma_addr);

/*
 * Detail information of load function
 */
int vipp_map_reg_load_addr(unsigned int id, vin_dma_addr_t vaddr);
void vipp_downsample_en(unsigned int id, unsigned int en);
void vipp_scaler_en(unsigned int id, unsigned int en);
void vipp_nearest_en(unsigned int id, unsigned int en);
void vipp_bilinear_en(unsigned int id, unsigned int en);
void vipp_chroma_ds_en(unsigned int id, unsigned int en);
void vipp_cgc_f2l_en(unsigned int id, unsigned int en);
void vipp_yuv2rgb_en(unsigned int id, unsigned int en);
void vipp_scaler_cfg(unsigned int id, struct vipp_scaler_config *cfg);
void vipp_bilinear_cfg(unsigned int id, struct vipp_bilinear_config *cfg);
void vipp_nearest_cfg(unsigned int id, struct vipp_bilinear_config *cfg);
void vipp_scaler_output_fmt(unsigned int id, enum vipp_format);
void vipp_output_fmt_cfg(unsigned int id, enum vipp_format out_fmt, enum vipp_format in_fmt);
void vipp_set_crop(unsigned int id, struct vipp_crop *crop);
void vipp_downsample_cfg(unsigned int id, struct vipp_ds_config *cfg);
void vipp_downsample_output_size(unsigned int id, struct vipp_ds_size *size);
void vipp_scaler_output_size(unsigned int id, struct vipp_scaler_size *size);
void vipp_cgc_gain_cfg(unsigned int id, int val[][4], int rows);
void vipp_cgc_clip_cfg(unsigned int id, int val[][4], int rows);
void vipp_yuv2rgb_coef_cfg(unsigned int id);
void vipp_ispfe_interp_cfg(unsigned int id, struct ispfe_interp_cfg *cfg);

/*
 * Detail information of vipp100 function
 */
void vipp_osd_cfg(unsigned int id, struct vipp_osd_config *cfg);
void vipp_osd_para_cfg(unsigned int id, struct vipp_osd_para_config *para,
				struct vipp_osd_config *cfg);
void vipp_set_osd_ov_update(unsigned int id, enum vipp_update_flag flag);
void vipp_set_osd_cv_update(unsigned int id, enum vipp_update_flag flag);
void vipp_set_osd_para_load_addr(unsigned int id, vin_dma_addr_t dma_addr);
int vipp_map_osd_para_load_addr(unsigned int id, vin_dma_addr_t vaddr);
void vipp_set_osd_stat_load_addr(unsigned int id, vin_dma_addr_t dma_addr);
void vipp_set_osd_bm_load_addr(unsigned int id, vin_dma_addr_t dma_addr);
void vipp_osd_en(unsigned int id, unsigned int en);
void vipp_osd_rgb2yuv(unsigned int id, struct vipp_rgb2yuv_factor *factor);
void vipp_osd_hvflip(unsigned int id, int hflip, int vflip);

#endif /* __VIPP213__REG___H__ */
