# SPDX-License-Identifier: GPL-2.0
ccflags-y := -DDYNAMIC_DEBUG_MODULE

obj-$(CONFIG_AW_XENO_DRIVERS_SPI) += xenomai-spi-sunxi.o

xenomai-spi-sunxi-objs += spi-sunxi.o spi-master.o spi-device.o calibrate/spi-sunxi-calibrate.o
xenomai-spi-sunxi-$(CONFIG_AW_XENO_DRIVERS_SPI_BIT) += bit/spi-sunxi-bit.o
xenomai-spi-sunxi-$(CONFIG_AW_XENO_DRIVERS_SPI_DBI) += dbi/spi-sunxi-dbi.o
xenomai-spi-sunxi-$(CONFIG_AW_XENO_DRIVERS_SPI_CAMERA) += camera/spi-sunxi-camera.o
