/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Copyright(c) 2020 - 2023 Allwinner Technology Co.,Ltd. All rights reserved. */
/*
 *
 * Copyright (c) 2021-2028 Allwinnertech Co., Ltd.
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * SUNXI Flash App Driver
 *
 * 2024.12.1  lujianliang <lujianliang@allwinnertech.com>
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/mtd/mtd.h>
#include <linux/slab.h>

#define SUNXI_FLASH_APP_NAME		"sunxi_flash_app"
#define SECURE_STORAGE_ITEMNUM		(8)
#define GPT_SIZE			(16 * 1024)

typedef enum {
	STORAGE_NAND = 0,
	STORAGE_SD,
	STORAGE_EMMC,
	STORAGE_NOR,
	STORAGE_EMMC3,
	STORAGE_SPI_NAND,
	STORAGE_SD1,
	STORAGE_EMMC0,
	STORAGE_UFS,
} SUNXI_BOOT_STORAGE;

struct rawpart_op_param {
	char name[16];
	loff_t offset;
	size_t len;
	__u64 user_data;
};

struct secstorage_op_param {
	int item;
	unsigned int *buf;
	unsigned int len;
};

#define RAWPART_READ		_IOR('M', 1, struct rawpart_op_param)
#define RAWPART_WRITE		_IOW('M', 2, struct rawpart_op_param)
#define SECURE_STORAGE_READ	_IO('M', 3)
#define SECURE_STORAGE_WRITE	_IO('M', 4)

struct mtd_info *__mtd_next_device(int i);
