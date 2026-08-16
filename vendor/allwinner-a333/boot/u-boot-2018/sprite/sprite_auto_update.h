/*
 * (C) Copyright 2013-2016
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */
#ifndef __SPRITE_AUTO_UPDATE_H__
#define __SPRITE_AUTO_UPDATE_H__

extern loff_t fat_fs_read(const char *filename, void *buf, uint offset, int len);

#define SUNXI_AUTO_UPDATE_SKIP				(0)
#define SUNXI_AUTO_UPDATE_ALWAYS			(1)
#define SUNXI_AUTO_UPDATE_GPADC_KEY			(2)

#endif /* __SPRITE_AUTO_UPDATE_H__ */
