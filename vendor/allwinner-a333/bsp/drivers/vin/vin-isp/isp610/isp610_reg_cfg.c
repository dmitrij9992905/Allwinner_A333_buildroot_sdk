/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Copyright(c) 2020 - 2023 Allwinner Technology Co.,Ltd. All rights reserved. */

/* SPDX-License-Identifier: GPL-2.0 */
/*
 * isp610_reg_cfg.c
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
#include <linux/io.h>
#include <linux/string.h>
#include "isp610_reg.h"
#include "isp610_reg_cfg.h"

//#define USE_DEF_PARA

#define addr_base_offset 0x4

int isp_virtual_find_ch[ISP610_MAX_NUM] = {
	0, 1, 2, 3,
};

int isp_virtual_find_logic[ISP610_MAX_NUM + 3] = {
       0, 0, 0, 0, 4, 5, 6,
};

int isp_virtual_find_sel[ISP610_MAX_NUM + 3] = {
       0, 0, 0, 0, 1, 2, 3,
};

int isp_ch_find[ISP610_MAX_NUM + 3] = {
       0, 1, 2, 3, 0, 0, 0,
};

/*
 *  Load ISP register variables
 */

struct isp610_reg {
	/*top reg*/
	ISP_TOP_CFG0_REG_t *isp_top_cfg;
	ISP_DBG_CTRL_REG_t *isp_dbg_ctrl;
	ISP_VER_CFG_REG_t *isp_ver_cfg;
	ISP_MAX_SIZE_REG_t *isp_max_size;
	ISP_MODULE_FET_REG_t *isp_module_fet;
	ISP_MCIC_CFG0_REG_t *isp_mcic_cfg0;
	ISP_MCIC_CFG1_REG_t *isp_mcic_cfg1;
	ISP_MCIC_STAT_REG_t *isp_mcic_status;
	ISP_DEEDBACK_TDM_CFG_REG_t *isp_feedback_tdm_cfg;
	ISP_HL_CBD_CFG_REG_t *isp_hl_cbd_cfg;
	ISP_MODULE_HL_CBD_CFG1_REG_t *isp_module_hl_cbd_cfg;

	/*ahb reg*/
	ISP_AHB_CFG0_REG_t *isp_update_ctrl;
	unsigned int *isp_load_addr;
	ISP_INT_BYPASS_REG_t *isp_int_bypass;
	ISP_INT_STATUS_REG_t *isp_int_status;
	ISP_INT_STATUS0_REG_t *isp_inter_status0;
	ISP_INT_STATUS1_REG_t *isp_inter_status1;
	unsigned int *isp_aiload_addr;
	unsigned int *isp_save_addr0;
	unsigned int *isp_save_load_addr;
	ISP_SAVE_OFFSET0_ADDR_REG_t *isp_save_offset0_addr;
	ISP_SAVE_OFFSET1_ADDR_REG_t *isp_save_offset1_addr;
	ISP_SAVE_OFFSET2_ADDR_REG_t *isp_save_offset2_addr;
	unsigned int *isp_save_addr1;

	/*debug reg*/
	ISP_DBG_TOP_REG_t *isp_dbg_top;
	ISP_AHB_MBUS_LOCK_REG_t *isp_ahb_mbus_lock;
	ISP_INFIFO_REG_t *isp_internal_fifo;
	ISP_INCMB0_REG_t *isp_internal_cmb0;
	ISP_INCMB1_REG_t *isp_internal_cmb1;
	ISP_INCMB2_REG_t *isp_internal_cmb2;
	ISP_INCMB3_REG_t *isp_internal_cmb3;
	ISP_INCMB4_REG_t *isp_internal_cmb4;

	/*load reg*/
	ISP_AHB_CFG0_REG_t *isp_update_flag;
	ISP_GLOBAL_CFG0_REG_t *isp_global_cfg0;
	ISP_GLOBAL_CFG1_REG_t *isp_global_cfg1;
	ISP_LBC_TIME_CYCLE_REG_t *isp_lbc_time_cycle;
	ISP_D3D_FBTDM_RDMA_FIFO_DEPTH_REG_t *isp_d3d_fbtdm_rdma_fifo_depth;
	ISP_D3D_FBTDM_WDMA_FIFO_DEPTH_REG_t *isp_d3d_fbtdm_wdma_fifo_depth;
	ISP_INPUT_SIZE_REG_t *isp_input_size;
	ISP_VALID_SIZE_REG_t *isp_valid_size;
	ISP_VALID_START_REG_t *isp_valid_start;
	ISP_MODULE_BYPASS0_REG_t *isp_module_bypass0;
	ISP_MODULE_BYPASS1_REG_t *isp_module_bypass1;
	ISP_MODULE_MODE0_REG_t *isp_module_mode0;
	unsigned int *isp_d3d_k0_addr;
	unsigned int *isp_d3d_k1_addr;
	unsigned int *isp_d3d_status_addr;
	unsigned int *isp_d2d_bayer_addr;
	ISP_WDNA_CFG1_REG_t *isp_wdma_cfg1;
	ISP_WDNA_CFG2_REG_t *isp_wdma_cfg2;
	ISP_VIN_CFG0_REG_t *isp_vin_cfg0;
	ISP_CH0_EXPAND_OFFSET0_REG_t *isp_ch0_expand_offset0;
	ISP_CH0_EXPAND_OFFSET1_REG_t *isp_ch0_expand_offset1;
	ISP_CH0_EXPAND_CFG0_REG_t *isp_ch0_expand_cfg0;
	ISP_CH1_EXPAND_OFFSET0_REG_t *isp_ch1_expand_offset0;
	ISP_CH1_EXPAND_OFFSET1_REG_t *isp_ch1_expand_offset1;
	ISP_CH1_EXPAND_CFG0_REG_t *isp_ch1_expand_cfg0;
	ISP_CH2_EXPAND_OFFSET0_REG_t *isp_ch2_expand_offset0;
	ISP_CH2_EXPAND_OFFSET1_REG_t *isp_ch2_expand_offset1;
	ISP_CH2_EXPAND_CFG0_REG_t *isp_ch2_expand_cfg0;
	ISP_D3D_CTRL_REG_t *isp_d3d_ctrl;
	ISP_D3D_LBC_CFG0_REG_t *isp_d3d_lbc_cfg0;
	ISP_D3D_LBC_CFG1_REG_t *isp_d3d_lbc_cfg1;
	ISP_D3D_LBC_CFG2_REG_t *isp_d3d_lbc_cfg2;
	ISP_D3D_LBC_CFG3_REG_t *isp_d3d_lbc_cfg3;
	ISP_D3D_LBC_CFG4_REG_t *isp_d3d_lbc_cfg4;
	ISP_D3D_LBC_CFG5_REG_t *isp_d3d_lbc_cfg5;
	ISP_D3D_LBC_CFG6_REG_t *isp_d3d_lbc_cfg6;
	ISP_STAT_CFG0_REG_t *isp_stat_cfg0;
	ISP_STAT_CFG1_REG_t *isp_stat_cfg1;
	ISP_STAT_CFG2_REG_t *isp_stat_cfg2;
	ISP_STAT_CFG3_REG_t *isp_stat_cfg3;
	ISP_MIN_QP_LUT_REG_t *isp_lbc_min_qp_lut0;
	ISP_MIN_QP_LUT_REG_t *isp_lbc_min_qp_lut1;
	ISP_MIN_QP_LUT_REG_t *isp_lbc_max_qp_lut0;
	ISP_MIN_QP_LUT_REG_t *isp_lbc_max_qp_lut1;
	ISP_THRESH_LUT_REG_t *isp_lbc_thresh_lut0;
	ISP_THRESH_LUT_REG_t *isp_lbc_thresh_lut1;
	ISP_THRESH_LUT_REG_t *isp_lbc_thresh_lut2;
	ISP_THRESH_LUT_REG_t *isp_lbc_thresh_lut3;
	ISP_THRESH_LUT_REG_t *isp_lbc_thresh_lut4;
	ISP_THRESH_LUT_REG_t *isp_lbc_thresh_lut5;
	ISP_THRESH_LUT_REG_t *isp_lbc_thresh_lut6;
	ISP_THRESH_LUT_REG_t *isp_lbc_thresh_lut7;
	ISP_TAR_BITS_ADJ_LUT_REG_t *isp_lbc_tar_bits_adj_lut0;
	ISP_TAR_BITS_ADJ_LUT_REG_t *isp_lbc_tar_bits_adj_lut1;
	ISP_TAR_BITS_ADJ_LUT_REG_t *isp_lbc_tar_bits_adj_lut2;
	ISP_TAR_BITS_ADJ_LUT_REG_t *isp_lbc_tar_bits_adj_lut3;
	ISP_TAR_BITS_ADJ_LUT_REG_t *isp_lbc_tar_bits_adj_lut4;
	ISP_TAR_BITS_ADJ_LUT_REG_t *isp_lbc_tar_bits_adj_lut5;
	ISP_TAR_BITS_ADJ_LUT_REG_t *isp_lbc_tar_bits_adj_lut6;
	ISP_TAR_BITS_ADJ_LUT_REG_t *isp_lbc_tar_bits_adj_lut7;
	ISP_PRE_BITS_ADJ_LUT_REG_t *isp_lbc_pre_bits_adj_lut;

	/*save_load reg*/
	ISP_SAVELOAD_CFG0_REG_t *isp_saveload_cfg0;
	unsigned int *isp_frm_cnt;
	ISP_NEXT_FRM_NUM_REG_t *isp_next_frm_number;
	unsigned int *isp_d3d_bayer_raddr;
	unsigned int *isp_d3d_bayer_waddr;
};

struct isp610_reg isp_regs[ISP610_MAX_NUM];

void bsp_isp_map_reg_addr(unsigned long id, vin_dma_addr_t base)
{
	base += isp_virtual_find_ch[id] * addr_base_offset;

	/*top reg*/
	isp_regs[id].isp_top_cfg = (ISP_TOP_CFG0_REG_t *) (base + ISP_TOP_CFG0_REG);
	isp_regs[id].isp_dbg_ctrl = (ISP_DBG_CTRL_REG_t *) (base + ISP_DBG_CTRL_REG);
	isp_regs[id].isp_ver_cfg = (ISP_VER_CFG_REG_t *) (base + ISP_VER_CFG_REG);
	isp_regs[id].isp_max_size = (ISP_MAX_SIZE_REG_t *) (base + ISP_MAX_WIDTH_REG);
	isp_regs[id].isp_module_fet = (ISP_MODULE_FET_REG_t *) (base + ISP_MODULE_FET_REG);
	isp_regs[id].isp_mcic_cfg0 = (ISP_MCIC_CFG0_REG_t *) (base + ISP_MCIC_CFG0_REG);
	isp_regs[id].isp_mcic_cfg1 = (ISP_MCIC_CFG1_REG_t *) (base + ISP_MCIC_CFG1_REG);
	isp_regs[id].isp_mcic_status = (ISP_MCIC_STAT_REG_t *) (base + ISP_MCIC_STAT_REG);
	isp_regs[id].isp_feedback_tdm_cfg = (ISP_DEEDBACK_TDM_CFG_REG_t *) (base + ISP_FEEDBACK_TDM_CFG_REG);
	isp_regs[id].isp_hl_cbd_cfg = (ISP_HL_CBD_CFG_REG_t *) (base + ISP_FRM_LEV_CLK_BDOOR_CFG_REG);
	isp_regs[id].isp_module_hl_cbd_cfg = (ISP_MODULE_HL_CBD_CFG1_REG_t *) (base + ISP_MOD_FRM_LEV_CLK_BDOOR_CFG1_REG);

	/*ahb reg*/
	isp_regs[id].isp_update_ctrl = (ISP_AHB_CFG0_REG_t *) (base + ISP_AHB_REG_OFFSET + id * ISP_AHB_AMONG_OFFSET + ISP_AHB_CFG0_REG);
	isp_regs[id].isp_load_addr = (unsigned int *) (base + ISP_AHB_REG_OFFSET + id * ISP_AHB_AMONG_OFFSET + ISP_LOAD_ADDR_REG);
	isp_regs[id].isp_int_bypass = (ISP_INT_BYPASS_REG_t *) (base + ISP_AHB_REG_OFFSET + id * ISP_AHB_AMONG_OFFSET + ISP_INT_BYPASS_REG);
	isp_regs[id].isp_int_status = (ISP_INT_STATUS_REG_t *) (base + ISP_AHB_REG_OFFSET + id * ISP_AHB_AMONG_OFFSET + ISP_INT_STATUS_REG);
	isp_regs[id].isp_inter_status0 = (ISP_INT_STATUS0_REG_t *) (base + ISP_AHB_REG_OFFSET + id * ISP_AHB_AMONG_OFFSET + ISP_INTER_STATUS0_REG);
	isp_regs[id].isp_inter_status1 = (ISP_INT_STATUS1_REG_t *) (base + ISP_AHB_REG_OFFSET + id * ISP_AHB_AMONG_OFFSET + ISP_INTER_STATUS1_REG);
	isp_regs[id].isp_aiload_addr = (unsigned int *) (base + ISP_AHB_REG_OFFSET + id * ISP_AHB_AMONG_OFFSET + ISP_AILOAD_ADDR_REG);
	isp_regs[id].isp_save_addr0 = (unsigned int *) (base + ISP_AHB_REG_OFFSET + id * ISP_AHB_AMONG_OFFSET + ISP_SAVE_ADDR0_REG);
	isp_regs[id].isp_save_load_addr = (unsigned int *) (base + ISP_AHB_REG_OFFSET + id * ISP_AHB_AMONG_OFFSET + ISP_SAVE_LOAD_ADDR_REG);
	isp_regs[id].isp_save_offset0_addr = (ISP_SAVE_OFFSET0_ADDR_REG_t *) (base + ISP_AHB_REG_OFFSET + id * ISP_AHB_AMONG_OFFSET + ISP_SAVE_OFFSET0_ADDR_REG);
	isp_regs[id].isp_save_offset1_addr = (ISP_SAVE_OFFSET1_ADDR_REG_t *) (base + ISP_AHB_REG_OFFSET + id * ISP_AHB_AMONG_OFFSET + ISP_SAVE_OFFSET1_ADDR_REG);
	isp_regs[id].isp_save_offset2_addr = (ISP_SAVE_OFFSET2_ADDR_REG_t *) (base + ISP_AHB_REG_OFFSET + id * ISP_AHB_AMONG_OFFSET + ISP_SAVE_OFFSET2_ADDR_REG);
	isp_regs[id].isp_save_addr1 = (unsigned int *) (base + ISP_AHB_REG_OFFSET + id * ISP_AHB_AMONG_OFFSET + ISP_SAVE_ADDR1_REG);

	/*debug reg*/
	isp_regs[id].isp_dbg_top = (ISP_DBG_TOP_REG_t *) (base + ISP_DBG_OFFSET + ISP_DBG_TOP_REG);
	isp_regs[id].isp_ahb_mbus_lock = (ISP_AHB_MBUS_LOCK_REG_t *) (base + ISP_DBG_OFFSET + ISP_AHB_MBUS_LOCK_REG);
	isp_regs[id].isp_internal_fifo = (ISP_INFIFO_REG_t *) (base + ISP_DBG_OFFSET + ISP_INTERNAL_FIFO_REG);
	isp_regs[id].isp_internal_cmb0 = (ISP_INCMB0_REG_t *) (base + ISP_DBG_OFFSET + ISP_INTERNAL_CMB0_REG);
	isp_regs[id].isp_internal_cmb1 = (ISP_INCMB1_REG_t *) (base + ISP_DBG_OFFSET + ISP_INTERNAL_CMB1_REG);
	isp_regs[id].isp_internal_cmb2 = (ISP_INCMB2_REG_t *) (base + ISP_DBG_OFFSET + ISP_INTERNAL_CMB2_REG);
	isp_regs[id].isp_internal_cmb3 = (ISP_INCMB3_REG_t *) (base + ISP_DBG_OFFSET + ISP_INTERNAL_CMB3_REG);
	isp_regs[id].isp_internal_cmb4 = (ISP_INCMB4_REG_t *) (base + ISP_DBG_OFFSET + ISP_INTERNAL_CMB4_REG);

#ifdef USE_DEF_PARA
	/*load reg*/
	isp_regs[id].isp_global_cfg0 = (ISP_GLOBAL_CFG0_REG_t *) (base + ISP_LOAD_REG_OFFSET + ISP_GLOBAL_CFG0_REG);
	isp_regs[id].isp_global_cfg1 = (ISP_GLOBAL_CFG1_REG_t *) (base + ISP_LOAD_REG_OFFSET + ISP_GLOBAL_CFG1_REG);
	isp_regs[id].isp_lbc_time_cycle = (ISP_LBC_TIME_CYCLE_REG_t *) (base + ISP_LOAD_REG_OFFSET + ISP_LBC_TIME_CYCLE_REG);
	isp_regs[id].isp_d3d_fbtdm_rdma_fifo_depth = (ISP_D3D_FBTDM_RDMA_FIFO_DEPTH_REG_t *) (base + ISP_LOAD_REG_OFFSET + ISP_LBC_TIME_CYCLE_REG);
	isp_regs[id].isp_d3d_fbtdm_wdma_fifo_depth = (ISP_D3D_FBTDM_WDMA_FIFO_DEPTH_REG_t *) (base + ISP_LOAD_REG_OFFSET + ISP_LBC_TIME_CYCLE_REG);
	isp_regs[id].isp_input_size = (ISP_INPUT_SIZE_REG_t *) (base + ISP_LOAD_REG_OFFSET + ISP_INPUT_SIZE_REG);
	isp_regs[id].isp_valid_size = (ISP_VALID_SIZE_REG_t *) (base + ISP_LOAD_REG_OFFSET + ISP_VALID_SIZE_REG);
	isp_regs[id].isp_valid_start = (ISP_VALID_START_REG_t *) (base + ISP_LOAD_REG_OFFSET + ISP_VALID_START_REG);
	isp_regs[id].isp_module_bypass0 = (ISP_MODULE_BYPASS0_REG_t *) (base + ISP_LOAD_REG_OFFSET + ISP_MODULE_BYPASS0_REG);
	isp_regs[id].isp_module_bypass1 = (ISP_MODULE_BYPASS1_REG_t *) (base + ISP_LOAD_REG_OFFSET + ISP_MODULE_BYPASS1_REG);
	isp_regs[id].isp_module_mode0 = (ISP_MODULE_MODE0_REG_t *) (base + ISP_LOAD_REG_OFFSET + ISP_MODULE_MODE0_REG);
	isp_regs[id].isp_d3d_k0_addr = (unsigned int *) (base + ISP_LOAD_REG_OFFSET + ISP_D3D_K0_ADDR_REG);
	isp_regs[id].isp_d3d_k1_addr = (unsigned int *) (base + ISP_LOAD_REG_OFFSET + ISP_D3D_K1_ADDR_REG);
	isp_regs[id].isp_d3d_status_addr = (unsigned int *) (base + ISP_LOAD_REG_OFFSET + ISP_D3D_STATUS_ADDR_REG);
	isp_regs[id].isp_d2d_bayer_addr = (unsigned int *) (base + ISP_LOAD_REG_OFFSET + ISP_D2D_BAYER_ADDR_REG);
	isp_regs[id].isp_d3d_ctrl = (ISP_D3D_CTRL_REG_t *) (base + ISP_LOAD_REG_OFFSET + ISP_D3D_CTRL_REG);
	isp_regs[id].isp_wdma_cfg1 = (ISP_WDNA_CFG1_REG_t *) (base + ISP_LOAD_REG_OFFSET + ISP_WRITE_DMA_CFG1_REG);
	isp_regs[id].isp_wdma_cfg2 = (ISP_WDNA_CFG2_REG_t *) (base + ISP_LOAD_REG_OFFSET + ISP_WRITE_DMA_CFG2_REG);
	isp_regs[id].isp_vin_cfg0 = (ISP_VIN_CFG0_REG_t *) (base + ISP_LOAD_REG_OFFSET + ISP_VIN_CFG0_REG);
	isp_regs[id].isp_ch0_expand_cfg0 = (ISP_CH0_EXPAND_CFG0_REG_t *) (base + ISP_LOAD_REG_OFFSET + ISP_CH0_EXPAND_CFG0_REG);
	isp_regs[id].isp_ch1_expand_cfg0 = (ISP_CH1_EXPAND_CFG0_REG_t *) (base + ISP_LOAD_REG_OFFSET + ISP_CH1_EXPAND_CFG0_REG);
	isp_regs[id].isp_ch2_expand_cfg0 = (ISP_CH2_EXPAND_CFG0_REG_t *) (base + ISP_LOAD_REG_OFFSET + ISP_CH2_EXPAND_CFG0_REG);
	isp_regs[id].isp_d3d_lbc_cfg0 = (ISP_D3D_LBC_CFG0_REG_t *) (base + ISP_LOAD_REG_OFFSET + ISP_D3D_LBC_CFG0_REG);
	isp_regs[id].isp_d3d_lbc_cfg1 = (ISP_D3D_LBC_CFG1_REG_t *) (base + ISP_LOAD_REG_OFFSET + ISP_D3D_LBC_CFG1_REG);
	isp_regs[id].isp_d3d_lbc_cfg2 = (ISP_D3D_LBC_CFG2_REG_t *) (base + ISP_LOAD_REG_OFFSET + ISP_D3D_LBC_CFG2_REG);
	isp_regs[id].isp_d3d_lbc_cfg3 = (ISP_D3D_LBC_CFG3_REG_t *) (base + ISP_LOAD_REG_OFFSET + ISP_D3D_LBC_CFG3_REG);
	isp_regs[id].isp_d3d_lbc_cfg4 = (ISP_D3D_LBC_CFG4_REG_t *) (base + ISP_LOAD_REG_OFFSET + ISP_D3D_LBC_CFG4_REG);
	isp_regs[id].isp_d3d_lbc_cfg5 = (ISP_D3D_LBC_CFG5_REG_t *) (base + ISP_LOAD_REG_OFFSET + ISP_D3D_LBC_CFG5_REG);
	isp_regs[id].isp_d3d_lbc_cfg6 = (ISP_D3D_LBC_CFG6_REG_t *) (base + ISP_LOAD_REG_OFFSET + ISP_D3D_LBC_CFG6_REG);
	isp_regs[id].isp_stat_cfg0 = (ISP_STAT_CFG0_REG_t *) (base + ISP_LOAD_REG_OFFSET + ISP_STAT_CFG0_REG);
	isp_regs[id].isp_stat_cfg1 = (ISP_STAT_CFG1_REG_t *) (base + ISP_LOAD_REG_OFFSET + ISP_STAT_CFG1_REG);
	isp_regs[id].isp_stat_cfg2 = (ISP_STAT_CFG2_REG_t *) (base + ISP_LOAD_REG_OFFSET + ISP_STAT_CFG2_REG);
	isp_regs[id].isp_stat_cfg3 = (ISP_STAT_CFG3_REG_t *) (base + ISP_LOAD_REG_OFFSET + ISP_STAT_CFG3_REG);
	isp_regs[id].isp_lbc_min_qp_lut0 = (ISP_MIN_QP_LUT_REG_t *) (base + ISP_LOAD_REG_OFFSET + ISP_LBC_MIN_QP_LUT0_REG);
	isp_regs[id].isp_lbc_min_qp_lut1 = (ISP_MIN_QP_LUT_REG_t *) (base + ISP_LOAD_REG_OFFSET + ISP_LBC_MIN_QP_LUT1_REG);
	isp_regs[id].isp_lbc_max_qp_lut0 = (ISP_MIN_QP_LUT_REG_t *) (base + ISP_LOAD_REG_OFFSET + ISP_LBC_MAX_QP_LUT0_REG);
	isp_regs[id].isp_lbc_max_qp_lut1 = (ISP_MIN_QP_LUT_REG_t *) (base + ISP_LOAD_REG_OFFSET + ISP_LBC_MAX_QP_LUT1_REG);
	isp_regs[id].isp_lbc_thresh_lut0 = (ISP_THRESH_LUT_REG_t *) (base + ISP_LOAD_REG_OFFSET + ISP_LBC_THRESH_LUT0_REG);
	isp_regs[id].isp_lbc_thresh_lut1 = (ISP_THRESH_LUT_REG_t *) (base + ISP_LOAD_REG_OFFSET + ISP_LBC_THRESH_LUT1_REG);
	isp_regs[id].isp_lbc_thresh_lut2 = (ISP_THRESH_LUT_REG_t *) (base + ISP_LOAD_REG_OFFSET + ISP_LBC_THRESH_LUT2_REG);
	isp_regs[id].isp_lbc_thresh_lut3 = (ISP_THRESH_LUT_REG_t *) (base + ISP_LOAD_REG_OFFSET + ISP_LBC_THRESH_LUT3_REG);
	isp_regs[id].isp_lbc_thresh_lut4 = (ISP_THRESH_LUT_REG_t *) (base + ISP_LOAD_REG_OFFSET + ISP_LBC_THRESH_LUT4_REG);
	isp_regs[id].isp_lbc_thresh_lut5 = (ISP_THRESH_LUT_REG_t *) (base + ISP_LOAD_REG_OFFSET + ISP_LBC_THRESH_LUT5_REG);
	isp_regs[id].isp_lbc_thresh_lut6 = (ISP_THRESH_LUT_REG_t *) (base + ISP_LOAD_REG_OFFSET + ISP_LBC_THRESH_LUT6_REG);
	isp_regs[id].isp_lbc_thresh_lut7 = (ISP_THRESH_LUT_REG_t *) (base + ISP_LOAD_REG_OFFSET + ISP_LBC_THRESH_LUT7_REG);
	isp_regs[id].isp_lbc_tar_bits_adj_lut0 = (ISP_TAR_BITS_ADJ_LUT_REG_t *) (base + ISP_LOAD_REG_OFFSET + ISP_LBC_TAR_BITS_ADJ_LUT0_REG);
	isp_regs[id].isp_lbc_tar_bits_adj_lut1 = (ISP_TAR_BITS_ADJ_LUT_REG_t *) (base + ISP_LOAD_REG_OFFSET + ISP_LBC_TAR_BITS_ADJ_LUT1_REG);
	isp_regs[id].isp_lbc_tar_bits_adj_lut2 = (ISP_TAR_BITS_ADJ_LUT_REG_t *) (base + ISP_LOAD_REG_OFFSET + ISP_LBC_TAR_BITS_ADJ_LUT2_REG);
	isp_regs[id].isp_lbc_tar_bits_adj_lut3 = (ISP_TAR_BITS_ADJ_LUT_REG_t *) (base + ISP_LOAD_REG_OFFSET + ISP_LBC_TAR_BITS_ADJ_LUT3_REG);
	isp_regs[id].isp_lbc_tar_bits_adj_lut4 = (ISP_TAR_BITS_ADJ_LUT_REG_t *) (base + ISP_LOAD_REG_OFFSET + ISP_LBC_TAR_BITS_ADJ_LUT4_REG);
	isp_regs[id].isp_lbc_tar_bits_adj_lut5 = (ISP_TAR_BITS_ADJ_LUT_REG_t *) (base + ISP_LOAD_REG_OFFSET + ISP_LBC_TAR_BITS_ADJ_LUT5_REG);
	isp_regs[id].isp_lbc_tar_bits_adj_lut6 = (ISP_TAR_BITS_ADJ_LUT_REG_t *) (base + ISP_LOAD_REG_OFFSET + ISP_LBC_TAR_BITS_ADJ_LUT6_REG);
	isp_regs[id].isp_lbc_tar_bits_adj_lut7 = (ISP_TAR_BITS_ADJ_LUT_REG_t *) (base + ISP_LOAD_REG_OFFSET + ISP_LBC_TAR_BITS_ADJ_LUT7_REG);
	isp_regs[id].isp_lbc_pre_bits_adj_lut = (ISP_PRE_BITS_ADJ_LUT_REG_t *) (base + ISP_LOAD_REG_OFFSET + ISP_LBC_PREP_BITS_ADJ_LUT_REG);
#endif
}

/*
 * Load DRAM Register Address
 */
void bsp_isp_map_load_dram_addr(unsigned long id, vin_dma_addr_t base)
{
#ifndef USE_DEF_PARA
	/*load reg*/
	//isp_regs[id].isp_update_flag = (ISP_AHB_CFG0_REG_t *) (base + ISP_AHB_REG_OFFSET + id * ISP_AHB_AMONG_OFFSET + ISP_AHB_CFG0_REG);
	isp_regs[id].isp_global_cfg0 = (ISP_GLOBAL_CFG0_REG_t *) (base + ISP_GLOBAL_CFG0_REG);
	isp_regs[id].isp_global_cfg1 = (ISP_GLOBAL_CFG1_REG_t *) (base + ISP_GLOBAL_CFG1_REG);
	isp_regs[id].isp_lbc_time_cycle = (ISP_LBC_TIME_CYCLE_REG_t *) (base + ISP_LBC_TIME_CYCLE_REG);
	isp_regs[id].isp_d3d_fbtdm_rdma_fifo_depth = (ISP_D3D_FBTDM_RDMA_FIFO_DEPTH_REG_t *) (base + ISP_LBC_TIME_CYCLE_REG);
	isp_regs[id].isp_d3d_fbtdm_wdma_fifo_depth = (ISP_D3D_FBTDM_WDMA_FIFO_DEPTH_REG_t *) (base + ISP_LBC_TIME_CYCLE_REG);
	isp_regs[id].isp_input_size = (ISP_INPUT_SIZE_REG_t *) (base + ISP_INPUT_SIZE_REG);
	isp_regs[id].isp_valid_size = (ISP_VALID_SIZE_REG_t *) (base + ISP_VALID_SIZE_REG);
	isp_regs[id].isp_valid_start = (ISP_VALID_START_REG_t *) (base + ISP_VALID_START_REG);
	isp_regs[id].isp_module_bypass0 = (ISP_MODULE_BYPASS0_REG_t *) (base + ISP_MODULE_BYPASS0_REG);
	isp_regs[id].isp_module_bypass1 = (ISP_MODULE_BYPASS1_REG_t *) (base + ISP_MODULE_BYPASS1_REG);
	isp_regs[id].isp_module_mode0 = (ISP_MODULE_MODE0_REG_t *) (base + ISP_MODULE_MODE0_REG);
	isp_regs[id].isp_d3d_k0_addr = (unsigned int *) (base + ISP_D3D_K0_ADDR_REG);
	isp_regs[id].isp_d3d_k1_addr = (unsigned int *) (base + ISP_D3D_K1_ADDR_REG);
	isp_regs[id].isp_d3d_status_addr = (unsigned int *) (base + ISP_D3D_STATUS_ADDR_REG);
	isp_regs[id].isp_d2d_bayer_addr = (unsigned int *) (base + ISP_D2D_BAYER_ADDR_REG);
	isp_regs[id].isp_wdma_cfg1 = (ISP_WDNA_CFG1_REG_t *) (base + ISP_WRITE_DMA_CFG1_REG);
	isp_regs[id].isp_wdma_cfg2 = (ISP_WDNA_CFG2_REG_t *) (base + ISP_WRITE_DMA_CFG2_REG);
	isp_regs[id].isp_vin_cfg0 = (ISP_VIN_CFG0_REG_t *) (base + ISP_VIN_CFG0_REG);
	isp_regs[id].isp_ch0_expand_offset0 = (ISP_CH0_EXPAND_OFFSET0_REG_t *) (base + ISP_CH0_EXPAND_OFFSET0_REG);
	isp_regs[id].isp_ch0_expand_offset1 = (ISP_CH0_EXPAND_OFFSET1_REG_t *) (base + ISP_CH0_EXPAND_OFFSET1_REG);
	isp_regs[id].isp_ch0_expand_cfg0 = (ISP_CH0_EXPAND_CFG0_REG_t *) (base + ISP_CH0_EXPAND_CFG0_REG);
	isp_regs[id].isp_ch1_expand_offset0 = (ISP_CH1_EXPAND_OFFSET0_REG_t *) (base + ISP_CH1_EXPAND_OFFSET0_REG);
	isp_regs[id].isp_ch1_expand_offset1 = (ISP_CH1_EXPAND_OFFSET1_REG_t *) (base + ISP_CH1_EXPAND_OFFSET1_REG);
	isp_regs[id].isp_ch1_expand_cfg0 = (ISP_CH1_EXPAND_CFG0_REG_t *) (base + ISP_CH1_EXPAND_CFG0_REG);
	isp_regs[id].isp_ch2_expand_offset0 = (ISP_CH2_EXPAND_OFFSET0_REG_t *) (base + ISP_CH2_EXPAND_OFFSET0_REG);
	isp_regs[id].isp_ch2_expand_offset1 = (ISP_CH2_EXPAND_OFFSET1_REG_t *) (base + ISP_CH2_EXPAND_OFFSET1_REG);
	isp_regs[id].isp_ch2_expand_cfg0 = (ISP_CH2_EXPAND_CFG0_REG_t *) (base + ISP_CH2_EXPAND_CFG0_REG);
	isp_regs[id].isp_d3d_ctrl = (ISP_D3D_CTRL_REG_t *) (base + ISP_D3D_CTRL_REG);
	isp_regs[id].isp_d3d_lbc_cfg0 = (ISP_D3D_LBC_CFG0_REG_t *) (base + ISP_D3D_LBC_CFG0_REG);
	isp_regs[id].isp_d3d_lbc_cfg1 = (ISP_D3D_LBC_CFG1_REG_t *) (base + ISP_D3D_LBC_CFG1_REG);
	isp_regs[id].isp_d3d_lbc_cfg2 = (ISP_D3D_LBC_CFG2_REG_t *) (base + ISP_D3D_LBC_CFG2_REG);
	isp_regs[id].isp_d3d_lbc_cfg3 = (ISP_D3D_LBC_CFG3_REG_t *) (base + ISP_D3D_LBC_CFG3_REG);
	isp_regs[id].isp_d3d_lbc_cfg4 = (ISP_D3D_LBC_CFG4_REG_t *) (base + ISP_D3D_LBC_CFG4_REG);
	isp_regs[id].isp_d3d_lbc_cfg5 = (ISP_D3D_LBC_CFG5_REG_t *) (base + ISP_D3D_LBC_CFG5_REG);
	isp_regs[id].isp_d3d_lbc_cfg6 = (ISP_D3D_LBC_CFG6_REG_t *) (base + ISP_D3D_LBC_CFG6_REG);
	isp_regs[id].isp_stat_cfg0 = (ISP_STAT_CFG0_REG_t *) (base + ISP_STAT_CFG0_REG);
	isp_regs[id].isp_stat_cfg1 = (ISP_STAT_CFG1_REG_t *) (base + ISP_STAT_CFG1_REG);
	isp_regs[id].isp_stat_cfg2 = (ISP_STAT_CFG2_REG_t *) (base + ISP_STAT_CFG2_REG);
	isp_regs[id].isp_stat_cfg3 = (ISP_STAT_CFG3_REG_t *) (base + ISP_STAT_CFG3_REG);
	isp_regs[id].isp_lbc_min_qp_lut0 = (ISP_MIN_QP_LUT_REG_t *) (base + ISP_LBC_MIN_QP_LUT0_REG);
	isp_regs[id].isp_lbc_min_qp_lut1 = (ISP_MIN_QP_LUT_REG_t *) (base + ISP_LBC_MIN_QP_LUT1_REG);
	isp_regs[id].isp_lbc_max_qp_lut0 = (ISP_MIN_QP_LUT_REG_t *) (base + ISP_LBC_MAX_QP_LUT0_REG);
	isp_regs[id].isp_lbc_max_qp_lut1 = (ISP_MIN_QP_LUT_REG_t *) (base + ISP_LBC_MAX_QP_LUT1_REG);
	isp_regs[id].isp_lbc_thresh_lut0 = (ISP_THRESH_LUT_REG_t *) (base + ISP_LBC_THRESH_LUT0_REG);
	isp_regs[id].isp_lbc_thresh_lut1 = (ISP_THRESH_LUT_REG_t *) (base + ISP_LBC_THRESH_LUT1_REG);
	isp_regs[id].isp_lbc_thresh_lut2 = (ISP_THRESH_LUT_REG_t *) (base + ISP_LBC_THRESH_LUT2_REG);
	isp_regs[id].isp_lbc_thresh_lut3 = (ISP_THRESH_LUT_REG_t *) (base + ISP_LBC_THRESH_LUT3_REG);
	isp_regs[id].isp_lbc_thresh_lut4 = (ISP_THRESH_LUT_REG_t *) (base + ISP_LBC_THRESH_LUT4_REG);
	isp_regs[id].isp_lbc_thresh_lut5 = (ISP_THRESH_LUT_REG_t *) (base + ISP_LBC_THRESH_LUT5_REG);
	isp_regs[id].isp_lbc_thresh_lut6 = (ISP_THRESH_LUT_REG_t *) (base + ISP_LBC_THRESH_LUT6_REG);
	isp_regs[id].isp_lbc_thresh_lut7 = (ISP_THRESH_LUT_REG_t *) (base + ISP_LBC_THRESH_LUT7_REG);
	isp_regs[id].isp_lbc_tar_bits_adj_lut0 = (ISP_TAR_BITS_ADJ_LUT_REG_t *) (base + ISP_LBC_TAR_BITS_ADJ_LUT0_REG);
	isp_regs[id].isp_lbc_tar_bits_adj_lut1 = (ISP_TAR_BITS_ADJ_LUT_REG_t *) (base + ISP_LBC_TAR_BITS_ADJ_LUT1_REG);
	isp_regs[id].isp_lbc_tar_bits_adj_lut2 = (ISP_TAR_BITS_ADJ_LUT_REG_t *) (base + ISP_LBC_TAR_BITS_ADJ_LUT2_REG);
	isp_regs[id].isp_lbc_tar_bits_adj_lut3 = (ISP_TAR_BITS_ADJ_LUT_REG_t *) (base + ISP_LBC_TAR_BITS_ADJ_LUT3_REG);
	isp_regs[id].isp_lbc_tar_bits_adj_lut4 = (ISP_TAR_BITS_ADJ_LUT_REG_t *) (base + ISP_LBC_TAR_BITS_ADJ_LUT4_REG);
	isp_regs[id].isp_lbc_tar_bits_adj_lut5 = (ISP_TAR_BITS_ADJ_LUT_REG_t *) (base + ISP_LBC_TAR_BITS_ADJ_LUT5_REG);
	isp_regs[id].isp_lbc_tar_bits_adj_lut6 = (ISP_TAR_BITS_ADJ_LUT_REG_t *) (base + ISP_LBC_TAR_BITS_ADJ_LUT6_REG);
	isp_regs[id].isp_lbc_tar_bits_adj_lut7 = (ISP_TAR_BITS_ADJ_LUT_REG_t *) (base + ISP_LBC_TAR_BITS_ADJ_LUT7_REG);
	isp_regs[id].isp_lbc_pre_bits_adj_lut = (ISP_PRE_BITS_ADJ_LUT_REG_t *) (base + ISP_LBC_PREP_BITS_ADJ_LUT_REG);
#endif
}

/*
 * save Load DRAM Register Address
 */
void bsp_isp_map_save_load_dram_addr(unsigned long id, vin_dma_addr_t base)
{
	isp_regs[id].isp_saveload_cfg0 = (ISP_SAVELOAD_CFG0_REG_t *) (base + ISP_SAVELOAD_CFG0_REG);
	isp_regs[id].isp_frm_cnt = (unsigned int *) (base + ISP_FRM_CNT_REG);
	isp_regs[id].isp_next_frm_number = (ISP_NEXT_FRM_NUM_REG_t *) (base + ISP_NEXT_FRM_NUM_REG);
	isp_regs[id].isp_d3d_bayer_raddr = (unsigned int *) (base + ISP_SAVELOAD_D3D_BATER_R_REG);
	isp_regs[id].isp_d3d_bayer_waddr = (unsigned int *) (base + ISP_SAVELOAD_D3D_BATER_W_REG);
}

/*******isp top control register which we can write directly to register*********/
void bsp_isp_enable(unsigned long id, unsigned int en)
{
	isp_regs[id].isp_top_cfg->bits.isp_enable = en;
}

void bsp_isp_mode(unsigned long id, unsigned int mode)
{
	isp_regs[id].isp_top_cfg->bits.isp_mode = mode;
}

void bsp_isp_top_capture_start(unsigned long id)
{
	isp_regs[id].isp_top_cfg->bits.isp_top_cap_en = 1;
}

void bsp_isp_top_capture_stop(unsigned long id)
{
	isp_regs[id].isp_top_cfg->bits.isp_top_cap_en = 0;
}

void bsp_isp_d3d_rec_reset_en(unsigned long id, unsigned int en)
{
	isp_regs[id].isp_top_cfg->bits.d3d_rec_reset_en = en;
}

void bsp_isp_ver_read_en(unsigned long id, unsigned int en)
{
	isp_regs[id].isp_top_cfg->bits.isp_ver_rd_en = en;
}

void bsp_isp_set_sram_clear(unsigned long id, unsigned int en)
{
	isp_regs[id].isp_top_cfg->bits.sram_clear = en;
}

void bsp_isp_set_save_load_ini_back(unsigned long id, unsigned int en)
{
	isp_regs[id].isp_top_cfg->bits.load_save_ini_back = en;
}

void bsp_isp_set_clk_back_door(unsigned long id, unsigned int en)
{
}

void bsp_isp_embedde_top_en(unsigned long id, unsigned int en)
{
	isp_regs[id].isp_top_cfg->bits.embedded_top_en = en;
}

void bsp_isp_ispinfo_embedded_en(unsigned long id, unsigned int en)
{
	isp_regs[id].isp_top_cfg->bits.ispinfo_embedded_en = en;
}

unsigned int bsp_isp_get_isp_ver(unsigned long id, unsigned int *major, unsigned int *minor)
{
	*major = isp_regs[id].isp_ver_cfg->bits.big_ver;
	*minor = isp_regs[id].isp_ver_cfg->bits.small_ver;
	return isp_regs[id].isp_ver_cfg->dwval;
}

unsigned int bsp_isp_get_max_width(unsigned long id)
{
	return isp_regs[id].isp_max_size->bits.max_width;
}

void bsp_isp_mcic_enable(unsigned long id, unsigned int irq_flag)
{
	isp_regs[id].isp_mcic_cfg0->dwval |= irq_flag;
}

void bsp_isp_mcic_disable(unsigned long id, unsigned int irq_flag)
{
	isp_regs[id].isp_mcic_cfg0->dwval &= ~irq_flag;
}

void bsp_isp_set_mcic0_cfg(unsigned long id, enum isp_mcic_sel *isp_mcic0_sel)
{
	isp_regs[id].isp_mcic_cfg1->bits.isp_mcic0_sel0 = isp_mcic0_sel[0];
	isp_regs[id].isp_mcic_cfg1->bits.isp_mcic0_sel1 = isp_mcic0_sel[1];
	isp_regs[id].isp_mcic_cfg1->bits.isp_mcic0_sel2 = isp_mcic0_sel[2];
	isp_regs[id].isp_mcic_cfg1->bits.isp_mcic0_sel3 = isp_mcic0_sel[3];
}

void bsp_isp_set_mcic1_cfg(unsigned long id, enum isp_mcic_sel *isp_mcic1_sel)
{
	isp_regs[id].isp_mcic_cfg1->bits.isp_mcic1_sel0 = isp_mcic1_sel[0];
	isp_regs[id].isp_mcic_cfg1->bits.isp_mcic1_sel1 = isp_mcic1_sel[1];
	isp_regs[id].isp_mcic_cfg1->bits.isp_mcic1_sel2 = isp_mcic1_sel[2];
	isp_regs[id].isp_mcic_cfg1->bits.isp_mcic1_sel3 = isp_mcic1_sel[3];
}

unsigned int bsp_isp_get_mcic_status(unsigned long id, unsigned int flag)
{
	unsigned int ret = 0;

	if ((flag == ISP_MCIC0_LOAD_STATUS) && (isp_regs[id].isp_mcic_cfg0->dwval & ISP_MCIC0_LOAD_EN))
		ret = isp_regs[id].isp_mcic_status->dwval & flag;
	else if ((flag == ISP_MCIC0_SAVE_STATUS) && (isp_regs[id].isp_mcic_cfg0->dwval & ISP_MCIC0_SAVE_EN))
		ret = isp_regs[id].isp_mcic_status->dwval & flag;
	else if ((flag == ISP_MCIC1_LOAD_STATUS) && (isp_regs[id].isp_mcic_cfg0->dwval & ISP_MCIC1_LOAD_EN))
		ret = isp_regs[id].isp_mcic_status->dwval & flag;
	else if ((flag == ISP_MCIC1_SAVE_STATUS) && (isp_regs[id].isp_mcic_cfg0->dwval & ISP_MCIC1_SAVE_EN))
		ret = isp_regs[id].isp_mcic_status->dwval & flag;

	return ret;
}

void bsp_isp_clr_mcic_status(unsigned long id, unsigned int flag)
{
	isp_regs[id].isp_mcic_status->dwval = flag;
}

void bsp_isp_rdma_clk_back_door_en(unsigned long id, unsigned int en)
{
	isp_regs[id].isp_hl_cbd_cfg->bits.rdma_rec_fl_cbd = en;
	isp_regs[id].isp_module_hl_cbd_cfg->bits.fl_edma_rec2_cbd = en;
}

/*******ispx ahb control register which we can write directly to register*********/
void bsp_isp_capture_start(unsigned long id)
{
	isp_regs[id].isp_update_ctrl->bits.cap_en = 1;
	isp_regs[id].isp_update_ctrl->bits.rec_rdma_feedback_en = 1;
	isp_regs[id].isp_update_ctrl->bits.rec_wdma_feedback_en = 1;
}

void bsp_isp_capture_stop(unsigned long id)
{
	isp_regs[id].isp_update_ctrl->bits.cap_en = 0;
	isp_regs[id].isp_update_ctrl->bits.rec_rdma_feedback_en = 0;
	isp_regs[id].isp_update_ctrl->bits.rec_wdma_feedback_en = 0;
}

void bsp_isp_set_para_ready(unsigned long id, int ready)
{
#ifndef USE_DEF_PARA
	isp_regs[id].isp_update_ctrl->bits.para_ready = ready;
#endif
}

void bsp_isp_update_table(unsigned long id, unsigned int table_update)
{
	isp_regs[id].isp_update_ctrl->bits.rsc_update = !!(table_update & LENS_UPDATE);
	isp_regs[id].isp_update_ctrl->bits.gamma_update = !!(table_update & GAMMA_UPDATE);
	isp_regs[id].isp_update_ctrl->bits.drc_update = !!(table_update & DRC_UPDATE);
	isp_regs[id].isp_update_ctrl->bits.d3d_update = !!(table_update & D3D_UPDATE);
	isp_regs[id].isp_update_ctrl->bits.pltm_update = !!(table_update & PLTM_UPDATE);
	isp_regs[id].isp_update_ctrl->bits.cem_update = !!(table_update & CEM_UPDATE);
	isp_regs[id].isp_update_ctrl->bits.msc_update = !!(table_update & MSC_UPDATE);
	isp_regs[id].isp_update_ctrl->bits.fe0_msc_update = !!(table_update & FE0_MSC_UPDATE);
	isp_regs[id].isp_update_ctrl->bits.d2d_update = !!(table_update & D2D_UPDATE);
	isp_regs[id].isp_update_ctrl->bits.fe1_msc_update = !!(table_update & FE1_MSC_UPDATE);
	isp_regs[id].isp_update_ctrl->bits.fe2_msc_update = !!(table_update & FE2_MSC_UPDATE);
	isp_regs[id].isp_update_ctrl->bits.dpc_update = !!(table_update & DPC_UPDATE);
	isp_regs[id].isp_update_ctrl->bits.gca_update = !!(table_update & GCA_UPDATE);
	isp_regs[id].isp_update_ctrl->bits.fpn_update = !!(table_update & FPN_UPDATE);
}

void bsp_isp_set_int_cmb_frm_interval(unsigned long id, unsigned int frm_interval)
{
	if (frm_interval == 0)
		isp_regs[id].isp_update_ctrl->bits.int_cmb_frm_interval = frm_interval;
	else
		isp_regs[id].isp_update_ctrl->bits.int_cmb_frm_interval = frm_interval - 1;
}

void bsp_isp_set_load_addr(unsigned long id, vin_dma_addr_t addr)
{
	writel(addr >> ISP_ADDR_BIT_R_SHIFT, isp_regs[id].isp_load_addr);
}

void bsp_isp_irq_enable(unsigned long id, unsigned int irq_flag)
{
	isp_regs[id].isp_int_bypass->dwval |= irq_flag;
}

void bsp_isp_irq_disable(unsigned long id, unsigned int irq_flag)
{
	isp_regs[id].isp_int_bypass->dwval &= ~irq_flag;
}

unsigned int bsp_isp_get_irq_status(unsigned long id, unsigned int flag)
{
	return (isp_regs[id].isp_int_status->dwval & flag) & (isp_regs[id].isp_int_bypass->dwval & flag);
}

void bsp_isp_clr_irq_status(unsigned long id, unsigned int flag)
{
	isp_regs[id].isp_int_status->dwval = flag;
}

unsigned int bsp_isp_get_internal_status0(unsigned long id, unsigned int flag)
{
	return isp_regs[id].isp_inter_status0->dwval & flag;
}

void bsp_isp_clr_internal_status0(unsigned long id, unsigned int flag)
{
	isp_regs[id].isp_inter_status0->dwval = flag;
}

unsigned int bsp_isp_get_internal_status1(unsigned long id, unsigned int flag)
{
	return isp_regs[id].isp_inter_status1->dwval & flag;
}

void bsp_isp_clr_internal_status1(unsigned long id, unsigned int flag)
{
	isp_regs[id].isp_inter_status1->dwval = flag;
}

void bsp_isp_set_aiload_addr(unsigned long id, vin_dma_addr_t addr)
{
	writel(addr >> ISP_ADDR_BIT_R_SHIFT, isp_regs[id].isp_aiload_addr);
}

void bsp_isp_set_saved_addr(unsigned long id, vin_dma_addr_t addr)
{
	writel(addr >> ISP_ADDR_BIT_R_SHIFT, isp_regs[id].isp_save_addr0);
}

void bsp_isp_set_statistics_addr(unsigned long id, vin_dma_addr_t addr)
{
	writel(addr >> ISP_ADDR_BIT_R_SHIFT, isp_regs[id].isp_save_addr0);
}

void bsp_isp_set_save_load_addr(unsigned long id, vin_dma_addr_t addr)
{
	writel(addr >> ISP_ADDR_BIT_R_SHIFT, isp_regs[id].isp_save_load_addr);
}

/*******isp debug register which we can read directly to register*********/
unsigned int bsp_isp_get_isp_cur_id(unsigned long id)
{
	return isp_regs[id].isp_dbg_top->bits.isp_cur_id;
}

unsigned int bsp_isp_get_isp_status(unsigned long id)
{
	return isp_regs[id].isp_dbg_top->bits.isp_status;
}

unsigned int bsp_isp_get_lock_id(unsigned long id)
{
	return isp_regs[id].isp_ahb_mbus_lock->bits.lock_id;
}

unsigned int bsp_isp_get_internal_fifo(unsigned long id, unsigned int flag)
{
	return isp_regs[id].isp_internal_fifo->dwval & flag;
}

unsigned int bsp_isp_get_internal_cmb0(unsigned long id, unsigned int flag)
{
	return isp_regs[id].isp_internal_cmb0->dwval & flag;
}

unsigned int bsp_isp_get_internal_cmb1(unsigned long id, unsigned int flag)
{
	return isp_regs[id].isp_internal_cmb1->dwval & flag;
}

unsigned int bsp_isp_get_internal_cmb2(unsigned long id, unsigned int flag)
{
	return isp_regs[id].isp_internal_cmb2->dwval & flag;
}

unsigned int bsp_isp_get_internal_cmb3(unsigned long id, unsigned int flag)
{
	return isp_regs[id].isp_internal_cmb3->dwval & flag;
}

unsigned int bsp_isp_get_internal_cmb4(unsigned long id, unsigned int flag)
{
	return isp_regs[id].isp_internal_cmb4->dwval & flag;
}

/*******isp load register which we should write to ddr first*********/
unsigned int bsp_isp_load_update_flag(unsigned long id)
{
#ifndef USE_DEF_PARA
	//return isp_regs[id].isp_update_flag->dwval;
	return 0;
#else
	return 0;
#endif
}

void bsp_isp_set_input_fmt(unsigned long id, unsigned int fmt)
{
	isp_regs[id].isp_global_cfg0->bits.input_fmt = fmt;
	isp_regs[id].isp_saveload_cfg0->bits.input_fmt = fmt;
}

void bsp_isp_set_byr_max_bit(unsigned long id, int bit)
{
	isp_regs[id].isp_global_cfg0->bits.byr_max_bit = bit;
}

void bsp_isp_set_byr_act_bit(unsigned long id, int bit)
{
	isp_regs[id].isp_global_cfg0->bits.byr_act_bit = bit;
}

void bsp_isp_set_last_blank_cycle(unsigned long id, unsigned int blank)
{
	isp_regs[id].isp_global_cfg1->bits.last_blank_cycle = blank;
}

void bsp_isp_set_speed_mode(unsigned long id, unsigned int speed)
{
	isp_regs[id].isp_global_cfg1->bits.speed_mode = speed;
}

void bsp_isp_set_line_int_num(unsigned long id, unsigned int line_num)
{
	isp_regs[id].isp_global_cfg1->bits.line_int_num = line_num;
}

void bsp_isp_debug_output_cfg(unsigned long id, int enable, int output_sel)
{
	isp_regs[id].isp_global_cfg0->bits.dbg_out_en = enable;
	isp_regs[id].isp_global_cfg1->bits.dbg_out_sel = output_sel;
}

void bsp_isp_set_size(unsigned long id, struct isp_size_settings *size)
{
	isp_regs[id].isp_input_size->bits.input_width = size->ob_black.width;
	isp_regs[id].isp_input_size->bits.input_height = size->ob_black.height;
	isp_regs[id].isp_valid_size->bits.valid_width = size->ob_valid.width;
	isp_regs[id].isp_valid_size->bits.valid_height = size->ob_valid.height;
	isp_regs[id].isp_valid_start->bits.valid_hor_start = size->ob_start.hor;
	isp_regs[id].isp_valid_start->bits.valid_ver_start = size->ob_start.ver;
}

void bsp_isp_module0_enable(unsigned long id, unsigned int module_flag)
{
	isp_regs[id].isp_module_bypass0->dwval |= module_flag;
}

void bsp_isp_module0_disable(unsigned long id, unsigned int module_flag)
{
	isp_regs[id].isp_module_bypass0->dwval &= ~module_flag;
}

void bsp_isp_module_enable(unsigned long id, unsigned int module_flag)
{
	isp_regs[id].isp_module_bypass1->dwval |= module_flag;
}

void bsp_isp_module_disable(unsigned long id, unsigned int module_flag)
{
	isp_regs[id].isp_module_bypass1->dwval &= ~module_flag;
}

void bsp_isp_d3d_ref_frm_mode(unsigned long id, unsigned int d3d_ref)
{
	isp_regs[id].isp_module_mode0->bits.d3d_ref_frm_mode = d3d_ref;
}

void bsp_isp_set_d3d_k0_addr(unsigned long id, vin_dma_addr_t addr)
{
	writel(addr >> ISP_ADDR_BIT_R_SHIFT, isp_regs[id].isp_d3d_k0_addr);
}

void bsp_isp_set_d3d_k1_addr(unsigned long id, vin_dma_addr_t addr)
{
	writel(addr >> ISP_ADDR_BIT_R_SHIFT, isp_regs[id].isp_d3d_k1_addr);
}

void bsp_isp_set_d3d_status_addr(unsigned long id, vin_dma_addr_t addr)
{
	writel(addr >> ISP_ADDR_BIT_R_SHIFT, isp_regs[id].isp_d3d_status_addr);
}

void bsp_isp_set_d2d_bayer_addr(unsigned long id, vin_dma_addr_t addr)
{
	writel(addr >> ISP_ADDR_BIT_R_SHIFT, isp_regs[id].isp_d2d_bayer_addr);
}

void bsp_isp_d3d_set_lbc_align_choose(unsigned long id, enum isp_lbc_align_choose align_choose)
{
	isp_regs[id].isp_wdma_cfg1->bits.align_mode = align_choose;
}

void bsp_isp_d2d_set_raw_size(unsigned long id, unsigned int size)
{
	isp_regs[id].isp_wdma_cfg2->bits.isp_d2d_raw_size = size;
}

void bsp_isp_set_ch_input_bit(unsigned long id, int ch, int bit)
{
	if (ch == 0)
		isp_regs[id].isp_ch0_expand_cfg0->bits.input_bit = bit;
	else if (ch == 1)
		isp_regs[id].isp_ch1_expand_cfg0->bits.input_bit = bit;
	else if (ch == 2)
		isp_regs[id].isp_ch2_expand_cfg0->bits.input_bit = bit;
}

void bsp_isp_set_ch_output_bit(unsigned long id, int ch, int bit)
{
	if (ch == 0)
		isp_regs[id].isp_ch0_expand_cfg0->bits.output_bit = bit;
	else if (ch == 1)
		isp_regs[id].isp_ch1_expand_cfg0->bits.output_bit = bit;
	else if (ch == 2)
		isp_regs[id].isp_ch2_expand_cfg0->bits.output_bit = bit;
}

void bsp_isp_clr_d3d_rec_en(unsigned long id)
{
	isp_regs[id].isp_d3d_ctrl->bits.d3d_rec_en = 0;
}

void bsp_isp_set_d3d_lbc_cfg(unsigned long id, struct isp_lbc_cfg d3d_lbc)
{
	/*CFG*/
	isp_regs[id].isp_d3d_lbc_cfg0->bits.is_lossy = d3d_lbc.is_lossy;
	isp_regs[id].isp_d3d_lbc_cfg0->bits.rc_ctrl_mode = d3d_lbc.rc_ctrl_mode;
	isp_regs[id].isp_d3d_lbc_cfg0->bits.start_qp = d3d_lbc.start_qp;
	isp_regs[id].isp_d3d_lbc_cfg0->bits.std_qp = d3d_lbc.std_qp;
	isp_regs[id].isp_d3d_lbc_cfg0->bits.glb_max_quo = d3d_lbc.glb_max_quo;
	isp_regs[id].isp_d3d_lbc_cfg0->bits.glb_max_k = d3d_lbc.glb_max_k;
	isp_regs[id].isp_d3d_lbc_cfg0->bits.ptr_buffer_init = d3d_lbc.ptr_buffer_init;

	isp_regs[id].isp_d3d_lbc_cfg1->bits.mb_num_in_line = d3d_lbc.mb_num_in_line;

	isp_regs[id].isp_d3d_lbc_cfg2->bits.ptr_buffer_fullness_max = d3d_lbc.ptr_buffer_fullness_max;
	isp_regs[id].isp_d3d_lbc_cfg2->bits.ptr_buffer_thr = d3d_lbc.ptr_buffer_thr;

	isp_regs[id].isp_d3d_lbc_cfg3->bits.line_max_bit = d3d_lbc.line_max_bit;
	isp_regs[id].isp_d3d_lbc_cfg3->bits.tar_bits_line_rc = d3d_lbc.tar_bits_line_rc;

	isp_regs[id].isp_d3d_lbc_cfg4->bits.line_tar_bit = d3d_lbc.line_tar_bit;
	isp_regs[id].isp_d3d_lbc_cfg4->bits.tar_bits = d3d_lbc.tar_bits;

	isp_regs[id].isp_d3d_lbc_cfg5->bits.frame_tar_bit = d3d_lbc.frame_tar_bit;

	isp_regs[id].isp_d3d_lbc_cfg6->bits.line_stride = d3d_lbc.line_stride;

	/*LUT*/
	isp_regs[id].isp_lbc_min_qp_lut0->bits.qp_s0 = d3d_lbc.lbc_min_qp[0];
	isp_regs[id].isp_lbc_min_qp_lut0->bits.qp_s1 = d3d_lbc.lbc_min_qp[1];
	isp_regs[id].isp_lbc_min_qp_lut0->bits.qp_s2 = d3d_lbc.lbc_min_qp[2];
	isp_regs[id].isp_lbc_min_qp_lut0->bits.qp_s3 = d3d_lbc.lbc_min_qp[3];
	isp_regs[id].isp_lbc_min_qp_lut0->bits.qp_s4 = d3d_lbc.lbc_min_qp[4];
	isp_regs[id].isp_lbc_min_qp_lut0->bits.qp_s5 = d3d_lbc.lbc_min_qp[5];
	isp_regs[id].isp_lbc_min_qp_lut0->bits.qp_s6 = d3d_lbc.lbc_min_qp[6];
	isp_regs[id].isp_lbc_min_qp_lut0->bits.qp_s7 = d3d_lbc.lbc_min_qp[7];
	isp_regs[id].isp_lbc_min_qp_lut1->bits.qp_s0 = d3d_lbc.lbc_min_qp[8];
	isp_regs[id].isp_lbc_min_qp_lut1->bits.qp_s1 = d3d_lbc.lbc_min_qp[9];
	isp_regs[id].isp_lbc_min_qp_lut1->bits.qp_s2 = d3d_lbc.lbc_min_qp[10];
	isp_regs[id].isp_lbc_min_qp_lut1->bits.qp_s3 = d3d_lbc.lbc_min_qp[11];
	isp_regs[id].isp_lbc_min_qp_lut1->bits.qp_s4 = d3d_lbc.lbc_min_qp[12];
	isp_regs[id].isp_lbc_min_qp_lut1->bits.qp_s5 = d3d_lbc.lbc_min_qp[13];
	isp_regs[id].isp_lbc_min_qp_lut1->bits.qp_s6 = d3d_lbc.lbc_min_qp[14];
	isp_regs[id].isp_lbc_min_qp_lut1->bits.qp_s7 = d3d_lbc.lbc_min_qp[15];

	isp_regs[id].isp_lbc_max_qp_lut0->bits.qp_s0 = d3d_lbc.lbc_max_qp[0];
	isp_regs[id].isp_lbc_max_qp_lut0->bits.qp_s1 = d3d_lbc.lbc_max_qp[1];
	isp_regs[id].isp_lbc_max_qp_lut0->bits.qp_s2 = d3d_lbc.lbc_max_qp[2];
	isp_regs[id].isp_lbc_max_qp_lut0->bits.qp_s3 = d3d_lbc.lbc_max_qp[3];
	isp_regs[id].isp_lbc_max_qp_lut0->bits.qp_s4 = d3d_lbc.lbc_max_qp[4];
	isp_regs[id].isp_lbc_max_qp_lut0->bits.qp_s5 = d3d_lbc.lbc_max_qp[5];
	isp_regs[id].isp_lbc_max_qp_lut0->bits.qp_s6 = d3d_lbc.lbc_max_qp[6];
	isp_regs[id].isp_lbc_max_qp_lut0->bits.qp_s7 = d3d_lbc.lbc_max_qp[7];
	isp_regs[id].isp_lbc_max_qp_lut1->bits.qp_s0 = d3d_lbc.lbc_max_qp[8];
	isp_regs[id].isp_lbc_max_qp_lut1->bits.qp_s1 = d3d_lbc.lbc_max_qp[9];
	isp_regs[id].isp_lbc_max_qp_lut1->bits.qp_s2 = d3d_lbc.lbc_max_qp[10];
	isp_regs[id].isp_lbc_max_qp_lut1->bits.qp_s3 = d3d_lbc.lbc_max_qp[11];
	isp_regs[id].isp_lbc_max_qp_lut1->bits.qp_s4 = d3d_lbc.lbc_max_qp[12];
	isp_regs[id].isp_lbc_max_qp_lut1->bits.qp_s5 = d3d_lbc.lbc_max_qp[13];
	isp_regs[id].isp_lbc_max_qp_lut1->bits.qp_s6 = d3d_lbc.lbc_max_qp[14];
	isp_regs[id].isp_lbc_max_qp_lut1->bits.qp_s7 = d3d_lbc.lbc_max_qp[15];

	isp_regs[id].isp_lbc_thresh_lut0->bits.th_s0 = d3d_lbc.lbc_thresh[0];
	isp_regs[id].isp_lbc_thresh_lut0->bits.th_s1 = d3d_lbc.lbc_thresh[1];
	isp_regs[id].isp_lbc_thresh_lut1->bits.th_s0 = d3d_lbc.lbc_thresh[2];
	isp_regs[id].isp_lbc_thresh_lut1->bits.th_s1 = d3d_lbc.lbc_thresh[3];
	isp_regs[id].isp_lbc_thresh_lut2->bits.th_s0 = d3d_lbc.lbc_thresh[4];
	isp_regs[id].isp_lbc_thresh_lut2->bits.th_s1 = d3d_lbc.lbc_thresh[5];
	isp_regs[id].isp_lbc_thresh_lut3->bits.th_s0 = d3d_lbc.lbc_thresh[6];
	isp_regs[id].isp_lbc_thresh_lut3->bits.th_s1 = d3d_lbc.lbc_thresh[7];
	isp_regs[id].isp_lbc_thresh_lut4->bits.th_s0 = d3d_lbc.lbc_thresh[8];
	isp_regs[id].isp_lbc_thresh_lut4->bits.th_s1 = d3d_lbc.lbc_thresh[9];
	isp_regs[id].isp_lbc_thresh_lut5->bits.th_s0 = d3d_lbc.lbc_thresh[10];
	isp_regs[id].isp_lbc_thresh_lut5->bits.th_s1 = d3d_lbc.lbc_thresh[11];
	isp_regs[id].isp_lbc_thresh_lut6->bits.th_s0 = d3d_lbc.lbc_thresh[12];
	isp_regs[id].isp_lbc_thresh_lut6->bits.th_s1 = d3d_lbc.lbc_thresh[13];
	isp_regs[id].isp_lbc_thresh_lut7->bits.th_s0 = d3d_lbc.lbc_thresh[14];
	isp_regs[id].isp_lbc_thresh_lut7->bits.th_s1 = d3d_lbc.lbc_thresh[15];

	isp_regs[id].isp_lbc_tar_bits_adj_lut0->bits.tar_bits_adj_s0 = d3d_lbc.lbc_tar_bits_adj[0];
	isp_regs[id].isp_lbc_tar_bits_adj_lut0->bits.tar_bits_adj_s1 = d3d_lbc.lbc_tar_bits_adj[1];
	isp_regs[id].isp_lbc_tar_bits_adj_lut1->bits.tar_bits_adj_s0 = d3d_lbc.lbc_tar_bits_adj[2];
	isp_regs[id].isp_lbc_tar_bits_adj_lut1->bits.tar_bits_adj_s1 = d3d_lbc.lbc_tar_bits_adj[3];
	isp_regs[id].isp_lbc_tar_bits_adj_lut2->bits.tar_bits_adj_s0 = d3d_lbc.lbc_tar_bits_adj[4];
	isp_regs[id].isp_lbc_tar_bits_adj_lut2->bits.tar_bits_adj_s1 = d3d_lbc.lbc_tar_bits_adj[5];
	isp_regs[id].isp_lbc_tar_bits_adj_lut3->bits.tar_bits_adj_s0 = d3d_lbc.lbc_tar_bits_adj[6];
	isp_regs[id].isp_lbc_tar_bits_adj_lut3->bits.tar_bits_adj_s1 = d3d_lbc.lbc_tar_bits_adj[7];
	isp_regs[id].isp_lbc_tar_bits_adj_lut4->bits.tar_bits_adj_s0 = d3d_lbc.lbc_tar_bits_adj[8];
	isp_regs[id].isp_lbc_tar_bits_adj_lut4->bits.tar_bits_adj_s1 = d3d_lbc.lbc_tar_bits_adj[9];
	isp_regs[id].isp_lbc_tar_bits_adj_lut5->bits.tar_bits_adj_s0 = d3d_lbc.lbc_tar_bits_adj[10];
	isp_regs[id].isp_lbc_tar_bits_adj_lut5->bits.tar_bits_adj_s1 = d3d_lbc.lbc_tar_bits_adj[11];
	isp_regs[id].isp_lbc_tar_bits_adj_lut6->bits.tar_bits_adj_s0 = d3d_lbc.lbc_tar_bits_adj[12];
	isp_regs[id].isp_lbc_tar_bits_adj_lut6->bits.tar_bits_adj_s1 = d3d_lbc.lbc_tar_bits_adj[13];
	isp_regs[id].isp_lbc_tar_bits_adj_lut7->bits.tar_bits_adj_s0 = d3d_lbc.lbc_tar_bits_adj[14];
	isp_regs[id].isp_lbc_tar_bits_adj_lut7->bits.tar_bits_adj_s1 = d3d_lbc.lbc_tar_bits_adj[15];

	isp_regs[id].isp_lbc_pre_bits_adj_lut->bits.pre_bits_adj_s0 = d3d_lbc.lbc_pre_bits_adj[0];
	isp_regs[id].isp_lbc_pre_bits_adj_lut->bits.pre_bits_adj_s1 = d3d_lbc.lbc_pre_bits_adj[1];
	isp_regs[id].isp_lbc_pre_bits_adj_lut->bits.pre_bits_adj_s2 = d3d_lbc.lbc_pre_bits_adj[2];
	isp_regs[id].isp_lbc_pre_bits_adj_lut->bits.pre_bits_adj_s3 = d3d_lbc.lbc_pre_bits_adj[3];
	isp_regs[id].isp_lbc_pre_bits_adj_lut->bits.pre_bits_adj_s4 = d3d_lbc.lbc_pre_bits_adj[4];
}

void bsp_isp_set_stat(unsigned long id, struct isp_stat_config *cfg)
{
	isp_regs[id].isp_stat_cfg0->bits.stat_valid_block_w_num = cfg->stat_valid_block_w_num - 1;
	isp_regs[id].isp_stat_cfg0->bits.stat_valid_block_h_num = cfg->stat_valid_block_h_num - 1;
	isp_regs[id].isp_stat_cfg0->bits.stat_valid_block_num = cfg->stat_valid_block_num;
	isp_regs[id].isp_stat_cfg1->bits.stat_last_block_w_start = cfg->stat_last_block_w_start;
	isp_regs[id].isp_stat_cfg1->bits.stat_last_block_h_comp = cfg->stat_last_block_h_comp;
	isp_regs[id].isp_stat_cfg2->bits.stat_div_para = cfg->stat_div_para - 1;
	isp_regs[id].isp_stat_cfg2->bits.stat_valid_block_width = cfg->stat_valid_block_width - 1;
	isp_regs[id].isp_stat_cfg2->bits.stat_valid_block_height = cfg->stat_valid_block_height - 1;
	isp_regs[id].isp_stat_cfg3->bits.stat_intp_w_step = cfg->stat_intp_w_step;
	isp_regs[id].isp_stat_cfg3->bits.stat_intp_h_step = cfg->stat_intp_h_step;
	isp_regs[id].isp_stat_cfg3->bits.stat_last_block_h_comp_line = cfg->stat_last_block_h_comp_line;
}

/*******isp save_load register which we should write to ddr first*********/
void bsp_isp_set_frm_cnt(unsigned long id, unsigned int frm_cnt)
{
	writel(frm_cnt, isp_regs[id].isp_frm_cnt);
}

void bsp_isp_set_next_frm_number(unsigned long id, unsigned int load_next_frm, unsigned int save_next_frm)
{
	isp_regs[id].isp_next_frm_number->bits.load_next_frm_num = load_next_frm;
	isp_regs[id].isp_next_frm_number->bits.save_next_frm_num = save_next_frm;
}

void bsp_isp_set_d3d_bayer_addr(unsigned long id, vin_dma_addr_t addr)
{
	writel(addr >> ISP_ADDR_BIT_R_SHIFT, isp_regs[id].isp_d3d_bayer_waddr);
	writel(addr >> ISP_ADDR_BIT_R_SHIFT, isp_regs[id].isp_d3d_bayer_raddr);
}

/*
 * syscfg Register Address
 */
void bsp_isp_map_syscfg_addr(unsigned long id, vin_dma_addr_t base)
{
}

void bsp_isp_sram_boot_mode_ctrl(unsigned long id, unsigned int mode)
{
}