/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Copyright(c) 2020 - 2023 Allwinner Technology Co.,Ltd. All rights reserved. */
/*
 * Copyright(c) 2014-2016 Allwinnertech Co., Ltd.
 *         http://www.allwinnertech.com
 *
 * Author: sunny <sunny@allwinnertech.com>
 *
 * allwinner sunxi soc chip version and chip id manager.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef __SUNXI_SID_V2_H
#define __SUNXI_SID_V2_H

#include <linux/types.h>
#include <linux/errno.h>

/* The interface functions */
#if IS_ENABLED(CONFIG_AW_SID_V2)
unsigned int sunxi_get_soc_ver(void);
int sunxi_get_sid_ver(u32 *ver);
unsigned int sunxi_get_platform_id(void);
int sunxi_get_soc_chipid(unsigned char *chipid);
int sunxi_get_soc_chipid_str(char *chipid);
int sunxi_get_soc_chipid_origin(char *chipid_origin);
int sunxi_get_soc_ft_zone_str(char *serial);
int sunxi_get_soc_rotpk_status_str(char *status);
int sunxi_get_serial(unsigned char *serial);
unsigned int sunxi_get_soc_bin(void);
int sunxi_soc_is_secure(void);
s32 sunxi_get_platform(s8 *buf, s32 size);
s32 sunxi_efuse_readn(s8 *key_name, void *buf, u32 n);
s32 sunxi_efuse_read(s8 *key_name, void *buf);
int sunxi_get_module_param_from_sid(u32 *dst, u32 offset, u32 len);
unsigned int sunxi_get_soc_markid(void);
int sunxi_sid_sram_read32(const char *key, u32 *data);
int sunxi_sid_get_ecc_status(void);
int sunxi_get_soc_dvfs(u32 *dvfs);
#else
unsigned int __attribute__((weak)) sunxi_get_soc_ver(void) { return -ENOSYS; }
int __attribute__((weak)) sunxi_get_sid_ver(u32 *ver) { return -ENOSYS; }
unsigned int __attribute__((weak)) sunxi_get_platform_id(void) { return -ENOSYS; }
int __attribute__((weak)) sunxi_get_soc_chipid(unsigned char *chipid) { return -ENOSYS; }
int __attribute__((weak)) sunxi_get_soc_chipid_str(char *chipid) { return -ENOSYS; }
int __attribute__((weak)) sunxi_get_soc_chipid_origin(char *chipid_origin) { return -ENOSYS; }
int __attribute__((weak)) sunxi_get_soc_ft_zone_str(char *serial) { return -ENOSYS; }
int __attribute__((weak)) sunxi_get_soc_rotpk_status_str(char *status) { return -ENOSYS; }
int __attribute__((weak)) sunxi_get_serial(unsigned char *serial) { return -ENOSYS; }
unsigned int __attribute__((weak)) sunxi_get_soc_bin(void) { return -ENOSYS; }
int __attribute__((weak)) sunxi_soc_is_secure(void) { return -ENOSYS; }
s32 __attribute__((weak)) sunxi_get_platform(s8 *buf, s32 size) { return -ENOSYS; }
s32 __attribute__((weak)) sunxi_efuse_readn(s8 *key_name, void *buf, u32 n) { return -ENOSYS; }
s32 __attribute__((weak)) sunxi_efuse_read(s8 *key_name, void *buf) { return -ENOSYS; }
int __attribute__((weak)) sunxi_get_module_param_from_sid(u32 *dst, u32 offset, u32 len) { return -ENOSYS; }
unsigned int __attribute__((weak)) sunxi_get_soc_markid(void) { return -ENOSYS; }
int __attribute__((weak))sunxi_sid_sram_read32(const char *key, u32 *data) { return -ENOSYS; }
int __attribute__((weak)) sunxi_sid_get_ecc_status(void) { return -ENOSYS; }
int __attribute__((weak)) sunxi_get_soc_dvfs(u32 *dvfs) { return -ENOSYS; }
#endif  /* CONFIG_AW_SID_V2 */

#endif  /* __SUNXI_SID_H */
