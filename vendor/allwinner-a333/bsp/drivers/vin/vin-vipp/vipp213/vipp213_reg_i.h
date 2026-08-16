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

#ifndef __VIPP213__REG__I__H__
#define __VIPP213__REG__I__H__

/*
 * Detail information of top registers
 */
#define VIPP_TOP_EN_REG_OFF			0X000
#define VIPP_CLK_GATING_EN			0
#define VIPP_CLK_GATING_EN_MASK			(0X1 << VIPP_CLK_GATING_EN)
#define VIPP_CAP_EN				1
#define VIPP_CAP_EN_MASK			(0X1 << VIPP_CAP_EN)
#define VIPP_WORK_MODE				4
#define VIPP_WORK_MODE_MASK			(0X1 << VIPP_WORK_MODE)
#define VIPP_MODULE_CLK_BACK_DOOR		6
#define VIPP_MODULE_CLK_BACK_DOOR_MASK		(0X1 << VIPP_MODULE_CLK_BACK_DOOR)
#define VIPP_VER_EN				7
#define VIPP_VER_EN_MASK			(0X1 << VIPP_VER_EN)
#define VIPP_SPEED_MODE_BD			28
#define VIPP_SPEED_MODE_BD_MASK			(0X3 << VIPP_SPEED_MODE_BD)
#define VIPP_SCALE_RESET_BD			31
#define VIPP_SCALE_RESET_BD_MASK		(0X1 << VIPP_SCALE_RESET_BD)

#define VIPP_VER_REG_OFF			0X010
#define VIPP_SMALL_VER				0
#define VIPP_SMALL_VER_MASK			(0XFFF << VIPP_SMALL_VER)
#define VIPP_BIG_VER				12
#define VIPP_BIG_VER_MASK			(0XFFF << VIPP_BIG_VER)

#define VIPP_FEATURE_REG_OFF			0X014
#define VIPP_YUV422TO420			8
#define VIPP_YUV422TO420_MASK			(0X1 << VIPP_YUV422TO420)
#define VIPP_ORL_EXIST				9
#define VIPP_ORL_EXIST_MASK			(0X1 << VIPP_ORL_EXIST)

#define VIPP_INT_BYPASS_REG_OFF			0X020
#define VIPP_CHN0_REG_LOAD_EN			8
#define VIPP_CHN0_REG_LOAD_EN_MASK		(0X1 << VIPP_CHN0_REG_LOAD_EN)
#define VIPP_CHN1_REG_LOAD_EN			14
#define VIPP_CHN1_REG_LOAD_EN_MASK		(0X1 << VIPP_CHN1_REG_LOAD_EN)
#define VIPP_CHN2_REG_LOAD_EN			20
#define VIPP_CHN2_REG_LOAD_EN_MASK		(0X1 << VIPP_CHN2_REG_LOAD_EN)
#define VIPP_CHN3_REG_LOAD_EN			26
#define VIPP_CHN3_REG_LOAD_EN_MASK		(0X1 << VIPP_CHN3_REG_LOAD_EN)

#define VIPP_INT_BYPASS1_REG_OFF		0X024

#define VIPP_INT_STATUS_REG_OFF			0X030
#define VIPP_ID_LOST_PD				0
#define VIPP_ID_LOST_PD_MASK			(0X1 << VIPP_ID_LOST_PD)
#define VIPP_AHB_MBUS_W_PD			1
#define VIPP_AHB_MBUS_W_PD_MASK			(0X1 << VIPP_AHB_MBUS_W_PD)
#define VIPP_CHN0_YUV2RGB_FMT_ERR_PD		4
#define VIPP_CHN0_YUV2RGB_FMT_ERR_PD_MASK	(0X1 << VIPP_CHN0_YUV2RGB_FMT_ERR_PD)
#define VIPP_CHN1_YUV2RGB_FMT_ERR_PD		5
#define VIPP_CHN1_YUV2RGB_FMT_ERR_PD_MASK	(0X1 << VIPP_CHN1_YUV2RGB_FMT_ERR_PD)
#define VIPP_CHN2_YUV2RGB_FMT_ERR_PD		6
#define VIPP_CHN2_YUV2RGB_FMT_ERR_PD_MASK	(0X1 << VIPP_CHN2_YUV2RGB_FMT_ERR_PD)
#define VIPP_CHN3_YUV2RGB_FMT_ERR_PD		7
#define VIPP_CHN3_YUV2RGB_FMT_ERR_PD_MASK	(0X1 << VIPP_CHN3_YUV2RGB_FMT_ERR_PD)

#define VIPP_CHN0_REG_LOAD_PD			8
#define VIPP_CHN0_REG_LOAD_PD_MASK		(0X1 << VIPP_CHN0_REG_LOAD_PD)
#define VIPP_CHN0_FRM_LOST_PD			9
#define VIPP_CHN0_FRM_LOST_PD_MASK		(0X1 << VIPP_CHN0_FRM_LOST_PD)
#define VIPP_CHN0_HB_SHORT_PD			10
#define VIPP_CHN0_HB_SHORT_PD_MASK		(0X1 << VIPP_CHN0_HB_SHORT_PD)
#define VIPP_CHN0_PAPA_NOTREADY_PD		11
#define VIPP_CHN0_PAPA_NOTREADY_PD_MASK		(0X1 << VIPP_CHN0_PAPA_NOTREADY_PD)
#define VIPP_CHN0_HSHORT_PD			12
#define VIPP_CHN0_HSHORT_PD_MASK		(0X1 << VIPP_CHN0_HSHORT_PD)
#define VIPP_CHN0_VSHORT_PD			13
#define VIPP_CHN0_VSHORT_PD_MASK		(0X1 << VIPP_CHN0_VSHORT_PD)

#define VIPP_CHN1_REG_LOAD_PD			14
#define VIPP_CHN1_REG_LOAD_PD_MASK		(0X1 << VIPP_CHN1_REG_LOAD_PD)
#define VIPP_CHN1_FRM_LOST_PD			15
#define VIPP_CHN1_FRM_LOST_PD_MASK		(0X1 << VIPP_CHN1_FRM_LOST_PD)
#define VIPP_CHN1_HB_SHORT_PD			16
#define VIPP_CHN1_HB_SHORT_PD_MASK		(0X1 << VIPP_CHN1_HB_SHORT_PD)
#define VIPP_CHN1_PAPA_NOTREADY_PD		17
#define VIPP_CHN1_PAPA_NOTREADY_PD_MASK		(0X1 << VIPP_CHN1_PAPA_NOTREADY_PD)
#define VIPP_CHN1_HSHORT_PD			18
#define VIPP_CHN1_HSHORT_PD_MASK		(0X1 << VIPP_CHN1_HSHORT_PD)
#define VIPP_CHN1_VSHORT_PD			19
#define VIPP_CHN1_VSHORT_PD_MASK		(0X1 << VIPP_CHN1_VSHORT_PD)

#define VIPP_CHN2_REG_LOAD_PD			20
#define VIPP_CHN2_REG_LOAD_PD_MASK		(0X1 << VIPP_CHN2_REG_LOAD_PD)
#define VIPP_CHN2_FRM_LOST_PD			21
#define VIPP_CHN2_FRM_LOST_PD_MASK		(0X1 << VIPP_CHN2_FRM_LOST_PD)
#define VIPP_CHN2_HB_SHORT_PD			22
#define VIPP_CHN2_HB_SHORT_PD_MASK		(0X1 << VIPP_CHN2_HB_SHORT_PD)
#define VIPP_CHN2_PAPA_NOTREADY_PD		23
#define VIPP_CHN2_PAPA_NOTREADY_PD_MASK		(0X1 << VIPP_CHN2_PAPA_NOTREADY_PD)
#define VIPP_CHN2_HSHORT_PD			24
#define VIPP_CHN2_HSHORT_PD_MASK		(0X1 << VIPP_CHN2_HSHORT_PD)
#define VIPP_CHN2_VSHORT_PD			25
#define VIPP_CHN2_VSHORT_PD_MASK		(0X1 << VIPP_CHN2_VSHORT_PD)

#define VIPP_CHN3_REG_LOAD_PD			26
#define VIPP_CHN3_REG_LOAD_PD_MASK		(0X1 << VIPP_CHN3_REG_LOAD_PD)
#define VIPP_CHN3_FRM_LOST_PD			27
#define VIPP_CHN3_FRM_LOST_PD_MASK		(0X1 << VIPP_CHN3_FRM_LOST_PD)
#define VIPP_CHN3_HB_SHORT_PD			28
#define VIPP_CHN3_HB_SHORT_PD_MASK		(0X1 << VIPP_CHN3_HB_SHORT_PD)
#define VIPP_CHN3_PAPA_NOTREADY_PD		29
#define VIPP_CHN3_PAPA_NOTREADY_PD_MASK		(0X1 << VIPP_CHN3_PAPA_NOTREADY_PD)
#define VIPP_CHN3_HSHORT_PD			30
#define VIPP_CHN3_HSHORT_PD_MASK		(0X1 << VIPP_CHN3_HSHORT_PD)
#define VIPP_CHN3_VSHORT_PD			31
#define VIPP_CHN3_VSHORT_PD_MASK		(0X1 << VIPP_CHN3_VSHORT_PD)

#define VIPP_INT_STATUS1_REG_OFF		0X034
#define VIPP_CHN0_FRM_START_PD			0
#define VIPP_CHN0_FRM_START_PD_MASK		(0X1 << VIPP_CHN0_FRM_START_PD)
#define VIPP_CHN0_FRM_DONE_PD			1
#define VIPP_CHN0_FRM_DONE_PD_MASK		(0X1 << VIPP_CHN0_FRM_DONE_PD)
#define VIPP_CHN1_FRM_START_PD			8
#define VIPP_CHN1_FRM_START_PD_MASK		(0X1 << VIPP_CHN1_FRM_START_PD)
#define VIPP_CHN1_FRM_DONE_PD			9
#define VIPP_CHN1_FRM_DONE_PD_MASK		(0X1 << VIPP_CHN1_FRM_DONE_PD)
#define VIPP_CHN2_FRM_START_PD			16
#define VIPP_CHN2_FRM_START_PD_MASK		(0X1 << VIPP_CHN2_FRM_START_PD)
#define VIPP_CHN2_FRM_DONE_PD			17
#define VIPP_CHN2_FRM_DONE_PD_MASK		(0X1 << VIPP_CHN2_FRM_DONE_PD)
#define VIPP_CHN3_FRM_START_PD			24
#define VIPP_CHN3_FRM_START_PD_MASK		(0X1 << VIPP_CHN3_FRM_START_PD)
#define VIPP_CHN3_FRM_DONE_PD			25
#define VIPP_CHN3_FRM_DONE_PD_MASK		(0X1 << VIPP_CHN3_FRM_DONE_PD)

#define VIPP_RETURN_INF_REG_OFF			0X038
#define VIPP_SUB_ST				0
#define VIPP_SUB_ST_MASK			(0XFFFF << VIPP_SUB_ST)
#define VIPP_SUB_ID				16
#define VIPP_SUB_ID_MASK			(0X3 << VIPP_SUB_ID)

/*
 * Detail information of ch registers
 */
#define VIPP_CH_OFFSET				0X80
#define VIPP_CH_AMONG_OFFSET			0X40

#define VIPP_CH_CTRL_REG_OFF			0X0
#define VIPP_CHN_CAP_EN				0
#define VIPP_CHN_CAP_EN_MASK			(0X1 << VIPP_CHN_CAP_EN)
#define VIPP_PARA_READY				2
#define VIPP_PARA_READY_MASK			(0X1 << VIPP_PARA_READY)
#define VIPP_BYPASS_MODE			4
#define VIPP_BYPASS_MODE_MASK			(0X1 << VIPP_BYPASS_MODE)
#define VIPP_EMBED_ISPBE_CFG_SEL		29
#define VIPP_EMBED_ISPBE_CFG_SEL_MASK		(0X1 << VIPP_EMBED_ISPBE_CFG_SEL)
#define VIPP_EMBED_ISPBE_INFO_EN		30
#define VIPP_EMBED_ISPBE_INFO_EN_MASK		(0X1 << VIPP_EMBED_ISPBE_INFO_EN)
#define VIPP_EMBED_TOP_EN			31
#define VIPP_EMBED_TOP_EN_MASK			(0X1 << VIPP_EMBED_TOP_EN)

#define VIPP_REG_LOAD_ADDR_REG_OFF		0X010

/*
 * Detail information of load registers
 */
#define VIPP_LOAD_OFFSET			0X240

#define VIPP_MODULE_EN_REG_OFF			0X000
#define VIPP_MODE_REG_OFF			0X004
#define VIPP_CROP_START_REG_OFF			0X010
#define VIPP_CROP_SIZE_REG_OFF			0X014
#define VIPP_DS_CFG_REG_OFF			0X020
#define VIPP_DS_SIZE_REG_OFF			0X024
#define VIPP_SC_SIZE_REG_OFF			0X030
#define VIPP_SC_CFG0_REG_OFF			0X034
#define VIPP_SC_CFG1_REG_OFF			0X038
#define VIPP_BILINEAR_CFG0_REG_OFF		0X040
#define VIPP_BILINEAR_CFG1_REG_OFF		0X044
#define VIPP_BILINEAR_CFG2_REG_OFF		0X048
#define VIPP_BILINEAR_CFG3_REG_OFF		0X04C

#define VIPP_CGC_GAIN_CTRL_REG_OFF		0X140
#define VIPP_CGC_CLIP_CTRL_REG			0X144
#define VIPP_YUV2RGB_CFG0_REG_OFF		0X150
#define VIPP_YUV2RGB_CFG1_REG_OFF		0X154
#define VIPP_YUV2RGB_CFG2_REG_OFF		0X158
#define VIPP_YUV2RGB_CFG3_REG_OFF		0X15c
#define VIPP_YUV2RGB_CFG4_REG_OFF		0X160
#define VIPP_YUV2RGB_CFG5_REG_OFF		0X164

#define VIPP_FE_IPL_CFG0_REG_OFF		0X170
#define VIPP_FE_IPL_CFG1_REG_OFF		0X174
#define VIPP_FE_IPL_CFG2_REG_OFF		0X178

/*
 * Detail information of load params struct
 */
typedef union {
	unsigned int dwval;
	struct {
		unsigned int ds_en:1;
		unsigned int sc_en:1;
		unsigned int nearest_en:1;
		unsigned int bilinear_en:1;
		unsigned int chroma_ds_en:1;
		unsigned int cgc_f2l_en:1;
		unsigned int yuv2rgb_en:1;
		unsigned int res0:25;
	} bits;
} VIPP_MODULE_EN_REG_t;

typedef union {
	unsigned int dwval;
	struct {
		unsigned int vipp_in_fmt:1;
		unsigned int res0:3;
		unsigned int sc_out_fmt:1;
		unsigned int nearest_out_fmt:1;
		unsigned int bilinear_out_fmt:1;
		unsigned int res1:1;
		unsigned int vipp_out_fmt:2;
		unsigned int res2:22;
	} bits;
} VIPP_OUTPUT_FMT_REG_t;

typedef union {
	unsigned int dwval;
	struct {
		unsigned int crop_hor_st:13;
		unsigned int res0:3;
		unsigned int crop_ver_st:13;
		unsigned int res1:3;
	} bits;
} VIPP_CROP_START_POSITION_REG_t;

typedef union {
	unsigned int dwval;
	struct {
		unsigned int crop_width:13;
		unsigned int res0:3;
		unsigned int crop_height:13;
		unsigned int res1:5;
	} bits;
} VIPP_CROP_SIZE_REG_t;

typedef union {
	unsigned int dwval;
	struct {
		unsigned int ds_w_num:3;
		unsigned int res0:1;
		unsigned int ds_h_num:3;
		unsigned int res1:1;
		unsigned int ds_phase:3;
		unsigned int res2:21;
	} bits;
} VIPP_DS_CFG_REG_t;

typedef union {
	unsigned int dwval;
	struct {
		unsigned int ds_width:13;
		unsigned int res0:3;
		unsigned int ds_height:13;
		unsigned int res1:3;
	} bits;
} VIPP_DS_OUTPUT_SIZE_REG_t;

typedef union {
	unsigned int dwval;
	struct {
		unsigned int sc_width:13;
		unsigned int res0:3;
		unsigned int sc_height:13;
		unsigned int res1:3;
	} bits;
} VIPP_SCALER_OUTPUT_SIZE_REG_t;

typedef union {
	unsigned int dwval;
	struct {
		unsigned int sc_weight_shift_y:4;
		unsigned int res0:4;
		unsigned int sc_weight_shift_c:5;
		unsigned int res1:3;
		unsigned int sc_ratio_precision:2;
		unsigned int res2:14;
	} bits;
} VIPP_SCALER_CFG0_REG_t;

typedef union {
	unsigned int dwval;
	struct {
		unsigned int sc_xratio:13;
		unsigned int res0:3;
		unsigned int sc_yratio:13;
		unsigned int res1:3;
	} bits;
} VIPP_SCALER_CFG1_REG_t;

typedef union {
	unsigned int dwval;
	struct {
		unsigned int bilinear_ratio_x:19;
		unsigned int res0:13;
	} bits;
} VIPP_BILINEAR_CFG0_REG_t;

typedef union {
	unsigned int dwval;
	struct {
		unsigned int bilinear_ratio_y:19;
		unsigned int res0:13;
	} bits;
} VIPP_BILINEAR_CFG1_REG_t;

typedef union {
	unsigned int dwval;
	struct {
		unsigned int bilinear_phase_x:17;
		unsigned int res0:15;
	} bits;
} VIPP_BILINEAR_CFG2_REG_t;

typedef union {
	unsigned int dwval;
	struct {
		unsigned int bilinear_phase_y:17;
		unsigned int res0:15;
	} bits;
} VIPP_BILINEAR_CFG3_REG_t;

typedef union {
	unsigned int dwval;
	struct {
		unsigned int cgc_f2l_gain_y:8;
		unsigned int cgc_f2l_gain_uv:8;
		unsigned int cgc_f2l_offset_y:8;
		unsigned int cgc_f2l_offset_uv:8;
	} bits;
} VIPP_CGC_GAIN_CTRL_REG_t;

typedef union {
	unsigned int dwval;
	struct {
		unsigned int cgc_f2l_y_min:8;
		unsigned int cgc_f2l_y_max:8;
		unsigned int cgc_f2l_uv_min:8;
		unsigned int cgc_f2l_uv_max:8;
	} bits;
} VIPP_CGC_CLIP_CTRL_REG_t;

typedef union {
	unsigned int dwval;
} VIPP_YUV2RGB_CFG_REG_t;

typedef union {
	unsigned int dwval;
	struct {
		unsigned int coord_x:5;
		unsigned int res0:3;
		unsigned int coord_y:5;
		unsigned int res1:3;
		unsigned int roi_w_num:5;
		unsigned int res2:3;
		unsigned int roi_h_num:5;
		unsigned int res3:3;
	} bits;
} VIPP_FE_IPL_CFG0_REG_t;

typedef union {
	unsigned int dwval;
	struct {
		unsigned int interp_init_x_phase:12;
		unsigned int res0:4;
		unsigned int interp_init_y_phase:12;
		unsigned int res1:4;
	} bits;
} VIPP_FE_IPL_CFG1_REG_t;

typedef union {
	unsigned int dwval;
	struct {
		unsigned int interp_x_step:12;
		unsigned int res0:4;
		unsigned int interp_y_step:12;
		unsigned int res1:4;
	} bits;
} VIPP_FE_IPL_CFG2_REG_t;

#endif /*__VIPP213__REG__I__H__*/
