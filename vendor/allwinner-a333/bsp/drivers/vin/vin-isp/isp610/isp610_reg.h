/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Copyright(c) 2020 - 2023 Allwinner Technology Co.,Ltd. All rights reserved. */

/* SPDX-License-Identifier: GPL-2.0 */
 /*
  * isp610_reg.h
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

#ifndef _ISP610_REG_H_
#define _ISP610_REG_H_

#define ISP_TOP_REG_OFFSET			0x0000
#define ISP_AHB_REG_OFFSET			0x0100
#define ISP_AHB_AMONG_OFFSET			0x0040
#define ISP_DBG_OFFSET				0x0200
#define ISP_LOAD_REG_OFFSET			0x0400
#define ISP_SAVE_REG_OFFSET			0x1300
#define ISP_SAVE_LOAD_REG_OFFSET		0x1400

/*top reg*/
#define ISP_TOP_CFG0_REG			0x000
#define ISP_DBG_CTRL_REG			0x004
#define ISP_RAM_SUP0_REG			0x008
#define ISP_RAM_SUP1_REG			0x00C
#define ISP_VER_CFG_REG				0x010
#define ISP_MAX_WIDTH_REG			0x014
#define ISP_MODULE_FET_REG			0x018
#define ISP_MCIC_CFG0_REG			0x020
#define ISP_MCIC_CFG1_REG			0x024
#define ISP_MCIC_STAT_REG			0x028
#define ISP_FEEDBACK_TDM_CFG_REG		0x02C
#define ISP_DBG_PIN_CTRL_REG			0x030
#define ISP_DBG_PIN_COMB_SRAM_CTRL_REG		0x034
#define ISP_MODULE_CLK_BDOOR_CFG0_REG		0x040
#define ISP_MODULE_CLK_BDOOR_CFG1_REG		0x044
#define ISP_FRM_LEV_CLK_BDOOR_CFG_REG		0x048
#define ISP_MOD_FRM_LEV_CLK_BDOOR_CFG0_REG	0x04C
#define ISP_MOD_FRM_LEV_CLK_BDOOR_CFG1_REG	0x050
#define ISP_MOD_FRM_LEV_CLK_BDOOR_CFG_REG	0x054
#define ISP_CLK_OFF_WAIT_NUMBER_CFG0_REG	0x058
#define ISP_CLK_OFF_WAIT_NUMBER_CFG1_REG	0x05C
#define ISP_EMBED_HBLANK_REG			0x060

/*ahb reg*/
#define ISP_AHB_CFG0_REG			0x000
#define ISP_LOAD_ADDR_REG			0x004
#define ISP_INT_BYPASS_REG			0x008
#define ISP_INT_STATUS_REG			0x00c
#define ISP_INTER_STATUS0_REG			0x010
#define ISP_INTER_STATUS1_REG			0x014
#define ISP_AILOAD_ADDR_REG			0x018
#define ISP_SAVE_ADDR0_REG			0x01c
#define ISP_SAVE_LOAD_ADDR_REG			0x020
#define ISP_SAVE_OFFSET0_ADDR_REG		0x024
#define ISP_SAVE_OFFSET1_ADDR_REG		0x028
#define ISP_SAVE_OFFSET2_ADDR_REG		0x02C
#define ISP_SAVE_ADDR1_REG			0x030

/*debug reg*/
#define ISP_DBG_TOP_REG				0x000
#define ISP_AHB_MBUS_LOCK_REG			0x038
#define ISP_INTERNAL_FIFO_REG			0x03C
#define ISP_INTERNAL_CMB0_REG			0x040
#define ISP_INTERNAL_CMB1_REG			0x044
#define ISP_INTERNAL_CMB2_REG			0x048
#define ISP_INTERNAL_CMB3_REG			0x04C
#define ISP_INTERNAL_CMB4_REG			0x050

/*load reg*/
#define ISP_GLOBAL_CFG0_REG			0x000
#define ISP_GLOBAL_CFG1_REG			0x004
#define ISP_LBC_TIME_CYCLE_REG			0x008
#define ISP_D3D_FBTDM_RDMA_FIFO_DEPTH_REG	0x010
#define ISP_D3D_FBTDM_WDMA_FIFO_DEPTH_REG	0x014
#define ISP_INPUT_SIZE_REG			0x020
#define ISP_VALID_SIZE_REG			0x024
#define ISP_VALID_START_REG			0x028
#define ISP_MODULE_BYPASS0_REG			0x030
#define ISP_MODULE_BYPASS1_REG			0x034
#define ISP_MODULE_MODE0_REG			0x038
#define ISP_MODULE_MODE1_REG			0x03c
#define ISP_D3D_K0_ADDR_REG			0x044
#define ISP_D3D_K1_ADDR_REG			0x048
#define ISP_D3D_STATUS_ADDR_REG			0x04c
#define ISP_D2D_BAYER_ADDR_REG			0x050
#define ISP_READ_DMA_CFG0_REG			0x058
#define ISP_READ_DMA_CFG1_REG			0x05C
#define ISP_WRITE_DMA_CFG0_REG			0x060
#define ISP_WRITE_DMA_CFG1_REG			0x064
#define ISP_WRITE_DMA_CFG2_REG			0x068
#define ISP_VIN_CFG0_REG			0x070
#define ISP_CH0_EXPAND_OFFSET0_REG		0x090
#define ISP_CH0_EXPAND_OFFSET1_REG		0x094
#define ISP_CH0_EXPAND_CFG0_REG			0x098
#define ISP_CH1_EXPAND_OFFSET0_REG		0x0b0
#define ISP_CH1_EXPAND_OFFSET1_REG		0x0b4
#define ISP_CH1_EXPAND_CFG0_REG			0x0b8
#define ISP_CH2_EXPAND_OFFSET0_REG		0x0d0
#define ISP_CH2_EXPAND_OFFSET1_REG		0x0d4
#define ISP_CH2_EXPAND_CFG0_REG			0x0d8
#define ISP_D3D_CTRL_REG			0x2a0
#define ISP_D3D_LBC_CFG0_REG			0x300
#define ISP_D3D_LBC_CFG1_REG			0x304
#define ISP_D3D_LBC_CFG2_REG			0x308
#define ISP_D3D_LBC_CFG3_REG			0x30C
#define ISP_D3D_LBC_CFG4_REG			0x310
#define ISP_D3D_LBC_CFG5_REG			0x314
#define ISP_D3D_LBC_CFG6_REG			0x318
#define ISP_STAT_CFG0_REG			0x670
#define ISP_STAT_CFG1_REG			0x674
#define ISP_STAT_CFG2_REG			0x678
#define ISP_STAT_CFG3_REG			0x67C

#define ISP_LBC_MIN_QP_LUT0_REG			0xE90
#define ISP_LBC_MIN_QP_LUT1_REG			0xE94
#define ISP_LBC_MAX_QP_LUT0_REG			0xE98
#define ISP_LBC_MAX_QP_LUT1_REG			0xE9C
#define ISP_LBC_THRESH_LUT0_REG			0xEA0
#define ISP_LBC_THRESH_LUT1_REG			0xEA4
#define ISP_LBC_THRESH_LUT2_REG			0xEA8
#define ISP_LBC_THRESH_LUT3_REG			0xEAC
#define ISP_LBC_THRESH_LUT4_REG			0xEB0
#define ISP_LBC_THRESH_LUT5_REG			0xEB4
#define ISP_LBC_THRESH_LUT6_REG			0xEB8
#define ISP_LBC_THRESH_LUT7_REG			0xEBC
#define ISP_LBC_TAR_BITS_ADJ_LUT0_REG		0xEC0
#define ISP_LBC_TAR_BITS_ADJ_LUT1_REG		0xEC4
#define ISP_LBC_TAR_BITS_ADJ_LUT2_REG		0xEC8
#define ISP_LBC_TAR_BITS_ADJ_LUT3_REG		0xECC
#define ISP_LBC_TAR_BITS_ADJ_LUT4_REG		0xED0
#define ISP_LBC_TAR_BITS_ADJ_LUT5_REG		0xED4
#define ISP_LBC_TAR_BITS_ADJ_LUT6_REG		0xED8
#define ISP_LBC_TAR_BITS_ADJ_LUT7_REG		0xEDC
#define ISP_LBC_PREP_BITS_ADJ_LUT_REG		0xEE0

/*save load reg*/
#define ISP_SAVELOAD_CFG0_REG			0x000
#define ISP_FRM_CNT_REG				0x004
#define ISP_NEXT_FRM_NUM_REG			0x008
#define ISP_SAVELOAD_D3D_BATER_R_REG		0x018
#define ISP_SAVELOAD_D3D_BATER_W_REG		0x01C

/*top reg*/
typedef union {
	unsigned int dwval;
	struct {
		unsigned int isp_enable:1;
		unsigned int isp_mode:1;
		unsigned int isp_top_cap_en:1;
		unsigned int isp_ver_rd_en:1;
		unsigned int sram_clear:1;
		unsigned int d3d_rec_reset_en:1;
		unsigned int tbl_dma_back_door:1;
		unsigned int frm_done_mbus_term_back_door:1;
		unsigned int stc_win_w_error_en:1;
		unsigned int load_save_ini_back:1;
		unsigned int save_dbuf_en:1;
		unsigned int sharp_txt_sts_fin_back_door:1;
		unsigned int tdm_pkg_back_door:1;
		unsigned int last_line_pause_back_door:1;
		unsigned int dpc_vld_ctrl_back_door:1;
		unsigned int wdr_vld_ctrl_back_door:1;
		unsigned int embedded_top_en:1;
		unsigned int ispinfo_embedded_en:1;
		unsigned int res1:11;
		unsigned int dbg_frm_prot_en:1;
		unsigned int cmb_fst_err_mode:1;
		unsigned int dma_error_mode:1;
	} bits;
} ISP_TOP_CFG0_REG_t;

typedef union {
	unsigned int dwval;
	struct {
		unsigned int dbg_mode0:4;
		unsigned int dbg_mode1:4;
		unsigned int dbg_mode2:6;
		unsigned int dbg_mode3:5;
		unsigned int dbg_mode4:5;
		unsigned int dbg_clr_mode:1;
		unsigned int dbg_clr:1;
		unsigned int dbg_hold_mode:1;
		unsigned int res0:1;
		unsigned int dbg_en:1;
		unsigned int res1:3;
	} bits;
} ISP_DBG_CTRL_REG_t;

typedef union {
	unsigned int dwval;
	struct {
		unsigned int random_en:1;
		unsigned int res0:3;
		unsigned int random_mode:2;
		unsigned int res1:2;
		unsigned int random_seed:24;
	} bits;
} ISP_RAM_SUP0_REG_t;

typedef union {
	unsigned int dwval;
	struct {
		unsigned int neg_num:16;
		unsigned int pos_num:16;
	} bits;
} ISP_RAM_SUP1_REG_t;

typedef union {
	unsigned int dwval;
	struct {
		unsigned int small_ver:12;
		unsigned int big_ver:12;
		unsigned int res0:8;
	} bits;
} ISP_VER_CFG_REG_t;

typedef union {
	unsigned int dwval;
	struct {
		unsigned int max_width:16;
		unsigned int max_height:16;
	} bits;
} ISP_MAX_SIZE_REG_t;

typedef union {
	unsigned int dwval;
	struct {
		unsigned int d3d_exist:1;
		unsigned int d3d_lbc_ext:1;
		unsigned int pltm_exist:1;
		unsigned int res0:5;
		unsigned int af_flt0_exist:1;
		unsigned int af_flt1_exist:1;
		unsigned int res1:6;
		unsigned int wdr_feature:2;
		unsigned int d2d_feature:2;
		unsigned int res2:4;
		unsigned int rgb_bit_mode:1;
		unsigned int res3:7;
	} bits;
} ISP_MODULE_FET_REG_t;

typedef union {
	unsigned int dwval;
	struct {
		unsigned int isp_mcic0_en:1;
		unsigned int isp_mcic0_load_en:1;
		unsigned int isp_mcic0_save_en:1;
		unsigned int res0:1;
		unsigned int isp_mcic1_en:1;
		unsigned int isp_mcic1_load_en:1;
		unsigned int isp_mcic1_save_en:1;
		unsigned int res1:25;
	} bits;
} ISP_MCIC_CFG0_REG_t;

typedef union {
	unsigned int dwval;
	struct {
		unsigned int isp_mcic0_sel0:4;
		unsigned int isp_mcic0_sel1:4;
		unsigned int isp_mcic0_sel2:4;
		unsigned int isp_mcic0_sel3:4;
		unsigned int isp_mcic1_sel0:4;
		unsigned int isp_mcic1_sel1:4;
		unsigned int isp_mcic1_sel2:4;
		unsigned int isp_mcic1_sel3:4;
	} bits;
} ISP_MCIC_CFG1_REG_t;

typedef union {
	unsigned int dwval;
	struct {
		unsigned int isp_mcic0_load_stat:1;
		unsigned int isp_mcic1_load_stat:1;
		unsigned int res0:2;
		unsigned int isp_mcic0_id0_load_stat:1;
		unsigned int isp_mcic0_id1_load_stat:1;
		unsigned int isp_mcic0_id2_load_stat:1;
		unsigned int isp_mcic0_id3_load_stat:1;
		unsigned int isp_mcic1_id0_load_stat:1;
		unsigned int isp_mcic1_id1_load_stat:1;
		unsigned int isp_mcic1_id2_load_stat:1;
		unsigned int isp_mcic1_id3_load_stat:1;
		unsigned int res1:4;
		unsigned int isp_mcic0_seve_stat:1;
		unsigned int isp_mcic1_seve_stat:1;
		unsigned int res2:2;
		unsigned int isp_mcic0_id0_save_stat:1;
		unsigned int isp_mcic0_id1_save_stat:1;
		unsigned int isp_mcic0_id2_save_stat:1;
		unsigned int isp_mcic0_id3_save_stat:1;
		unsigned int isp_mcic1_id0_save_stat:1;
		unsigned int isp_mcic1_id1_save_stat:1;
		unsigned int isp_mcic1_id2_save_stat:1;
		unsigned int isp_mcic1_id3_save_stat:1;
		unsigned int res3:4;
	} bits;
} ISP_MCIC_STAT_REG_t;

typedef union {
	unsigned int dwval;
	struct {
		unsigned int line_last_msk_cycle:6;
		unsigned int res0:26;
	} bits;
} ISP_DEEDBACK_TDM_CFG_REG_t;

typedef union {
	unsigned int dwval;
	struct {
		unsigned int res0:17;
		unsigned int rdma_rec_fl_cbd:1;
		unsigned int res1:14;
	} bits;
} ISP_HL_CBD_CFG_REG_t;

typedef union {
	unsigned int dwval;
	struct {
		unsigned int res0:29;
		unsigned int fl_edma_rec2_cbd:1;
		unsigned int res1:2;
	} bits;
} ISP_MODULE_HL_CBD_CFG1_REG_t;

typedef union {
	unsigned int dwval;
	struct {
		unsigned int d2d_clkoff_num:8;
		unsigned int d3d_clkoff_num:8;
		unsigned int pltm_clkoff_num:8;
		unsigned int sharp_clkoff_num:8;
	} bits;
} ISP_CLK_WAIT_NUM_CFG0_REG_t;

typedef union {
	unsigned int dwval;
	struct {
		unsigned int lbd_clkoff_num:8;
		unsigned int dpkg_clkoff_num:8;
		unsigned int res0:16;
	} bits;
} ISP_CLK_WAIT_NUM_CFG1_REG_t;

typedef union {
	unsigned int dwval;
	struct {
		unsigned int hblank_size:16;
		unsigned int res0:16;
	} bits;
} ISP_EMB_HBLANK_REG_t;

/*ahb reg*/
typedef union {
	unsigned int dwval;
	struct {
		unsigned int cap_en:1;
		unsigned int para_ready:1;
		unsigned int res0:6;
		unsigned int rsc_update:1;
		unsigned int gamma_update:1;
		unsigned int drc_update:1;
		unsigned int res1:2;
		unsigned int d3d_update:1;
		unsigned int pltm_update:1;
		unsigned int cem_update:1;
		unsigned int msc_update:1;
		unsigned int fe0_msc_update:1;
		unsigned int d2d_update:1;
		unsigned int fe1_msc_update:1;
		unsigned int fe2_msc_update:1;
		unsigned int dpc_update:1;
		unsigned int gca_update:1;
		unsigned int fpn_update:1;
		unsigned int sharp_mot_rxt_update:1;
		unsigned int save_out_mode:1;
		unsigned int rec_wdma_feedback_en:1;
		unsigned int rec_rdma_feedback_en:1;
		unsigned int int_cmb_frm_interval:4;
	} bits;
}  ISP_AHB_CFG0_REG_t;

typedef union {
	unsigned int dwval;
	struct {
		unsigned int finish_int_en:1;
		unsigned int start_int_en:1;
		unsigned int para_save_int_en:1;
		unsigned int para_load_int_en:1;
		unsigned int n_line_start_int_en:1;
		unsigned int res0:3;
		unsigned int frame_lost_int_en:1;
		unsigned int ahb_mbus_w_int_en:1;
		unsigned int res1:6;
		unsigned int hb_short_int_en:1;
		unsigned int cfg_error_int_en:1;
		unsigned int inter_fifo_full_int_en:1;
		unsigned int wdma_fifo_full_int_en:1;
		unsigned int wdma_over_end_int_en:1;
		unsigned int rdma_fifo_empty_int_en:1;
		unsigned int lbc_error_int_en:1;
		unsigned int lbd_error_int_en:1;
		unsigned int cmb0_ack_to_int_en:1;
		unsigned int cmb1_ack_to_int_en:1;
		unsigned int cmb2_ack_to_int_en:1;
		unsigned int cmb3_ack_to_int_en:1;
		unsigned int cmb4_ack_to_int_en:1;
		unsigned int res2:3;
	} bits;
} ISP_INT_BYPASS_REG_t;

typedef union {
	unsigned int dwval;
	struct {
		unsigned int finish_int_pd:1;
		unsigned int start_int_pd:1;
		unsigned int para_save_int_pd:1;
		unsigned int para_load_int_pd:1;
		unsigned int n_line_start_int_pd:1;
		unsigned int res0:3;
		unsigned int frame_lost_int_pd:1;
		unsigned int ahb_mbus_w_int_pd:1;
		unsigned int res1:6;
		unsigned int hb_short_int_pd:1;
		unsigned int cfg_error_int_pd:1;
		unsigned int inter_fifo_full_int_pd:1;
		unsigned int wdma_fifo_full_int_pd:1;
		unsigned int wdma_over_end_int_pd:1;
		unsigned int rdma_fifo_empty_int_pd:1;
		unsigned int lbc_error_int_pd:1;
		unsigned int lbd_error_int_pd:1;
		unsigned int cmb0_ack_to_int_pd:1;
		unsigned int cmb1_ack_to_int_pd:1;
		unsigned int cmb2_ack_to_int_pd:1;
		unsigned int cmb3_ack_to_int_pd:1;
		unsigned int cmb4_ack_to_int_pd:1;
		unsigned int res2:3;
	} bits;
} ISP_INT_STATUS_REG_t;

typedef union {
	unsigned int dwval;
} ISP_INT_STATUS0_REG_t;

typedef union {
	unsigned int dwval;
} ISP_INT_STATUS1_REG_t;

typedef union {
	unsigned int dwval;
	struct {
		unsigned int isp_save_motion_offset:4;
		unsigned int isp_save_ae_offset:9;
		unsigned int res0:3;
		unsigned int isp_save_awb_offset:10;
		unsigned int res1:6;
	} bits;
} ISP_SAVE_OFFSET0_ADDR_REG_t;

typedef union {
	unsigned int dwval;
	struct {
		unsigned int isp_save_hist0_offset:6;
		unsigned int res0:2;
		unsigned int isp_save_hist1_offset:6;
		unsigned int res1:2;
		unsigned int isp_save_pltm_offset:7;
		unsigned int res2:1;
		unsigned int isp_save_af_offset:8;
	} bits;
} ISP_SAVE_OFFSET1_ADDR_REG_t;

typedef union {
	unsigned int dwval;
	struct {
		unsigned int isp_save_texture_offset:5;
		unsigned int res0:27;
	} bits;
} ISP_SAVE_OFFSET2_ADDR_REG_t;

/*debug reg*/
typedef union {
	unsigned int dwval;
	struct {
		unsigned int isp_status:16;
		unsigned int isp_cur_id:4;
		unsigned int isp_id_ready:1;
		unsigned int csi_end_lev:1;
		unsigned int frm_down_mbus_term:1;
		unsigned int boot_mode:1;
		unsigned int wdma_feedback:1;
		unsigned int rdma_feedback:1;
		unsigned int res0:5;
		unsigned int mbus_stop_flag:1;
	} bits;
} ISP_DBG_TOP_REG_t;

typedef union {
	unsigned int dwval;
	struct {
		unsigned int lock_id:11;
		unsigned int res0:21;
	} bits;
} ISP_AHB_MBUS_LOCK_REG_t;

typedef union {
	unsigned int dwval;
} ISP_INFIFO_REG_t;

typedef union {
	unsigned int dwval;
} ISP_INCMB0_REG_t;

typedef union {
	unsigned int dwval;
} ISP_INCMB1_REG_t;

typedef union {
	unsigned int dwval;
} ISP_INCMB2_REG_t;

typedef union {
	unsigned int dwval;
} ISP_INCMB3_REG_t;

typedef union {
	unsigned int dwval;
} ISP_INCMB4_REG_t;

/*load reg*/
typedef union {
	unsigned int dwval;
	struct {
		unsigned int input_fmt:3;
		unsigned int res0:1;
		unsigned int byr_act_bit:5;
		unsigned int res1:3;
		unsigned int byr_max_bit:3;
		unsigned int res2:1;
		unsigned int rgb_cfg_bit:4;
		unsigned int res3:11;
		unsigned int dbg_out_en:1;
	} bits;
} ISP_GLOBAL_CFG0_REG_t;

typedef union {
	unsigned int dwval;
	struct {
		unsigned int speed_mode:3;
		unsigned int res0:1;
		unsigned int last_blank_cycle:3;
		unsigned int res1:1;
		unsigned int dbg_out_sel:6;
		unsigned int dbg_out_mode:2;
		unsigned int line_int_num:13;
		unsigned int dbg_out_shft_bit:3;
	} bits;
} ISP_GLOBAL_CFG1_REG_t;

typedef union {
	unsigned int dwval;
	struct {
		unsigned int lbc_tx_t1_cycle:16;
		unsigned int lbc_tx_t2_cycle:16;
	} bits;
} ISP_LBC_TIME_CYCLE_REG_t;

typedef union {
	unsigned int dwval;
	struct {
		unsigned int rec_fifo_low_limit:16;
		unsigned int rec_fifo_high_limit:16;
	} bits;
} ISP_D3D_FBTDM_RDMA_FIFO_DEPTH_REG_t;

typedef union {
	unsigned int dwval;
	struct {
		unsigned int rec_fifo_low_limit:16;
		unsigned int rec_fifo_high_limit:16;
	} bits;
} ISP_D3D_FBTDM_WDMA_FIFO_DEPTH_REG_t;

typedef union {
	unsigned int dwval;
	struct {
		unsigned int input_width:14;
		unsigned int res0:2;
		unsigned int input_height:14;
		unsigned int res1:2;
	} bits;
} ISP_INPUT_SIZE_REG_t;

typedef union {
	unsigned int dwval;
	struct {
		unsigned int valid_width:14;
		unsigned int res0:2;
		unsigned int valid_height:14;
		unsigned int res1:2;
	} bits;
} ISP_VALID_SIZE_REG_t;

typedef union {
	unsigned int dwval;
	struct {
		unsigned int valid_hor_start:13;
		unsigned int res0:3;
		unsigned int valid_ver_start:13;
		unsigned int res1:3;
	} bits;
} ISP_VALID_START_REG_t;

typedef union {
	unsigned int dwval;
	struct {
		unsigned int wdr_en:1;
		unsigned int wdr_split_en:1;
		unsigned int res0:6;
		unsigned int ch0_dg_en:1;
		unsigned int ch1_dg_en:1;
		unsigned int ch2_dg_en:1;
		unsigned int res1:21;
	} bits;
} ISP_MODULE_BYPASS0_REG_t;

typedef union {
	unsigned int dwval;
	struct {
		unsigned int wdr_stitch_en:1;
		unsigned int dpc_en:1;
		unsigned int ctc_en:1;
		unsigned int gca_en:1;
		unsigned int d2d_en:1;
		unsigned int d3d_en:1;
		unsigned int blc_en:1;
		unsigned int wb_en:1;
		unsigned int dg_en:1;
		unsigned int rsc_en:1;
		unsigned int msc_en:1;
		unsigned int pltm_en:1;
		unsigned int lca_en:1;
		unsigned int sharp_en:1;
		unsigned int ccm_en:1;
		unsigned int res0:1;
		unsigned int drc_en:1;
		unsigned int gamma_en:1;
		unsigned int cem_en:1;
		unsigned int res1:1;
		unsigned int fpn_en:1;
		unsigned int dpc_dle_en:1;
		unsigned int ae0_en:1;
		unsigned int ae1_en:1;
		unsigned int ae2_en:1;
		unsigned int af_en:1;
		unsigned int awb_en:1;
		unsigned int afs_en:1;
		unsigned int hist0_en:1;
		unsigned int hist1_en:1;
		unsigned int sharp_txt_info_init_en:1;
		unsigned int d3d_mot_info_init_en:1;
	} bits;
} ISP_MODULE_BYPASS1_REG_t;

typedef union {
	unsigned int dwval;
	struct {
		unsigned int res0:19;
		unsigned int d3d_ref_frm_mode:1;
		unsigned int res1:12;
	} bits;
} ISP_MODULE_MODE0_REG_t;

typedef union {
	unsigned int dwval;
	struct {
		unsigned int res0:9;
		unsigned int align_mode:1;
		unsigned int res1:22;
	} bits;
} ISP_WDNA_CFG1_REG_t;

typedef union {
	unsigned int dwval;
	struct {
		unsigned int isp_d2d_pef_w:8;
		unsigned int res0:4;
		unsigned int isp_d2d_raw_size:20;
	} bits;
} ISP_WDNA_CFG2_REG_t;

typedef union {
	unsigned int dwval;
	struct {
		unsigned int input_cfg:4;
		unsigned int res0:4;
		unsigned int output_cfg:4;
		unsigned int res1:4;
		unsigned int output_chn0_data:3;
		unsigned int res2:1;
		unsigned int output_chn1_data:3;
		unsigned int res3:1;
		unsigned int output_chn2_data:3;
		unsigned int res4:5;
	} bits;
} ISP_VIN_CFG0_REG_t;

typedef union {
	unsigned int dwval;
	struct {
		unsigned int r_offset:13;
		unsigned int res0:3;
		unsigned int gr_offset:13;
		unsigned int res1:3;
	} bits;
} ISP_CH0_EXPAND_OFFSET0_REG_t;

typedef union {
	unsigned int dwval;
	struct {
		unsigned int gb_offset:13;
		unsigned int res0:3;
		unsigned int b_offset:13;
		unsigned int res1:3;
	} bits;
} ISP_CH0_EXPAND_OFFSET1_REG_t;

typedef union {
	unsigned int dwval;
	struct {
		unsigned int blc_en:1;
		unsigned int inv_blc_en:1;
		unsigned int res0:2;
		unsigned int input_bit:5;
		unsigned int res1:7;
		unsigned int output_bit:5;
		unsigned int res2:11;
	} bits;
} ISP_CH0_EXPAND_CFG0_REG_t;

typedef union {
	unsigned int dwval;
	struct {
		unsigned int r_offset:13;
		unsigned int res0:3;
		unsigned int gr_offset:13;
		unsigned int res1:3;
	} bits;
} ISP_CH1_EXPAND_OFFSET0_REG_t;

typedef union {
	unsigned int dwval;
	struct {
		unsigned int gb_offset:13;
		unsigned int res0:3;
		unsigned int b_offset:13;
		unsigned int res1:3;
	} bits;
} ISP_CH1_EXPAND_OFFSET1_REG_t;

typedef union {
	unsigned int dwval;
	struct {
		unsigned int blc_en:1;
		unsigned int inv_blc_en:1;
		unsigned int res0:2;
		unsigned int input_bit:5;
		unsigned int res1:7;
		unsigned int output_bit:5;
		unsigned int res2:11;
	} bits;
} ISP_CH1_EXPAND_CFG0_REG_t;

typedef union {
	unsigned int dwval;
	struct {
		unsigned int r_offset:13;
		unsigned int res0:3;
		unsigned int gr_offset:13;
		unsigned int res1:3;
	} bits;
} ISP_CH2_EXPAND_OFFSET0_REG_t;

typedef union {
	unsigned int dwval;
	struct {
		unsigned int gb_offset:13;
		unsigned int res0:3;
		unsigned int b_offset:13;
		unsigned int res1:3;
	} bits;
} ISP_CH2_EXPAND_OFFSET1_REG_t;

typedef union {
	unsigned int dwval;
	struct {
		unsigned int blc_en:1;
		unsigned int inv_blc_en:1;
		unsigned int res0:2;
		unsigned int input_bit:5;
		unsigned int res1:7;
		unsigned int output_bit:5;
		unsigned int res2:11;
	} bits;
} ISP_CH2_EXPAND_CFG0_REG_t;

typedef union {
	unsigned int dwval;
	struct {
		unsigned int d3d_rec_en:1;
		unsigned int res0:31;
	} bits;
} ISP_D3D_CTRL_REG_t;

typedef union {
	unsigned int dwval;
	struct {
		unsigned int is_lossy:1;
		unsigned int rc_ctrl_mode:1;
		unsigned int start_qp:2;
		unsigned int std_qp:4;
		unsigned int glb_max_quo:4;
		unsigned int glb_max_k:3;
		unsigned int res0:1;
		unsigned int ptr_buffer_init:11;
		unsigned int res1:5;
	} bits;
} ISP_D3D_LBC_CFG0_REG_t;

typedef union {
	unsigned int dwval;
	struct {
		unsigned int mb_num_in_line:9;
		unsigned int res0:23;
	} bits;
} ISP_D3D_LBC_CFG1_REG_t;

typedef union {
	unsigned int dwval;
	struct {
		unsigned int ptr_buffer_fullness_max:13;
		unsigned int res0:3;
		unsigned int ptr_buffer_thr:13;
		unsigned int res1:3;
	} bits;
} ISP_D3D_LBC_CFG2_REG_t;

typedef union {
	unsigned int dwval;
	struct {
		unsigned int line_max_bit:18;
		unsigned int res0:2;
		unsigned int tar_bits_line_rc:10;
		unsigned int res1:2;
	} bits;
} ISP_D3D_LBC_CFG3_REG_t;

typedef union {
	unsigned int dwval;
	struct {
		unsigned int line_tar_bit:18;
		unsigned int res0:2;
		unsigned int tar_bits:10;
		unsigned int res1:2;
	} bits;
} ISP_D3D_LBC_CFG4_REG_t;

typedef union {
	unsigned int dwval;
	struct {
		unsigned int frame_tar_bit:31;
		unsigned int res0:1;
	} bits;
} ISP_D3D_LBC_CFG5_REG_t;

typedef union {
	unsigned int dwval;
	struct {
		unsigned int line_stride:16;
		unsigned int res0:16;
	} bits;
} ISP_D3D_LBC_CFG6_REG_t;

typedef union {
	unsigned int dwval;
	struct {
		unsigned int stat_valid_block_w_num:5;
		unsigned int res0:3;
		unsigned int stat_valid_block_h_num:5;
		unsigned int res1:3;
		unsigned int stat_valid_block_num:10;
		unsigned int res2:6;
	} bits;
} ISP_STAT_CFG0_REG_t;

typedef union {
	unsigned int dwval;
	struct {
		unsigned int stat_last_block_w_start:13;
		unsigned int res0:3;
		unsigned int stat_last_block_h_comp:16;
	} bits;
} ISP_STAT_CFG1_REG_t;

typedef union {
	unsigned int dwval;
	struct {
		unsigned int stat_div_para:16;
		unsigned int stat_valid_block_width:8;
		unsigned int stat_valid_block_height:8;
	} bits;
} ISP_STAT_CFG2_REG_t;

typedef union {
	unsigned int dwval;
	struct {
		unsigned int stat_intp_w_step:12;
		unsigned int stat_intp_h_step:12;
		unsigned int stat_last_block_h_comp_line:8;
	} bits;
} ISP_STAT_CFG3_REG_t;

typedef union {
	unsigned int dwval;
	struct {
		unsigned int qp_s0:4;
		unsigned int qp_s1:4;
		unsigned int qp_s2:4;
		unsigned int qp_s3:4;
		unsigned int qp_s4:4;
		unsigned int qp_s5:4;
		unsigned int qp_s6:4;
		unsigned int qp_s7:4;
	} bits;
} ISP_MIN_QP_LUT_REG_t;

typedef union {
	unsigned int dwval;
	struct {
		unsigned int th_s0:13;
		unsigned int res0:3;
		unsigned int th_s1:13;
		unsigned int res1:3;

	} bits;
} ISP_THRESH_LUT_REG_t;

typedef union {
	unsigned int dwval;
	struct {
		unsigned int tar_bits_adj_s0:10;
		unsigned int res0:6;
		unsigned int tar_bits_adj_s1:10;
		unsigned int res1:6;

	} bits;
} ISP_TAR_BITS_ADJ_LUT_REG_t;

typedef union {
	unsigned int dwval;
	struct {
		unsigned int pre_bits_adj_s0:1;
		unsigned int pre_bits_adj_s1:6;
		unsigned int pre_bits_adj_s2:7;
		unsigned int pre_bits_adj_s3:8;
		unsigned int pre_bits_adj_s4:9;
		unsigned int res0:1;

	} bits;
} ISP_PRE_BITS_ADJ_LUT_REG_t;

/*save_load reg*/
typedef union {
	unsigned int dwval;
	struct {
		unsigned int mbus_stop_flag:1;
		unsigned int res0:15;
		unsigned int save_addr_sel:1;
		unsigned int res1:11;
		unsigned int input_fmt:3;
		unsigned int d3d_k_state:1;
	} bits;
} ISP_SAVELOAD_CFG0_REG_t;

typedef union {
	unsigned int dwval;
	struct {
		unsigned int load_next_frm_num:4;
		unsigned int save_next_frm_num:4;
		unsigned int res0:24;
	} bits;
} ISP_NEXT_FRM_NUM_REG_t;

#endif /*_ISP610_REG_H_*/
