// SPDX-License-Identifier: GPL-2.0-or-later
/* Copyright(c) 2020 - 2023 Allwinner Technology Co.,Ltd. All rights reserved. */
/*
 * Allwinner SoCs display driver.
 *
 * Copyright (c) 2007-2017 Allwinnertech Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include "include.h"
#ifndef __DISP_DISPLAY_H__
#define __DISP_DISPLAY_H__

struct sunxi_lcd_fb_disp_dev_t {
	u32 print_level;
	u32 lcd_registered[3];
};

extern struct sunxi_lcd_fb_disp_dev_t g_lcd_fb_disp;

void LCD_FB_OPEN_FUNC(u32 screen_id, LCD_FUNC func, u32 delay);

void LCD_FB_CLOSE_FUNC(u32 screen_id, LCD_FUNC func, u32 delay);

s32 sunxi_lcd_fb_bsp_disp_get_screen_physical_width(u32 disp);

s32 sunxi_lcd_fb_bsp_disp_get_screen_physical_height(u32 disp);

s32 sunxi_lcd_fb_bsp_disp_get_screen_width(u32 disp);

s32 sunxi_lcd_fb_bsp_disp_get_screen_height(u32 disp);

/* lcd */
s32 sunxi_lcd_fb_bsp_disp_lcd_set_panel_funs(
	char *name, struct sunxi_lcd_fb_disp_lcd_panel_fun *lcd_cfg);

s32 sunxi_lcd_fb_bsp_disp_lcd_backlight_enable(u32 disp);

s32 sunxi_lcd_fb_bsp_disp_lcd_backlight_disable(u32 disp);

s32 sunxi_lcd_fb_bsp_disp_lcd_pwm_enable(u32 disp);

s32 sunxi_lcd_fb_bsp_disp_lcd_pwm_disable(u32 disp);

s32 sunxi_lcd_fb_bsp_disp_lcd_power_enable(u32 disp, u32 power_id);

s32 sunxi_lcd_fb_bsp_disp_lcd_power_disable(u32 disp, u32 power_id);

s32 sunxi_lcd_fb_bsp_disp_lcd_set_bright(u32 disp, u32 bright);

s32 sunxi_lcd_fb_bsp_disp_lcd_get_bright(u32 disp);

s32 sunxi_lcd_fb_bsp_disp_lcd_pin_cfg(u32 disp, u32 en);

s32 sunxi_lcd_fb_bsp_disp_lcd_gpio_set_value(u32 disp, u32 io_index, u32 value);

s32 sunxi_lcd_fb_bsp_disp_lcd_gpio_set_direction(u32 disp, u32 io_index,
						 u32 direction);

struct sunxi_lcd_fb_disp_lcd_flow *bsp_disp_lcd_get_open_flow(u32 disp);

struct sunxi_lcd_fb_disp_lcd_flow *bsp_disp_lcd_get_close_flow(u32 disp);

s32 sunxi_lcd_fb_bsp_disp_get_panel_info(
	u32 disp, struct sunxi_lcd_fb_disp_panel_para *info);

int sunxi_lcd_fb_bsp_disp_get_display_size(u32 disp, u32 *width, u32 *height);

void *sunxi_lcd_fb_dma_malloc(u32 num_bytes, void *phys_addr);

void sunxi_lcd_fb_dma_free(void *virt_addr, void *phys_addr, u32 num_bytes);

struct sunxi_lcd_fb_device *sunxi_sunxi_lcd_fb_device_get(int disp);

s32 sunxi_sunxi_lcd_fb_device_register(struct sunxi_lcd_fb_device *dispdev);

s32 sunxi_sunxi_lcd_fb_device_unregister(struct sunxi_lcd_fb_device *dispdev);

s32 sunxi_lcd_fb_bsp_disp_lcd_set_layer(u32 disp, struct fb_info *p_info);

s32 sunxi_lcd_fb_bsp_disp_lcd_blank(u32 disp, u32 en);

s32 sunxi_lcd_fb_bsp_disp_lcd_set_var(u32 disp, struct fb_info *p_info);

s32 sunxi_lcd_fb_bsp_disp_lcd_cmd_write(u32 screen_id, u8 cmd);

s32 sunxi_lcd_fb_bsp_disp_lcd_para_write(u32 screen_id, u8 para);

s32 sunxi_lcd_fb_bsp_disp_lcd_cmd_read(u32 screen_id, u8 cmd, u8 *rx_buf,
				       u8 len);

s32 sunxi_lcd_fb_bsp_disp_qspi_lcd_cmd_write(u32 screen_id, u8 cmd);

s32 sunxi_lcd_fb_bsp_disp_qspi_lcd_para_write(u32 screen_id, u8 cmd, u8 para);

s32 sunxi_lcd_fb_bsp_disp_qspi_lcd_multi_para_write(u32 screen_id, u8 cmd,
						    u8 *tx_buf, u32 len);

s32 sunxi_lcd_fb_bsp_disp_qspi_lcd_cmd_read(u32 screen_id, u8 cmd, u8 *rx_buf,
					    u8 len);

s32 sunxi_lcd_fb_bsp_disp_lcd_wait_for_vsync(u32 disp);

#endif
