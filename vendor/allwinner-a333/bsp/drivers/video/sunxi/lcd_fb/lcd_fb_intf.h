// SPDX-License-Identifier: GPL-2.0-or-later
/* Copyright(c) 2020 - 2023 Allwinner Technology Co.,Ltd. All rights reserved. */
/*
 * Allwinner SoCs display driver.
 *
 * Copyright (C) 2016 Allwinner.
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2.  This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */
#include "include.h"

#ifndef _LCD_FB_INTF_
#define _LCD_FB_INTF_

struct disp_gpio_info {
	unsigned gpio;
	char name[32];
	int value;
};

#define DISP_IRQ_RETURN IRQ_HANDLED
#define DISP_PIN_STATE_ACTIVE "active"
#define DISP_PIN_STATE_SLEEP "sleep"

int sunxi_lcd_fb_register_irq(u32 irq_num, u32 flags, void *handler,
			      void *p_arg, u32 data_size, u32 prio);

void sunxi_lcd_fb_unregister_irq(u32 irq_num, void *handler, void *p_arg);

void sunxi_lcd_fb_disable_irq(u32 irq_num);

void sunxi_lcd_fb_enable_irq(u32 irq_num);

/* returns: 0:invalid, 1: int; 2:str, 3: gpio */
int sunxi_lcd_fb_script_get_item(struct device_node *node, char *sub_name, int value[],
				 int count);

int sunxi_lcd_fb_get_ic_ver(void);

int sunxi_lcd_fb_gpio_request(struct disp_gpio_info *gpio_info);

int sunxi_lcd_fb_gpio_release(struct disp_gpio_info *gpio_info);

/* direction: 0:input, 1:output */
int sunxi_lcd_fb_gpio_set_direction(u32 p_handler, u32 direction,
				    const char *gpio_name);

int sunxi_lcd_fb_gpio_get_value(u32 p_handler, const char *gpio_name);

int sunxi_lcd_fb_gpio_set_value(u32 p_handler, u32 value_to_gpio,
				const char *gpio_name);

int sunxi_lcd_fb_pin_set_state(struct sunxi_lcd_fb_device *lcd, char *name);

int sunxi_lcd_fb_power_enable(char *name);

int sunxi_lcd_fb_power_disable(char *name);

void *sunxi_lcd_fb_malloc(u32 size);

uintptr_t sunxi_lcd_fb_pwm_request(u32 pwm_id);

int sunxi_lcd_fb_pwm_free(uintptr_t p_handler);

int sunxi_lcd_fb_pwm_enable(uintptr_t p_handler);

int sunxi_lcd_fb_pwm_disable(uintptr_t p_handler);

int sunxi_lcd_fb_pwm_config(uintptr_t p_handler, int duty_ns, int period_ns);

int sunxi_lcd_fb_pwm_set_polarity(uintptr_t p_handler, int polarity);

s32 sunxi_lcd_fb_disp_delay_ms(u32 ms);

s32 sunxi_lcd_fb_disp_delay_us(u32 us);

#endif
