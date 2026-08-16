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

#ifndef __CSIC__TDM230__REG__I__H__
#define __CSIC__TDM230__REG__I__H__

/*
 * Detail information of registers
 */
/*
 * tdm top registers
 */
#define TDM_GOLBAL_CFG0_REG_OFF			0X000
#define TDM_TOP_EN				0
#define TDM_TOP_EN_MASK				(0X1 << TDM_TOP_EN)
#define TDM_RX2_RX3_MUX_LBC_RST			2
#define TDM_RX2_RX3_MUX_LBC_RST_MASK		(0X1 << TDM_RX2_RX3_MUX_LBC_RST)
#define VGM_EN					3
#define VGM_EN_MASK				(0X1 << VGM_EN)
#define TDM_SPEED_DN_EN				4
#define TDM_SPEED_DN_EN_MASK			(0X1 << TDM_SPEED_DN_EN)
#define TDM_LBC_ALIGN_CHOOSE			8
#define TDM_LBC_ALIGN_CHOOSE_MASK		(0X1 << TDM_LBC_ALIGN_CHOOSE)
#define RX_CHN_CFG_MODE				12
#define RX_CHN_CFG_MODE_MASK			(0X3 << RX_CHN_CFG_MODE)
#define RX_WORK_MODE				14
#define RX_WORK_MODE_MASK			(0X1 << RX_WORK_MODE)
#define TDM_OKG_BACK_DOOR			15
#define TDM_OKG_BACK_DOOR_MASK			(0X1 << TDM_OKG_BACK_DOOR)
#define TX_CHN_CFG_MODE				16
#define TX_CHN_CFG_MODE_MASK			(0X7 << TX_CHN_CFG_MODE)
#define BUF_MODE_BACK_DOOR0			19
#define BUF_MODE_BACK_DOOR0_MASK		(0X1 << BUF_MODE_BACK_DOOR0)
#define BUF_MODE_BACK_DOOR1			20
#define BUF_MODE_BACK_DOOR1_MASK		(0X1 << BUF_MODE_BACK_DOOR1)
#define TDM_VER_EN       			24
#define TDM_VER_EN_MASK		                (0X1 << TDM_VER_EN)
#define TDM_EMBED_EN       			25
#define TDM_EMBED_EN_MASK                       (0X1 << TDM_EMBED_EN)
#define TDM_TIMER_EN       			26
#define TDM_TIMER_EN_MASK                       (0X1 << TDM_TIMER_EN)
#define RX_FIFO_FRESH_EN                        28
#define RX_FIFO_FRESH_EN_MASK                   (0X1 << RX_FIFO_FRESH_EN)
#define TX_FIFO_FRESH_EN                        29
#define TX_FIFO_FRESH_EN_MASK                   (0X1 << TX_FIFO_FRESH_EN)
#define RX_DATA_LINE_FRESH_EN			30
#define RX_DATA_LINE_FRESH_EN_MASK		(0X1 << RX_DATA_LINE_FRESH_EN)
#define TX_DATA_LINE_FRESH_EN			31
#define TX_DATA_LINE_FRESH_EN_MASK		(0X1 << TX_DATA_LINE_FRESH_EN)

#define TDM_INT_BYPASS0_REG_OFF			0X010

#define TDM_INT_STATUS0_REG_OFF			0X018
#define RX_FRM_LOST_PD				0
#define RX_FRM_LOST_PD_MASK			(0X1 << RX_FRM_LOST_PD)
#define RX_FRM_ERR_PD				1
#define RX_FRM_ERR_PD_MASK			(0X1 << RX_FRM_ERR_PD)
#define RX_BTYPE_ERR_PD				2
#define RX_BTYPE_ERR_PD_MASK			(0X1 << RX_BTYPE_ERR_PD)
#define RX_BUF_FULL_PD				3
#define RX_BUF_FULL_PD_MASK			(0X1 << RX_BUF_FULL_PD)
#define SPEED_DN_HSYN_PD			4
#define SPEED_DN_HSYN_PD_MASK			(0X1 << SPEED_DN_HSYN_PD)
#define RX_HB_SHORT_PD				5
#define RX_HB_SHORT_PD_MASK			(0X1 << RX_HB_SHORT_PD)
#define RX_FIFO_FULL_PD				6
#define RX_FIFO_FULL_PD_MASK			(0X1 << RX_FIFO_FULL_PD)
#define TDM_LBD_ERROR_PD			7
#define TDM_LBD_ERROR_PD_MASK			(0X1 << TDM_LBD_ERROR_PD)
#define TDM_LBC_ERROR_PD			8
#define TDM_LBC_ERROR_PD_MASK			(0X1 << TDM_LBC_ERROR_PD)
#define TX_FIFO_UNDER_PD			9
#define TX_FIFO_UNDER_PD_MASK			(0X1 << TX_FIFO_UNDER_PD)
#define TX_FRM_DONE_PD				10
#define TX_FRM_DONE_PD_MASK			(0X1 << TX_FRM_DONE_PD)
#define SPEED_DN_FIFO_FULL_PD			11
#define SPEED_DN_FIFO_FULL_PD_MASK		(0X1 << SPEED_DN_FIFO_FULL_PD)
#define RX0_FRM_START_PD			12
#define RX0_FRM_START_PD_MASK                   (0X1 << RX0_FRM_START_PD)
#define RX1_FRM_START_PD			13
#define RX1_FRM_START_PD_MASK                   (0X1 << RX1_FRM_START_PD)
#define RX2_FRM_START_PD			14
#define RX2_FRM_START_PD_MASK                   (0X1 << RX2_FRM_START_PD)
#define RX3_FRM_START_PD			15
#define RX3_FRM_START_PD_MASK                   (0X1 << RX3_FRM_START_PD)
#define RX0_FRM_DONE_PD				16
#define RX0_FRM_DONE_PD_MASK			(0X1 << RX0_FRM_DONE_PD)
#define RX1_FRM_DONE_PD				17
#define RX1_FRM_DONE_PD_MASK			(0X1 << RX1_FRM_DONE_PD)
#define RX2_FRM_DONE_PD				18
#define RX2_FRM_DONE_PD_MASK			(0X1 << RX2_FRM_DONE_PD)
#define RX3_FRM_DONE_PD				19
#define RX3_FRM_DONE_PD_MASK			(0X1 << RX3_FRM_DONE_PD)
#define RX0_N_LINE_START_PD			20
#define RX0_N_LINE_START_PD_MASK		(0X1 << RX0_N_LINE_START_PD)
#define RX1_N_LINE_START_PD			21
#define RX1_N_LINE_START_PD_MASK		(0X1 << RX1_N_LINE_START_PD)
#define RX2_N_LINE_START_PD			22
#define RX2_N_LINE_START_PD_MASK		(0X1 << RX2_N_LINE_START_PD)
#define RX3_N_LINE_START_PD			23
#define RX3_N_LINE_START_PD_MASK		(0X1 << RX3_N_LINE_START_PD)
#define RX_CHN_CFG_MODE_PD			24
#define RX_CHN_CFG_MODE_PD_MASK			(0X1 << RX_CHN_CFG_MODE_PD)
#define TX_CHN_CFG_MODE_PD			25
#define TX_CHN_CFG_MODE_PD_MASK			(0X1 << TX_CHN_CFG_MODE_PD)
#define AWNN_ID_BACK_ERR_PD			26
#define AWNN_ID_BACK_ERR_PD_MASK		(0X1 << AWNN_ID_BACK_ERR_PD)
#define RX_LBC_MUX_CONF_TIME_OUT_PD		27
#define RX_LBC_MUX_CONF_TIME_OUT_PD_MASK	(0X1 << RX_LBC_MUX_CONF_TIME_OUT_PD)
#define AWNN_ID_FRM_DONE_PD			28
#define AWNN_ID_FRM_DONE_PD_MASK		(0X1 << AWNN_ID_FRM_DONE_PD)
#define RX_2TO1_FIFO_OV_PD			29
#define RX_2TO1_FIFO_OV_PD_MASK			(0X1 << RX_2TO1_FIFO_OV_PD)
#define AWNN_TIME_OUT_BY_AWNN_PD		30
#define AWNN_TIME_OUT_BY_AWNN_PD_MASK		(0X1 << AWNN_TIME_OUT_BY_AWNN_PD)
#define RX_W_ADDR_EXCEED_PD			31
#define RX_W_ADDR_EXCEED_PD_MASK		(0X1 << RX_W_ADDR_EXCEED_PD)

#define TDM_INTERNAL_STATUS0_REG_OFF		0X020
#define RX0_FRM_LOST_PD				(1 << 0)
#define RX1_FRM_LOST_PD				(1 << 1)
#define RX2_FRM_LOST_PD				(1 << 2)
#define RX3_FRM_LOST_PD				(1 << 3)
#define RX0_FRM_ERR_PD				(1 << 8)
#define RX1_FRM_ERR_PD				(1 << 9)
#define RX2_FRM_ERR_PD				(1 << 10)
#define RX3_FRM_ERR_PD				(1 << 11)
#define RX0_BTYPE_ERR_PD			(1 << 16)
#define RX1_BTYPE_ERR_PD			(1 << 17)
#define RX2_BTYPE_ERR_PD			(1 << 18)
#define RX3_BTYPE_ERR_PD			(1 << 19)
#define RX0_BUF_FULL_PD				(1 << 24)
#define RX1_BUF_FULL_PD				(1 << 25)
#define RX2_BUF_FULL_PD				(1 << 26)
#define RX3_BUF_FULL_PD				(1 << 27)

#define TDM_INTERNAL_STATUS1_REG_OFF		0X024
#define TX0_FIFO_UNDER				(1 << 0)
#define TX1_FIFO_UNDER				(1 << 1)
#define TX2_FIFO_UNDER				(1 << 2)
#define EX0_W_ADDR_EXCEED_PD			(1 << 4)
#define EX1_W_ADDR_EXCEED_PD			(1 << 5)
#define EX2_W_ADDR_EXCEED_PD			(1 << 6)
#define EX3_W_ADDR_EXCEED_PD			(1 << 7)
#define RX0_HB_SHORT_PD				(1 << 8)
#define RX1_HB_SHORT_PD				(1 << 9)
#define RX2_HB_SHORT_PD				(1 << 10)
#define RX3_HB_SHORT_PD				(1 << 11)
#define RX0_LBC_MUX_CONF_PD			(1 << 12)
#define RX1_LBC_MUX_CONF_PD			(1 << 13)
#define RX2_LBC_MUX_CONF_PD			(1 << 14)
#define RX3_LBC_MUX_CONF_PD			(1 << 15)
#define RX0_FIFO_FULL_PD			(1 << 16)
#define RX1_FIFO_FULL_PD			(1 << 17)
#define RX2_FIFO_FULL_PD			(1 << 18)
#define RX3_FIFO_FULL_PD			(1 << 19)
#define RX0_HEAD_FIFO_FULL_0			(1 << 20)
#define RX0_HEAD_FIFO_FULL_1			(1 << 21)
#define RX1_HEAD_FIFO_FULL_0			(1 << 22)
#define RX1_HEAD_FIFO_FULL_1			(1 << 23)
#define RX2_HEAD_FIFO_FULL_0			(1 << 24)
#define RX2_HEAD_FIFO_FULL_1			(1 << 25)
#define RX3_HEAD_FIFO_FULL_0			(1 << 26)
#define RX3_HEAD_FIFO_FULL_1			(1 << 27)
#define LBC0_ERROR				(1 << 28)
#define LBC1_ERROR				(1 << 29)
#define LBC2_ERROR				(1 << 30)
#define LBC3_ERROR				(1 << 31)

#define TDM_INTERNAL_STATUS2_REG_OFF		0X028
#define AWNN_ID0_FRM_DONE_PD			(1 << 0)
#define AWNN_ID1_FRM_DONE_PD			(1 << 1)
#define AWNN_ID2_FRM_DONE_PD			(1 << 2)
#define AWNN_ID3_FRM_DONE_PD			(1 << 3)
#define RX0_2TO1_FIFO_OV_PD			(1 << 4)
#define RX1_2TO1_FIFO_OV_PD			(1 << 5)
#define RX2_2TO1_FIFO_OV_PD			(1 << 6)
#define RX3_2TO1_FIFO_OV_PD			(1 << 7)
#define TX_LBD0_ERR_PD  			(1 << 8)
#define TX_LBD1_ERR_PD  			(1 << 9)
#define TX_LBD2_ERR_PD  			(1 << 10)
#define AWNN_ID0_TIME_OUT_BY_AWNN_PD  		(1 << 12)
#define AWNN_ID1_TIME_OUT_BY_AWNN_PD  		(1 << 13)
#define AWNN_ID2_TIME_OUT_BY_AWNN_PD  		(1 << 14)
#define AWNN_ID3_TIME_OUT_BY_AWNN_PD  		(1 << 15)
#define AWNN_ID0_TIME_OUT_BY_AWNN_INDEX  	(1 << 24)
#define AWNN_ID0_TIME_OUT_BY_AWNN_INDEX_MASK  	(0xFF << AWNN_ID0_TIME_OUT_BY_AWNN_INDEX)

#define TDM_CLK_FREQ_REG_OFF	        	0X050
#define TDM_CLK_FREQ_SET			0
#define TDM_CLK_FREQ_SET_MASK			(0x3FFF << TDM_CLK_FREQ_SET)

#define TDM_TIME_BASE_REG_OFF	        	0X054
#define TDM_TIME_US			        0
#define TDM_TIME_US_MASK			(0x3FF << TDM_TIME_US)
#define TDM_TIME_MS			        10
#define TDM_TIME_MS_MASK			(0x3FF << TDM_TIME_MS)
#define TDM_TIME_S			        20
#define TDM_TIME_S_MASK		        	(0x3F << TDM_TIME_S)
#define TDM_TIME_MIN	        	        26
#define TDM_TIME_MIN_MASK			(0x3F << TDM_TIME_MIN)

/*
 * VGM registers
 */
#define TMD_VGM_OFFSET				0x060

#define TDM_VGM_CFG0_REG_OFF			0X000
#define VGM_DMODE				0
#define VGM_DMODE_MASK				(0X7 << VGM_DMODE)
#define VGM_SMODE				4
#define VGM_SMODE_MASK				(0X1 << VGM_SMODE)
#define VGM_START				5
#define VGM_START_MASK				(0X1 << VGM_START)
#define VGM_BCYCLE				8
#define VGM_BCYCLE_MASK				(0XFF << VGM_BCYCLE)
#define VGM_MODE				31
#define VGM_MODE_MASK				(0x1 << VGM_MODE)

#define TDM_VGM_CFG1_REG_OFF			0X004
#define VGM_INPUT_FMT				0
#define VGM_INPUT_FMT_MASK			(0X7 << VGM_INPUT_FMT)
#define VGM_DATA_TYPE				4
#define VGM_DATA_TYPE_MASK			(0X3 << VGM_DATA_TYPE)

#define TDM_VGM_CFG2_REG_OFF			0X008
#define VGM_PARA0				0
#define VGM_PARA0_MASK				(0X7 << VGM_PARA0)
#define VGM_PARA1				4
#define VGM_PARA1_MASK				(0X3 << VGM_PARA1)

/*
 * tdm tx registers
 */
#define TMD_TX_OFFSET				0x0a0

#define TDM_TX_CFG0_REG_OFF			0X000
#define TDM_TX_EN				0
#define TDM_TX_EN_MASK				(0X1 << TDM_TX_EN)
#define TDM_TX_CAP_EN				1
#define TDM_TX_CAP_EN_MASK			(0X1 << TDM_TX_CAP_EN)
#define TDM_TX_STATUS_BD			30
#define TDM_TX_STATUS_BD_MASK			(0X1 << TDM_TX_STATUS_BD)
#define TDM_TX_FIFO_MODE			31
#define TDM_TX_FIFO_MODE_MASK			(0X1 << TDM_TX_FIFO_MODE)

#define TDM_TX_CFG1_REG_OFF			0X004
#define TDM_TX_H_BLANK				0
#define TDM_TX_H_BLANK_MASK			(0X3FFF << TDM_TX_H_BLANK)
#define TDM_TX_CYCLE_CNT_EN			16
#define TDM_TX_CYCLE_CNT_EN_MASK		(0X1 << TDM_TX_CYCLE_CNT_EN)

#define TDM_TX_CFG2_REG_OFF			0X008
#define TDM_TX_V_BLANK_FE			0
#define TDM_TX_V_BLANK_FE_MASK			(0X3FFF << TDM_TX_V_BLANK_FE)
#define TDM_TX_V_BLANK_BE			16
#define TDM_TX_V_BLANK_BE_MASK			(0X3FFF << TDM_TX_V_BLANK_BE)

#define TDM_TX_TIME1_CYCLE_OFF			0X00C
#define TDM_TX_T1_CYCLE				0
#define TDM_TX_T1_CYCLE_MASK			(0XFFFFFFFF << TDM_TX_T1_CYCLE)

#define TDM_TX_TIME2_CYCLE_OFF			0X010
#define TDM_TX_T2_CYCLE				0
#define TDM_TX_T2_CYCLE_MASK			(0XFFFFFFFF << TDM_TX_T2_CYCLE)

#define TDM_TX_FIFO_DEPTH_OFF			0X014
#define TDM_TX_HEAD_FIFO			0
#define TDM_TX_HEAD_FIFO_MASK			(0XFFFF << TDM_TX_HEAD_FIFO)
#define TDM_TX_DATA_FIFO			16
#define TDM_TX_DATA_FIFO_MASK			(0XFFFF << TDM_TX_DATA_FIFO)

#define TDM_TX_DATA_RATE_REG_OFF		0X018
#define TDM_TX_INVALID_NUM			0
#define TDM_TX_INVALID_NUM_MASK			(0XFF << TDM_TX_INVALID_NUM)
#define TDM_TX_VALID_NUM			16
#define TDM_TX_VALID_NUM_MASK			(0XFF << TDM_TX_VALID_NUM)

#define TDM_TX_CTRL_ST_REG_OFF			0X020
#define TX_CTRL_ST				0
#define TX_CTRL_ST_MASK				(0XFF << TX_CTRL_ST)

#define TDM_TX_FIFO_INFO0_REG_OFF		0X028
#define TX_DATA_FIFO_LAYER			0
#define TX_DATA_FIFO_LAYER_MASK			(0XFFF << TX_DATA_FIFO_LAYER)
#define TX_HEAD_FIFO_LAYER			12
#define TX_HEAD_FIFO_LAYER_MASK			(0XFFF << TX_HEAD_FIFO_LAYER)
#define TX_DATA_CMD_FINISH			24
#define TX_DATA_CMD_FINISH_MASK			(0x1 << TX_DATA_CMD_FINISH)
#define TX_HEAD_CMD_FINISH			25
#define TX_HEAD_CMD_FINISH_MASK			(0x1 << TX_HEAD_CMD_FINISH)
#define TX_DATA_MBUS_IDLE			26
#define TX_DATA_MBUS_IDLE_MASK			(0x1 << TX_DATA_MBUS_IDLE)
#define TX_HEAD_MBUS_IDLE			27
#define TX_HEAD_MBUS_IDLE_MASK			(0x1 << TX_HEAD_MBUS_IDLE)

#define TDM_TX_FIFO_INFO1_REG_OFF		0X02C
#define TX_DATA_RDMA_LINE			0
#define TX_DATA_RDMA_LINE_MASK			(0X3FFF << TX_DATA_RDMA_LINE)
#define TX_HEAD_RDMA_LINE			16
#define TX_HEAD_RDMA_LINE_MASK			(0X3FFF << TX_HEAD_RDMA_LINE)

#define TDM_TX_ID0_FIFO_DEPTH_OFF		0X040
#define TDM_TX_ID0_HEAD_FIFO			0
#define TDM_TX_ID0_HEAD_FIFO_MASK		(0XFF << TDM_TX_ID0_HEAD_FIFO)
#define TDM_TX_ID0_DATA_FIFO			16
#define TDM_TX_ID0_DATA_FIFO_MASK		(0XFFF << TDM_TX_ID0_DATA_FIFO)

#define TDM_TX_ID1_FIFO_DEPTH_OFF		0X044
#define TDM_TX_ID1_HEAD_FIFO			0
#define TDM_TX_ID1_HEAD_FIFO_MASK		(0XFF << TDM_TX_ID1_HEAD_FIFO)
#define TDM_TX_ID1_DATA_FIFO			16
#define TDM_TX_ID1_DATA_FIFO_MASK		(0XFFF << TDM_TX_ID1_DATA_FIFO)

#define TDM_TX_ID2_FIFO_DEPTH_OFF		0X048
#define TDM_TX_ID2_HEAD_FIFO			0
#define TDM_TX_ID2_HEAD_FIFO_MASK		(0XFF << TDM_TX_ID2_HEAD_FIFO)
#define TDM_TX_ID2_DATA_FIFO			16
#define TDM_TX_ID2_DATA_FIFO_MASK		(0XFFF << TDM_TX_ID2_DATA_FIFO)

#define TDM_TX_ID3_FIFO_DEPTH_OFF		0X04C
#define TDM_TX_ID3_HEAD_FIFO			0
#define TDM_TX_ID3_HEAD_FIFO_MASK		(0XFF << TDM_TX_ID3_HEAD_FIFO)
#define TDM_TX_ID3_DATA_FIFO			16
#define TDM_TX_ID3_DATA_FIFO_MASK		(0XFFF << TDM_TX_ID3_DATA_FIFO)

#define TDM_TX_HWC_FIFO_ADDR_OFF		0X050
#define TDM_TX0_HWC_FIFO_ADDR			0
#define TDM_TX0_HWC_FIFO_ADDR_MASK		(0X3FFF << TDM_TX0_HWC_FIFO_ADDR)

/*
 * tdm rx registers
 */
#define TMD_RX0_OFFSET				0x100
#define TMD_RX1_OFFSET				0x180
#define TMD_RX2_OFFSET				0x200
#define TMD_RX3_OFFSET				0x280

#define AMONG_RX_OFFSET				0x80

#define TDM_RX_CFG0_REG_OFF			0X000
#define TDM_RX_EN				0
#define TDM_RX_EN_MASK				(0X1 << TDM_RX_EN)
#define TDM_RX_CAP_EN				1
#define TDM_RX_CAP_EN_MASK			(0X1 << TDM_RX_CAP_EN)
#define TDM_RX_CAP_ST				2
#define TDM_RX_CAP_ST_MASK			(0X1 << TDM_RX_CAP_ST)
#define TDM_RX_SEQ_INIT				8
#define TDM_RX_SEQ_INIT_MASK			(0X1 << TDM_RX_SEQ_INIT)

#define TDM_RX_CFG1_REG_OFF			0X004
#define TDM_RX_PRE_W_PARA_EN			0
#define TDM_RX_PRE_W_PARA_EN_MASK		(0X1 << TDM_RX_PRE_W_PARA_EN)
#define TDM_RX_ABD_EN				2
#define TDM_RX_ABD_EN_MASK			(0X1 << TDM_RX_ABD_EN)
#define TDM_RX_TX_EN				3
#define TDM_RX_TX_EN_MASK			(0X1 << TDM_RX_TX_EN)
#define TDM_RX_LBC_EN				4
#define TDM_RX_LBC_EN_MASK			(0X1 << TDM_RX_LBC_EN)
#define TDM_RX_PKG_EN				5
#define TDM_RX_PKG_EN_MASK			(0X1 << TDM_RX_PKG_EN)
#define TDM_RX_SYN_EN				6
#define TDM_RX_SYN_EN_MASK			(0X1 << TDM_RX_SYN_EN)
#define TDM_LINE_NUM_DDR_EN			7
#define TDM_LINE_NUM_DDR_EN_MASK		(0X1 << TDM_LINE_NUM_DDR_EN)
#define TDM_RX_BUF_NUM				8
#define TDM_RX_BUF_NUM_MASK			(0XF << TDM_RX_BUF_NUM)
#define TDM_RX_NORMAL_EN			12
#define TDM_RX_NORMAL_EN_MASK			(0X1 << TDM_RX_NORMAL_EN)
#define TDM_RX_AWNN_EN	        		13
#define TDM_RX_AWNN_EN_MASK			(0X1 << TDM_RX_AWNN_EN)
#define TDM_RX_START_MODE        		15
#define TDM_RX_START_MODE_MASK			(0X1 << TDM_RX_START_MODE)
#define TDM_RX_MIN_DDR_SIZE			16
#define TDM_RX_MIN_DDR_SIZE_MASK		(0X3 << TDM_RX_MIN_DDR_SIZE)
#define TDM_ACT_CNT_CLEAR        		18
#define TDM_ACT_CNT_CLEAR_MASK			(0X1 << TDM_ACT_CNT_CLEAR)
#define TDM_RX_TO_TX_MODE        		20
#define TDM_RX_TO_TX_MODE_MASK			(0X3 << TDM_RX_TO_TX_MODE)
#define TDM_RX_SW_FIMISH_FLAG        		23
#define TDM_RX_SW_FIMISH_FLAG_MASK		(0X1 << TDM_RX_SW_FIMISH_FLAG)
#define TDM_RX_INPUT_FMT			24
#define TDM_RX_INPUT_FMT_MASK			(0X7 << TDM_RX_INPUT_FMT)
#define TDM_ADDR_ROLL_BACK_EN			27
#define TDM_ADDR_ROLL_BACK_EN_MASK		(0X1 << TDM_ADDR_ROLL_BACK_EN)
#define TDM_INPUT_BIT				28
#define TDM_INPUT_BIT_MASK			(0X7 << TDM_INPUT_BIT)
#define TDM_FRM_CNT_CLEAR			31
#define TDM_FRM_CNT_CLEAR_MASK	        	(0X1 << TDM_FRM_CNT_CLEAR)

#define TDM_RX_CFG2_REG_OFF			0X008
#define TDM_RX_WIDTH				0
#define TDM_RX_WIDTH_MASK			(0X3FFF << TDM_RX_WIDTH)
#define TDM_RX_HEIGHT				16
#define TDM_RX_HEIGHT_MASK			(0X3FFF << TDM_RX_HEIGHT)

#define TDM_RX_CFG3_REG_OFF			0X00C
#define TDM_RX_BLC_EN				0
#define TDM_RX_BLC_EN_MASK			(0X1 << TDM_RX_BLC_EN)
#define TDM_RX_INV_BLC_EN			1
#define TDM_RX_INV_BLC_EN_MASK			(0X1 << TDM_RX_INV_BLC_EN)
#define TDM_RX_NORM_EN				2
#define TDM_RX_NORM_EN_MASK			(0X1 << TDM_RX_NORM_EN)
#define TDM_RX_INV_NORM_EN			3
#define TDM_RX_INV_NORM_EN_MASK			(0X1 << TDM_RX_INV_NORM_EN)
#define TDM_RX_GM_EN				4
#define TDM_RX_GM_EN_MASK			(0X1 << TDM_RX_GM_EN)
#define TDM_RX_INV_GM_EN			5
#define TDM_RX_INV_GM_EN_MASK			(0X1 << TDM_RX_INV_GM_EN)
#define TDM_RX_TX_BLC_EN			12
#define TDM_RX_TX_BLC_EN_MASK			(0X1 << TDM_RX_TX_BLC_EN)
#define TDM_RX_TX_INV_BLC_EN			13
#define TDM_RX_TX_INV_BLC_EN_MASK		(0X1 << TDM_RX_TX_INV_BLC_EN)
#define TDM_RX_TX_NORM_EN			14
#define TDM_RX_TX_NORM_EN_MASK			(0X1 << TDM_RX_TX_NORM_EN)
#define TDM_RX_TX_INV_NORM_EN			15
#define TDM_RX_TX_INV_NORM_EN_MASK		(0X1 << TDM_RX_TX_INV_NORM_EN)
#define TDM_BUF_OV_CHECK_BYP_MODE                27
#define TDM_BUF_OV_CHECK_BYP_MODE_MASK		(0X1 << TDM_BUF_OV_CHECK_BYP_MODE)
#define TDM_RX_LBC_ERR_CHBYP_MODE		28
#define TDM_RX_LBC_ERR_CHBYP_MODE_MASK		(0X1 << TDM_RX_LBC_ERR_CHBYP_MODE)
#define TDM_RX_2TO1_FIFOF_CHBYP_MODE    	29
#define TDM_RX_2TO1_FIFOF_CHBYP_MODE_MASK	(0X1 << TDM_RX_2TO1_FIFOF_CHBYP_MODE)
#define TDM_RX_ADDR_EXCEED_CHBYP_MODE		30
#define TDM_RX_ADDR_EXCEED_CHBYP_MODE_MASK      (0X1 << TDM_RX_ADDR_EXCEED_CHBYP_MODE)
#define TDM_RX_BTYPE_ERR_CHBYP_MODE		31
#define TDM_RX_BTYPE_ERR_CHBYP_MODE_MASK	(0X1 << TDM_RX_BTYPE_ERR_CHBYP_MODE)

#define TDM_RX_CFG4_REG_OFF			0X010
#define TDM_RX_LINE_INT_NUM			0
#define TDM_RX_LINE_INT_NUM_MASK		(0X3FFF << TDM_RX_LINE_INT_NUM)

#define TDM_RX_HWC_FIFO_ADDR_REG_OFF		0X014
#define TDM_RX_LINE_INT_NUM			0
#define TDM_RX_LINE_INT_NUM_MASK		(0X3FFF << TDM_RX_LINE_INT_NUM)

#define TDM_RX_FIFO_DEPTH_REG_OFF		0X018
#define TDM_RX_PKG_LINE_WORDS			0
#define TDM_RX_PKG_LINE_WORDS_MASK		(0xFFF << TDM_RX_PKG_LINE_WORDS)
#define TDM_RX_HEAD_FIFO_DEPTH			12
#define TDM_RX_HEAD_FIFO_DEPTH_MASK		(0xFF << TDM_RX_HEAD_FIFO_DEPTH)
#define TDM_RX_DATA_FIFO_DEPTH			20
#define TDM_RX_DATA_FIFO_DEPTH_MASK		(0xFFF << TDM_RX_DATA_FIFO_DEPTH)

#define TDM_RX_LINE_STRIDE_REG_OFF		0X01C
#define TDM_RX_LINE_STRIDE0		        0
#define TDM_RX_LINE_STRIDE0_MASK		(0xFFF << TDM_RX_LINE_STRIDE0)
#define TDM_RX_LINE_STRIDE1		        16
#define TDM_RX_LINE_STRIDE1_MASK		(0xFFF << TDM_RX_LINE_STRIDE1)

#define TDM_RX_LNDEX_REG_OFF		        0X020
#define TDM_RX_IO_INDEX		                0
#define TDM_RX_IO_INDEX_MASK	        	(0xF << TDM_RX_IO_INDEX)
#define TDM_RX_IO_TYPE		                4
#define TDM_RX_IO_TYPE_MASK	        	(0x3 << TDM_RX_IO_TYPE)

#define TDM_RX_INPUT_OUTPUT_REG_OFF             0X024
#define TDM_RX_IO		                0
#define TDM_RX_IO_MASK                          (0xFFFFFFFF << TDM_RX_IO)

#define TDM_RX_ADDR_OFF1_REG_OFF		0X028
#define TDM_RX_ADDR_OFF1			0
#define TDM_RX_ADDR_OFF1_MASK			(0XFFFFFFFF << TDM_RX_ADDR_OFF1)

#define TDM_RX_MAX_SIZE_REG_OFF 		0X034
#define TDM_RX_BUF_MAX_LINE			0
#define TDM_RX_BUF_MAX_LINE_MASK		(0X3FFF << TDM_RX_BUF_MAX_LINE)

#define TDM_RX_LINE_OFFSET_REG_OFF 		0X038
#define TDM_RX_BUF_OV_RX_OFFSET			0
#define TDM_RX_BUF_OV_RX_OFFSET_MASK		(0X3FFF << TDM_RX_BUF_OV_RX_OFFSET)
#define TDM_RX_BUF_OV_TX_OFFSET			16
#define TDM_RX_BUF_OV_TX_OFFSET_MASK		(0X3FFF << TDM_RX_BUF_OV_TX_OFFSET)

#define TDM_RX_BLC_OFFSET_REG_OFF 		0X040
#define TDM_RX_R_OFFSET 			0
#define TDM_RX_R_OFFSET_MASK	        	(0X3FF << TDM_RX_R_OFFSET)
#define TDM_RX_G_OFFSET 			10
#define TDM_RX_G_OFFSET_MASK	        	(0X3FF << TDM_RX_G_OFFSET)
#define TDM_RX_B_OFFSET 			20
#define TDM_RX_B_OFFSET_MASK	        	(0X3FF << TDM_RX_B_OFFSET)

#define TDM_RX_NORM_COEF_REG_OFF 		0X048
#define TDM_RX_R_COEF 		        	0
#define TDM_RX_R_COEF_MASK	        	(0X1FF << TDM_RX_R_COEF)
#define TDM_RX_G_COEF 		        	10
#define TDM_RX_G_COEF_MASK	        	(0X1FF << TDM_RX_G_COEF)
#define TDM_RX_B_COEF 		        	20
#define TDM_RX_B_COEF_MASK	        	(0X1FF << TDM_RX_B_COEF)

#define TDM_RX_INV_NORM_COEF_REG_OFF 		0X04C
#define TDM_RX_I_R_COEF 		        0
#define TDM_RX_I_R_COEF_MASK	        	(0X1FF << TDM_RX_I_R_COEF)
#define TDM_RX_I_G_COEF 		        10
#define TDM_RX_I_G_COEF_MASK	        	(0X1FF << TDM_RX_I_G_COEF)
#define TDM_RX_I_B_COEF 		        20
#define TDM_RX_I_B_COEF_MASK	        	(0X1FF << TDM_RX_I_B_COEF)

#define TDM_RX_FRAME_ERR_REG_OFF		0X058
#define TDM_RX_ERR_WIDTH		               0
#define TDM_RX_ERR_WIDTH_MASK	        	(0X3FFF << TDM_RX_ERR_WIDTH)
#define TDM_RX_ERR_HEIGHT    		        16
#define TDM_RX_ERR_HEIGHT_MASK	        	(0X3FFF << TDM_RX_ERR_HEIGHT)

#define TDM_RX_HB_SHORT_REG_OFF			0X05C
#define TDM_RX_HB_MAX		                0
#define TDM_RX_HB_MAX_MASK	        	(0XFFFF << TDM_RX_HB_MAX)
#define TDM_RX_HB_MIN    		        16
#define TDM_RX_HB_MIN_MASK	        	(0XFFFF << TDM_RX_HB_MIN)

#define TDM_RX_LAYER_REG_OFF			0X060
#define TDM_RX_FIFO_LAYER                       0
#define TDM_RX_FIFO_LAYER_MASK	        	(0X7FF << TDM_RX_FIFO_LAYER)
#define TDM_RX_HEAD_FIFO_LAYER                  16
#define TDM_RX_HEAD_FIFO_LAYER_MASK	        (0XFFFF << TDM_RX_HEAD_FIFO_LAYER)

#define TMD_RX_FRM_CNT_REG_OFF                  0x074
#define TMD_RX_TIME_BASE_REG_OFF	        0x078

#define TMD_RX_TIMEOFFSET_REG_OFF	        0x07C
#define TDM_RX_CYCLE  	        		0
#define TDM_RX_CYCLE_MASK		       	(0xFFFFFF << TDM_RX_CYCLE)


/*
 * tdm lbc registers
 */
#define TMD_LBC0_OFFSET				0x300
#define TMD_LBC1_OFFSET				0x380
#define TMD_LBC2_OFFSET				0x400
#define TMD_LBC3_OFFSET				0x480
#define AMONG_LBC_OFFSET			0x80

#define TMD_LBC_CFG0_REG_OFF			0x000
#define IS_LOSSY				0
#define IS_LOSSY_MASK				(0x1 << IS_LOSSY)
#define RC_CTRL_MODE				1
#define RC_CTRL_MODE_MASK			(0x1 << RC_CTRL_MODE)
#define STATUS_QP				2
#define STATUS_QP_MASK				(0x3 << STATUS_QP)
#define STD_QP		        		4
#define STD_QP_MASK				(0xF << STD_QP)
#define GLB_MAX_QUO				8
#define GLB_MAX_QUO_MASK			(0xF << GLB_MAX_QUO)
#define GLB_MAX_K				12
#define GLB_MAX_K_MASK			        (0x7 << GLB_MAX_K)
#define PTR_BUFFER_INIT				16
#define PTR_BUFFER_INIT_MASK		        (0x7FF << PTR_BUFFER_INIT)

#define TMD_LBC_CFG1_REG_OFF			0x004
#define MB_NUM_IN_LINE				0
#define MB_NUM_IN_LINE_MASK			(0x1FF << MB_NUM_IN_LINE)

#define TMD_LBC_CFG2_REG_OFF			0x008
#define PTR_BUFFER_FULLNESS_MAX			0
#define PTR_BUFFER_FULLNESS_MAX_MASK		(0x1FFF << PTR_BUFFER_FULLNESS_MAX)
#define PTR_BUFFER_THR				16
#define PTR_BUFFER_THR_MASK			(0x1FFF << PTR_BUFFER_THR)

#define TMD_LBC_CFG3_REG_OFF			0x00C
#define LINE_MAX_BIT				0
#define LINE_MAX_BIT_MASK			(0x3FFFF << LINE_MAX_BIT)
#define TAR_BITS_LINE_RC			20
#define TAR_BITS_LINE_RC_MASK			(0x3FF << TAR_BITS_LINE_RC)

#define TMD_LBC_CFG4_REG_OFF			0x010
#define LINE_ATR_BIT				0
#define LINE_ATR_BIT_MASK			(0x3FFFF << LINE_ATR_BIT)
#define TAR_BITS		        	20
#define TAR_BITS_MASK		        	(0x3FF << TAR_BITS)

#define TMD_LBC_CFG5_REG_OFF			0x014
#define FRAME_TAR_BIT				0
#define FRAME_TAR_BIT_MASK			(0x7FFFFFFF << FRAME_TAR_BIT)

#define TMD_LBC_MIN_QP_LUT0_REG_OFF		0x020
#define MIN_QP_S0				0
#define MIN_QP_S0_MASK		        	(0xF << MIN_QP_S0)
#define MIN_QP_S1				4
#define MIN_QP_S1_MASK		        	(0xF << MIN_QP_S1)
#define MIN_QP_S2				8
#define MIN_QP_S2_MASK		        	(0xF << MIN_QP_S2)
#define MIN_QP_S3				12
#define MIN_QP_S3_MASK		        	(0xF << MIN_QP_S3)
#define MIN_QP_S4				16
#define MIN_QP_S4_MASK		        	(0xF << MIN_QP_S4)
#define MIN_QP_S5 				20
#define MIN_QP_S5_MASK		        	(0xF << MIN_QP_S5)
#define MIN_QP_S6				24
#define MIN_QP_S6_MASK		        	(0xF << MIN_QP_S6)
#define MIN_QP_S7				28
#define MIN_QP_S7_MASK		        	(0xF << MIN_QP_S7)

#define TMD_LBC_MIN_QP_LUT1_REG_OFF		0x024
#define TMD_LBC_MAX_QP_LUT0_REG_OFF		0x028
#define TMD_LBC_MAX_QP_LUT1_REG_OFF		0x02C

#define TMD_LBC_THRESH_LUT0_REG_OFF		0x030
#define TH_S0   				0
#define TH_S0_MASK		        	(0x1FFF << TH_S0)
#define TH_S1		        		16
#define TH_S1_MASK		        	(0x1FFF << TH_S1)

#define TMD_LBC_THRESH_LUT1_REG_OFF		0x034
#define TMD_LBC_THRESH_LUT2_REG_OFF		0x038
#define TMD_LBC_THRESH_LUT3_REG_OFF		0x03C
#define TMD_LBC_THRESH_LUT4_REG_OFF		0x040
#define TMD_LBC_THRESH_LUT5_REG_OFF		0x044
#define TMD_LBC_THRESH_LUT6_REG_OFF		0x048
#define TMD_LBC_THRESH_LUT7_REG_OFF		0x04C

#define TMD_LBC_TAR_BITS_ADJ_LUT0_REG_OFF	0x050
#define TAR_BITS_ADJ_S0   			0
#define TAR_BITS_ADJ_S0_MASK		       	(0x3FF << TAR_BITS_ADJ_S0)
#define TAR_BITS_ADJ_S1		        	16
#define TAR_BITS_ADJ_S1_MASK		       	(0x3FF << TAR_BITS_ADJ_S1)

#define TMD_LBC_TAR_BITS_ADJ_LUT1_REG_OFF	0x054
#define TMD_LBC_TAR_BITS_ADJ_LUT2_REG_OFF	0x058
#define TMD_LBC_TAR_BITS_ADJ_LUT3_REG_OFF	0x05C
#define TMD_LBC_TAR_BITS_ADJ_LUT4_REG_OFF	0x060
#define TMD_LBC_TAR_BITS_ADJ_LUT5_REG_OFF	0x064
#define TMD_LBC_TAR_BITS_ADJ_LUT6_REG_OFF	0x068
#define TMD_LBC_TAR_BITS_ADJ_LUT7_REG_OFF	0x06C

#define TMD_LBC_PREP_BITS_ADJ_LUT_REG_OFF	0x070
#define PRE_BITS_ADJ_S0  			0
#define PRE_BITS_ADJ_S0_MASK		       	(0x1 << PRE_BITS_ADJ_S0)
#define PRE_BITS_ADJ_S1  			1
#define PRE_BITS_ADJ_S1_MASK		       	(0x3F << PRE_BITS_ADJ_S1)
#define PRE_BITS_ADJ_S2  			7
#define PRE_BITS_ADJ_S2_MASK		       	(0x7F << PRE_BITS_ADJ_S2)
#define PRE_BITS_ADJ_S3  			14
#define PRE_BITS_ADJ_S3_MASK		       	(0xFF << PRE_BITS_ADJ_S3)
#define PRE_BITS_ADJ_S4  		        22
#define PRE_BITS_ADJ_S4_MASK		       	(0x1FF << PRE_BITS_ADJ_S4)

#endif /* __CSIC__TDM230__REG__I__H__ */
