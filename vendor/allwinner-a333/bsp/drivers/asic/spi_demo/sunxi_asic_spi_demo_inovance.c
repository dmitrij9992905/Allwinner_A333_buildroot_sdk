/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Copyright(c) 2020 - 2023 Allwinner Technology Co.,Ltd. All rights reserved. */
/*
* Allwinner 1935 spi_inovance driver.
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

static struct aw_spi_inovance_data *global_aw_spi_inovance_data;
struct aw_spi_inovance_data {
	struct spi_device       *spi;
	struct device *dev;
	struct mutex            lock;
};

static int aw_spi_inovance_read_data(struct aw_spi_inovance_data *spi_inovance_data, u16 reg, u32 *reg_result, bool is_32bit)
{
	unsigned char txbuf[7] = {0x00};
	unsigned char read_buf[7] = {0x00};
	u8 read_bit = 0x02;
	u32 reg_result_tmp = 0;
	int ret, i;
	struct spi_message	msg;
	struct spi_transfer	t;
	u16 opcode_addr = 0;

	if (spi_inovance_data == NULL) {
		sunxi_err(NULL, " aw_spi_inovance_read_data spi_inovance_data is nullptr\n");
		return -ENOMEM;
	}

	sunxi_debug(NULL, "aw_spi_inovance_read_data target reg is 0x%08x\n", reg);

	mutex_lock(&spi_inovance_data->lock);
	if (is_32bit) {
		opcode_addr |= (((reg & 0x1FFF) << 3) | read_bit);
		txbuf[0] |= ((opcode_addr >> 8) & 0xFF);
		txbuf[1] |= (opcode_addr & 0xFF);

		for (i = 0; i < 7; i++) {
			sunxi_debug(NULL, "aw_spi_inovance_read_data txbuf %d is 0x%02x\n", i, txbuf[i]);
		}

		spi_message_init(&msg);
		memset(&t, 0, sizeof(t));
		t.tx_buf = txbuf;
		t.len = 7;
		t.rx_buf = read_buf;
		spi_message_add_tail(&t, &msg);
		ret = spi_sync(spi_inovance_data->spi, &msg);
		if (ret) {
			sunxi_err(NULL, " aw_spi_inovance_read_data spi_inovance read reg data failed\n");
			return -1;
		}

		for (i = 0; i < 7; i++) {
			sunxi_debug(NULL, "aw_spi_inovance_read_data read_buf %d is 0x%02x\n", i, read_buf[i]);
		}

		reg_result_tmp |= ((((u32)read_buf[3]) << 24) | (((u32)read_buf[4]) << 16) | (((u32)read_buf[5]) << 8) | ((u32)read_buf[6]));

	} else {

		opcode_addr |= (((reg & 0x1FFF) << 3) | read_bit);
		txbuf[0] |= ((opcode_addr >> 8) & 0xFF);
		txbuf[1] |= (opcode_addr & 0xFF);

		for (i = 0; i < 7; i++) {
			sunxi_debug(NULL, "aw_spi_inovance_read_data txbuf %d is 0x%02x\n", i, txbuf[i]);
		}

		spi_message_init(&msg);
		memset(&t, 0, sizeof(t));
		t.tx_buf = txbuf;
		t.len = 5;
		t.rx_buf = read_buf;
		spi_message_add_tail(&t, &msg);
		ret = spi_sync(spi_inovance_data->spi, &msg);
		if (ret) {
			sunxi_err(NULL, " aw_spi_inovance_read_data spi_inovance read reg data failed\n");
			return -1;
		}

		for (i = 0; i < 7; i++) {
			sunxi_debug(NULL, "aw_spi_inovance_read_data read_buf %d is 0x%02x\n", i, read_buf[i]);
		}

		reg_result_tmp |= ((((u32)read_buf[3]) << 8) | (((u32)read_buf[4])));
	}

	*reg_result = reg_result_tmp;
	mutex_unlock(&spi_inovance_data->lock);
	return ret;
}

static int aw_spi_inovance_write_data(struct aw_spi_inovance_data *spi_inovance_data, u16 reg, u32 reg_val, bool is_32bit)
{
	unsigned char txbuf[6] = {0x00};
	unsigned char rxbuf[6] = {0x00};
	int ret, i;
	struct spi_message	msg;
	struct spi_transfer	t;
	u8 write_bit = 0x04;
	u16 opcode_addr = 0;

	sunxi_debug(NULL, "aw_spi_inovance_write_data target reg is 0x%08x\n", reg);
	sunxi_debug(NULL, "aw_spi_inovance_write_data target reg_val is 0x%08x\n", reg_val);

	if (is_32bit) {
		opcode_addr |= (((reg & 0x1FFF) << 3) | write_bit);
		txbuf[0] |= ((opcode_addr >> 8) & 0xFF);
		txbuf[1] |= (opcode_addr & 0xFF);
		/* 4-byte mode */
		txbuf[2] |= ((reg_val >> 24) & 0xFF);
		txbuf[3] |= ((reg_val >> 16) & 0xFF);
		txbuf[4] |= ((reg_val >> 8) & 0xFF);
		txbuf[5] |= (reg_val & 0xFF);

		for (i = 0; i < 6; i++) {
			sunxi_debug(NULL, "aw_spi_inovance_write_data txbuf %d is 0x%02x\n", i, txbuf[i]);
		}
		mutex_lock(&spi_inovance_data->lock);
		spi_message_init(&msg);
		memset(&t, 0, sizeof(t));
		t.tx_buf = txbuf;
		t.len = 6;
		t.rx_buf = rxbuf;
		spi_message_add_tail(&t, &msg);
		ret = spi_sync(spi_inovance_data->spi, &msg);
		if (ret) {
			sunxi_err(NULL, " aw_spi_inovance_write_data spi_inovance write reg data failed\n");
			return -1;
		}
	} else {

		opcode_addr |= (((reg & 0x1FFF) << 3) | write_bit);
		txbuf[0] |= ((opcode_addr >> 8) & 0xFF);
		txbuf[1] |= (opcode_addr & 0xFF);
		/* 2-byte mode */
		txbuf[2] |= ((reg_val >> 8) & 0xFF);
		txbuf[3] |= (reg_val & 0xFF);

		for (i = 0; i < 6; i++) {
			sunxi_debug(NULL, "aw_spi_inovance_write_data txbuf %d is 0x%02x\n", i, txbuf[i]);
		}

		mutex_lock(&spi_inovance_data->lock);
		spi_message_init(&msg);
		memset(&t, 0, sizeof(t));
		t.tx_buf = txbuf;
		t.len = 4;
		t.rx_buf = rxbuf;
		spi_message_add_tail(&t, &msg);
		ret = spi_sync(spi_inovance_data->spi, &msg);
		if (ret) {
			sunxi_err(NULL, " aw_spi_inovance_write_data spi_inovance write reg data failed\n");
			return -1;
		}
	}

	mutex_unlock(&spi_inovance_data->lock);
	return ret;
}

static ssize_t sunxi_spi_inovance_write_store_16bit(struct device *dev,
				struct device_attribute *attr, const char *buf, size_t count)
{
	struct aw_spi_inovance_data *spi_inovance_data = dev_get_drvdata(dev);
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

	aw_spi_inovance_write_data(spi_inovance_data, reg, reg_val, false);
	return count;
}

static ssize_t sunxi_spi_inovance_write_store_32bit(struct device *dev,
				struct device_attribute *attr, const char *buf, size_t count)
{
	struct aw_spi_inovance_data *spi_inovance_data = dev_get_drvdata(dev);
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

	aw_spi_inovance_write_data(spi_inovance_data, reg, reg_val, true);
	return count;
}

static ssize_t sunxi_spi_inovance_read_store_16bit(struct device *dev,
				struct device_attribute *attr, const char *buf, size_t count)
{
	struct aw_spi_inovance_data *spi_inovance_data = dev_get_drvdata(dev);
	u16 reg;
	u32 reg_result = 0;
	int err;

	err = kstrtou16(buf, 16, &reg);
	if (err) {
		sunxi_err(NULL, "String conversion failed!\n");
		return -ERANGE;
	}
	aw_spi_inovance_read_data(spi_inovance_data, reg, &reg_result, false);
	sunxi_info(NULL, "aw_spi_inovance_read_data reg 0x%02x val is 0x%08x\n", reg, reg_result);
	return count;
}

static ssize_t sunxi_spi_inovance_read_store_32bit(struct device *dev,
				struct device_attribute *attr, const char *buf, size_t count)
{
	struct aw_spi_inovance_data *spi_inovance_data = dev_get_drvdata(dev);
	u16 reg;
	u32 reg_result = 0;
	int err;

	err = kstrtou16(buf, 16, &reg);
	if (err) {
		sunxi_err(NULL, "String conversion failed!\n");
		return -ERANGE;
	}
	aw_spi_inovance_read_data(spi_inovance_data, reg, &reg_result, true);
	sunxi_info(NULL, "aw_spi_inovance_read_data reg 0x%02x val is 0x%08x\n", reg, reg_result);
	return count;
}

static struct device_attribute sunxi_spi_inovance_debug_attr[] = {
	__ATTR(writeReg_inovance_16bit, S_IWUSR, NULL, sunxi_spi_inovance_write_store_16bit),
	__ATTR(readReg_inovance_16bit, S_IWUSR, NULL, sunxi_spi_inovance_read_store_16bit),
	__ATTR(writeReg_inovance_32bit, S_IWUSR, NULL, sunxi_spi_inovance_write_store_32bit),
	__ATTR(readReg_inovance_32bit, S_IWUSR, NULL, sunxi_spi_inovance_read_store_32bit),
};

static void sunxi_spi_inovance_create_sysfs(struct aw_spi_inovance_data *spi_inovance_data)
{
	int i;
	struct spi_device *spi = spi_inovance_data->spi;

	for (i = 0; i < ARRAY_SIZE(sunxi_spi_inovance_debug_attr); i++)
		device_create_file(&spi->dev, &sunxi_spi_inovance_debug_attr[i]);
}

static void sunxi_spi_inovance_remove_sysfs(struct spi_device *spi)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(sunxi_spi_inovance_debug_attr); i++)
		device_remove_file(&spi->dev, &sunxi_spi_inovance_debug_attr[i]);
}

int sunxi_inovance_write_reg_value_16bit(u16 reg, u32 reg_val)
{
	return aw_spi_inovance_write_data(global_aw_spi_inovance_data, reg, reg_val, false);
}
EXPORT_SYMBOL_GPL(sunxi_inovance_write_reg_value_16bit);

int sunxi_inovance_write_reg_value_32bit(u16 reg, u32 reg_val)
{
	return aw_spi_inovance_write_data(global_aw_spi_inovance_data, reg, reg_val, true);
}
EXPORT_SYMBOL_GPL(sunxi_inovance_write_reg_value_32bit);

u32 sunxi_inovance_read_reg_value_16bit(u16 reg)
{
	u32 reg_result = 0;
	if (aw_spi_inovance_read_data(global_aw_spi_inovance_data, reg, &reg_result, false)) {
		sunxi_err(NULL, "inovance spi read reg value error");
		return -EPERM;
	}
	return reg_result;
}
EXPORT_SYMBOL_GPL(sunxi_inovance_read_reg_value_16bit);

u32 sunxi_inovance_read_reg_value_32bit(u16 reg)
{
	u32 reg_result = 0;
	if (aw_spi_inovance_read_data(global_aw_spi_inovance_data, reg, &reg_result, true)) {
		sunxi_err(NULL, "inovance spi read reg value error");
		return -EPERM;
	}
	return reg_result;
}
EXPORT_SYMBOL_GPL(sunxi_inovance_read_reg_value_32bit);

static int aw_spi_inovance_probe(struct spi_device *spi)
{
	struct aw_spi_inovance_data *spi_inovance_data;

	spi_inovance_data = devm_kzalloc(&spi->dev, sizeof(*spi_inovance_data), GFP_KERNEL);
	if (!spi_inovance_data)
		return -ENOMEM;

	spi_inovance_data->spi = spi;
	spi_inovance_data->dev = &spi->dev;
	mutex_init(&spi_inovance_data->lock);
	sunxi_spi_inovance_create_sysfs(spi_inovance_data);
	dev_set_drvdata(spi_inovance_data->dev, spi_inovance_data);
	global_aw_spi_inovance_data = spi_inovance_data;
	sunxi_info(NULL, "aw_spi_inovance driver probe success!!!\n");

	return 0;
}

static int aw_spi_inovance_remove(struct spi_device *spi)
{
	sunxi_spi_inovance_remove_sysfs(spi);
	return 0;
}

static const struct of_device_id aw_spi_inovance_of_ids[] = {
	{ .compatible = "allwinner, aw-spi-inovance" },
	{ /* sentinel */ },
};

static struct spi_driver aw_spi_inovance_drv = {
	.driver = {
		.name = "allwinner, aw-spi-inovance",
		.of_match_table = of_match_ptr(aw_spi_inovance_of_ids),
	},
	.probe = aw_spi_inovance_probe,
	.remove = aw_spi_inovance_remove,
};
module_spi_driver(aw_spi_inovance_drv);

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("liujing <luiujingswc@allwinnertech.com>");
MODULE_DESCRIPTION("Allwinner's 1935 spi_inovance controll localbus register");
MODULE_VERSION("1.0.0");
