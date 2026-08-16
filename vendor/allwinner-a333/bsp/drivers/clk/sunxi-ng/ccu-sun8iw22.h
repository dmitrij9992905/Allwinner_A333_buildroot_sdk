/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright(c) 2020 - 2023 Allwinner Technology Co.,Ltd. All rights reserved. */
/*
 * Copyright (c) 2024 haili@allwinnertech.com
 */

#ifndef _CCU_SUN8IW22_H_
#define _CCU_SUN8IW22_H_

#include <dt-bindings/clock/sun8iw22-ccu.h>
#include <dt-bindings/reset/sun8iw22-ccu.h>

#define SUN8IW22_AHB_GATE_EN_REG		0x05C0
#define SUN8IW22_AHB_MONITOR_ENABLE		31
#define SUN8IW22_SD_MONITOR_ENABLE		29
#define SUN8IW22_PLL_PERI0_GATE_EN_REG		0x1908
#define SUN8IW22_PLL_PERI1_GATE_EN_REG		0x190C
#define SUN8IW22_PLL_VIDEO_GATE_EN_REG		0x1910
#define SUN8IW22_PLL_AUDIO_GATE_EN_REG		0x191C


#define CLK_NUMBER		CLK_MAX_NO

#endif /* _CCU_SUN8IW22_H_ */
