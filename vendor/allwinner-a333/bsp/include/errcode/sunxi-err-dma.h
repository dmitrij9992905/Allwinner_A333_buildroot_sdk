/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright(c) 2020 - 2023 Allwinner Technology Co.,Ltd. All rights reserved. */
/*
 * Allwinner's errcode for audio
 *
 * Copyright (c) 2023, emma<liujuan1@allwinnertech.com>
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

#ifndef __SUNXI_ERR_DMA_H__
#define __SUNXI_ERR_DMA_H__

enum sunxi_err_dma_func {
	SW_DEP_MEM = 0x0,
	SYS_IRQ_GET,
	SYS_POOL_GET,
	ARG_CLK,
	ARG_CHANNEL,
};

enum sunxi_err_dma {
	/* E_DMA_HW_XXX	= E_DMA_HW_ERR0	| E_USER(XXX), */

	E_DMA_SW_DEP_MEM		= E_DMA_SW_DEP_ERR0	| E_USER(SW_DEP_MEM),
	E_DMA_SYS_IRQ_GET		= E_DMA_SW_SYS_ERR0	| E_USER(SYS_IRQ_GET),
	E_DMA_SYS_POOL_GET		= E_DMA_SW_SYS_ERR0	| E_USER(SYS_POOL_GET),
	E_DMA_ARG_CLK			= E_DMA_SW_ARG_ERR0	| E_USER(ARG_CLK),
	E_DMA_ARG_CHANNEL		= E_DMA_SW_ARG_ERR0	| E_USER(ARG_CHANNEL),
};

#endif
