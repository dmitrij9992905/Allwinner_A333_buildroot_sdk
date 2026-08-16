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

#ifndef __SUNXI_SID_V1_H
#define __SUNXI_SID_V1_H

#include <linux/types.h>
#include <linux/errno.h>

/* About ChipID of version */
#define SUNXI_CHIP_REV(p, v)  (p + v)

#define SUNXI_CHIP_SUN8IW6   (0x16730000)
#define SUN8IW6P1_REV_A SUNXI_CHIP_REV(SUNXI_CHIP_SUN8IW6, 0x0000)
#define SUN8IW6P1_REV_B SUNXI_CHIP_REV(SUNXI_CHIP_SUN8IW6, 0x0001)

#define SUNXI_CHIP_SUN8IW7   (0x16800000)
#define SUN8IW7P1_REV_A SUNXI_CHIP_REV(SUNXI_CHIP_SUN8IW7, 0x0000)
#define SUN8IW7P1_REV_B SUNXI_CHIP_REV(SUNXI_CHIP_SUN8IW7, 0x0001)
#define SUN8IW7P2_REV_A SUNXI_CHIP_REV(SUNXI_CHIP_SUN8IW7, 0x0100)
#define SUN8IW7P2_REV_B SUNXI_CHIP_REV(SUNXI_CHIP_SUN8IW7, 0x0101)

#define SUNXI_CHIP_SUN8IW8P1 (0x16810000)
#define SUN8IW8P1_REV_A SUNXI_CHIP_REV(SUNXI_CHIP_SUN8IW8P1, 0x0000)
#define SUN8IW8P1_REV_B SUNXI_CHIP_REV(SUNXI_CHIP_SUN8IW8P1, 0x0001)

#define SUNXI_CHIP_SUN8IW11   (0x17010000)
#define SUN8IW11P1_REV_A SUNXI_CHIP_REV(SUNXI_CHIP_SUN8IW11, 0x0000)
#define SUN8IW11P2_REV_A SUNXI_CHIP_REV(SUNXI_CHIP_SUN8IW11, 0x0001)
#define SUN8IW11P3_REV_A SUNXI_CHIP_REV(SUNXI_CHIP_SUN8IW11, 0x0011)
#define SUN8IW11P4_REV_A SUNXI_CHIP_REV(SUNXI_CHIP_SUN8IW11, 0x0101)

#define SUNXI_CHIP_SUN8IW12   (0x17210000)
#define SUN8IW12P1_REV_A SUNXI_CHIP_REV(SUNXI_CHIP_SUN8IW12, 0x0000)

#define SUNXI_CHIP_SUN8IW15   (0x17550000)
#define SUN8IW15P1_REV_A SUNXI_CHIP_REV(SUNXI_CHIP_SUN8IW15, 0x0000)

#define SUNXI_CHIP_SUN8IW16   (0x18160000)
#define SUN8IW16P1_REV_A SUNXI_CHIP_REV(SUNXI_CHIP_SUN8IW16, 0x0000)
#define SUN8IW16P1_REV_B SUNXI_CHIP_REV(SUNXI_CHIP_SUN8IW16, 0x0001)

#define SUNXI_CHIP_SUN8IW19   (0x18170000)
#define SUN8IW19P1_REV_A SUNXI_CHIP_REV(SUNXI_CHIP_SUN8IW19, 0x0000)

#define SUNXI_CHIP_SUN8IW21   (0x18860000)
#define SUN8IW21P1_REV_A SUNXI_CHIP_REV(SUNXI_CHIP_SUN8IW21, 0x0000)

#define SUNXI_CHIP_SUN8IW17   (0x17080000)
#define SUN8IW17P1_REV_A SUNXI_CHIP_REV(SUNXI_CHIP_SUN8IW17, 0x0000)

#define SUNXI_CHIP_SUN8IW18   (0x18210000)
#define SUN8IW18P1_REV_A SUNXI_CHIP_REV(SUNXI_CHIP_SUN8IW18, 0x0000)

#define SUNXI_CHIP_SUN50IW1   (0x16890000)
#define SUN50IW1P1_REV_A	SUNXI_CHIP_REV(SUNXI_CHIP_SUN50IW1, 0x0)

#define SUNXI_CHIP_SUN50IW2   (0x17180000)
#define SUN50IW2P1_REV_A	SUNXI_CHIP_REV(SUNXI_CHIP_SUN50IW2, 0x0)

#define SUNXI_CHIP_SUN50IW3   (0x17190000)
#define SUN50IW3P1_REV_A	SUNXI_CHIP_REV(SUNXI_CHIP_SUN50IW3, 0x0)

#define SUNXI_CHIP_SUN50IW6   (0x17280000)
#define SUN50IW6P1_REV_A	SUNXI_CHIP_REV(SUNXI_CHIP_SUN50IW6, 0x0)

#define SUNXI_CHIP_SUN50IW9   (0x18230000)
#define SUN50IW9P1_REV_A	SUNXI_CHIP_REV(SUNXI_CHIP_SUN50IW9, 0x0)
#define SUN50IW9P1_REV_B	SUNXI_CHIP_REV(SUNXI_CHIP_SUN50IW9, 0x1)

#define SUNXI_CHIP_SUN50IW10  (0x18550000)
#define SUN50IW10P1_REV_A	SUNXI_CHIP_REV(SUNXI_CHIP_SUN50IW10, 0x0)

#define SUNXI_CHIP_SUN50IW11  (0x18510000)
#define SUN50IW11P1_REV_A	SUNXI_CHIP_REV(SUNXI_CHIP_SUN50IW11, 0x0)
#define SUN50IW11P1_REV_B	SUNXI_CHIP_REV(SUNXI_CHIP_SUN50IW11, 0x1)
#define SUN50IW11P1_REV_C	SUNXI_CHIP_REV(SUNXI_CHIP_SUN50IW11, 0x2)

#define SID_PRCTL		0x40
#define SID_RDKEY		0x60
#define SID_OP_LOCK		0xAC  /* In SID_PRCTL */

#define EFUSE_CHIPID_BASE	"allwinner,sunxi-chipid"
#define EFUSE_SID_BASE		"allwinner,sunxi-sid"
#define SRAM_CTRL_BASE		"allwinner,sram_ctrl"

#define EFUSE_RW_MAX_LEN        (64)
#define SUNXI_EFUSE_RAM_OFFSET	0x200

#define sunxi_efuse_read(key_name, read_buf) \
		sunxi_efuse_readn(key_name, read_buf, 1024)

/* The interface functions */
#if IS_ENABLED(CONFIG_AW_SID)
unsigned int sunxi_get_soc_ver(void);
int sunxi_get_sid_ver(u32 *ver);
unsigned int sunxi_get_soc_ver_from_reg(void);
unsigned int sunxi_get_platform_id(void);
int sunxi_get_soc_chipid(unsigned char *chipid);
int sunxi_get_soc_chipid_str(char *chipid);
int sunxi_get_soc_chipid_origin(char *chipid_origin);
int sunxi_get_soc_ft_zone_str(char *serial);
int sunxi_get_soc_rotpk_status_str(char *status);
int sunxi_get_pmu_chipid(unsigned char *chipid);
int sunxi_get_serial(unsigned char *serial);
unsigned int sunxi_get_soc_bin(void);
int sunxi_soc_is_secure(void);
s32 sunxi_get_platform(s8 *buf, s32 size);
s32 sunxi_efuse_readn(s8 *key_name, void *buf, u32 n);
int sunxi_get_module_param_from_sid(u32 *dst, u32 offset, u32 len);
unsigned int sunxi_get_soc_markid(void);
int sunxi_sid_sram_read32(const char *key, u32 *data);
int sunxi_sid_get_ecc_status(void);
int sunxi_get_soc_dvfs(u32 *dvfs);
#else
unsigned int __attribute__((weak)) sunxi_get_soc_ver(void) { return -ENOSYS; }
int __attribute__((weak)) sunxi_get_sid_ver(u32 *ver) { return -ENOSYS; }
unsigned int __attribute__((weak)) sunxi_get_soc_ver_from_reg(void) { return -ENOSYS; }
unsigned int __attribute__((weak)) sunxi_get_platform_id(void) { return -ENOSYS; }
int __attribute__((weak)) sunxi_get_soc_chipid(unsigned char *chipid) { return -ENOSYS; }
int __attribute__((weak)) sunxi_get_soc_chipid_str(char *chipid) { return -ENOSYS; }
int __attribute__((weak)) sunxi_get_soc_chipid_origin(char *chipid_origin) { return -ENOSYS; }
int __attribute__((weak)) sunxi_get_soc_ft_zone_str(char *serial) { return -ENOSYS; }
int __attribute__((weak)) sunxi_get_soc_rotpk_status_str(char *status) { return -ENOSYS; }
int __attribute__((weak)) sunxi_get_pmu_chipid(unsigned char *chipid) { return -ENOSYS; }
int __attribute__((weak)) sunxi_get_serial(unsigned char *serial) { return -ENOSYS; }
unsigned int __attribute__((weak)) sunxi_get_soc_bin(void) { return -ENOSYS; }
int __attribute__((weak)) sunxi_soc_is_secure(void) { return -ENOSYS; }
s32 __attribute__((weak)) sunxi_get_platform(s8 *buf, s32 size) { return -ENOSYS; }
s32 __attribute__((weak)) sunxi_efuse_readn(s8 *key_name, void *buf, u32 n) { return -ENOSYS; }
int __attribute__((weak)) sunxi_get_module_param_from_sid(u32 *dst, u32 offset, u32 len) { return -ENOSYS; }
unsigned int __attribute__((weak)) sunxi_get_soc_markid(void) { return -ENOSYS; }
int __attribute__((weak))sunxi_sid_sram_read32(const char *key, u32 *data) { return -ENOSYS; }
int __attribute__((weak)) sunxi_sid_get_ecc_status(void) { return -ENOSYS; }
int __attribute__((weak)) sunxi_get_soc_dvfs(u32 *dvfs) { return -ENOSYS; }
#endif  /* CONFIG_AW_SID */

#endif  /* __SUNXI_SID_V1_H */
