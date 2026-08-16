/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Copyright(c) 2020 - 2023 Allwinner Technology Co.,Ltd. All rights reserved. */
/*
 * SUNXI ASIC SPI DEMO INCLUDE
 *
 */

#ifndef _SUNXI_ASIC_SPI_DEMO
#define _SUNXI_ASIC_SPI_DEMO

extern int sunxi_spi2apb_write_reg_value(u16 reg, u32 reg_val);
extern u32 sunxi_spi2apb_read_reg_value(u16 reg);
extern int sunxi_inovance_write_reg_value_16bit(u16 reg, u32 reg_val);
extern u32 sunxi_inovance_read_reg_value_16bit(u16 reg);
extern int sunxi_inovance_write_reg_value_32bit(u16 reg, u32 reg_val);
extern u32 sunxi_inovance_read_reg_value_32bit(u16 reg);

#endif