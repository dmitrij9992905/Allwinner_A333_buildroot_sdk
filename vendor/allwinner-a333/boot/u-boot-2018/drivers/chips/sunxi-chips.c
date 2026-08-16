/* SPDX-License-Identifier: GPL-2.0+
 *
 * (C) Copyright 2022-2025
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 *
 * lujianliang <lujianliang@allwinnertech.com>
 */

#include <sunxi-chips.h>

int sunxi_chip_alter_version(void)
{
#if IS_ENABLED(CONFIG_ARCH_SUN300IW1)
	#define PRCM_ROM_REG		(0x4A000040)
	u32 val;
	static int ver = -1;

	if (ver == -1) {
		val = readl(PRCM_ROM_REG);
		ver = ((val == 0x2) ? SUNXI_CHIP_ALTER_VERSION_V821 : SUNXI_CHIP_ALTER_VERSION_V821B);
	}

	return ver;
#else
	return SUNXI_CHIP_ALTER_VERSION_DEFAULT;
#endif
}

