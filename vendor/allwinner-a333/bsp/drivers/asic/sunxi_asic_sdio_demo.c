/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Copyright(c) 2020 - 2023 Allwinner Technology Co.,Ltd. All rights reserved. */
/*
* Sunxi SD/MMC host driver
*
* Copyright (C) 2015 AllWinnertech Ltd.
* Author: lixiang <lixiang@allwinnertech>
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 as
* published by the Free Software Foundation.
*
* This program is distributed "as is" WITHOUT ANY WARRANTY of any
* kind, whether express or implied; without even the implied warranty
* of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*/


#include <linux/clk.h>
#include <linux/reset/sunxi.h>

#include <linux/gpio.h>
#include <linux/platform_device.h>
#include <linux/spinlock.h>
#include <linux/scatterlist.h>
#include <linux/dma-mapping.h>
#include <linux/slab.h>
#include <linux/reset.h>

#include <linux/of_address.h>
#include <linux/of_gpio.h>
#include <linux/of_platform.h>
#include <linux/stat.h>

#include <linux/mmc/core.h>
#include <linux/mmc/host.h>
#include <linux/mmc/card.h>
#include <linux/delay.h>

static struct device_attribute sdio_test_read;
static struct device_attribute sdio_test_write;

int __mmc_claim_host(struct mmc_host *host, struct mmc_ctx *ctx,
		     atomic_t *abort);
void mmc_release_host(struct mmc_host *host);

extern int mmc_io_rw_extended(struct mmc_card *card, int write, unsigned fn,
	unsigned addr, int incr_addr, u8 *buf, unsigned blocks, unsigned blksz);

/**
 *	sunxi_asic_mmc_claim_host - exclusively claim a bus for RW asic reg
 */
void sunxi_asic_mmc_claim_host(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct mmc_host	*mmc = platform_get_drvdata(pdev);
	__mmc_claim_host(mmc, NULL, NULL);
}
EXPORT_SYMBOL_GPL(sunxi_asic_mmc_claim_host);

/**
 *	sunxi_asic_mmc_claim_host - exclusively release a bus for RW asic reg
 */
void sunxi_asic_mmc_release_host(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct mmc_host	*mmc = platform_get_drvdata(pdev);
	mmc_release_host(mmc);
}
EXPORT_SYMBOL_GPL(sunxi_asic_mmc_release_host);

/**
 *	sunxi_asic_sdio_read_reg - read the value from the address
 *	@dev: sdio device
 *	@addr: the address need read
 *	@buf: buffer to store the read data
 *	@fn: function num
 *
 */
int sunxi_asic_sdio_read_reg(struct device *dev, unsigned addr, u8 *buf, unsigned fn)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct mmc_host	*mmc = platform_get_drvdata(pdev);
	return mmc_io_rw_extended(mmc->card, 0, fn, addr, 1, buf, 0, 0x4);
}
EXPORT_SYMBOL_GPL(sunxi_asic_sdio_read_reg);

/**
 *	sunxi_asic_sdio_write_reg - write the value to the address
 *	@dev: sdio device
 *	@addr: the address need read
 *	@buf: buffer to store the read data
 *	@fn: function num
 *
 */
int sunxi_asic_sdio_write_reg(struct device *dev, unsigned addr, u8 *buf, unsigned fn)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct mmc_host	*mmc = platform_get_drvdata(pdev);
	return mmc_io_rw_extended(mmc->card, 1, fn, addr, 1, buf, 0, 0x4);
}
EXPORT_SYMBOL_GPL(sunxi_asic_sdio_write_reg);

/**
 * __parse_dump_str - parse the input string for dump attri.
 * @buf: the input string, eg: "0x01c20000,0x01c20300".
 * @size: buf size.
 * @start: store the start reg's addr parsed from buf, eg 0x01c20000.
 * @end: store the end reg's addr parsed from buf, eg 0x01c20300.
 *
 * return 0 if success, otherwise failed.
 */
static int __parse_dump_str(const char *buf, size_t size,
			    unsigned long *start, unsigned long *end)
{
	char *ptr = NULL;
	char *ptr2 = (char *)buf;
	int ret = 0, times = 0;

	/* Support single address mode, some time it haven't ',' */
next:
	/*
	 * Default dump only one register(*start =*end).
	 * If ptr is not NULL, we will cover the default value of end.
	 */
	if (times == 1)
		*start = *end;

	if (!ptr2 || (ptr2 - buf) >= size)
		goto out;

	ptr = ptr2;
	ptr2 = strnchr(ptr, size - (ptr - buf), ',');
	if (ptr2) {
		*ptr2 = '\0';
		ptr2++;
	}

	ptr = strim(ptr);
	if (!strlen(ptr))
		goto next;

	ret = kstrtoul(ptr, 16, end);
	if (!ret) {
		times++;
		goto next;
	} else
		printk("String syntax errors: \"%s\"\n", ptr);

out:
	return ret;
}

static ssize_t
sunxi_sdio_test_read(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t count)
{
	u32 read_buf;
	unsigned long read_addr = 0;

	if (__parse_dump_str(buf, count, &read_addr, &read_addr)) {
		printk("%s,%d err, invalid para!\n", __func__, __LINE__);
		return -1;
	}
	printk("read_addr = 0x%lx \n", read_addr);
	if (read_addr > 0x1ffff) {
		printk("%s,%d wrwrite_addr exceeds max. write_addr = -x%lx\n", __func__, __LINE__, read_addr);
		return -1;
	}

	sunxi_asic_mmc_claim_host(dev);
	//read reg
	sunxi_asic_sdio_read_reg(dev, (int)read_addr, (u8 *)&read_buf, 0);
	printk("reg0x%lx = 0x%x\n", read_addr, read_buf);

	sunxi_asic_mmc_release_host(dev);

	return count;
}

static ssize_t
sunxi_sdio_test_write(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t count)
{
	unsigned long write_addr = 0;
	unsigned long write_data = 0;

	if (__parse_dump_str(buf, count, &write_addr, &write_data)) {
		printk("%s,%d err, invalid para!\n", __func__, __LINE__);
		return -1;
	}
	printk("write_addr = 0x%lx write_data = 0x%lx\n", write_addr, write_data);
	if (write_addr > 0x1ffff) {
		printk("%s,%d wrwrite_addr exceeds max. write_addr = -x%lx\n", __func__, __LINE__, write_addr);
		return -1;
	}

	sunxi_asic_mmc_claim_host(dev);
	//write reg
	sunxi_asic_sdio_write_reg(dev, (int)write_addr, (u8 *)&write_data, 1);

	sunxi_asic_mmc_release_host(dev);

	return count;
}

int sunxi_asic_mmc_create_sys_fs(struct platform_device *pdev)
{
	int ret;

	sdio_test_read.show = NULL;
	sdio_test_read.store = sunxi_sdio_test_read;
	sysfs_attr_init(&(sdio_test_read.attr));
	sdio_test_read.attr.name = "sunxi_asic_sdio_read_reg";
	sdio_test_read.attr.mode = S_IWUSR;
	ret = device_create_file(&pdev->dev, &sdio_test_read);
	if (ret)
		return ret;

	sdio_test_write.show = NULL;
	sdio_test_write.store = sunxi_sdio_test_write;
	sysfs_attr_init(&(sdio_test_write.attr));
	sdio_test_write.attr.name = "sunxi_asic_sdio_write_reg";
	sdio_test_write.attr.mode = S_IWUSR;
	ret = device_create_file(&pdev->dev, &sdio_test_write);
	if (ret)
		return ret;

	return ret;
}

void sunxi_asic_mmc_remove_sys_fs(struct platform_device *pdev)
{
	device_remove_file(&pdev->dev, &sdio_test_read);
	device_remove_file(&pdev->dev, &sdio_test_write);
}
