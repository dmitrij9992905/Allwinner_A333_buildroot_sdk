/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright(c) 2020 - 2025 Allwinner Technology Co.,Ltd. All rights reserved. */
/*
 * Copyright (c) 2025 haili@allwinnertech.com
 */

#ifndef _CCU_SUN55IW7_H_
#define _CCU_SUN55IW7_H_

#include <dt-bindings/clock/sun55iw7-ccu.h>
#include <dt-bindings/reset/sun55iw7-ccu.h>

#define SUN55IW7_AHB_GATE_EN_REG		0x05C0
#define SUN55IW7_AHB_MONITOR_ENABLE		31
#define SUN55IW7_SD_MONITOR_ENABLE		29
#define SUN55IW7_PLL_PERI0_GATE_EN_REG		0x1908
#define SUN55IW7_PLL_PERI1_GATE_EN_REG		0x190C
#define SUN55IW7_PLL_VIDEO_GATE_EN_REG		0x1910
#define SUN55IW7_PLL_GPU_GATE_EN_REG		0x1914
#define SUN55IW7_PLL_VE_GATE_EN_REG		0x1918
#define SUN55IW7_PLL_AUDIO_GATE_EN_REG		0x191C
#define SUN55IW7_PLL_ADC_GATE_EN_REG		0x1920


#define CLK_NUMBER		CLK_MAX_NO

#endif /* _CCU_SUN55IW7_H_ */
