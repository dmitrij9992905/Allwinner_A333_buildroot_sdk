/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright(c) 2020 - 2023 Allwinner Technology Co.,Ltd. All rights reserved. */
 /*
  * Hawkview ISP - sun252iw1_vin_cfg.h module
  *
  * Copyright (c) 2024 by Allwinnertech Co., Ltd.  http://www.allwinnertech.com
  *
  * Authors:  zhengzequn <zequnzheng@allwinnertech.com>
  *
  * This program is free software; you can redistribute it and/or modify
  * it under the terms of the GNU General Public License version 2 as
  * published by the Free Software Foundation.
  */

#ifndef _SUN252IW1_VIN_CFG_H_
#define _SUN252IW1_VIN_CFG_H_

#define CSI_CCU_REGS_BASE     0x05800000
#define CSI_TOP_REGS_BASE     0x05800800

#define CSI0_REGS_BASE        0x05820000
#define CSI1_REGS_BASE        0x05821000
#define CSI2_REGS_BASE        0x05822000

#define TDM_REGS_BASE         0x05908000
#define ISP_REGS_BASE         0x05900000

#define VIPP0_REGS_BASE       0x05910000
#define VIPP1_REGS_BASE       0x05910400
#define VIPP2_REGS_BASE       0x05910800
#define VIPP3_REGS_BASE       0x05910C00

#define CSI_DMA0_REG_BASE     0x05830000
#define CSI_DMA1_REG_BASE     0x05831000
#define CSI_DMA2_REG_BASE     0x05832000
#define CSI_DMA3_REG_BASE     0x05833000

#define VIN_MAX_DEV           16
#define VIN_MAX_CSI           3
#define VIN_MAX_TDM           1
#define VIN_MAX_CCI           3
#define VIN_MAX_MIPI          3
#define VIN_MAX_ISP           7
#define VIN_MAX_SCALER        16
#define VIN_MAX_PINCTRL       3

#define MAX_CH_NUM            4

#endif /* _SUN252IW1_VIN_CFG_H_ */
