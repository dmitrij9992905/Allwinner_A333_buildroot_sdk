/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Copyright(c) 2020 - 2023 Allwinner Technology Co.,Ltd. All rights reserved. */

/* SPDX-License-Identifier: GPL-2.0 */
/*
 * isp610_reg_cfg.h
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

#ifndef _ISP610_REG_CFG_H_
#define _ISP610_REG_CFG_H_

#include "../../platform/platform_cfg.h"

#define ISP610_MAX_NUM 4
#define ISP_VIRT_NUM 4

#define ISP_ADDR_BIT_R_SHIFT 2

/*load data*/
#define ISP_LOAD_DRAM_SIZE			0x101C0
#define ISP_AHB_REG_SIZE			0x40
#define ISP_LOAD_REG_SIZE			0xF00
#define ISP_FE_TBL_SIZE				0x1800
#define ISP_BAYER_TABLE_SIZE			0x8580
#define ISP_RGB_TABLE_SIZE			0x0500
#define ISP_YUV_TABLE_SIZE			0x5040

/*save data*/
#define ISP_SAVE_DRAM_SIZE			0x26500
#define ISP_SAVE_REG_SIZE			0x0100
#define ISP_STATISTIC_SIZE			0x26400
#define ISP_STAT_TOTAL_SIZE			0x26500

/*save and load data*/
#define ISP_SAVE_LOAD_DRAM_SIZE			0x1600
#define ISP_SAVE_LOAD_REG_SIZE			0x0100
#define ISP_SAVE_LOAD_STATISTIC_SIZE		0x1500

/*fe table*/
#define ISP_CH0_MSC_FE_TBL_SIZE			0x0800
#define ISP_CH1_MSC_FE_TBL_SIZE			0x0800
#define ISP_CH2_MSC_FE_TBL_SIZE			0x0800

/*bayer table*/
#define ISP_RSC_TBL_SIZE			0x0800
//#define ISP_MSC_TBL_SIZE			0x0800
#define ISP_D2D_EB_DNR_TBL_SIZE			0x0300
#define ISP_RESERVE0_TBL_SIZE			0x0100
#define ISP_D3D_DK_TBL_SIZE			0x0800
#define ISP_D3D_EB_DNR_TBL_SIZE			0x0300
#define ISP_DPC_SDPIXEL_TBL_SIZE		0x1000
#define ISP_DPC_SPIXELG_POS_TBL_SIZE		0x01E0
#define ISP_GCA_HV_OFFSET_TBL_SIZE		0x0D48
#define ISP_FPN_CF_GAIN_TBL_SIZE		0x2000
#define ISP_PLTM_GTM_LM_TBL_SIZE		0x2000

/*rgb table*/
#define ISP_RGB_GAMMA_TBL_SIZE			0x0400
#define ISP_RGB_SHARP_TBL_SIZE			0x0100

/*yuv table*/
//#define ISP_CEM_TBL0_SIZE			0x1700
//#define ISP_CEM_TBL1_SIZE			0x1440
//#define ISP_CEM_TBL2_SIZE			0x1200
//#define ISP_CEM_TBL3_SIZE			0x1200
#define ISP_DRC_TBL_SIZE			0x0100

/*save statistics*/
#define ISP_STAT_AFS_MEM_SIZE			0x0200
#define ISP_STAT_D3DM_MEM_SIZE			0x0300
#define ISP_STAT_SHARP_TEXTURE_MEM_SIZE		0x0300
#define ISP_STAT_HIST0_MEM_SIZE			0x0400
#define ISP_STAT_HIST1_MEM_SIZE			0x0400
#define ISP_STAT_PLTM_LUM_SIZE			0x1800
#define ISP_STAT_AF_HL_CNT_SIZE			0x0c00
#define ISP_STAT_AF_IIR0_AC_SIZE		0x0c00
#define ISP_STAT_AF_FIR1_AC_SIZE		0x0c00
#define ISP_STAT_AE_MEM_SIZE			0x8000
#define ISP_STAT_AWB_RGB_MEM_SIZE		0x18000

#define ISP_STAT_HIST0_MEM_OFS			0x0800

/*save and load statistics*/
#define ISP_STAT_PLTM_PKX_SIZE			0x0C00
#define ISP_STAT_D3D_K_SIZE			0x0300
#define ISP_STAT_D3D_MOTION_SIZE		0x0300
#define ISP_STAT_SHARP_TEXTURE_SIZE		0x0300

#define LENS_UPDATE				(1 << 8)
#define GAMMA_UPDATE				(1 << 9)
#define DRC_UPDATE				(1 << 10)
#define D3D_UPDATE				(1 << 13)
#define PLTM_UPDATE				(1 << 14)
#define CEM_UPDATE				(1 << 15)
#define MSC_UPDATE				(1 << 16)
#define FE0_MSC_UPDATE				(1 << 17)
#define D2D_UPDATE				(1 << 18)
#define FE1_MSC_UPDATE				(1 << 19)
#define FE2_MSC_UPDATE				(1 << 20)
#define DPC_UPDATE				(1 << 21)
#define GCA_UPDATE				(1 << 22)
#define FPN_UPDATE				(1 << 23)
#define SHARP_MOT_TXT_UPDATE			(1 << 24)

#define TABLE_UPDATE_ALL 			0x1FFE700

/*
 *  ISP MCIC enable
 */
#define ISP_MCIC0_EN				(1 << 0)
#define ISP_MCIC0_LOAD_EN			(1 << 1)
#define ISP_MCIC0_SAVE_EN			(1 << 2)
#define ISP_MCIC1_EN				(1 << 4)
#define ISP_MCIC1_LOAD_EN			(1 << 5)
#define ISP_MCIC1_SAVE_EN			(1 << 6)

#define ISP_MCIC0_LOAD_STATUS			(1 << 0)
#define ISP_MCIC1_LOAD_STATUS			(1 << 1)
#define ISP_MCIC0_SAVE_STATUS			(1 << 16)
#define ISP_MCIC1_SAVE_STATUS			(1 << 17)
#define ISP_MCIC_ALL_STATUS			(ISP_MCIC0_LOAD_STATUS | ISP_MCIC0_SAVE_STATUS | ISP_MCIC1_LOAD_STATUS | ISP_MCIC1_SAVE_STATUS)

/*
 *  ISP Module enable
 */
#define WDR_STITCH_EN				(1 << 0)
#define DPC_EN					(1 << 1)
#define CTC_EN					(1 << 2)
#define GCA_EN					(1 << 3)
#define D2D_EN					(1 << 4)
#define D3D_EN					(1 << 5)
#define BLC_EN					(1 << 6)
#define WB_EN					(1 << 7)
#define DG_EN					(1 << 8)
#define RSC_EN     				(1 << 9)
#define MSC_EN     				(1 << 10)
#define PLTM_EN					(1 << 11)
#define LCA_EN					(1 << 12)
#define SHARP_EN				(1 << 13)
#define CCM_EN					(1 << 14)
//#define CNR_EN					(1 << 15)
#define DRC_EN					(1 << 16)
#define GAMMA_EN				(1 << 17)
#define CEM_EN					(1 << 18)
#define FPN_EN					(1 << 20)
#define DPC_DLE_EN				(1 << 21)
#define AE0_EN					(1 << 22)
//#define AE1_EN					(1 << 23)
#define AE2_EN					(1 << 24)
#define AF_EN					(1 << 25)
#define AWB_EN					(1 << 26)
#define AFS_EN					(1 << 27)
#define HIST0_EN				(1 << 28)
#define HIST1_EN				(1 << 29)
#define SHARP_TXT_INFO_INIT_EN			(1 << 30)
#define D3D_MOT_INFO_INIT_EN			(1 << 31)

#define WDR_EN					(1 << 0)
//#define WDR_SPLIT_EN				(1 << 1)
#define CH0_DG_EN				(1 << 8)
#define CH1_DG_EN				(1 << 9)
#define CH2_DG_EN				(1 << 10)
//#define CH0_MSC_EN				(1 << 12)
//#define CH1_MSC_EN				(1 << 13)
//#define CH2_MSC_EN				(1 << 14)

#define ISP_MODULE0_EN_ALL			(0xffffffff)
#define ISP_MODULE1_EN_ALL			(0xffffffff)
/*
 *  ISP interrupt enable
 */
#define FINISH_INT_EN				(1 << 0)
#define START_INT_EN				(1 << 1)
#define PARA_SAVE_INT_EN			(1 << 2)
#define PARA_LOAD_INT_EN			(1 << 3)
#define N_LINE_START_INT_EN			(1 << 4)
#define FRAME_LOST_INT_EN			(1 << 8)
#define AHB_MBUS_W_INT_EN			(1 << 9)
#define HB_SHORT_INT_EN				(1 << 16)
#define CFG_ERROR_INT_EN			(1 << 17)
#define INTER_FIFO_FULL_INT_EN			(1 << 18)
#define WDMA_FIFO_FULL_INT_EN			(1 << 19)
#define WDMA_OVER_BND_INT_EN			(1 << 20)
#define RDMA_FIFO_EMPTY_INT_EN			(1 << 21)
#define LBC_ERROR_INT_EN			(1 << 22)
#define LBD_ERROR_INT_EN			(1 << 23)
#define CMB0_ACK_TO_INI_EN			(1 << 24)
#define CMB1_ACK_TO_INI_EN			(1 << 25)
#define CMB2_ACK_TO_INI_EN			(1 << 26)
#define CMB3_ACK_TO_INI_EN			(1 << 27)
#define CMB4_ACK_TO_INI_EN			(1 << 28)

#define ISP_IRQ_EN_ALL				0xffffffff

/*
 *  ISP interrupt status
 */
#define FINISH_PD				(1 << 0)
#define START_PD				(1 << 1)
#define PARA_SAVE_PD				(1 << 2)
#define PARA_LOAD_PD				(1 << 3)
#define N_LINE_START_PD				(1 << 4)
#define FRAME_LOST_PD				(1 << 8)
#define AHB_MBUS_W_PD				(1 << 9)
#define HB_SHORT_PD				(1 << 16)
#define CFG_ERROR_PD				(1 << 17)
#define INTER_FIFO_FULL_PD			(1 << 18)
#define WDMA_FIFO_FULL_PD			(1 << 19)
#define WDMA_OVER_BND_PD			(1 << 20)
#define RDMA_FIFO_EMPTY_PD			(1 << 21)
#define LBC_ERROR_PD				(1 << 22)
#define LBD_ERROR_PD				(1 << 23)
#define CMB0_ACK_TO_PD				(1 << 24)
#define CMB1_ACK_TO_PD				(1 << 25)
#define CMB2_ACK_TO_PD				(1 << 26)
#define CMB3_ACK_TO_PD				(1 << 27)
#define CMB4_ACK_TO_PD				(1 << 28)

#define ISP_IRQ_STATUS_ALL			0xffffffff

/*
 *  ISP internal0 status
 */
#define VSYNC_ERROR				(1 << 0)
#define HSYNC_ERROR				(1 << 1)
#define DVLD_ERROR				(1 << 2)
#define CH2_BTYPE_ERROR				(1 << 3)
#define CH1_BTYPE_ERROR				(1 << 4)
#define CH0_BTYPE_ERROR				(1 << 5)
#define TWO_BYTE_ERROR				(1 << 7)
#define WIDTH_ERROR				(1 << 8)
#define HEIGHT_ERROR				(1 << 9)
#define FMT_CHG_ERROR				(1 << 10)
#define AE_OUT_OF_PICTURE			(1 << 11)
#define AWB_OUT_OF_PICTURE			(1 << 12)
#define AWB_WIN_PIXEL_NUM_OVER			(1 << 13)
#define STC_WIN_W_ERROR				(1 << 14)
#define LBD_DEC_ERROR				(1 << 15)
#define LBD_DEC_SHORT_ERROR			(1 << 16)
#define LBD_DEC_LONG_ERROR			(1 << 17)

/*
 *  ISP internal status
 */
#define WDMA_D3D_REC_FIFO_OV			(1 << 0)
#define WDMA_LBC_H4DDR_FIFO_OV			(1 << 1)
#define WDMA_LBC_H4DT_FIFO_OV			(1 << 2)
#define WDMA_D2D_REC_FIFO_OV			(1 << 3)
#define WDMA_D3D_CURK_FIFO_OV			(1 << 4)
#define WDMA_D3D_CNTR_FIFO_OV			(1 << 5)
#define WDMA_PLTM_PKX_FIFO_OV			(1 << 6)
#define WDMA_PLTM_LUM_FIFO_OV			(1 << 7)
#define WDMA_AE_FIFO_OV				(1 << 8)
#define WDMA_AWB_FIFO_OV			(1 << 9)
#define RDMA_D3D_REC_FIFO_UV			(1 << 16)
#define RDMA_D2D_REC_FIFO_UV			(1 << 17)
#define RDMA_D3D_CHNK_ITP_TO			(1 << 18)
#define RDMA_D3D_CURK_FIFO_UV			(1 << 19)
#define RDMA_D3D_CNTR_FIFO_UV			(1 << 20)
#define RDMA_D3D_PREK_FIFO_UV			(1 << 21)
#define RDMA_PLTM_PKX_FIFO_UV			(1 << 22)

/*
 *  ISP debug reg -- isp status
 */
#define ISP_STATUS_IDLE				0
#define ISP_STATUS_WAIT_ID_RDY			(1 << 0)
#define ISP_STATUS_ID_REF			(1 << 1)
#define ISP_STATUS_ID_CHECK			(1 << 2)
#define ISP_STATUS_READ_SDRAM			(1 << 3)
#define ISP_STATUS_INIT				(1 << 4)
#define ISP_STATUS_RX_RDY			(1 << 5)
#define ISP_STATUS_ISP_PRO_J			(1 << 6)
#define ISP_STATUS_ISP_PRO0			(1 << 7)
#define ISP_STATUS_ISP_PRO1			(1 << 8)
#define ISP_STATUS_EMBEM_WAIT			(1 << 9)
#define ISP_STATUS_EMBEM_TX			(1 << 10)
#define ISP_STATUS_WRITE_SDRAM			(1 << 11)
#define ISP_STATUS_FINISH_END_WAIT		(1 << 12)
#define ISP_STATUS_FINISH_END			(1 << 13)
#define ISP_STATUS_CSI_END			(1 << 14)
#define ISP_STATUS_MASK				(0xffff)

/*
 *  ISP debug reg -- isp internal fifo
 */
#define ISP_ITFIFO_GCA_OV			(1 << 0)
#define ISP_ITFIFO_CTC_OV			(1 << 1)
#define ISP_ITFIFO_D3D_DIFF_OV			(1 << 4)
#define ISP_ITFIFO_D3D_LP0_OV			(1 << 5)
#define ISP_ITFIFO_D3D_LP1_OV			(1 << 6)
#define ISP_ITFIFO_D3D_KPACK_OV			(1 << 7)
#define ISP_ITFIFO_D3D_CURK_OV			(1 << 8)
#define ISP_ITFIFO_D3D_KSTP_OV			(1 << 9)
#define ISP_ITFIFO_D3D_BLK_DNR_OV		(1 << 10)
#define ISP_ITFIFO_DMSC_RGB_OV			(1 << 12)
#define ISP_ITFIFO_DMSC_RGB_UV			(1 << 13)
#define ISP_ITFIFO_DMSC_RATIO_OV		(1 << 14)
#define ISP_ITFIFO_DMSC_RATIO_UV		(1 << 15)
#define ISP_ITFIFO_LCA_BAYER_OV			(1 << 16)
#define ISP_ITFIFO_LCA_BAYER_UV			(1 << 17)
#define ISP_ITFIFO_LCA_RGB_OV			(1 << 18)
#define ISP_ITFIFO_LCA_RGB_UV			(1 << 19)
#define ISP_ITFIFO_SHARP_BAYER_OV		(1 << 20)
#define ISP_ITFIFO_SHARP_HSV_OV			(1 << 21)

/*
 *  ISP debug reg -- isp internal cmb0
 */
#define ISP_ITCMB0_D2D_MOT_TO			(1 << 0)
#define ISP_ITCMB0_D2D_BLK_DNR_TO		(1 << 1)
#define ISP_ITCMB0_D3D_MOT_TO			(1 << 2)
#define ISP_ITCMB0_D3D_KBA_TO			(1 << 3)
#define ISP_ITCMB0_SHARP_MIX_RATIO_TO		(1 << 4)

/*
 *  ISP debug reg -- isp internal cmb1
 */
#define ISP_ITCMB1_D3D_KP_WR_TO			(1 << 0)
#define ISP_ITCMB1_D3D_KP_RD_TO			(1 << 1)
#define ISP_ITCMB1_D3D_KM_MID0_WR_TO		(1 << 2)
#define ISP_ITCMB1_D3D_KM_MID1_WR_TO		(1 << 3)
#define ISP_ITCMB1_D3D_KM_MID2_WR_TO		(1 << 4)
#define ISP_ITCMB1_D3D_KM_MID0_RD_TO		(1 << 5)
#define ISP_ITCMB1_D3D_KM_MID1_RD_TO		(1 << 6)
#define ISP_ITCMB1_D3D_MOT_RST_WR_TO		(1 << 7)
#define ISP_ITCMB1_D3D_KST_RST_WR_TO		(1 << 8)
//#define ISP_ITCMB1_D3D_MOT_INTP_TO		(1 << 9)
//#define ISP_ITCMB1_D3D_KBA_INTP_TO		(1 << 10)
#define ISP_ITCMB1_SHARP_TXT_RST_WR_TO		(1 << 16)
#define ISP_ITCMB1_SHARP_TXT_MID0_RD_TO		(1 << 17)
#define ISP_ITCMB1_SHARP_TXT_MID1_RD_TO		(1 << 18)
#define ISP_ITCMB1_SHARP_TXT_MID0_WR_TO		(1 << 19)
#define ISP_ITCMB1_SHARP_TXT_MID1_WR_TO		(1 << 20)
#define ISP_ITCMB1_SHARP_TXT_MID2_WR_TO		(1 << 21)
#define ISP_ITCMB1_AFK_WR_TO			(1 << 28)

/*
 *  ISP debug reg -- isp internal cmb2
 */
#define ISP_ITCMB2_D3D_PCNT_RW_TO		(1 << 0)

/*
 *  ISP debug reg -- isp internal cmb3
 */
#define ISP_ITCMB3_AWB_RD_TO			(1 << 0)
#define ISP_ITCMB3_AWB_WR_TO			(1 << 1)

/*
 *  ISP debug reg -- isp internal cmb4
 */
#define ISP_ITCMB4_AE0_ACC_RD_TO		(1 << 0)
#define ISP_ITCMB4_AE1_ACC_RD_TO		(1 << 1)
#define ISP_ITCMB4_AE2_ACC_RD_TO		(1 << 2)
#define ISP_ITCMB4_AE0_ACC_WR_TO		(1 << 3)
#define ISP_ITCMB4_AE1_ACC_WR_TO		(1 << 4)
#define ISP_ITCMB4_AE2_ACC_WR_TO		(1 << 5)
#define ISP_ITCMB4_AE_OUT0_RD_TO		(1 << 6)
#define ISP_ITCMB4_AE_OUT1_RD_TO		(1 << 7)
#define ISP_ITCMB4_AE0_OUT_WR_TO		(1 << 8)
#define ISP_ITCMB4_AE1_OUT_WR_TO		(1 << 9)
#define ISP_ITCMB4_AE2_OUT_WR_TO		(1 << 10)

struct isp_lbc_cfg {
	unsigned char is_lossy;
	unsigned char rc_ctrl_mode;
	unsigned char start_qp;
	unsigned char std_qp;
	unsigned char glb_max_quo;
	unsigned char glb_max_k;
	unsigned int ptr_buffer_init;

	unsigned int mb_num_in_line;

	unsigned int ptr_buffer_fullness_max;
	unsigned int ptr_buffer_thr;

	unsigned int line_max_bit;
	unsigned int tar_bits_line_rc;

	unsigned int line_tar_bit;
	unsigned int tar_bits;

	unsigned int frame_tar_bit;

	unsigned int line_stride;

	unsigned char lbc_min_qp[16];
	unsigned char lbc_max_qp[16];
	unsigned int lbc_thresh[16];
	int lbc_tar_bits_adj[16];
	unsigned int lbc_pre_bits_adj[5];

	unsigned int cmp_ratio;
};

struct isp_stat_config {
	unsigned char stat_valid_block_w_num;
	unsigned char stat_valid_block_h_num;
	unsigned short stat_valid_block_num;
	unsigned short stat_last_block_w_start;
	unsigned short stat_last_block_h_comp;
	unsigned int stat_div_para;
	unsigned short stat_valid_block_width;
	unsigned short stat_valid_block_height;
	unsigned short stat_intp_w_step;
	unsigned short stat_intp_h_step;
	unsigned char stat_last_block_h_comp_line;
};

struct isp_size {
	u32 width;
	u32 height;
};

struct coor {
	u32 hor;
	u32 ver;
};

struct isp_size_settings {
	struct coor ob_start;
	struct isp_size ob_black;
	struct isp_size ob_valid;
	u32 set_cnt;
};

enum isp_mcic_sel {
	ISP_ID0_SELECT = 0,
	ISP_ID1_SELECT = 1,
	ISP_ID2_SELECT = 2,
	ISP_ID3_SELECT = 3,
	ISP_NONE_SELECT = 15,
};

enum ready_flag {
	PARA_NOT_READY = 0,
	PARA_READY = 1,
};

enum enable_flag {
	DISABLE    = 0,
	ENABLE     = 1,
};

enum isp_input_seq {
	ISP_BGGR = 4,
	ISP_RGGB = 5,
	ISP_GBRG = 6,
	ISP_GRBG = 7,
};

enum isp_lbc_align_choose {
	ISP_ALIGN_256 = 0x0,
	ISP_ALIGN_512,
};

extern int isp_virtual_find_ch[ISP610_MAX_NUM];
extern int isp_virtual_find_logic[ISP610_MAX_NUM + 3];
extern int isp_virtual_find_sel[ISP610_MAX_NUM + 3];
extern int isp_ch_find[ISP610_MAX_NUM + 3];

void bsp_isp_map_reg_addr(unsigned long id, vin_dma_addr_t base);
void bsp_isp_map_load_dram_addr(unsigned long id, vin_dma_addr_t base);
void bsp_isp_map_save_load_dram_addr(unsigned long id, vin_dma_addr_t base);

/*******isp top control register which we can write directly to register*********/
void bsp_isp_enable(unsigned long id, unsigned int en);
void bsp_isp_mode(unsigned long id, unsigned int mode);
void bsp_isp_top_capture_start(unsigned long id);
void bsp_isp_top_capture_stop(unsigned long id);
void bsp_isp_d3d_rec_reset_en(unsigned long id, unsigned int en);
void bsp_isp_ver_read_en(unsigned long id, unsigned int en);
void bsp_isp_set_sram_clear(unsigned long id, unsigned int en);
void bsp_isp_set_save_load_ini_back(unsigned long id, unsigned int en);
void bsp_isp_set_clk_back_door(unsigned long id, unsigned int en);
void bsp_isp_embedde_top_en(unsigned long id, unsigned int en);
void bsp_isp_ispinfo_embedded_en(unsigned long id, unsigned int en);
unsigned int bsp_isp_get_isp_ver(unsigned long id, unsigned int *major, unsigned int *minor);
unsigned int bsp_isp_get_max_width(unsigned long id);
void bsp_isp_mcic_enable(unsigned long id, unsigned int irq_flag);
void bsp_isp_mcic_disable(unsigned long id, unsigned int irq_flag);
void bsp_isp_set_mcic0_cfg(unsigned long id, enum isp_mcic_sel *isp_mcic0_sel);
void bsp_isp_set_mcic1_cfg(unsigned long id, enum isp_mcic_sel *isp_mcic1_sel);
unsigned int bsp_isp_get_mcic_status(unsigned long id, unsigned int flag);
void bsp_isp_clr_mcic_status(unsigned long id, unsigned int flag);
void bsp_isp_rdma_clk_back_door_en(unsigned long id, unsigned int en);

/*******ispx ahb control register which we can write directly to register*********/
void bsp_isp_capture_start(unsigned long id);
void bsp_isp_capture_stop(unsigned long id);
void bsp_isp_set_para_ready(unsigned long id, int ready);
void bsp_isp_update_table(unsigned long id, unsigned int table_update);
void bsp_isp_set_int_cmb_frm_interval(unsigned long id, unsigned int frm_interval);
void bsp_isp_set_load_addr(unsigned long id, vin_dma_addr_t addr);
void bsp_isp_irq_enable(unsigned long id, unsigned int irq_flag);
void bsp_isp_irq_disable(unsigned long id, unsigned int irq_flag);
unsigned int bsp_isp_get_irq_status(unsigned long id, unsigned int flag);
void bsp_isp_clr_irq_status(unsigned long id, unsigned int flag);
unsigned int bsp_isp_get_internal_status0(unsigned long id, unsigned int flag);
void bsp_isp_clr_internal_status0(unsigned long id, unsigned int flag);
unsigned int bsp_isp_get_internal_status1(unsigned long id, unsigned int flag);
void bsp_isp_clr_internal_status1(unsigned long id, unsigned int flag);
void bsp_isp_set_aiload_addr(unsigned long id, vin_dma_addr_t addr);
void bsp_isp_set_saved_addr(unsigned long id, vin_dma_addr_t addr);
void bsp_isp_set_statistics_addr(unsigned long id, vin_dma_addr_t addr);
void bsp_isp_set_save_load_addr(unsigned long id, vin_dma_addr_t addr);

/*******isp debug register which we can read directly to register*********/
unsigned int bsp_isp_get_isp_cur_id(unsigned long id);
unsigned int bsp_isp_get_isp_status(unsigned long id);
unsigned int bsp_isp_get_lock_id(unsigned long id);
unsigned int bsp_isp_get_internal_fifo(unsigned long id, unsigned int flag);
unsigned int bsp_isp_get_internal_cmb0(unsigned long id, unsigned int flag);
unsigned int bsp_isp_get_internal_cmb1(unsigned long id, unsigned int flag);
unsigned int bsp_isp_get_internal_cmb2(unsigned long id, unsigned int flag);
unsigned int bsp_isp_get_internal_cmb3(unsigned long id, unsigned int flag);
unsigned int bsp_isp_get_internal_cmb4(unsigned long id, unsigned int flag);

/*******isp load register which we should write to ddr first*********/
unsigned int bsp_isp_load_update_flag(unsigned long id);
void bsp_isp_set_input_fmt(unsigned long id, unsigned int fmt);
void bsp_isp_set_byr_max_bit(unsigned long id, int bit);
void bsp_isp_set_byr_act_bit(unsigned long id, int bit);
void bsp_isp_set_last_blank_cycle(unsigned long id, unsigned int blank);
void bsp_isp_set_speed_mode(unsigned long id, unsigned int speed);
void bsp_isp_set_line_int_num(unsigned long id, unsigned int line_num);
void bsp_isp_debug_output_cfg(unsigned long id, int enable, int output_sel);
void bsp_isp_set_size(unsigned long id, struct isp_size_settings *size);
void bsp_isp_module0_enable(unsigned long id, unsigned int module_flag);
void bsp_isp_module0_disable(unsigned long id, unsigned int module_flag);
void bsp_isp_module_enable(unsigned long id, unsigned int module_flag);
void bsp_isp_module_disable(unsigned long id, unsigned int module_flag);
void bsp_isp_d3d_ref_frm_mode(unsigned long id, unsigned int d3d_ref);
void bsp_isp_set_d3d_k0_addr(unsigned long id, vin_dma_addr_t addr);
void bsp_isp_set_d3d_k1_addr(unsigned long id, vin_dma_addr_t addr);
void bsp_isp_set_d3d_status_addr(unsigned long id, vin_dma_addr_t addr);
void bsp_isp_set_d2d_bayer_addr(unsigned long id, vin_dma_addr_t addr);
void bsp_isp_d3d_set_lbc_align_choose(unsigned long id, enum isp_lbc_align_choose align_choose);
void bsp_isp_d2d_set_raw_size(unsigned long id, unsigned int size);
void bsp_isp_set_ch_input_bit(unsigned long id, int ch, int bit);
void bsp_isp_set_ch_output_bit(unsigned long id, int ch, int bit);
void bsp_isp_clr_d3d_rec_en(unsigned long id);
void bsp_isp_set_d3d_lbc_cfg(unsigned long id, struct isp_lbc_cfg d3d_lbc);
void bsp_isp_set_stat(unsigned long id, struct isp_stat_config *cfg);

/*******isp save_load register which we should write to ddr first*********/
void bsp_isp_set_frm_cnt(unsigned long id, unsigned int frm_cnt);
void bsp_isp_set_next_frm_number(unsigned long id, unsigned int load_next_frm, unsigned int save_next_frm);
void bsp_isp_set_d3d_bayer_addr(unsigned long id, vin_dma_addr_t addr);

/*******syscfg sram boot mode ctrl*********/
void bsp_isp_map_syscfg_addr(unsigned long id, vin_dma_addr_t base);
void bsp_isp_sram_boot_mode_ctrl(unsigned long id, unsigned int mode);

#endif /*_ISP610_REG_CFG_H_*/
