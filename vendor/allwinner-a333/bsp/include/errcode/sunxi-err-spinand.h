/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright(c) 2020 - 2023 Allwinner Technology Co.,Ltd. All rights reserved. */
/*
 * Allwinner's errcode for spinand
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

#ifndef __SUNXI_ERR_SPINAND_H__
#define __SUNXI_ERR_SPINAND_H__

enum sunxi_err_spinand_func {
	SPINAND_MOD_INIT = 0x0,

	SPINAND_SPIC,
	SPINAND_PARAMS, /* including system call parameter, DTS parameters */
	SPINAND_MEMORY,
	SPINAND_STANDBY,
	SPINAND_SYS_CALL,

	SPINAND_ERASE,
	SPINAND_READ,
	SPINAND_WRITE,
	SPINAND_ECC,
	SPINAND_SECURE,
	SPINAND_OP_STATUS,
	SPINAND_ID,

	SPINAND_E_SWARG_END = 0xFFFF,
};

enum sunxi_err_spinand {
	/* E_SPINAND_HW_XXX	= E_SPINAND_HW_ERR0	| E_USER(XXX), */

	E_SPINAND_SW_DEP_SPIC		= E_SPINAND_SW_DEP_ERR0	| E_USER(SPINAND_SPIC),
	E_SPINAND_SW_ARG_PARAMS		= E_SPINAND_SW_ARG_ERR0	| E_USER(SPINAND_PARAMS),
	E_SPINAND_SW_SYS_MOD_INIT	= E_SPINAND_SW_SYS_ERR0	| E_USER(SPINAND_MOD_INIT),
	E_SPINAND_SW_SYS_ERASE		= E_SPINAND_SW_SYS_ERR0 | E_USER(SPINAND_ERASE),
	E_SPINAND_SW_SYS_READ		= E_SPINAND_SW_SYS_ERR0 | E_USER(SPINAND_READ),
	E_SPINAND_SW_SYS_WRITE		= E_SPINAND_SW_SYS_ERR0 | E_USER(SPINAND_WRITE),
	E_SPINAND_SW_SYS_ECC		= E_SPINAND_SW_SYS_ERR0 | E_USER(SPINAND_ECC),
	E_SPINAND_SW_SYS_OP_STATUS	= E_SPINAND_SW_SYS_ERR0 | E_USER(SPINAND_OP_STATUS),
	E_SPINAND_SW_SYS_ID		= E_SPINAND_SW_SYS_ERR0 | E_USER(SPINAND_ID),
	E_SPINAND_SW_SYS_STANDBY	= E_SPINAND_SW_SYS_ERR0 | E_USER(SPINAND_STANDBY),
	E_SPINAND_SW_SYS_SECURE		= E_SPINAND_SW_SYS_ERR0 | E_USER(SPINAND_SECURE),
	E_SPINAND_SW_SYS_MEMORY		= E_SPINAND_SW_SYS_ERR0 | E_USER(SPINAND_MEMORY),
	E_SPINAND_SW_SYS_CALL		= E_SPINAND_SW_SYS_ERR0 | E_USER(SPINAND_SYS_CALL),
};

#endif
