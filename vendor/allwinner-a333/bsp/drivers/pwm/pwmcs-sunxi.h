/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Copyright(c) 2020 - 2023 Allwinner Technology Co.,Ltd. All rights reserved. */
/*
 * Allwinnertech pulse-width-modulation controller driver
 *
 * Copyright (C) 2018 AllWinner
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2. This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */
#ifndef PWMCS_SUNXI_H
#define PWMCS_SUNXI_H

/* PWMCS configuration */
#define PWMCS_CFG_MODE_SHIFT	0xd
#define PWMCS_CFG_MODE_WIDTH    0x1

#define PWMCS_ACT_CYCLES_SHIFT 0x0
#define PWMCS_ACT_CYCLES_WIDTH 0x16

#define PWMCS_ENTIRE_CYCLES_SHIFT 0
#define PWMCS_ENTIRE_CYCLES_WIDTH 0x16

#define PWMCS_OUT_EN_SHIFT	0x0
#define PWMCS_OUT_EN_WIDTH  0x2

#define PWMCS_CFG_MODE_SHIFT	0xd
#define PWMCS_CFG_MODE_WIDTH    0x1

#define PWMCS_NEW_DATA_SHIFT 0xc
#define PWMCS_NEW_DATA_WIDTH 0x1

#define PWMCS_UPDATE_MODE_SHIFT 0x14
#define PWMCS_UPDATE_MODE_WIDTH 0x1

#define PWMCS_CLK_SRC_SHIFT	0x7
#define PWMCS_CLK_SRC_WIDTH	0x2

#define PWMCS_DIV_M_SHIFT		0x0
#define PWMCS_DIV_M_WIDTH		0x4

#define PWMCS_PRESCAL_SHIFT	0x0
#define PWMCS_PRESCAL_WIDTH	0x8

#define PWMCS_ACT_STA_SHIFT	0x8
#define PWMCS_ACT_STA_WIDTH	0x1

#define PWMCS_CH_BASE_ADDR(channel) \
    (((channel) < 2) ? 0x1000 : \
     ((channel) < 4) ? 0x2000 : \
     ((channel) < 6) ? 0x3000 : \
     0x4000)

#define PWMCS_CH_REG_ADDR(channel, offset) \
    (PWMCS_CH_BASE_ADDR(channel) + (offset))

#define PWMCS_CH_UNIFORM_OFFSET(channel) \
    (((channel) % 2) * 0x100)

/* PWMCS TOP configuration */
#define PWMCS_TIER          0x00  /*PWMCS TOP IRQ enable register 0x00*/
#define PWMCS_TISR          0x04  /*PWMCS TOP IRQ status register 0x04*/
#define PWMCS_PGR0          0x08  /*PWMCS OUT group0 register 0x08*/
#define PWMCS_PGR1          0x0c  /*PWMCS OUT group1 register 0x0c*/
#define PWMCS_PGR2          0x10  /*PWMCS OUT group2 register 0x10*/
#define PWMCS_PGR3          0x14  /*PWMCS OUT group3 register 0x14*/
#define PWMCS_CFG_DATA      0x18  /*PWMCS configuration data register*/
#define PWMCS_CFG           0x1c  /*PWMCS configuration register*/
#define PWMCS_O_MULT_CFG    0x20  /*PWMCS PWMCS_O mulitiplex configuration register*/
#define PWMCS_I_MULT_CFG    0x24  /*PWMCS PWMCS_I mulitiplex configuration register*/
#define PWMCS_O_MULT_EN     0x28  /*PWMCS PWMCS_O Filter enable configuration register*/
#define PWMCS_I_FLT_EN      0x2c  /*PWMCS PWMCS_I Filter enable register*/
#define PWMCS_O_FLT_V       0x30  /*PWMCS PWMCS_O Filter value register*/
#define PWMCS_I_FLT_V       0x34  /*PWMCS PWMCS_I Filter value register*/
#define PWMCS_I_LVL_DET_V   0x38  /*PWMCS PWMCS_I Level detect value register*/
#define PWMCS_GCGR          0x3c  /*PWMCS group clock gating register*/
#define PWMCS_VER           0xfc  /*PWMCS version register*/

#define PCS_PCIER          	0x00  /* PWMCS OUT Channel IRQ Enable Register */
#define PCS_PCISR          	0x04  /* PWMCS OUT Channel IRQ Status Register */
#define PCS_CCIER           0x08  /* PWMCS Capture Channel IRQ Enable Register */
#define PCS_CCISR           0x0c  /* PWMCS Capture Channel IRQ Status Register */
#define PCS_CCCIER          0x10  /* PWMCS Capture Cycle Counter IRQ Enable Register */
#define PCS_CCCISR          0x14  /* PWMCS Capture Cycle Counter IRQ Status Register */
#define PCS_POS_IRQ_EN      0x18  /* PWMCS Position IRQEnable Register */
#define PCS_POS_IRQ_STS     0x1C  /* PWMCS Position IRQ StatusRegister */
#define PCS_PCCR01          0x20  /* PWMCS OUTO1 Clock Configuration Register */
#define PCS_PWM01_OUT_CON   0x24  /* PWMCS OUT O1 Output Configuration Register */
#define PCS_PCGR            0x28  /* PWMCS OUT Clock Gating Register */
#define PCS_PER             0x2C  /* PWMCS OUT Enable Register */
#define PCS_PDZCR01         0x30  /* PWMCS OUT01 Dead Zone Control Register */
#define PCS_CCCER           0x34  /* PWMCS Capture Cycle Counter Enable Register */
#define PCS_CER             0x38  /* PWMCS Capture Enable Register */
#define PCS_CAP01_CCR       0x3C  /* PWMCS CAPO1 Clock Configuration Register */
#define PCS_CCGR            0x40  /* PWMCS Capture Clock Gating Register */
#define PCS_CAP_POS_CNT_V   0x44  /* PWMCS Capture Position Counter Value Register */
#define PCS_CAP_POS_CNT_SP  0x48  /* PWMCS Capture Position Counter Start Point Value Register */
#define PCS_CAPPOS_CNT_EP   0x4C  /* PWMCS Capture Position Counter End Point Value Register */
#define PCS_CAP_POS_CNT_C   0x50  /* PWMCS Capture Position Counter Compare Value Register */

#define PCS_CAPPOS_CNT_TM   0x54  /* PWMCS Capture Position Counter Time Complete Register */
#define PCS_CAPV            0x58  /* PWMCSCapture Value Register */
#define PCS_CAP_TMRPRD_V    0x5C  /* PWMCSCapture Time Period Value Register */
#define PCS_CAP_DEC_CONF    0x60  /* PWMCs Capture Decode Configuration Register */
#define PCS_CAP_POS_CNT_CO  0x64  /* PWMCS Capture Position Counter Configuration Register */
#define PCS_CAP_TMR_CMP_C   0x68  /* PWMCS Capture Position Count Compare Configuration Register */
#define PCS_CAP_RTCC        0x6C  /* PWMCS Capture Rotation Timer Capture Configuration Register */
#define PCS_CAP_RTCV        0x70  /* PWMCS Capture Rotation Timer Capture Value Register */
#define PCS_CAPRTCDTDV      0x74  /* PWMCS Capture Rotation Timer Capture Delta Timer Direction Value Register */
#define PCS_CAPRTCV_LH      0x7C  /* PWMCSRotation Timer Capture Value Latch Register */
#define PCS_CAP_RTC_DTV_LH  0x80  /* PWMCSRotation Timer Capture Delta Timer Value Latch Register */
#define PCS_CAP_POS_CNT_IC  0x84  /* PWMCS Capture Position Counter Index Capture Register */
#define PCS_CAPPOS_CNT      0x88  /* Value Register for Capture Position Counter */
#define PCS_CAPDIR_ERR_VA   0x8C  /* PWMCS Capture Direction Error Valve Register */
#define PCS_CAP_MODEO_A_P   0x90  /* PWMCS Capture Mode OA Period Valve Register */
#define PCS_CAP_MODEO_AB    0x94  /* PWMCS Capture Mode OAB Period Valve Register */
#define PCS_CAP_POS_CNT_C_LH 0x98 /* PWMCS Capture Position Counter Compare Latch Register */
#define PCS_DBG_SEL         0x9C  /* PWMCS Debug Signal Select Register */

#define PWMCS_BASE          (0x20a0000)
#define PCS_PCR             (0x0100+0x0000) /*PWM Contorl Register N = 0~1*/
#define PCS_PWM_PUL_NUM     (0x0100+0x00004) /* PWM pulse number Register N = 0~1*/
#define PCS_PWM_ENTIRE_CY   (0x0100 + 0x0008) /* PWMCS OUT Entire Cycle Register, N = 0~1 */
#define PCS_PWM_ACT_CYCLE   (0x0100 + 0x000C) /* PWMCS OUT Active Cycle Register, N = 0~1 */
#define PCS_PPCNTCR         (0x0100 + 0x0010) /* PWMICS OUT Counter Configuration Register, N = 0~1 */
#define PCS_PPCNT_NM_STS    (0x0100 + 0x0014) /* PWMCS OUT Counter Number Status Register, N = 0~1 */
#define PCS_PPCNTR          (0x0100 + 0x0018) /* PWMCS OUT Pulse Counter Register, N = 0~1 */
#define PCS_PWM_OUT_STP     (0x0100 + 0x001C) /* PWMCS OUT Output Pulse Stop Register, N = 0~1 */
#define PCS_PWM_OUT_DLY     (0x0100 + 0x0020) /* PWMCS OUT Output Pulse Delay Register, N = 0~1 */
#define PCS_PWM_CFG_FIFO_S  (0x0100 + 0x0024) /* PWMCS OUT Configuration FIFO Status Register, N = 0~1 */
#define PCS_PWM_MAXFENTI    (0x0100 + 0x0028) /* PWMCS OUT Max Frequency Entire Cycle Register, N = 0~1 */
#define PCS_PWM_MINF_ENTI   (0x0100 + 0x002C) /* PWMCS OUT Minimum Frequency Entire Cycle Register, N = 0~1 */
#define PCS_PWM_CNT_PHV     (0x0100 + 0x0030) /* PWMCS OUT Counter Phase Value Register, N = 0~1 */
#define PCS_CCR             (0x0100 + 0x0034) /* PWMCS Capture Control Register, N = 0~1 */
#define PCS_CRLR            (0x0100 + 0x0038) /* PWMCS Capture Rise Lock Register, N = 0~1 */
#define PCS_CFLR            (0x0100 + 0x003C) /* PWMCS Capture Fall Lock Register, N = 0~1 */
#define PCS_CCCSR           (0x0100 + 0x0040) /* PWMCS Capture CYCLE counter Register, N = 0~1 */
#define PCS_CCCNR           (0x0100 + 0x0044) /* PWMCS Capture CYCLE counter number Register, N = 0~1 */

#endif