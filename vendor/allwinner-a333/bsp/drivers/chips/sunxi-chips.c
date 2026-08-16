/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Copyright(c) 2020 - 2023 Allwinner Technology Co.,Ltd. All rights reserved. */
//#define DEBUG
#include <linux/module.h>
#include <sunxi-chips.h>
#include <sunxi-log.h>
#include <asm/io.h>

int sunxi_chip_alter_version(void)
{
#if IS_ENABLED(CONFIG_ARCH_SUN300IW1)
	#define PRCM_ROM_REG		(0x4A000040)
	u32 val;
	static int ver = -1;
	void __iomem *rom_reg;

	if (ver == -1) {
		rom_reg = ioremap(PRCM_ROM_REG, 0x4);
		val = readl(rom_reg);
		ver = ((val == 0x2) ? SUNXI_CHIP_ALTER_VERSION_V821 : SUNXI_CHIP_ALTER_VERSION_V821B);
		iounmap(rom_reg);
	}

	return ver;
#else
	return SUNXI_CHIP_ALTER_VERSION_DEFAULT;
#endif
}
EXPORT_SYMBOL_GPL(sunxi_chip_alter_version);

static int __init sunxi_chips_init(void)
{
	return 0;
}

static void __exit sunxi_chips_exit(void)
{
}

early_initcall(sunxi_chips_init);
module_exit(sunxi_chips_exit);

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Martin <wuyan@allwinnertech.com>");
MODULE_DESCRIPTION("sunxi chip level APIs");
MODULE_VERSION("0.0.1");
