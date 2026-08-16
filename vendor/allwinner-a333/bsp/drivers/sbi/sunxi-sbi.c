/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Copyright(c) 2020 - 2023 Allwinner Technology Co.,Ltd. All rights reserved. */
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/device.h>
#include <linux/miscdevice.h>
#include <asm/sbi.h>
#include <sunxi-sbi.h>

int sbi_efuse_write(phys_addr_t key_buf)
{
	struct sbiret ret;
	ret = sbi_ecall(SBI_EXT_SUNXI, SBI_EXT_SUNXI_EFUSE_WRITE, (unsigned long)key_buf, 0, 0, 0, 0, 0);
	if (ret.error) {
		return ret.error;
	}
	return ret.value;
}
EXPORT_SYMBOL_GPL(sbi_efuse_write);

static int __init sunxi_sbi_init(void)
{
	pr_info("sunxi sbi init success\n");
	return 0;
}

static void __exit sunxi_sbi_exit(void)
{
}

module_init(sunxi_sbi_init);
module_exit(sunxi_sbi_exit);
MODULE_LICENSE("GPL v2");
MODULE_VERSION("1.1.1");
MODULE_AUTHOR("shaosidi <shaosidi@allwinnertech.com>");
MODULE_DESCRIPTION("sunxi sbi driver");
