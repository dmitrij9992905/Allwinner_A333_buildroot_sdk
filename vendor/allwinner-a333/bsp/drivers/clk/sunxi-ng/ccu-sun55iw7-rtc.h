/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright(c) 2020 - 2025 Allwinner Technology Co.,Ltd. All rights reserved. */
/*
 * Copyright (c) 2025 haili@allwinnertech.com
 */

#ifndef _CCU_SUN55IW7_RTC_H_
#define _CCU_SUN55IW7_RTC_H_

#include <dt-bindings/clock/sun55iw7-rtc.h>

#define LOSC_CTRL_REG			0x00
#define KEY_FIELD_MAGIC_NUM_RTC		0x16AA0000
#define INTOSC_CLK_AUTO_CALI_REG	0x0C
#define LOSC_OUT_GATING_REG		0x60  /* Or: 32K_FOUT_CTRL_GATING_REG */
#define XO_CTRL_REG			0x160
#define CALI_CTRL_REG                   0x164
#define LOSC_AUTO_SWT_STA_REG		0x04

#define CLK_NUMBER			(CLK_RTC_MAX_NO)

#endif /* _CCU_SUN55IW7_RTC_H_ */
