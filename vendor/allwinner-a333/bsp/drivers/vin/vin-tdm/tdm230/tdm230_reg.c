/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright(c) 2020 - 2023 Allwinner Technology Co.,Ltd. All rights reserved. */
/*
 *
 * Copyright (c) 2007-2019 Allwinnertech Co., Ltd.
 *
 * Authors:  Zheng Zequn <zequnzheng@allwinnertech.com>
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

#include <linux/kernel.h>
#include "tdm230_reg.h"

#include "../../utility/vin_io.h"
#include "../../platform/platform_cfg.h"

volatile void __iomem *csic_tdm_base[VIN_MAX_TDM];

#define TDM_ADDR_BIT_R_SHIFT 2

int csic_tdm_set_base_addr(unsigned int sel, vin_dma_addr_t addr)
{
	if (sel > VIN_MAX_TDM - 1)
		return -1;
	csic_tdm_base[sel] = (volatile void __iomem *)addr;

	return 0;
}

/*
 * function about tdm top registers
 */
void csic_tdm_top_enable(unsigned int sel)
{
	vin_reg_clr_set(csic_tdm_base[sel] + TDM_GOLBAL_CFG0_REG_OFF,
			TDM_TOP_EN_MASK, 1 << TDM_TOP_EN);
}

void csic_tdm_top_disable(unsigned int sel)
{
	vin_reg_clr_set(csic_tdm_base[sel] + TDM_GOLBAL_CFG0_REG_OFF,
			TDM_TOP_EN_MASK, 0 << TDM_TOP_EN);
}

void csic_tdm_enable(unsigned int sel)
{
	//vin_reg_clr_set(csic_tdm_base[sel] + TDM_GOLBAL_CFG0_REG_OFF,
	//		TDM_EN_MASK, 1 << TDM_EN);
}

void csic_tdm_disable(unsigned int sel)
{
	//vin_reg_clr_set(csic_tdm_base[sel] + TDM_GOLBAL_CFG0_REG_OFF,
	//		TDM_EN_MASK, 0 << TDM_EN);
}

void csic_tdm_vgm_enable(unsigned int sel)
{
	vin_reg_clr_set(csic_tdm_base[sel] + TDM_GOLBAL_CFG0_REG_OFF,
			VGM_EN_MASK, 1 << VGM_EN);
}

void csic_tdm_vgm_disable(unsigned int sel)
{
	vin_reg_clr_set(csic_tdm_base[sel] + TDM_GOLBAL_CFG0_REG_OFF,
			VGM_EN_MASK, 0 << VGM_EN);
}

void csic_tdm_set_speed_dn(unsigned int sel, unsigned int en)
{
	vin_reg_clr_set(csic_tdm_base[sel] + TDM_GOLBAL_CFG0_REG_OFF,
			TDM_SPEED_DN_EN_MASK, en << TDM_SPEED_DN_EN);
}

void csic_tdm_set_lbc_align_choose(unsigned int sel, enum lbc_align_choose align)
{
	vin_reg_clr_set(csic_tdm_base[sel] + TDM_GOLBAL_CFG0_REG_OFF,
			TDM_LBC_ALIGN_CHOOSE_MASK, align << TDM_LBC_ALIGN_CHOOSE);
}

void csic_tdm_set_rx_chn_cfg_mode(unsigned int sel, enum rx_chn_cfg_mode mode)
{
	vin_reg_clr_set(csic_tdm_base[sel] + TDM_GOLBAL_CFG0_REG_OFF,
			RX_CHN_CFG_MODE_MASK, mode << RX_CHN_CFG_MODE);
}

void csic_tdm_set_tx_chn_cfg_mode(unsigned int sel, enum tx_chn_cfg_mode mode)
{
	vin_reg_clr_set(csic_tdm_base[sel] + TDM_GOLBAL_CFG0_REG_OFF,
			TX_CHN_CFG_MODE_MASK, mode << TX_CHN_CFG_MODE);
}

unsigned char csic_tdm_get_tx_chn_cfg_mode(unsigned int sel)
{
	return (vin_reg_readl(csic_tdm_base[sel] + TDM_GOLBAL_CFG0_REG_OFF) & TX_CHN_CFG_MODE_MASK) >> TX_CHN_CFG_MODE;
}

void csic_tdm_set_work_mode(unsigned int sel, enum tdm_work_mode mode)
{
	vin_reg_clr_set(csic_tdm_base[sel] + TDM_GOLBAL_CFG0_REG_OFF,
			RX_WORK_MODE_MASK, mode << RX_WORK_MODE);
}

void csic_tdm_time_embed_en(unsigned int sel, unsigned int en)
{
	vin_reg_clr_set(csic_tdm_base[sel] + TDM_GOLBAL_CFG0_REG_OFF,
			TDM_EMBED_EN_MASK, en << TDM_EMBED_EN);
	vin_reg_clr_set(csic_tdm_base[sel] + TDM_GOLBAL_CFG0_REG_OFF,
			TDM_TIMER_EN_MASK, en << TDM_TIMER_EN);
}

void csic_tdm_set_line_fresh(unsigned int sel, unsigned int en)
{
	vin_reg_clr_set(csic_tdm_base[sel] + TDM_GOLBAL_CFG0_REG_OFF,
			RX_DATA_LINE_FRESH_EN_MASK, en << RX_DATA_LINE_FRESH_EN);
	vin_reg_clr_set(csic_tdm_base[sel] + TDM_GOLBAL_CFG0_REG_OFF,
			TX_DATA_LINE_FRESH_EN_MASK, en << TX_DATA_LINE_FRESH_EN);
}

void csic_tdm_int_enable(unsigned int sel, enum tdm_int_sel interrupt)
{
	vin_reg_set(csic_tdm_base[sel] + TDM_INT_BYPASS0_REG_OFF, interrupt);
}

void csic_tdm_int_disable(unsigned int sel, enum tdm_int_sel interrupt)
{
	vin_reg_clr(csic_tdm_base[sel] + TDM_INT_BYPASS0_REG_OFF, interrupt);
}

void csic_tdm_int_get_status(unsigned int sel, struct tdm_int_status *status)
{
	unsigned int reg_val = vin_reg_readl(csic_tdm_base[sel] + TDM_INT_STATUS0_REG_OFF);
	unsigned int irq_enable = vin_reg_readl(csic_tdm_base[sel] + TDM_INT_BYPASS0_REG_OFF);

	status->rx_frm_lost = (reg_val & RX_FRM_LOST_PD_MASK) >> RX_FRM_LOST_PD & (irq_enable & RX_FRM_LOST_PD_MASK) >> RX_FRM_LOST_PD;
	status->rx_frm_err = (reg_val & RX_FRM_ERR_PD_MASK) >> RX_FRM_ERR_PD & (irq_enable & RX_FRM_ERR_PD_MASK) >> RX_FRM_ERR_PD;
	status->rx_btype_err = (reg_val & RX_BTYPE_ERR_PD_MASK) >> RX_BTYPE_ERR_PD & (irq_enable & RX_BTYPE_ERR_PD_MASK) >> RX_BTYPE_ERR_PD;
	status->rx_buf_full = (reg_val & RX_BUF_FULL_PD_MASK) >> RX_BUF_FULL_PD & (irq_enable & RX_BUF_FULL_PD_MASK) >> RX_BUF_FULL_PD;
	status->speed_dn_hsync = (reg_val & SPEED_DN_HSYN_PD_MASK) >> SPEED_DN_HSYN_PD & (irq_enable & SPEED_DN_HSYN_PD_MASK) >> SPEED_DN_HSYN_PD;
	status->rx_hb_short = (reg_val & RX_HB_SHORT_PD_MASK) >> RX_HB_SHORT_PD & (irq_enable & RX_HB_SHORT_PD_MASK) >> RX_HB_SHORT_PD;
	status->rx_fifo_full = (reg_val & RX_FIFO_FULL_PD_MASK) >> RX_FIFO_FULL_PD & (irq_enable & RX_FIFO_FULL_PD_MASK) >> RX_FIFO_FULL_PD;
	status->tdm_lbd_err = (reg_val & TDM_LBD_ERROR_PD_MASK) >> TDM_LBD_ERROR_PD & (irq_enable & TDM_LBD_ERROR_PD_MASK) >> TDM_LBD_ERROR_PD;
	status->tdm_lbc_err = (reg_val & TDM_LBC_ERROR_PD_MASK) >> TDM_LBC_ERROR_PD & (irq_enable & TDM_LBC_ERROR_PD_MASK) >> TDM_LBC_ERROR_PD;
	status->tx_fifo_under = (reg_val & TX_FIFO_UNDER_PD_MASK) >> TX_FIFO_UNDER_PD & (irq_enable & TX_FIFO_UNDER_PD_MASK) >> TX_FIFO_UNDER_PD;
	status->tx_frm_done = (reg_val & TX_FRM_DONE_PD_MASK) >> TX_FRM_DONE_PD & (irq_enable & TX_FRM_DONE_PD_MASK) >> TX_FRM_DONE_PD;
	status->speed_dn_fifo_full = (reg_val & SPEED_DN_FIFO_FULL_PD_MASK) >> SPEED_DN_FIFO_FULL_PD & (irq_enable & SPEED_DN_FIFO_FULL_PD_MASK) >> SPEED_DN_FIFO_FULL_PD;

	status->rx0_frm_start = (reg_val & RX0_FRM_START_PD_MASK) >> RX0_FRM_START_PD & (irq_enable & RX0_FRM_START_PD_MASK) >> RX0_FRM_START_PD;
	status->rx1_frm_start = (reg_val & RX1_FRM_START_PD_MASK) >> RX1_FRM_START_PD & (irq_enable & RX1_FRM_START_PD_MASK) >> RX1_FRM_START_PD;
	status->rx2_frm_start = (reg_val & RX2_FRM_START_PD_MASK) >> RX2_FRM_START_PD & (irq_enable & RX2_FRM_START_PD_MASK) >> RX2_FRM_START_PD;
	status->rx3_frm_start = (reg_val & RX3_FRM_START_PD_MASK) >> RX3_FRM_START_PD & (irq_enable & RX3_FRM_START_PD_MASK) >> RX3_FRM_START_PD;
	status->rx0_frm_done = (reg_val & RX0_FRM_DONE_PD_MASK) >> RX0_FRM_DONE_PD & (irq_enable & RX0_FRM_DONE_PD_MASK) >> RX0_FRM_DONE_PD;
	status->rx1_frm_done = (reg_val & RX1_FRM_DONE_PD_MASK) >> RX1_FRM_DONE_PD & (irq_enable & RX1_FRM_DONE_PD_MASK) >> RX1_FRM_DONE_PD;
	status->rx2_frm_done = (reg_val & RX2_FRM_DONE_PD_MASK) >> RX2_FRM_DONE_PD & (irq_enable & RX2_FRM_DONE_PD_MASK) >> RX2_FRM_DONE_PD;
	status->rx3_frm_done = (reg_val & RX3_FRM_DONE_PD_MASK) >> RX3_FRM_DONE_PD & (irq_enable & RX3_FRM_DONE_PD_MASK) >> RX3_FRM_DONE_PD;
	status->rx0_n_line_start = (reg_val & RX0_N_LINE_START_PD_MASK) >> RX0_N_LINE_START_PD & (irq_enable & RX0_N_LINE_START_PD_MASK) >> RX0_N_LINE_START_PD;
	status->rx1_n_line_start = (reg_val & RX1_N_LINE_START_PD_MASK) >> RX1_N_LINE_START_PD & (irq_enable & RX1_N_LINE_START_PD_MASK) >> RX1_N_LINE_START_PD;
	status->rx2_n_line_start = (reg_val & RX2_N_LINE_START_PD_MASK) >> RX2_N_LINE_START_PD & (irq_enable & RX2_N_LINE_START_PD_MASK) >> RX2_N_LINE_START_PD;
	status->rx3_n_line_start = (reg_val & RX3_N_LINE_START_PD_MASK) >> RX3_N_LINE_START_PD & (irq_enable & RX3_N_LINE_START_PD_MASK) >> RX3_N_LINE_START_PD;

	status->rx_chn_cfg_mode = (reg_val & RX_CHN_CFG_MODE_PD_MASK) >> RX_CHN_CFG_MODE_PD & (irq_enable & RX_CHN_CFG_MODE_PD_MASK) >> RX_CHN_CFG_MODE_PD;
	status->tx_chn_cfg_mode = (reg_val & TX_CHN_CFG_MODE_PD_MASK) >> TX_CHN_CFG_MODE_PD & (irq_enable & TX_CHN_CFG_MODE_PD_MASK) >> TX_CHN_CFG_MODE_PD;
	status->awnn_id_back_err = (reg_val & AWNN_ID_BACK_ERR_PD_MASK) >> AWNN_ID_BACK_ERR_PD & (irq_enable & AWNN_ID_BACK_ERR_PD_MASK) >> AWNN_ID_BACK_ERR_PD;
	status->rx_lbc_mux_conf_time_out = (reg_val & RX_LBC_MUX_CONF_TIME_OUT_PD_MASK) >> RX_LBC_MUX_CONF_TIME_OUT_PD & (irq_enable & RX_LBC_MUX_CONF_TIME_OUT_PD_MASK) >> RX_LBC_MUX_CONF_TIME_OUT_PD;
	status->awnn_id_frm_done = (reg_val & AWNN_ID_FRM_DONE_PD_MASK) >> AWNN_ID_FRM_DONE_PD & (irq_enable & AWNN_ID_FRM_DONE_PD_MASK) >> AWNN_ID_FRM_DONE_PD;
	status->rx_2to1_fifo_ov = (reg_val & RX_2TO1_FIFO_OV_PD_MASK) >> RX_2TO1_FIFO_OV_PD & (irq_enable & RX_2TO1_FIFO_OV_PD_MASK) >> RX_2TO1_FIFO_OV_PD;
	status->awnn_time_out_by_awnn = (reg_val & AWNN_TIME_OUT_BY_AWNN_PD_MASK) >> AWNN_TIME_OUT_BY_AWNN_PD & (irq_enable & AWNN_TIME_OUT_BY_AWNN_PD_MASK) >> AWNN_TIME_OUT_BY_AWNN_PD;
	status->rx_w_addr_exceed = (reg_val & RX_W_ADDR_EXCEED_PD_MASK) >> RX_W_ADDR_EXCEED_PD & (irq_enable & RX_W_ADDR_EXCEED_PD_MASK) >> RX_W_ADDR_EXCEED_PD;
}

bool csic_tdm_int_sel_get_status(unsigned int sel, enum tdm_int_sel interrupt)
{
	return vin_reg_readl(csic_tdm_base[sel] + TDM_INT_STATUS0_REG_OFF) & interrupt;
}

void csic_tdm_int_clear_status(unsigned int sel, enum tdm_int_sel interrupt)
{
	vin_reg_writel(csic_tdm_base[sel] + TDM_INT_STATUS0_REG_OFF, interrupt);
}

unsigned int csic_tdm_internal_get_status(unsigned int sel, unsigned int status)
{
	return (vin_reg_readl(csic_tdm_base[sel] + TDM_INT_STATUS0_REG_OFF)) & status;
}

unsigned int csic_tdm_internal_get_status0(unsigned int sel, unsigned int status)
{
	return (vin_reg_readl(csic_tdm_base[sel] + TDM_INTERNAL_STATUS0_REG_OFF)) & status;
}

void csic_tdm_internal_clear_status0(unsigned int sel, unsigned int status)
{
	vin_reg_writel(csic_tdm_base[sel] + TDM_INTERNAL_STATUS0_REG_OFF, status);
}

unsigned int csic_tdm_internal_get_status1(unsigned int sel, unsigned int status)
{
	return (vin_reg_readl(csic_tdm_base[sel] + TDM_INTERNAL_STATUS1_REG_OFF)) & status;
}

void csic_tdm_internal_clear_status1(unsigned int sel, unsigned int status)
{
	vin_reg_writel(csic_tdm_base[sel] + TDM_INTERNAL_STATUS1_REG_OFF, status);
}

unsigned int csic_tdm_internal_get_status2(unsigned int sel, unsigned int status)
{
	return (vin_reg_readl(csic_tdm_base[sel] + TDM_INTERNAL_STATUS2_REG_OFF)) & status;
}

void csic_tdm_internal_clear_status2(unsigned int sel, unsigned int status)
{
	vin_reg_writel(csic_tdm_base[sel] + TDM_INTERNAL_STATUS2_REG_OFF, status);
}

void csic_tdm_set_clk_freq(unsigned int sel, unsigned int clk_mz)
{
	vin_reg_clr_set(csic_tdm_base[sel] + TDM_CLK_FREQ_REG_OFF,
			TDM_CLK_FREQ_SET_MASK, clk_mz << TDM_CLK_FREQ_SET);
}

void csic_tdm_set_time_base(unsigned int sel, unsigned int base)
{
	vin_reg_writel(csic_tdm_base[sel] + TDM_TIME_BASE_REG_OFF, base);
}

/*
 * function about tdm vgm registers
 */
#if VIN_FALSE
void csic_tdm_set_vgm_data_mode(unsigned int sel, enum vgm_data_mode mode)
{
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_VGM_OFFSET + TDM_VGM_CFG0_REG_OFF,
			VGM_DMODE_MASK, mode << VGM_DMODE);
}

void csic_tdm_set_vgm_smode(unsigned int sel, enum vgm_smode mode)
{
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_VGM_OFFSET + TDM_VGM_CFG0_REG_OFF,
			VGM_SMODE_MASK, mode << VGM_SMODE);
}

void csic_tdm_vgm_start(unsigned int sel)
{
	 vin_reg_clr_set(csic_tdm_base[sel] + TMD_VGM_OFFSET + TDM_VGM_CFG0_REG_OFF,
			 VGM_START_MASK, 1 << VGM_START);
}

void csic_tdm_vgm_stop(unsigned int sel, enum vgm_data_mode mode)
{
	 vin_reg_clr_set(csic_tdm_base[sel] + TMD_VGM_OFFSET + TDM_VGM_CFG0_REG_OFF,
			 VGM_START_MASK, 0 << VGM_START);
}

void csic_tdm_set_vgm_bcycle(unsigned int sel, unsigned int cycle)
{
	 vin_reg_clr_set(csic_tdm_base[sel] + TMD_VGM_OFFSET + TDM_VGM_CFG0_REG_OFF,
			 VGM_BCYCLE_MASK, cycle << VGM_BCYCLE);
}

void csic_tdm_set_vgm_input_fmt(unsigned int sel, enum tdm_input_fmt fmt)
{
	 vin_reg_clr_set(csic_tdm_base[sel] + TMD_VGM_OFFSET + TDM_VGM_CFG1_REG_OFF,
			 VGM_INPUT_FMT_MASK, fmt << VGM_INPUT_FMT);
}

void csic_tdm_set_vgm_data_type(unsigned int sel, enum vgm_data_type type)
{
	 vin_reg_clr_set(csic_tdm_base[sel] + TMD_VGM_OFFSET + TDM_VGM_CFG1_REG_OFF,
			 VGM_DATA_TYPE_MASK, type << VGM_DATA_TYPE);
}

void csic_tdm_set_vgm_para0(unsigned int sel, unsigned int para0, unsigned int para1)
{
	 vin_reg_clr_set(csic_tdm_base[sel] + TMD_VGM_OFFSET + TDM_VGM_CFG2_REG_OFF,
			 VGM_PARA0_MASK, para0 << VGM_PARA0);
	 vin_reg_clr_set(csic_tdm_base[sel] + TMD_VGM_OFFSET + TDM_VGM_CFG2_REG_OFF,
			 VGM_PARA1_MASK, para1 << VGM_PARA1);
}
#endif

/*
 * function about tdm tx registers
 */
void csic_tdm_tx_cap_enable(unsigned int sel)
{
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_TX_OFFSET + TDM_TX_CFG0_REG_OFF,
				TDM_TX_EN_MASK, 1 << TDM_TX_EN);
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_TX_OFFSET + TDM_TX_CFG0_REG_OFF,
				TDM_TX_CAP_EN_MASK, 1 << TDM_TX_CAP_EN);
}

void csic_tdm_tx_cap_disable(unsigned int sel)
{
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_TX_OFFSET + TDM_TX_CFG0_REG_OFF,
				TDM_TX_CAP_EN_MASK, 0 << TDM_TX_CAP_EN);
}

void csic_tdm_tx_enable(unsigned int sel)
{
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_TX_OFFSET + TDM_TX_CFG0_REG_OFF,
				TDM_TX_EN_MASK, 1 << TDM_TX_EN);
}

void csic_tdm_tx_disable(unsigned int sel)
{
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_TX_OFFSET + TDM_TX_CFG0_REG_OFF,
				TDM_TX_EN_MASK, 0 << TDM_TX_EN);
}

void csic_tdm_omode(unsigned int sel, unsigned int mode)
{
}

void csic_tdm_fifo_mode(unsigned int sel, unsigned int mode)
{
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_TX_OFFSET + TDM_TX_CFG0_REG_OFF,
				TDM_TX_FIFO_MODE_MASK, mode << TDM_TX_FIFO_MODE);
}

void csic_tdm_set_hblank(unsigned int sel, unsigned int hblank)
{
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_TX_OFFSET + TDM_TX_CFG1_REG_OFF,
				TDM_TX_H_BLANK_MASK, hblank << TDM_TX_H_BLANK);
}

void csic_tdm_set_bblank_fe(unsigned int sel, unsigned int bblank_fe)
{
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_TX_OFFSET + TDM_TX_CFG2_REG_OFF,
				TDM_TX_V_BLANK_FE_MASK, bblank_fe << TDM_TX_V_BLANK_FE);
}

void csic_tdm_set_bblank_be(unsigned int sel, unsigned int bblank_be)
{
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_TX_OFFSET + TDM_TX_CFG2_REG_OFF,
				TDM_TX_V_BLANK_BE_MASK, bblank_be << TDM_TX_V_BLANK_BE);
}

void csic_tdm_set_tx_t1_cycle(unsigned int sel, unsigned int cycle)
{
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_TX_OFFSET + TDM_TX_TIME1_CYCLE_OFF,
				TDM_TX_T1_CYCLE_MASK, cycle << TDM_TX_T1_CYCLE);
}

void csic_tdm_set_tx_t2_cycle(unsigned int sel, unsigned int cycle)
{
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_TX_OFFSET + TDM_TX_TIME2_CYCLE_OFF,
				TDM_TX_T2_CYCLE_MASK, cycle << TDM_TX_T2_CYCLE);
}

void csic_tdm_set_tx_fifo_depth(unsigned int sel, unsigned int head_depth, unsigned int data_depth)
{
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_TX_OFFSET + TDM_TX_FIFO_DEPTH_OFF,
				TDM_TX_HEAD_FIFO_MASK, head_depth << TDM_TX_HEAD_FIFO);
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_TX_OFFSET + TDM_TX_FIFO_DEPTH_OFF,
				TDM_TX_DATA_FIFO_MASK, data_depth << TDM_TX_DATA_FIFO);
}

#if VIN_FALSE
void csic_tdm_set_tx_invalid_num(unsigned int sel, unsigned int num)
{
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_TX_OFFSET + TDM_TX_DATA_RATE_REG_OFF,
				TDM_TX_INVALID_NUM_MASK, num << TDM_TX_INVALID_NUM);
}

void csic_tdm_set_tx_valid_num(unsigned int sel, unsigned int num)
{
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_TX_OFFSET + TDM_TX_DATA_RATE_REG_OFF,
				TDM_TX_VALID_NUM_MASK, num << TDM_TX_VALID_NUM);
}
#else
void csic_tdm_set_tx_data_rate(unsigned int sel, unsigned int valid_num, unsigned int invalid_num)
{
	unsigned int num;

	num = ((valid_num << TDM_TX_VALID_NUM) & TDM_TX_VALID_NUM_MASK) + ((invalid_num << TDM_TX_INVALID_NUM) & TDM_TX_INVALID_NUM_MASK);
	vin_reg_writel(csic_tdm_base[sel] + TMD_TX_OFFSET + TDM_TX_DATA_RATE_REG_OFF, num);
}
#endif

unsigned int csic_tdm_get_tx_ctrl_status(unsigned int sel)
{
	return (vin_reg_readl(csic_tdm_base[sel] + TMD_TX_OFFSET + TDM_TX_CTRL_ST_REG_OFF) & TX_CTRL_ST_MASK) >> TX_CTRL_ST;
}

void csic_tdm_set_tx_id0_fifo_depth(unsigned int sel, unsigned int head_depth, unsigned int data_depth)
{
	int i = 0;
	for (i = 0; i < TDM_RX_NUM; i++) {
		vin_reg_clr_set(csic_tdm_base[sel] + TMD_TX_OFFSET + TDM_TX_ID0_FIFO_DEPTH_OFF + 4 * i,
					TDM_TX_ID0_HEAD_FIFO_MASK, head_depth << TDM_TX_ID0_HEAD_FIFO);
		vin_reg_clr_set(csic_tdm_base[sel] + TMD_TX_OFFSET + TDM_TX_ID0_FIFO_DEPTH_OFF + 4 * i,
					TDM_TX_ID0_DATA_FIFO_MASK, data_depth << TDM_TX_ID0_DATA_FIFO);
	}
}

void csic_tdm_tx_hwc_fifo_set_address(unsigned int sel, unsigned int address_offset)
{
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_TX_OFFSET + TDM_TX_HWC_FIFO_ADDR_OFF,
					TDM_TX0_HWC_FIFO_ADDR_MASK, address_offset << TDM_TX0_HWC_FIFO_ADDR);
}

/*
 * function about tdm rx registers
 */
void csic_tdm_rx_enable(unsigned int sel, unsigned int ch)
{
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_RX0_OFFSET + ch*AMONG_RX_OFFSET + TDM_RX_CFG0_REG_OFF,
					TDM_RX_EN_MASK, 1 << TDM_RX_EN);
}

void csic_tdm_rx_disable(unsigned int sel, unsigned int ch)
{
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_RX0_OFFSET + ch*AMONG_RX_OFFSET + TDM_RX_CFG0_REG_OFF,
					TDM_RX_EN_MASK, 0 << TDM_RX_EN);
}

void csic_tdm_rx_cap_enable(unsigned int sel, unsigned int ch)
{
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_RX0_OFFSET + ch*AMONG_RX_OFFSET + TDM_RX_CFG0_REG_OFF,
					TDM_RX_CAP_EN_MASK, 1 << TDM_RX_CAP_EN);
}

void csic_tdm_rx_cap_disable(unsigned int sel, unsigned int ch)
{
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_RX0_OFFSET + ch*AMONG_RX_OFFSET + TDM_RX_CFG0_REG_OFF,
					TDM_RX_CAP_EN_MASK, 0 << TDM_RX_CAP_EN);
}

void csic_tdm_rx_seq_init(unsigned int sel, unsigned int ch, unsigned int reset)
{
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_RX0_OFFSET + ch*AMONG_RX_OFFSET + TDM_RX_CFG0_REG_OFF,
					TDM_RX_SEQ_INIT_MASK, 1 << TDM_RX_SEQ_INIT);
}

void csic_tdm_rx_pre_w_para_en(unsigned int sel, unsigned int ch, unsigned int en)
{
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_RX0_OFFSET + ch*AMONG_RX_OFFSET + TDM_RX_CFG1_REG_OFF,
					TDM_RX_PRE_W_PARA_EN_MASK, en << TDM_RX_PRE_W_PARA_EN);
}

void csic_tdm_rx_tx_enable(unsigned int sel, unsigned int ch)
{
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_RX0_OFFSET + ch*AMONG_RX_OFFSET + TDM_RX_CFG1_REG_OFF,
					TDM_RX_TX_EN_MASK, 1 << TDM_RX_TX_EN);
}

void csic_tdm_rx_tx_disable(unsigned int sel, unsigned int ch)
{
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_RX0_OFFSET + ch*AMONG_RX_OFFSET + TDM_RX_CFG1_REG_OFF,
					TDM_RX_TX_EN_MASK, 0 << TDM_RX_TX_EN);
}

void csic_tdm_rx_lbc_enable(unsigned int sel, unsigned int ch)
{
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_RX0_OFFSET + ch*AMONG_RX_OFFSET + TDM_RX_CFG1_REG_OFF,
					TDM_RX_LBC_EN_MASK, 1 << TDM_RX_LBC_EN);
}

void csic_tdm_rx_lbc_disable(unsigned int sel, unsigned int ch)
{
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_RX0_OFFSET + ch*AMONG_RX_OFFSET + TDM_RX_CFG1_REG_OFF,
					TDM_RX_LBC_EN_MASK, 0 << TDM_RX_LBC_EN);
}

void csic_tdm_rx_pkg_enable(unsigned int sel, unsigned int ch)
{
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_RX0_OFFSET + ch*AMONG_RX_OFFSET + TDM_RX_CFG1_REG_OFF,
					TDM_RX_PKG_EN_MASK, 1 << TDM_RX_PKG_EN);
}

void csic_tdm_rx_pkg_disable(unsigned int sel, unsigned int ch)
{
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_RX0_OFFSET + ch*AMONG_RX_OFFSET + TDM_RX_CFG1_REG_OFF,
					TDM_RX_PKG_EN_MASK, 0 << TDM_RX_PKG_EN);
}

void csic_tdm_rx_sync_enable(unsigned int sel, unsigned int ch)
{
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_RX0_OFFSET + ch*AMONG_RX_OFFSET + TDM_RX_CFG1_REG_OFF,
					TDM_RX_SYN_EN_MASK, 1 << TDM_RX_SYN_EN);
}

void csic_tdm_rx_sync_disable(unsigned int sel, unsigned int ch)
{
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_RX0_OFFSET + ch*AMONG_RX_OFFSET + TDM_RX_CFG1_REG_OFF,
					TDM_RX_SYN_EN_MASK, 0 << TDM_RX_SYN_EN);
}

void csic_tdm_rx_set_line_num_ddr(unsigned int sel, unsigned int ch, unsigned int en)
{
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_RX0_OFFSET + ch*AMONG_RX_OFFSET + TDM_RX_CFG1_REG_OFF,
					TDM_LINE_NUM_DDR_EN_MASK, en << TDM_LINE_NUM_DDR_EN);
}

void csic_tdm_rx_set_buf_num(unsigned int sel, unsigned int ch, unsigned int num)
{
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_RX0_OFFSET + ch*AMONG_RX_OFFSET + TDM_RX_CFG1_REG_OFF,
					TDM_RX_BUF_NUM_MASK, num << TDM_RX_BUF_NUM);
}

void csic_tdm_rx_normal_en(unsigned int sel, unsigned int ch, unsigned int en)
{
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_RX0_OFFSET + ch*AMONG_RX_OFFSET + TDM_RX_CFG1_REG_OFF,
					TDM_RX_NORMAL_EN_MASK, en << TDM_RX_NORMAL_EN);
}

void csic_tdm_rx_awnn_en(unsigned int sel, unsigned int ch, unsigned int en)
{
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_RX0_OFFSET + ch*AMONG_RX_OFFSET + TDM_RX_CFG1_REG_OFF,
					TDM_RX_AWNN_EN_MASK, en << TDM_RX_AWNN_EN);
}

void csic_tdm_rx_start_mode(unsigned int sel, unsigned int ch, enum tdm_tx_start_mode mode)
{
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_RX0_OFFSET + ch*AMONG_RX_OFFSET + TDM_RX_CFG1_REG_OFF,
					TDM_RX_START_MODE_MASK, mode << TDM_RX_START_MODE);
}

void csic_tdm_rx_ch0_en(unsigned int sel, unsigned int ch, unsigned int en)
{
}

void csic_tdm_rx_set_min_ddr_size(unsigned int sel, unsigned int ch, enum min_ddr_size_sel ddr_size)
{
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_RX0_OFFSET + ch*AMONG_RX_OFFSET + TDM_RX_CFG1_REG_OFF,
					TDM_RX_MIN_DDR_SIZE_MASK, ddr_size << TDM_RX_MIN_DDR_SIZE);
}

void csic_tdm_rx_act_cnt_clear(unsigned int sel, unsigned int ch, unsigned int en)
{
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_RX0_OFFSET + ch*AMONG_RX_OFFSET + TDM_RX_CFG1_REG_OFF,
					TDM_ACT_CNT_CLEAR_MASK, en << TDM_ACT_CNT_CLEAR);
}

void csic_tdm_rx_to_tx_mode(unsigned int sel, unsigned int ch, enum tdm_rx_to_tx_mode mode)
{
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_RX0_OFFSET + ch*AMONG_RX_OFFSET + TDM_RX_CFG1_REG_OFF,
					TDM_RX_TO_TX_MODE_MASK, mode << TDM_RX_TO_TX_MODE);
}

void csic_tdm_rx_sw_finish_flag(unsigned int sel, unsigned int ch, unsigned int flag)
{
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_RX0_OFFSET + ch*AMONG_RX_OFFSET + TDM_RX_CFG1_REG_OFF,
					TDM_RX_TO_TX_MODE_MASK, flag << TDM_RX_TO_TX_MODE);
}

void csic_tdm_rx_input_fmt(unsigned int sel, unsigned int ch, enum tdm_input_fmt fmt)
{
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_RX0_OFFSET + ch*AMONG_RX_OFFSET + TDM_RX_CFG1_REG_OFF,
					TDM_RX_INPUT_FMT_MASK, fmt << TDM_RX_INPUT_FMT);
}

void csic_tdm_rx_input_bit(unsigned int sel, unsigned int ch, enum input_image_type_sel input_tpye)
{
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_RX0_OFFSET + ch*AMONG_RX_OFFSET + TDM_RX_CFG1_REG_OFF,
					TDM_INPUT_BIT_MASK, input_tpye << TDM_INPUT_BIT);
}

void csic_tdm_rx_aiisp_switch(unsigned int sel, unsigned int ch, enum aiisp_switch_dir dir)
{
	if (dir == AIISP_TO_NORISP)
		vin_reg_clr_set(csic_tdm_base[sel] + TMD_RX0_OFFSET + ch*AMONG_RX_OFFSET + TDM_RX_CFG1_REG_OFF,
				TDM_RX_NORMAL_EN_MASK | TDM_RX_AWNN_EN_MASK | TDM_RX_TO_TX_MODE_MASK, (1 << TDM_RX_NORMAL_EN) | (0 << TDM_RX_AWNN_EN) | (RX_TO_TX_ISP << TDM_RX_TO_TX_MODE));
	else if (dir == NORISP_TO_AIISP)
		vin_reg_clr_set(csic_tdm_base[sel] + TMD_RX0_OFFSET + ch*AMONG_RX_OFFSET + TDM_RX_CFG1_REG_OFF,
				TDM_RX_NORMAL_EN_MASK | TDM_RX_AWNN_EN_MASK | TDM_RX_TO_TX_MODE_MASK, (0 << TDM_RX_NORMAL_EN) | (1 << TDM_RX_AWNN_EN) | (RX_TO_TX_AI_ISP << TDM_RX_TO_TX_MODE));
}

void csic_tdm_rx_input_size(unsigned int sel, unsigned int ch, unsigned int width, unsigned int height)
{
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_RX0_OFFSET + ch*AMONG_RX_OFFSET + TDM_RX_CFG2_REG_OFF,
					TDM_RX_WIDTH_MASK, width << TDM_RX_WIDTH);
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_RX0_OFFSET + ch*AMONG_RX_OFFSET + TDM_RX_CFG2_REG_OFF,
					TDM_RX_HEIGHT_MASK, height << TDM_RX_HEIGHT);
}

void csic_tdm_rx_blc_en(unsigned int sel, unsigned int ch, unsigned int en)
{
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_RX0_OFFSET + ch*AMONG_RX_OFFSET + TDM_RX_CFG3_REG_OFF,
					TDM_RX_EN_MASK, en << TDM_RX_BLC_EN);
}
void csic_tdm_rx_inv_blc_en(unsigned int sel, unsigned int ch, unsigned int en)
{
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_RX0_OFFSET + ch*AMONG_RX_OFFSET + TDM_RX_CFG3_REG_OFF,
					TDM_RX_INV_BLC_EN_MASK, en << TDM_RX_INV_BLC_EN);
}
void csic_tdm_rx_normalize_en(unsigned int sel, unsigned int ch, unsigned int en)
{
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_RX0_OFFSET + ch*AMONG_RX_OFFSET + TDM_RX_CFG3_REG_OFF,
					TDM_RX_NORM_EN_MASK, en << TDM_RX_NORM_EN);
}
void csic_tdm_rx_inv_normalize_en(unsigned int sel, unsigned int ch, unsigned int en)
{
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_RX0_OFFSET + ch*AMONG_RX_OFFSET + TDM_RX_CFG3_REG_OFF,
					TDM_RX_INV_NORM_EN_MASK, en << TDM_RX_INV_NORM_EN);
}
void csic_tdm_rx_gamma_en(unsigned int sel, unsigned int ch, unsigned int en)
{
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_RX0_OFFSET + ch*AMONG_RX_OFFSET + TDM_RX_CFG3_REG_OFF,
					TDM_RX_GM_EN_MASK, en << TDM_RX_GM_EN);
}
void csic_tdm_rx_inv_gamma_en(unsigned int sel, unsigned int ch, unsigned int en)
{
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_RX0_OFFSET + ch*AMONG_RX_OFFSET + TDM_RX_CFG3_REG_OFF,
					TDM_RX_INV_GM_EN_MASK, en << TDM_RX_INV_GM_EN);
}
void csic_tdm_tx_blc_en(unsigned int sel, unsigned int ch, unsigned int en)
{
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_RX0_OFFSET + ch*AMONG_RX_OFFSET + TDM_RX_CFG3_REG_OFF,
					TDM_RX_TX_BLC_EN_MASK, en << TDM_RX_TX_BLC_EN);
}
void csic_tdm_tx_inv_blc_en(unsigned int sel, unsigned int ch, unsigned int en)
{
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_RX0_OFFSET + ch*AMONG_RX_OFFSET + TDM_RX_CFG3_REG_OFF,
					TDM_RX_TX_INV_BLC_EN_MASK, en << TDM_RX_TX_INV_BLC_EN);
}
void csic_tdm_tx_normalize_en(unsigned int sel, unsigned int ch, unsigned int en)
{
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_RX0_OFFSET + ch*AMONG_RX_OFFSET + TDM_RX_CFG3_REG_OFF,
					TDM_RX_TX_NORM_EN_MASK, en << TDM_RX_TX_NORM_EN);
}
void csic_tdm_tx_inv_normalize_en(unsigned int sel, unsigned int ch, unsigned int en)
{
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_RX0_OFFSET + ch*AMONG_RX_OFFSET + TDM_RX_CFG3_REG_OFF,
					TDM_RX_TX_INV_NORM_EN_MASK, en << TDM_RX_TX_INV_NORM_EN);
}
void csic_tdm_rx_aiisp_cfg0(unsigned int sel, unsigned int ch, struct rx_aiisp_cfg0_t *rx_aiisp_cfg0)
{
	csic_tdm_rx_blc_en(sel, ch, rx_aiisp_cfg0->rx_blc_en);
	csic_tdm_rx_inv_blc_en(sel, ch, rx_aiisp_cfg0->rx_inv_blc_en);
	csic_tdm_rx_normalize_en(sel, ch, rx_aiisp_cfg0->rx_normal_en);
	csic_tdm_rx_inv_normalize_en(sel, ch, rx_aiisp_cfg0->rx_inv_normal_en);
	csic_tdm_rx_gamma_en(sel, ch, rx_aiisp_cfg0->rx_gamma_en);
	csic_tdm_rx_inv_gamma_en(sel, ch, rx_aiisp_cfg0->rx_inv_gamma_en);

	csic_tdm_tx_blc_en(sel, ch, rx_aiisp_cfg0->rx_tx_blc_en);
	csic_tdm_tx_inv_blc_en(sel, ch, rx_aiisp_cfg0->rx_tx_inv_blc_en);
	csic_tdm_tx_normalize_en(sel, ch, rx_aiisp_cfg0->rx_tx_normal_en);
	csic_tdm_tx_inv_normalize_en(sel, ch, rx_aiisp_cfg0->rx_tx_inv_normal_en);
}

void csic_tdm_set_line_int_num(unsigned int sel, unsigned int ch, unsigned int line_num)
{
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_RX0_OFFSET + ch*AMONG_RX_OFFSET + TDM_RX_CFG4_REG_OFF,
			TDM_RX_LINE_INT_NUM_MASK, line_num << TDM_RX_LINE_INT_NUM);
}

void csic_tdm_rx_hwc_fifo_set_address(unsigned int sel, unsigned int ch, unsigned int address_offset)
{
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_RX0_OFFSET + ch*AMONG_RX_OFFSET + TDM_RX_HWC_FIFO_ADDR_REG_OFF,
					TDM_RX_LINE_INT_NUM_MASK, address_offset << TDM_RX_LINE_INT_NUM);
}

void csic_tdm_rx_data_fifo_depth(unsigned int sel, unsigned int ch, unsigned int depth)
{
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_RX0_OFFSET + ch*AMONG_RX_OFFSET + TDM_RX_FIFO_DEPTH_REG_OFF,
				TDM_RX_DATA_FIFO_DEPTH_MASK, depth << TDM_RX_DATA_FIFO_DEPTH);
}

void csic_tdm_rx_head_fifo_depth(unsigned int sel, unsigned int ch, unsigned int depth)
{
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_RX0_OFFSET + ch*AMONG_RX_OFFSET + TDM_RX_FIFO_DEPTH_REG_OFF,
			TDM_RX_HEAD_FIFO_DEPTH_MASK, depth << TDM_RX_HEAD_FIFO_DEPTH);
}

void csic_tdm_rx_data_fifo_clear(unsigned int sel)
{
	int i = 0;
	for (i = 0; i < 4; i++) {
		vin_reg_clr_set(csic_tdm_base[sel] + TMD_RX0_OFFSET + i*AMONG_RX_OFFSET + TDM_RX_FIFO_DEPTH_REG_OFF,
			TDM_RX_HEAD_FIFO_DEPTH_MASK, 0 << TDM_RX_HEAD_FIFO_DEPTH);
		vin_reg_clr_set(csic_tdm_base[sel] + TMD_RX0_OFFSET + i*AMONG_RX_OFFSET + TDM_RX_FIFO_DEPTH_REG_OFF,
				TDM_RX_DATA_FIFO_DEPTH_MASK, 0 << TDM_RX_DATA_FIFO_DEPTH);
	}
}

void csic_tdm_rx_pkg_line_words(unsigned int sel, unsigned int ch, unsigned int words)
{
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_RX0_OFFSET + ch*AMONG_RX_OFFSET + TDM_RX_FIFO_DEPTH_REG_OFF,
					TDM_RX_PKG_LINE_WORDS_MASK, words << TDM_RX_PKG_LINE_WORDS);
}

void csic_tdm_rx_set_line_stride0(unsigned int sel, unsigned int ch, unsigned int line_stride)
{
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_RX0_OFFSET + ch*AMONG_RX_OFFSET + TDM_RX_LINE_STRIDE_REG_OFF,
					TDM_RX_LINE_STRIDE0_MASK, line_stride << TDM_RX_LINE_STRIDE0);
}

void csic_tdm_rx_set_line_stride1(unsigned int sel, unsigned int ch, unsigned int line_stride)
{
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_RX0_OFFSET + ch*AMONG_RX_OFFSET + TDM_RX_LINE_STRIDE_REG_OFF,
					TDM_RX_LINE_STRIDE1_MASK, line_stride << TDM_RX_LINE_STRIDE1);
}

void csic_tdm_rx_set_io_index(unsigned int sel, unsigned int ch, unsigned int index)
{
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_RX0_OFFSET + ch*AMONG_RX_OFFSET + TDM_RX_LNDEX_REG_OFF,
					TDM_RX_IO_INDEX_MASK, index << TDM_RX_IO_INDEX);
}

void csic_tdm_rx_set_io_tpye(unsigned int sel, unsigned int ch, enum tdm_rx_io_tpye tpye)
{
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_RX0_OFFSET + ch*AMONG_RX_OFFSET + TDM_RX_LNDEX_REG_OFF,
					TDM_RX_IO_TYPE_MASK, tpye << TDM_RX_IO_TYPE);
}

void csic_tdm_rx_set_address(unsigned int sel, unsigned int ch, vin_dma_addr_t address)
{
	vin_reg_writel(csic_tdm_base[sel] + TMD_RX0_OFFSET + ch*AMONG_RX_OFFSET + TDM_RX_INPUT_OUTPUT_REG_OFF,
					address >> TDM_ADDR_BIT_R_SHIFT);
}

unsigned int csic_tdm_rx_get_io_val(unsigned int sel, unsigned int ch)
{
	return vin_reg_readl(csic_tdm_base[sel] + TMD_RX0_OFFSET + ch*AMONG_RX_OFFSET + TDM_RX_INPUT_OUTPUT_REG_OFF);
}

void csic_tdm_rx_set_address_offset1(unsigned int sel, unsigned int ch, unsigned int address)
{
	vin_reg_writel(csic_tdm_base[sel] + TMD_RX0_OFFSET + ch*AMONG_RX_OFFSET + TDM_RX_ADDR_OFF1_REG_OFF,
					address);
}

void csic_tdm_rx_set_max_line(unsigned int sel, unsigned int ch, unsigned int max_line)
{
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_RX0_OFFSET + ch*AMONG_RX_OFFSET + TDM_RX_MAX_SIZE_REG_OFF,
					TDM_RX_BUF_MAX_LINE_MASK, max_line << TDM_RX_BUF_MAX_LINE);
}

void csic_tdm_rx_set_line_offset(unsigned int sel, unsigned int ch, unsigned int buf_ov_rx_offset, unsigned int buf_ov_tx_offset)
{
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_RX0_OFFSET + ch*AMONG_RX_OFFSET + TDM_RX_LINE_OFFSET_REG_OFF,
					TDM_RX_BUF_OV_RX_OFFSET_MASK, buf_ov_rx_offset << TDM_RX_BUF_OV_RX_OFFSET);
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_RX0_OFFSET + ch*AMONG_RX_OFFSET + TDM_RX_LINE_OFFSET_REG_OFF,
					TDM_RX_BUF_OV_TX_OFFSET_MASK, buf_ov_tx_offset << TDM_RX_BUF_OV_TX_OFFSET);
}

void csic_tdm_rx_set_blc_offset(unsigned int sel, unsigned int ch, struct rx_blc_offset *offset)
{
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_RX0_OFFSET + ch*AMONG_RX_OFFSET + TDM_RX_BLC_OFFSET_REG_OFF,
					TDM_RX_R_OFFSET_MASK, offset->r_offset << TDM_RX_R_OFFSET);
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_RX0_OFFSET + ch*AMONG_RX_OFFSET + TDM_RX_BLC_OFFSET_REG_OFF,
					TDM_RX_G_OFFSET_MASK, offset->g_offset << TDM_RX_G_OFFSET);
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_RX0_OFFSET + ch*AMONG_RX_OFFSET + TDM_RX_BLC_OFFSET_REG_OFF,
					TDM_RX_B_OFFSET_MASK, offset->b_offset << TDM_RX_B_OFFSET);
}

void csic_tdm_rx_set_normalize_coef(unsigned int sel, unsigned int ch, struct rx_coef_offset *offset)
{
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_RX0_OFFSET + ch*AMONG_RX_OFFSET + TDM_RX_NORM_COEF_REG_OFF,
					TDM_RX_R_COEF_MASK, offset->r_coef << TDM_RX_R_COEF);
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_RX0_OFFSET + ch*AMONG_RX_OFFSET + TDM_RX_NORM_COEF_REG_OFF,
					TDM_RX_G_COEF_MASK, offset->g_coef << TDM_RX_G_COEF);
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_RX0_OFFSET + ch*AMONG_RX_OFFSET + TDM_RX_NORM_COEF_REG_OFF,
					TDM_RX_B_COEF_MASK, offset->b_coef << TDM_RX_B_COEF);
}

void csic_tdm_rx_set_inv_normalize_coef(unsigned int sel, unsigned int ch, struct rx_coef_offset *offset)
{
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_RX0_OFFSET + ch*AMONG_RX_OFFSET + TDM_RX_INV_NORM_COEF_REG_OFF,
					TDM_RX_I_R_COEF_MASK, offset->r_coef << TDM_RX_I_R_COEF);
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_RX0_OFFSET + ch*AMONG_RX_OFFSET + TDM_RX_INV_NORM_COEF_REG_OFF,
					TDM_RX_I_G_COEF_MASK, offset->g_coef << TDM_RX_I_G_COEF);
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_RX0_OFFSET + ch*AMONG_RX_OFFSET + TDM_RX_INV_NORM_COEF_REG_OFF,
					TDM_RX_I_B_COEF_MASK, offset->b_coef << TDM_RX_I_B_COEF);
}

void csic_tdm_rx_get_size(unsigned int sel, unsigned int ch, unsigned int *width, unsigned int *heigth)
{
	unsigned int regval;

	regval = vin_reg_readl(csic_tdm_base[sel] + TMD_RX0_OFFSET + ch*AMONG_RX_OFFSET + TDM_RX_FRAME_ERR_REG_OFF);
	*width = (regval & TDM_RX_ERR_WIDTH_MASK) >> TDM_RX_ERR_WIDTH;
	*heigth = (regval & TDM_RX_ERR_HEIGHT_MASK) >> TDM_RX_ERR_HEIGHT;
}

void csic_tdm_rx_get_hblank(unsigned int sel, unsigned int ch, unsigned int *hb_min, unsigned int *hb_max)
{
	unsigned int regval;

	regval = vin_reg_readl(csic_tdm_base[sel] + TMD_RX0_OFFSET + ch*AMONG_RX_OFFSET + TDM_RX_HB_SHORT_REG_OFF);
	*hb_max = (regval & TDM_RX_HB_MAX_MASK) >> TDM_RX_HB_MAX;
	*hb_min = (regval & TDM_RX_HB_MIN_MASK) >> TDM_RX_HB_MIN;
}

void csic_tdm_rx_get_layer(unsigned int sel, unsigned int ch, unsigned int *head_fifo, unsigned int *fifo)
{
	unsigned int regval;

	regval = vin_reg_readl(csic_tdm_base[sel] + TMD_RX0_OFFSET + ch*AMONG_RX_OFFSET + TDM_RX_LAYER_REG_OFF);
	*fifo = (regval & TDM_RX_FIFO_LAYER_MASK) >> TDM_RX_FIFO_LAYER;
	*head_fifo = (regval & TDM_RX_HEAD_FIFO_LAYER_MASK) >> TDM_RX_HEAD_FIFO_LAYER;
}

unsigned int csic_tdm_rx_get_frame_cnt(unsigned int sel, unsigned int ch)
{
	return vin_reg_readl(csic_tdm_base[sel] + TMD_RX0_OFFSET + ch*AMONG_RX_OFFSET + TMD_RX_FRM_CNT_REG_OFF);
}

unsigned int csic_tdm_rx_get_base_time(unsigned int sel, unsigned int ch)
{
	return vin_reg_readl(csic_tdm_base[sel] + TMD_RX0_OFFSET + ch*AMONG_RX_OFFSET + TMD_RX_TIME_BASE_REG_OFF);
}

unsigned int csic_tdm_rx_get_cycle(unsigned int sel, unsigned int ch)
{
	return (vin_reg_readl(csic_tdm_base[sel] + TMD_RX0_OFFSET + ch*AMONG_RX_OFFSET + TMD_RX_TIMEOFFSET_REG_OFF) & TDM_RX_CYCLE_MASK) >> TDM_RX_CYCLE;
}

/*
 * function about tdm lbc registers
 */
void csic_tdm_lbc_cfg(unsigned int sel, unsigned int ch, struct tdm_rx_lbc *lbc)
{
	/* CFG */
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_LBC0_OFFSET + ch*AMONG_LBC_OFFSET + TMD_LBC_CFG0_REG_OFF,
					IS_LOSSY_MASK, lbc->is_lossy << IS_LOSSY);
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_LBC0_OFFSET + ch*AMONG_LBC_OFFSET + TMD_LBC_CFG0_REG_OFF,
					RC_CTRL_MODE_MASK, lbc->rc_ctrl_mode << RC_CTRL_MODE);
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_LBC0_OFFSET + ch*AMONG_LBC_OFFSET + TMD_LBC_CFG0_REG_OFF,
					STATUS_QP_MASK, lbc->start_qp << STATUS_QP);
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_LBC0_OFFSET + ch*AMONG_LBC_OFFSET + TMD_LBC_CFG0_REG_OFF,
					STD_QP_MASK, lbc->std_qp << STD_QP);
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_LBC0_OFFSET + ch*AMONG_LBC_OFFSET + TMD_LBC_CFG0_REG_OFF,
					GLB_MAX_QUO_MASK, lbc->glb_max_quo << GLB_MAX_QUO);
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_LBC0_OFFSET + ch*AMONG_LBC_OFFSET + TMD_LBC_CFG0_REG_OFF,
					GLB_MAX_K_MASK, lbc->glb_max_k << GLB_MAX_K);
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_LBC0_OFFSET + ch*AMONG_LBC_OFFSET + TMD_LBC_CFG0_REG_OFF,
					PTR_BUFFER_INIT_MASK, lbc->ptr_buffer_init << PTR_BUFFER_INIT);

	vin_reg_clr_set(csic_tdm_base[sel] + TMD_LBC0_OFFSET + ch*AMONG_LBC_OFFSET + TMD_LBC_CFG1_REG_OFF,
					MB_NUM_IN_LINE_MASK, lbc->mb_num_in_line << MB_NUM_IN_LINE);

	vin_reg_clr_set(csic_tdm_base[sel] + TMD_LBC0_OFFSET + ch*AMONG_LBC_OFFSET + TMD_LBC_CFG2_REG_OFF,
					PTR_BUFFER_FULLNESS_MAX_MASK, lbc->ptr_buffer_fullness_max << PTR_BUFFER_FULLNESS_MAX);
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_LBC0_OFFSET + ch*AMONG_LBC_OFFSET + TMD_LBC_CFG2_REG_OFF,
					PTR_BUFFER_THR_MASK, lbc->ptr_buffer_thr << PTR_BUFFER_THR);

	vin_reg_clr_set(csic_tdm_base[sel] + TMD_LBC0_OFFSET + ch*AMONG_LBC_OFFSET + TMD_LBC_CFG3_REG_OFF,
					LINE_MAX_BIT_MASK, lbc->line_max_bit << LINE_MAX_BIT);
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_LBC0_OFFSET + ch*AMONG_LBC_OFFSET + TMD_LBC_CFG3_REG_OFF,
					TAR_BITS_LINE_RC_MASK, lbc->tar_bits_line_rc << TAR_BITS_LINE_RC);

	vin_reg_clr_set(csic_tdm_base[sel] + TMD_LBC0_OFFSET + ch*AMONG_LBC_OFFSET + TMD_LBC_CFG4_REG_OFF,
					LINE_ATR_BIT_MASK, lbc->line_tar_bit << LINE_ATR_BIT);
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_LBC0_OFFSET + ch*AMONG_LBC_OFFSET + TMD_LBC_CFG4_REG_OFF,
					TAR_BITS_MASK, lbc->tar_bits << TAR_BITS);

	vin_reg_clr_set(csic_tdm_base[sel] + TMD_LBC0_OFFSET + ch*AMONG_LBC_OFFSET + TMD_LBC_CFG5_REG_OFF,
					FRAME_TAR_BIT_MASK, lbc->frame_tar_bit << FRAME_TAR_BIT);

	/* LUT */
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_LBC0_OFFSET + ch*AMONG_LBC_OFFSET + TMD_LBC_MIN_QP_LUT0_REG_OFF,
					MIN_QP_S0_MASK, lbc->lbc_min_qp[0] << MIN_QP_S0);
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_LBC0_OFFSET + ch*AMONG_LBC_OFFSET + TMD_LBC_MIN_QP_LUT0_REG_OFF,
					MIN_QP_S1_MASK, lbc->lbc_min_qp[1] << MIN_QP_S1);
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_LBC0_OFFSET + ch*AMONG_LBC_OFFSET + TMD_LBC_MIN_QP_LUT0_REG_OFF,
					MIN_QP_S2_MASK, lbc->lbc_min_qp[2] << MIN_QP_S2);
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_LBC0_OFFSET + ch*AMONG_LBC_OFFSET + TMD_LBC_MIN_QP_LUT0_REG_OFF,
					MIN_QP_S3_MASK, lbc->lbc_min_qp[3] << MIN_QP_S3);
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_LBC0_OFFSET + ch*AMONG_LBC_OFFSET + TMD_LBC_MIN_QP_LUT0_REG_OFF,
					MIN_QP_S4_MASK, lbc->lbc_min_qp[4] << MIN_QP_S4);
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_LBC0_OFFSET + ch*AMONG_LBC_OFFSET + TMD_LBC_MIN_QP_LUT0_REG_OFF,
					MIN_QP_S5_MASK, lbc->lbc_min_qp[5] << MIN_QP_S5);
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_LBC0_OFFSET + ch*AMONG_LBC_OFFSET + TMD_LBC_MIN_QP_LUT0_REG_OFF,
					MIN_QP_S6_MASK, lbc->lbc_min_qp[6] << MIN_QP_S6);
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_LBC0_OFFSET + ch*AMONG_LBC_OFFSET + TMD_LBC_MIN_QP_LUT0_REG_OFF,
					MIN_QP_S7_MASK, lbc->lbc_min_qp[7] << MIN_QP_S7);

	vin_reg_clr_set(csic_tdm_base[sel] + TMD_LBC0_OFFSET + ch*AMONG_LBC_OFFSET + TMD_LBC_MIN_QP_LUT1_REG_OFF,
					MIN_QP_S0_MASK, lbc->lbc_min_qp[8] << MIN_QP_S0);
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_LBC0_OFFSET + ch*AMONG_LBC_OFFSET + TMD_LBC_MIN_QP_LUT1_REG_OFF,
					MIN_QP_S1_MASK, lbc->lbc_min_qp[9] << MIN_QP_S1);
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_LBC0_OFFSET + ch*AMONG_LBC_OFFSET + TMD_LBC_MIN_QP_LUT1_REG_OFF,
					MIN_QP_S2_MASK, lbc->lbc_min_qp[10] << MIN_QP_S2);
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_LBC0_OFFSET + ch*AMONG_LBC_OFFSET + TMD_LBC_MIN_QP_LUT1_REG_OFF,
					MIN_QP_S3_MASK, lbc->lbc_min_qp[11] << MIN_QP_S3);
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_LBC0_OFFSET + ch*AMONG_LBC_OFFSET + TMD_LBC_MIN_QP_LUT1_REG_OFF,
					MIN_QP_S4_MASK, lbc->lbc_min_qp[12] << MIN_QP_S4);
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_LBC0_OFFSET + ch*AMONG_LBC_OFFSET + TMD_LBC_MIN_QP_LUT1_REG_OFF,
					MIN_QP_S5_MASK, lbc->lbc_min_qp[13] << MIN_QP_S5);
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_LBC0_OFFSET + ch*AMONG_LBC_OFFSET + TMD_LBC_MIN_QP_LUT1_REG_OFF,
					MIN_QP_S6_MASK, lbc->lbc_min_qp[14] << MIN_QP_S6);
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_LBC0_OFFSET + ch*AMONG_LBC_OFFSET + TMD_LBC_MIN_QP_LUT1_REG_OFF,
					MIN_QP_S7_MASK, lbc->lbc_min_qp[15] << MIN_QP_S7);

	vin_reg_clr_set(csic_tdm_base[sel] + TMD_LBC0_OFFSET + ch*AMONG_LBC_OFFSET + TMD_LBC_MAX_QP_LUT0_REG_OFF,
					MIN_QP_S0_MASK, lbc->lbc_max_qp[0] << MIN_QP_S0);
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_LBC0_OFFSET + ch*AMONG_LBC_OFFSET + TMD_LBC_MAX_QP_LUT0_REG_OFF,
					MIN_QP_S1_MASK, lbc->lbc_max_qp[1] << MIN_QP_S1);
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_LBC0_OFFSET + ch*AMONG_LBC_OFFSET + TMD_LBC_MAX_QP_LUT0_REG_OFF,
					MIN_QP_S2_MASK, lbc->lbc_max_qp[2] << MIN_QP_S2);
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_LBC0_OFFSET + ch*AMONG_LBC_OFFSET + TMD_LBC_MAX_QP_LUT0_REG_OFF,
					MIN_QP_S3_MASK, lbc->lbc_max_qp[3] << MIN_QP_S3);
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_LBC0_OFFSET + ch*AMONG_LBC_OFFSET + TMD_LBC_MAX_QP_LUT0_REG_OFF,
					MIN_QP_S4_MASK, lbc->lbc_max_qp[4] << MIN_QP_S4);
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_LBC0_OFFSET + ch*AMONG_LBC_OFFSET + TMD_LBC_MAX_QP_LUT0_REG_OFF,
					MIN_QP_S5_MASK, lbc->lbc_max_qp[5] << MIN_QP_S5);
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_LBC0_OFFSET + ch*AMONG_LBC_OFFSET + TMD_LBC_MAX_QP_LUT0_REG_OFF,
					MIN_QP_S6_MASK, lbc->lbc_max_qp[6] << MIN_QP_S6);
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_LBC0_OFFSET + ch*AMONG_LBC_OFFSET + TMD_LBC_MAX_QP_LUT0_REG_OFF,
					MIN_QP_S7_MASK, lbc->lbc_max_qp[7] << MIN_QP_S7);

	vin_reg_clr_set(csic_tdm_base[sel] + TMD_LBC0_OFFSET + ch*AMONG_LBC_OFFSET + TMD_LBC_MAX_QP_LUT1_REG_OFF,
					MIN_QP_S0_MASK, lbc->lbc_max_qp[8] << MIN_QP_S0);
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_LBC0_OFFSET + ch*AMONG_LBC_OFFSET + TMD_LBC_MAX_QP_LUT1_REG_OFF,
					MIN_QP_S1_MASK, lbc->lbc_max_qp[9] << MIN_QP_S1);
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_LBC0_OFFSET + ch*AMONG_LBC_OFFSET + TMD_LBC_MAX_QP_LUT1_REG_OFF,
					MIN_QP_S2_MASK, lbc->lbc_max_qp[10] << MIN_QP_S2);
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_LBC0_OFFSET + ch*AMONG_LBC_OFFSET + TMD_LBC_MAX_QP_LUT1_REG_OFF,
					MIN_QP_S3_MASK, lbc->lbc_max_qp[11] << MIN_QP_S3);
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_LBC0_OFFSET + ch*AMONG_LBC_OFFSET + TMD_LBC_MAX_QP_LUT1_REG_OFF,
					MIN_QP_S4_MASK, lbc->lbc_max_qp[12] << MIN_QP_S4);
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_LBC0_OFFSET + ch*AMONG_LBC_OFFSET + TMD_LBC_MAX_QP_LUT1_REG_OFF,
					MIN_QP_S5_MASK, lbc->lbc_max_qp[13] << MIN_QP_S5);
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_LBC0_OFFSET + ch*AMONG_LBC_OFFSET + TMD_LBC_MAX_QP_LUT1_REG_OFF,
					MIN_QP_S6_MASK, lbc->lbc_max_qp[14] << MIN_QP_S6);
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_LBC0_OFFSET + ch*AMONG_LBC_OFFSET + TMD_LBC_MAX_QP_LUT1_REG_OFF,
					MIN_QP_S7_MASK, lbc->lbc_max_qp[15] << MIN_QP_S7);

	vin_reg_clr_set(csic_tdm_base[sel] + TMD_LBC0_OFFSET + ch*AMONG_LBC_OFFSET + TMD_LBC_THRESH_LUT0_REG_OFF,
					TH_S0_MASK, lbc->lbc_thresh[0] << TH_S0);
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_LBC0_OFFSET + ch*AMONG_LBC_OFFSET + TMD_LBC_THRESH_LUT0_REG_OFF,
					TH_S1_MASK, lbc->lbc_thresh[1] << TH_S1);

	vin_reg_clr_set(csic_tdm_base[sel] + TMD_LBC0_OFFSET + ch*AMONG_LBC_OFFSET + TMD_LBC_THRESH_LUT1_REG_OFF,
					TH_S0_MASK, lbc->lbc_thresh[2] << TH_S0);
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_LBC0_OFFSET + ch*AMONG_LBC_OFFSET + TMD_LBC_THRESH_LUT1_REG_OFF,
					TH_S1_MASK, lbc->lbc_thresh[3] << TH_S1);

	vin_reg_clr_set(csic_tdm_base[sel] + TMD_LBC0_OFFSET + ch*AMONG_LBC_OFFSET + TMD_LBC_THRESH_LUT2_REG_OFF,
					TH_S0_MASK, lbc->lbc_thresh[4] << TH_S0);
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_LBC0_OFFSET + ch*AMONG_LBC_OFFSET + TMD_LBC_THRESH_LUT2_REG_OFF,
					TH_S1_MASK, lbc->lbc_thresh[5] << TH_S1);

	vin_reg_clr_set(csic_tdm_base[sel] + TMD_LBC0_OFFSET + ch*AMONG_LBC_OFFSET + TMD_LBC_THRESH_LUT3_REG_OFF,
					TH_S0_MASK, lbc->lbc_thresh[6] << TH_S0);
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_LBC0_OFFSET + ch*AMONG_LBC_OFFSET + TMD_LBC_THRESH_LUT3_REG_OFF,
					TH_S1_MASK, lbc->lbc_thresh[7] << TH_S1);

	vin_reg_clr_set(csic_tdm_base[sel] + TMD_LBC0_OFFSET + ch*AMONG_LBC_OFFSET + TMD_LBC_THRESH_LUT4_REG_OFF,
					TH_S0_MASK, lbc->lbc_thresh[8] << TH_S0);
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_LBC0_OFFSET + ch*AMONG_LBC_OFFSET + TMD_LBC_THRESH_LUT4_REG_OFF,
					TH_S1_MASK, lbc->lbc_thresh[9] << TH_S1);

	vin_reg_clr_set(csic_tdm_base[sel] + TMD_LBC0_OFFSET + ch*AMONG_LBC_OFFSET + TMD_LBC_THRESH_LUT5_REG_OFF,
					TH_S0_MASK, lbc->lbc_thresh[10] << TH_S0);
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_LBC0_OFFSET + ch*AMONG_LBC_OFFSET + TMD_LBC_THRESH_LUT5_REG_OFF,
					TH_S1_MASK, lbc->lbc_thresh[11] << TH_S1);

	vin_reg_clr_set(csic_tdm_base[sel] + TMD_LBC0_OFFSET + ch*AMONG_LBC_OFFSET + TMD_LBC_THRESH_LUT6_REG_OFF,
					TH_S0_MASK, lbc->lbc_thresh[12] << TH_S0);
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_LBC0_OFFSET + ch*AMONG_LBC_OFFSET + TMD_LBC_THRESH_LUT6_REG_OFF,
					TH_S1_MASK, lbc->lbc_thresh[13] << TH_S1);

	vin_reg_clr_set(csic_tdm_base[sel] + TMD_LBC0_OFFSET + ch*AMONG_LBC_OFFSET + TMD_LBC_THRESH_LUT7_REG_OFF,
					TH_S0_MASK, lbc->lbc_thresh[14] << TH_S0);
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_LBC0_OFFSET + ch*AMONG_LBC_OFFSET + TMD_LBC_THRESH_LUT7_REG_OFF,
					TH_S1_MASK, lbc->lbc_thresh[15] << TH_S1);

	vin_reg_clr_set(csic_tdm_base[sel] + TMD_LBC0_OFFSET + ch*AMONG_LBC_OFFSET + TMD_LBC_TAR_BITS_ADJ_LUT0_REG_OFF,
					TAR_BITS_ADJ_S0_MASK, lbc->lbc_tar_bits_adj[0] << TAR_BITS_ADJ_S0);
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_LBC0_OFFSET + ch*AMONG_LBC_OFFSET + TMD_LBC_TAR_BITS_ADJ_LUT0_REG_OFF,
					TAR_BITS_ADJ_S1_MASK, lbc->lbc_tar_bits_adj[1] << TAR_BITS_ADJ_S1);

	vin_reg_clr_set(csic_tdm_base[sel] + TMD_LBC0_OFFSET + ch*AMONG_LBC_OFFSET + TMD_LBC_TAR_BITS_ADJ_LUT1_REG_OFF,
					TAR_BITS_ADJ_S0_MASK, lbc->lbc_tar_bits_adj[2] << TAR_BITS_ADJ_S0);
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_LBC0_OFFSET + ch*AMONG_LBC_OFFSET + TMD_LBC_TAR_BITS_ADJ_LUT1_REG_OFF,
					TAR_BITS_ADJ_S1_MASK, lbc->lbc_tar_bits_adj[3] << TAR_BITS_ADJ_S1);

	vin_reg_clr_set(csic_tdm_base[sel] + TMD_LBC0_OFFSET + ch*AMONG_LBC_OFFSET + TMD_LBC_TAR_BITS_ADJ_LUT2_REG_OFF,
					TAR_BITS_ADJ_S0_MASK, lbc->lbc_tar_bits_adj[4] << TAR_BITS_ADJ_S0);
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_LBC0_OFFSET + ch*AMONG_LBC_OFFSET + TMD_LBC_TAR_BITS_ADJ_LUT2_REG_OFF,
					TAR_BITS_ADJ_S1_MASK, lbc->lbc_tar_bits_adj[5] << TAR_BITS_ADJ_S1);

	vin_reg_clr_set(csic_tdm_base[sel] + TMD_LBC0_OFFSET + ch*AMONG_LBC_OFFSET + TMD_LBC_TAR_BITS_ADJ_LUT3_REG_OFF,
					TAR_BITS_ADJ_S0_MASK, lbc->lbc_tar_bits_adj[6] << TAR_BITS_ADJ_S0);
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_LBC0_OFFSET + ch*AMONG_LBC_OFFSET + TMD_LBC_TAR_BITS_ADJ_LUT3_REG_OFF,
					TAR_BITS_ADJ_S1_MASK, lbc->lbc_tar_bits_adj[7] << TAR_BITS_ADJ_S1);

	vin_reg_clr_set(csic_tdm_base[sel] + TMD_LBC0_OFFSET + ch*AMONG_LBC_OFFSET + TMD_LBC_TAR_BITS_ADJ_LUT4_REG_OFF,
					TAR_BITS_ADJ_S0_MASK, lbc->lbc_tar_bits_adj[8] << TAR_BITS_ADJ_S0);
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_LBC0_OFFSET + ch*AMONG_LBC_OFFSET + TMD_LBC_TAR_BITS_ADJ_LUT4_REG_OFF,
					TAR_BITS_ADJ_S1_MASK, lbc->lbc_tar_bits_adj[9] << TAR_BITS_ADJ_S1);

	vin_reg_clr_set(csic_tdm_base[sel] + TMD_LBC0_OFFSET + ch*AMONG_LBC_OFFSET + TMD_LBC_TAR_BITS_ADJ_LUT5_REG_OFF,
					TAR_BITS_ADJ_S0_MASK, lbc->lbc_tar_bits_adj[10] << TAR_BITS_ADJ_S0);
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_LBC0_OFFSET + ch*AMONG_LBC_OFFSET + TMD_LBC_TAR_BITS_ADJ_LUT5_REG_OFF,
					TAR_BITS_ADJ_S1_MASK, lbc->lbc_tar_bits_adj[11] << TAR_BITS_ADJ_S1);

	vin_reg_clr_set(csic_tdm_base[sel] + TMD_LBC0_OFFSET + ch*AMONG_LBC_OFFSET + TMD_LBC_TAR_BITS_ADJ_LUT6_REG_OFF,
					TAR_BITS_ADJ_S0_MASK, lbc->lbc_tar_bits_adj[12] << TAR_BITS_ADJ_S0);
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_LBC0_OFFSET + ch*AMONG_LBC_OFFSET + TMD_LBC_TAR_BITS_ADJ_LUT6_REG_OFF,
					TAR_BITS_ADJ_S1_MASK, lbc->lbc_tar_bits_adj[13] << TAR_BITS_ADJ_S1);

	vin_reg_clr_set(csic_tdm_base[sel] + TMD_LBC0_OFFSET + ch*AMONG_LBC_OFFSET + TMD_LBC_TAR_BITS_ADJ_LUT7_REG_OFF,
					TAR_BITS_ADJ_S0_MASK, lbc->lbc_tar_bits_adj[14] << TAR_BITS_ADJ_S0);
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_LBC0_OFFSET + ch*AMONG_LBC_OFFSET + TMD_LBC_TAR_BITS_ADJ_LUT7_REG_OFF,
					TAR_BITS_ADJ_S1_MASK, lbc->lbc_tar_bits_adj[15] << TAR_BITS_ADJ_S1);

	vin_reg_clr_set(csic_tdm_base[sel] + TMD_LBC0_OFFSET + ch*AMONG_LBC_OFFSET + TMD_LBC_PREP_BITS_ADJ_LUT_REG_OFF,
					PRE_BITS_ADJ_S0_MASK, lbc->lbc_pre_bits_adj[0] << PRE_BITS_ADJ_S0);
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_LBC0_OFFSET + ch*AMONG_LBC_OFFSET + TMD_LBC_PREP_BITS_ADJ_LUT_REG_OFF,
					PRE_BITS_ADJ_S1_MASK, lbc->lbc_pre_bits_adj[1] << PRE_BITS_ADJ_S1);
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_LBC0_OFFSET + ch*AMONG_LBC_OFFSET + TMD_LBC_PREP_BITS_ADJ_LUT_REG_OFF,
					PRE_BITS_ADJ_S2_MASK, lbc->lbc_pre_bits_adj[2] << PRE_BITS_ADJ_S2);
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_LBC0_OFFSET + ch*AMONG_LBC_OFFSET + TMD_LBC_PREP_BITS_ADJ_LUT_REG_OFF,
					PRE_BITS_ADJ_S3_MASK, lbc->lbc_pre_bits_adj[3] << PRE_BITS_ADJ_S3);
	vin_reg_clr_set(csic_tdm_base[sel] + TMD_LBC0_OFFSET + ch*AMONG_LBC_OFFSET + TMD_LBC_PREP_BITS_ADJ_LUT_REG_OFF,
					PRE_BITS_ADJ_S4_MASK, lbc->lbc_pre_bits_adj[4] << PRE_BITS_ADJ_S4);
}
