/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright(c) 2020 - 2023 Allwinner Technology Co.,Ltd. All rights reserved. */
/*
 * Allwinner's errcode for timer
 *
 * Copyright (c) 2023, emma<wangxinfeng@allwinnertech.com>
 *
 * This program is free software, you can redistribute it and/or modify
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

#ifndef __SUNXI_ERR_TIMER_H__
#define __SUNXI_ERR_TIMER_H__

enum sunxi_err_timer_func {
	TIMER_CLK_GET = 0x0,
	TIMER_CLK_INIT,
	TIMER_CLK_DEINIT,
};

enum sunxi_err_timer {
	E_TIMER_SWSYS_TIMER_CLK_GET		= E_TIMER_SW_SYS_ERR0 | E_USER(TIMER_CLK_GET),
	E_TIMER_SWSYS_TIMER_CLK_INIT		= E_TIMER_SW_SYS_ERR0 | E_USER(TIMER_CLK_INIT),
	E_TIMER_SWSYS_TIMER_CLK_DEINIT		= E_TIMER_SW_SYS_ERR0 | E_USER(TIMER_CLK_DEINIT),
};

#endif
