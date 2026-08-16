/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright(c) 2020 - 2023 Allwinner Technology Co.,Ltd. All rights reserved. */
/*
 * Allwinner's errcode for spi
 *
 * Copyright (c) 2023, lujianliang <lujianliang@allwinnertech.com>
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

#ifndef __SUNXI_ERR_SPI_NG_H__
#define __SUNXI_ERR_SPI_NG_H__

enum sunxi_err_spi_ng_func {
	SPI_NG_MOD_INIT = 0x0,
	SPI_NG_MOD_EXIT,

	SPI_NG_CLK_INIT,
	SPI_NG_CLK_EN,
	SPI_NG_CLK_SET,

	SPI_NG_IRQ_GET,
	SPI_NG_IRQ_HANDLER,

	SPI_NG_DMA_INIT,
	SPI_NG_DMA_MAP_SG,

	SPI_NG_HW_FIFO,
	SPI_NG_HW_RESET,

	SPI_NG_PARAMS, /* including system call parameter, DTS parameters */
	SPI_NG_PLAT_DEV,
	SPI_NG_MEMORY,
	SPI_NG_STANDBY,

	SPI_NG_TRANSFER,
	SPI_NG_TIMEOUT,

	SPI_NG_E_SWARG_END = 0xFFFF,
};

enum sunxi_err_spi_ng {
	/* E_SPI_NG_HW_XXX	= E_SPI_NG_HW_ERR0	| E_USER(XXX), */

	E_SPI_NG_HW_FIFO		= E_SPI_NG_HW_ERR0	| E_USER(SPI_NG_HW_FIFO),
	E_SPI_NG_HW_RESET		= E_SPI_NG_HW_ERR0	| E_USER(SPI_NG_HW_RESET),
	E_SPI_NG_HW_IRQ_HANDLER		= E_SPI_NG_HW_ERR0	| E_USER(SPI_NG_IRQ_HANDLER),
	E_SPI_NG_SW_DEP_CLK_INTI	= E_SPI_NG_SW_DEP_ERR0	| E_USER(SPI_NG_CLK_INIT),
	E_SPI_NG_SW_DEP_CLK_EN		= E_SPI_NG_SW_DEP_ERR0	| E_USER(SPI_NG_CLK_EN),
	E_SPI_NG_SW_DEP_CLK_SET		= E_SPI_NG_SW_DEP_ERR0	| E_USER(SPI_NG_CLK_SET),
	E_SPI_NG_SW_DEP_DMA_INIT	= E_SPI_NG_SW_DEP_ERR0	| E_USER(SPI_NG_DMA_INIT),
	E_SPI_NG_SW_DEP_IRQ_GET		= E_SPI_NG_SW_DEP_ERR0	| E_USER(SPI_NG_IRQ_GET),
	E_SPI_NG_SW_DEP_DMA_MAP_SG	= E_SPI_NG_SW_DEP_ERR0	| E_USER(SPI_NG_DMA_MAP_SG),
	E_SPI_NG_SW_ARG_PARAMS		= E_SPI_NG_SW_ARG_ERR0	| E_USER(SPI_NG_PARAMS),
	E_SPI_NG_SW_SYS_MOD_INIT	= E_SPI_NG_SW_SYS_ERR0  | E_USER(SPI_NG_MOD_INIT),
	E_SPI_NG_SW_SYS_MOD_EXIT	= E_SPI_NG_SW_SYS_ERR0  | E_USER(SPI_NG_MOD_EXIT),
	E_SPI_NG_SW_SYS_TRANSFER	= E_SPI_NG_SW_SYS_ERR0  | E_USER(SPI_NG_TRANSFER),
	E_SPI_NG_SW_SYS_TIMEOUT		= E_SPI_NG_SW_SYS_ERR0  | E_USER(SPI_NG_TIMEOUT),
	E_SPI_NG_SW_SYS_STANDBY		= E_SPI_NG_SW_SYS_ERR0  | E_USER(SPI_NG_STANDBY),
	E_SPI_NG_SW_SYS_PLAT_DEV	= E_SPI_NG_SW_SYS_ERR0  | E_USER(SPI_NG_PLAT_DEV),
	E_SPI_NG_SW_SYS_MEMORY		= E_SPI_NG_SW_SYS_ERR0  | E_USER(SPI_NG_MEMORY),
};

#endif
