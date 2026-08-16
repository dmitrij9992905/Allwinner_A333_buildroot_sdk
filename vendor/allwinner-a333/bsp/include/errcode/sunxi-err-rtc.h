/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright(c) 2020 - 2023 Allwinner Technology Co.,Ltd. All rights reserved. */
/*
 * Allwinner's errcode for audio
 *
 * Copyright (c) 2023, wangxinfeng <wangxinfeng@allwinnertech.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 */

 #ifndef __SUNXI_ERR_RTC_H__
 #define __SUNXI_ERR_RTC_H__

enum sunxi_err_rtc_func {
	RTC_CLK_ERR = 0x0,
	RTC_REG_ACCESS_ERR,
	RTC_TIME_ERR,
	RTC_ALARM_ERR,
	RTC_INVAL_ARG,
	RTC_IRQ_ERR,
};

enum sunxi_err_rtc {
	E_RTC_HW_CLK				= E_RTC_HW_ERR0 | E_USER(RTC_CLK_ERR),
	E_RTC_HW_REG_ACCESS			= E_RTC_HW_ERR0 | E_USER(RTC_REG_ACCESS_ERR),
	E_RTC_SWARG_TIME			= E_RTC_SW_ARG_ERR0 | E_USER(RTC_TIME_ERR),
	E_RTC_SWARG_ALARM			= E_RTC_SW_ARG_ERR0 | E_USER(RTC_ALARM_ERR),
	E_RTC_SWARG_INVAL			= E_RTC_SW_ARG_ERR0 | E_USER(RTC_INVAL_ARG),
	E_RTC_HW_IRQ				= E_RTC_HW_ERR0 | E_USER(RTC_IRQ_ERR),
};

#endif
