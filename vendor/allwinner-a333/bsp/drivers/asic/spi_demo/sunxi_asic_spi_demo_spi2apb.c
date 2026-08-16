/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Copyright(c) 2020 - 2023 Allwinner Technology Co.,Ltd. All rights reserved. */
/*
* Allwinner 1935 spi2apb driver.
*
* Copyright(c) 2022-2027 Allwinnertech Co., Ltd.
*
* This file is licensed under the terms of the GNU General Public
* License version 2.  This program is licensed "as is" without any
* warranty of any kind, whether express or implied.
*/

#include <sunxi-log.h>

#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/ctype.h>
#include <linux/bitfield.h>
#include <linux/err.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/reset.h>
#include <linux/regulator/consumer.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/spi/spi.h>
#include <linux/spi/sunxi-spi.h>
#include <dt-bindings/spi/sunxi-spi.h>
#include "sunxi_asic_spi_demo.h"

static struct aw_spi2apb_data *global_aw_spi2apb_data;
struct aw_spi2apb_data {
	struct spi_device       *spi;
	struct device *dev;
	struct mutex            lock;
};

static int aw_spi2apb_read_data(struct aw_spi2apb_data *spi2apb_data, u16 reg, u32 *reg_result)
{
	unsigned char txbuf[8] = {0x00};
	unsigned char read_buf[8] = {0x00};
	u8 read_bit = 0x01;
	u32 reg_result_tmp = 0;
	int ret, i;
	struct spi_message	msg;
	struct spi_transfer	t;
	u16 opcode_addr = 0;
	u16 reg_tmp = reg;


	sunxi_debug(NULL, "aw_spi2apb_read_data target reg is 0x%08x\n", reg_tmp);

	reg = reg / 4;
	opcode_addr |= ((read_bit << 12) | (reg & 0xFFF));
	txbuf[0] |= ((opcode_addr >> 8) & 0xFF);
	txbuf[1] |= (opcode_addr & 0xFF);

	for (i = 0; i < 8; i++) {
		sunxi_debug(NULL, "aw_spi2apb_read_data txbuf %d is 0x%02x\n", i, txbuf[i]);
	}

	if (spi2apb_data == NULL) {
		sunxi_debug(NULL, " aw_spi2apb_read_data spi2apb_data is nullptr\n");
		return -ENOMEM;
	}

	mutex_lock(&spi2apb_data->lock);
	spi_message_init(&msg);
	memset(&t, 0, sizeof(t));
	t.tx_buf = txbuf;
	t.len = 8;
	t.rx_buf = read_buf;
	spi_message_add_tail(&t, &msg);
	ret = spi_sync(spi2apb_data->spi, &msg);
	if (ret) {
		sunxi_err(NULL, " aw_spi2apb_read_data spi2apb read reg data failed\n");
		return -1;
	}

	for (i = 0; i < 8; i++) {
		sunxi_debug(NULL, "aw_spi2apb_read_data read_buf %d is 0x%02x\n", i, read_buf[i]);
	}
	reg_result_tmp |= ((((u32)read_buf[4]) << 24) | (((u32)read_buf[5]) << 16) | (((u32)read_buf[6]) << 8) | ((u32)read_buf[7]));
	*reg_result = reg_result_tmp;
	mutex_unlock(&spi2apb_data->lock);
	return ret;
}

static int aw_spi2apb_write_data(struct aw_spi2apb_data *spi2apb_data, u16 reg, u32 reg_val)
{
	unsigned char txbuf[8] = {0x00};
	unsigned char rxbuf[8] = {0x00};
	int ret, i;
	struct spi_message	msg;
	struct spi_transfer	t;
	u16 opcode_addr = 0;
	u8 write_bit = 0x08;

	sunxi_debug(NULL, "aw_spi2apb_write_data target reg is 0x%08x\n", reg);
	sunxi_debug(NULL, "aw_spi2apb_write_data target reg_val is 0x%08x\n", reg_val);

	reg = reg / 4;
	opcode_addr |= ((write_bit << 12) | (reg & 0xFFF));
	txbuf[0] |= ((opcode_addr >> 8) & 0xFF);
	txbuf[1] |= (opcode_addr & 0xFF);
	txbuf[2] |= ((reg_val >> 24) & 0xFF);
	txbuf[3] |= ((reg_val >> 16) & 0xFF);
	txbuf[4] |= ((reg_val >> 8) & 0xFF);
	txbuf[5] |= (reg_val & 0xFF);

	for (i = 0; i < 8; i++) {
		sunxi_debug(NULL, "aw_spi2apb_write_data txbuf %d is 0x%02x\n", i, txbuf[i]);
	}

	mutex_lock(&spi2apb_data->lock);
	spi_message_init(&msg);
	memset(&t, 0, sizeof(t));
	t.tx_buf = txbuf;
	t.len = 8;
	t.rx_buf = rxbuf;
	spi_message_add_tail(&t, &msg);
	ret = spi_sync(spi2apb_data->spi, &msg);
	if (ret) {
		sunxi_debug(NULL, " aw_spi2apb_write_data spi2apb write reg data failed\n");
		return -1;
	}

	mutex_unlock(&spi2apb_data->lock);
	return ret;
}

static ssize_t sunxi_spi2apb_write_store(struct device *dev,
				struct device_attribute *attr, const char *buf, size_t count)
{
	struct aw_spi2apb_data *spi2apb_data = dev_get_drvdata(dev);
	u16 reg = 0;
	int ret;
	unsigned int reg_val = 0;
	unsigned int temp1, temp2;

	ret = sscanf(buf, "0x%x,0x%x", &temp1, &temp2);
	if (ret != 2) {
		sunxi_err(NULL, "Invalid format, expected: 0x00,0x12212235\n");
		return -EINVAL;
	}

	reg = temp1;
	reg_val = temp2;

	aw_spi2apb_write_data(spi2apb_data, reg, reg_val);
	return count;
}

static ssize_t sunxi_spi2apb_read_store(struct device *dev,
				struct device_attribute *attr, const char *buf, size_t count)
{
	struct aw_spi2apb_data *spi2apb_data = dev_get_drvdata(dev);
	u16 reg;
	u32 reg_result;
	int err;

	err = kstrtou16(buf, 16, &reg);
	if (err) {
		sunxi_err(NULL, "String conversion failed!\n");
		return -ERANGE;
	}
	aw_spi2apb_read_data(spi2apb_data, reg, &reg_result);
	sunxi_info(NULL, "aw_spi2apb_read_data reg 0x%04x val is 0x%08x\n", reg, reg_result);
	return count;
}

static struct device_attribute sunxi_spi2apb_debug_attr[] = {
	__ATTR(writeReg_spi2apb, S_IWUSR, NULL, sunxi_spi2apb_write_store),
	__ATTR(readReg_spi2apb, S_IWUSR, NULL, sunxi_spi2apb_read_store),
};

static void sunxi_spi2apb_create_sysfs(struct aw_spi2apb_data *spi2apb_data)
{
	int i;
	struct spi_device *spi = spi2apb_data->spi;

	for (i = 0; i < ARRAY_SIZE(sunxi_spi2apb_debug_attr); i++)
		device_create_file(&spi->dev, &sunxi_spi2apb_debug_attr[i]);
}

static void sunxi_spi2apb_remove_sysfs(struct spi_device *spi)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(sunxi_spi2apb_debug_attr); i++)
		device_remove_file(&spi->dev, &sunxi_spi2apb_debug_attr[i]);
}

int sunxi_spi2apb_write_reg_value(u16 reg, u32 reg_val)
{
	return aw_spi2apb_write_data(global_aw_spi2apb_data, reg, reg_val);
}
EXPORT_SYMBOL_GPL(sunxi_spi2apb_write_reg_value);

u32 sunxi_spi2apb_read_reg_value(u16 reg)
{
	u32 reg_result = 0;
	if (aw_spi2apb_read_data(global_aw_spi2apb_data, reg, &reg_result)) {
		sunxi_err(NULL, "spi2apb read reg value error");
		return -EPERM;
	}
	return reg_result;
}
EXPORT_SYMBOL_GPL(sunxi_spi2apb_read_reg_value);

static int aw_spi2apb_probe(struct spi_device *spi)
{
	struct aw_spi2apb_data *spi2apb_data;

	spi2apb_data = devm_kzalloc(&spi->dev, sizeof(*spi2apb_data), GFP_KERNEL);
	if (!spi2apb_data)
		return -ENOMEM;

	spi2apb_data->spi = spi;
	spi2apb_data->dev = &spi->dev;
	mutex_init(&spi2apb_data->lock);
	sunxi_spi2apb_create_sysfs(spi2apb_data);
	dev_set_drvdata(spi2apb_data->dev, spi2apb_data);
	global_aw_spi2apb_data = spi2apb_data;
	sunxi_info(NULL, "aw_spi2apb driver probe success!!!\n");

	return 0;
}

static int aw_spi2apb_remove(struct spi_device *spi)
{
	sunxi_spi2apb_remove_sysfs(spi);
	return 0;
}

static const struct of_device_id aw_spi2apb_of_ids[] = {
	{ .compatible = "allwinner, aw-spi2apb" },
	{ /* sentinel */ },
};

static struct spi_driver aw_spi2apb_drv = {
	.driver = {
		.name = "allwinner, aw-spi2apb",
		.of_match_table = of_match_ptr(aw_spi2apb_of_ids),
	},
	.probe = aw_spi2apb_probe,
	.remove = aw_spi2apb_remove,
};
module_spi_driver(aw_spi2apb_drv);

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("liujing <luiujingswc@allwinnertech.com>");
MODULE_DESCRIPTION("Allwinner's 1935 spi2apb controll localbus register");
MODULE_VERSION("1.0.0");
