/* SPDX-License-Identifier:	GPL-2.0+
 *
 * (C) Copyright 2022-2025
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 *
 * lujianliang <lujianliang@allwinnertech.com>
 */

#ifndef __SUNXI_CHIPS_H__
#define __SUNXI_CHIPS_H__

#define SUNXI_CHIP_ALTER_VERSION_DEFAULT	0
#define SUNXI_CHIP_ALTER_VERSION_V821   	1
#define SUNXI_CHIP_ALTER_VERSION_V821B  	2

#if IS_ENABLED(CONFIG_SUNXI_CHIPS)

int sunxi_chip_alter_version(void);

#else

__weak int sunxi_chip_alter_version(void) { return SUNXI_CHIP_ALTER_VERSION_DEFAULT; }

#endif  /* CONFIG_AW_CHIPS */

#endif  /* __SUNXI_CHIPS_H__ */

