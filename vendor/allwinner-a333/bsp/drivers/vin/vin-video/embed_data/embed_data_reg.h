/* SPDX-License-Identifier: GPL-2.0 */

 /*
  * embed_data_reg.h
  *
  * Copyright (c) 2007-2024 Allwinnertech Co., Ltd.
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
#ifndef _EMBED_DATA_REG_H_
#define _EMBED_DATA_REG_H_

#define TDM_INFO_SIZE  (8 << 2)
#define ISP_INFO_SIZE  (8 << 2)
#define ITP_REGS_SIZE  (8 << 2)
#define VIPP_INFO_SIZE (8 << 2)
#define MOTION_SIZE    (192 << 2)
#define TEXTURE_SIZE   (192 << 2)

#define TDM_INFO_REG_OFFSET  (0)
#define ISP_INFO_REG_OFFSET  (TDM_INFO_REG_OFFSET + TDM_INFO_SIZE)
#define ITP_REGS_REG_OFFSET  (ISP_INFO_REG_OFFSET + ISP_INFO_SIZE)
#define VIPP_INFO_REG_OFFSET (ITP_REGS_REG_OFFSET + ITP_REGS_SIZE)
#define MOTION_REG_OFFSET    (VIPP_INFO_REG_OFFSET + VIPP_INFO_SIZE)
#define TEXTURE_REG_OFFSET   (MOTION_REG_OFFSET + MOTION_SIZE)

/* tdm info */
#define ISP_FRM_FLAG_REG                        0x000
#define TDM_TIME_BASE_REG                       0x004
#define TDM_RX_TIME_OFFSET_REG                  0x008
#define TDM_RX_FRM_CONT_REG                     0x00c

/* iso info */
#define ISPFE_STAT_EN_REG                       0x000
#define ISP_VALID_IMAGE_SIZE_REG                0x004

/* itp info */
#define ISPFE_INTPO_CFG0_REG                    0x000
#define ISPFE_INTPO_CFG2_REG                    0x004
#define ISPFE_INTPO_CFG3_REG                    0x008
#define ISPFE_INTPO_CFG1_REG                    0x00C

/* vipp info */
#define VIPP_CORP_START_POS_REG                 0x000
#define VIPP_CORP_SIZE_REG                      0x004
#define VIPP_SC_OUT_SIZE_REG                    0x00C

/* tdm info reg */
typedef union {
	unsigned int dwval;
	struct {
		unsigned int isp_frm_error_flag:1;
		unsigned int res0:3;
		unsigned int bk_frm_error_flag:1;
		unsigned int res1:3;
		unsigned int bk_vflip_en:1;
		unsigned int bk_hflip_en:1;
		unsigned int res2:22;
	} bits;
} ISP_FRM_FLAG_REG_t;

typedef union {
	unsigned int dwval;
	struct {
		unsigned int cycle:24;
		unsigned int tdm_rx_finish_st:1;
		unsigned int awnn_bk_finish_st:1;
		unsigned int tdm_rx_to_tx_mode:2;
		unsigned int tdm_rx_npu_en:1;
		unsigned int tdm_rx_awnn_en:1;
		unsigned int tdm_rx_normal_en:1;
		unsigned int res0:1;
	} bits;
} TDM_RX_TIME_OFFSET_REG_t;

/* isp info reg */
typedef union {
	unsigned int dwval;
	struct {
		unsigned int d3d_enable:1;
		unsigned int res0:3;
		unsigned int sharp_enable:1;
		unsigned int res1:27;
	} bits;
} ISPFE_STAT_EN_REG_t;

typedef union {
	unsigned int dwval;
	struct {
		unsigned int valid_width:14;
		unsigned int res0:2;
		unsigned int valid_height:1;
		unsigned int res1:2;
	} bits;
} ISP_VALID_IMAGE_SIZE_REG_t;

/* itp info reg */
typedef union {
	unsigned int dwval;
	struct {
		unsigned int inter_block_w_num:5;
		unsigned int res0:3;
		unsigned int inter_block_h_num:5;
		unsigned int res1:19;
	} bits;
} ISPFE_INTPO_CFG0_REG_t;

typedef union {
	unsigned int dwval;
	struct {
		unsigned int coord_x:5;
		unsigned int res0:3;
		unsigned int coord_y:5;
		unsigned int res1:3;
		unsigned int roi_wnum:5;
		unsigned int res2:3;
		unsigned int roi_hnum:5;
		unsigned int res3:3;
	} bits;
} ISPFE_INTPO_CFG2_REG_t;

typedef union {
	unsigned int dwval;
	struct {
		unsigned int intrp_init_xphase:12;
		unsigned int res0:4;
		unsigned int intrp_init_yphase:12;
		unsigned int res1:4;
	} bits;
} ISPFE_INTPO_CFG3_REG_t;

typedef union {
	unsigned int dwval;
	struct {
		unsigned int intrp_x_step:12;
		unsigned int res0:4;
		unsigned int intrp_y_step:12;
		unsigned int res1:4;
	} bits;
} ISPFE_INTPO_CFG1_REG_t;

/* vipp info reg */
typedef union {
	unsigned int dwval;
	struct {
		unsigned int crop_hor_st:13;
		unsigned int res0:3;
		unsigned int crop_ver_st:13;
		unsigned int res1:3;
	} bits;
} VIPP_CORP_START_POS_REG_t;

typedef union {
	unsigned int dwval;
	struct {
		unsigned int crop_width:13;
		unsigned int res0:3;
		unsigned int crop_height:13;
		unsigned int res1:3;
	} bits;
} VIPP_CORP_SIZE_REG_t;

typedef union {
	unsigned int dwval;
	struct {
		unsigned int sc_width:13;
		unsigned int res0:3;
		unsigned int sc_height:13;
		unsigned int res1:3;
	} bits;
} VIPP_SC_OUT_SIZE_REG_t;

#endif /* end of _EMBED_DATA_REG_H_ */