/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Copyright(c) 2020 - 2023 Allwinner Technology Co.,Ltd. All rights reserved. */
/*
 * SUNXI SPI Controller Calibrate Driver
 *
 */

#define SUNXI_MODNAME "spi"
#include <sunxi-log.h>
#include "spi-sunxi-calibrate.h"
#include "../spi-sunxi-debug.h"

static const u32 sunxi_spi_sample_mode[] = {
	0x100, /* SUNXI_SPI_SAMP_DELAY_CYCLE_0_0 */
	0x000, /* SUNXI_SPI_SAMP_DELAY_CYCLE_0_5 */
	0x010, /* SUNXI_SPI_SAMP_DELAY_CYCLE_1_0 */
	0x110, /* SUNXI_SPI_SAMP_DELAY_CYCLE_1_5 */
	0x101, /* SUNXI_SPI_SAMP_DELAY_CYCLE_2_0 */
	0x001, /* SUNXI_SPI_SAMP_DELAY_CYCLE_2_5 */
	0x011  /* SUNXI_SPI_SAMP_DELAY_CYCLE_3_0 */
};

static void sunxi_spi_bus_sample_type(struct sunxi_spi *sspi, u32 type)
{
	u32 reg_val = readl(sspi->base_addr + SUNXI_SPI_GC_REG);
	u32 reg_old = reg_val;

	switch (type) {
	case SUNXI_SPI_SAMP_TYPE_OLD:
		reg_val &= ~SUNXI_SPI_GC_MODE_SEL;
		break;
	case SUNXI_SPI_SAMP_TYPE_NEW:
		reg_val |= SUNXI_SPI_GC_MODE_SEL;
		break;
	}
	if (reg_val != reg_old)
		writel(reg_val, sspi->base_addr + SUNXI_SPI_GC_REG);
}

static void sunxi_spi_set_sample_delay_sw(struct sunxi_spi *sspi, u32 status)
{
	u32 reg_val = readl(sspi->base_addr + SUNXI_SPI_SAMP_DL_REG);
	u32 reg_old = reg_val;

	if (status)
		reg_val |= SUNXI_SPI_SAMP_DL_SW_EN;
	else
		reg_val &= ~SUNXI_SPI_SAMP_DL_SW_EN;

	if (reg_val != reg_old)
		writel(reg_val, sspi->base_addr + SUNXI_SPI_SAMP_DL_REG);
}

static void sunxi_spi_set_sample_mode(struct sunxi_spi *sspi, u32 mode)
{
	u32 reg_old, reg_new;
	u32 sdm, sdc, sdc1;

	if (mode >= ARRAY_SIZE(sunxi_spi_sample_mode)) {
		sunxi_err(sspi->dev, "sample mode out of range %d\n", mode);
		return ;
	}

	reg_new = reg_old = readl(sspi->base_addr + SUNXI_SPI_TC_REG);

	sdm = (sunxi_spi_sample_mode[mode] >> 8) & 0xf;
	sdc = (sunxi_spi_sample_mode[mode] >> 4) & 0xf;
	sdc1 = (sunxi_spi_sample_mode[mode] >> 0) & 0xf;

	if (sdm)
		reg_new |= SUNXI_SPI_TC_SDM;
	else
		reg_new &= ~SUNXI_SPI_TC_SDM;

	if (sdc)
		reg_new |= SUNXI_SPI_TC_SDC;
	else
		reg_new &= ~SUNXI_SPI_TC_SDC;

	if (sdc1)
		reg_new |= SUNXI_SPI_TC_SDC1;
	else
		reg_new &= ~SUNXI_SPI_TC_SDC1;

	if (reg_new != reg_old)
		writel(reg_new, sspi->base_addr + SUNXI_SPI_TC_REG);
}

static void sunxi_spi_set_sample_delay(struct sunxi_spi *sspi, u32 sample_delay)
{
	u32 reg_val = readl(sspi->base_addr + SUNXI_SPI_SAMP_DL_REG);
	u32 reg_old = reg_val;
	reg_val &= ~SUNXI_SPI_SAMP_DL_SW;
	reg_val |= FIELD_PREP(SUNXI_SPI_SAMP_DL_SW, sample_delay);
	if (reg_val != reg_old)
		writel(reg_val, sspi->base_addr + SUNXI_SPI_SAMP_DL_REG);
}

void sunxi_spi_delay_chain_init_xenomai(struct sunxi_spi *sspi)
{
	sunxi_spi_bus_sample_type(sspi, sspi->bus_sample_type);
	if (sspi->bus_sample_type == SUNXI_SPI_SAMP_TYPE_NEW && sspi->bus_sample_mode == SUNXI_SPI_SAMP_MODE_MANUAL) {
		/* new sample mode use only manual control */
		sunxi_spi_set_sample_mode(sspi, sspi->spi_sample_mode);
		sunxi_spi_set_sample_delay_sw(sspi, true);
		sunxi_spi_set_sample_delay(sspi, sspi->spi_sample_delay);
	} else {
		/* old sample mode use both manual/auto control */
		sunxi_spi_set_sample_mode(sspi, sspi->spi_sample_mode);
		sunxi_spi_set_sample_delay_sw(sspi, false);
	}
}

void sunxi_spi_set_delay_chain_xenomai(struct sunxi_spi *sspi, u32 clk)
{
	u32 spi_sample_mode;

	switch (sspi->bus_sample_mode) {
	case SUNXI_SPI_SAMP_MODE_AUTO:
		if (sspi->bus_sample_type == SUNXI_SPI_SAMP_TYPE_OLD) {
			if (clk >= SUNXI_SPI_SAMP_HIGH_FREQ)
				spi_sample_mode = SUNXI_SPI_SAMP_DELAY_CYCLE_1_0;
			else if (clk <= SUNXI_SPI_SAMP_LOW_FREQ)
				spi_sample_mode = SUNXI_SPI_SAMP_DELAY_CYCLE_0_0;
			else
				spi_sample_mode = SUNXI_SPI_SAMP_DELAY_CYCLE_0_5;
			if (sspi->spi_sample_mode != spi_sample_mode) {
				sunxi_spi_set_sample_mode(sspi, spi_sample_mode);
				sspi->spi_sample_mode = spi_sample_mode;
			}
		} else {
			sunxi_err(sspi->dev, "sample auto mode unsupport new type\n");
		}
		break;
	case SUNXI_SPI_SAMP_MODE_MANUAL:
		break;
	default:
		sunxi_err(sspi->dev, "unsupport sample mode %d\n", sspi->bus_sample_mode);
	}
}

void sunxi_spi_resource_get_calibrate_xenomai(struct sunxi_spi *sspi)
{
	struct device_node *np = sspi->pdev->dev.of_node;
	int ret = 0;

	ret = of_property_read_u32(np, "sample_mode", &sspi->spi_sample_mode);
	if (ret)
		sspi->spi_sample_mode = SUNXI_SPI_SAMP_MODE_DL_DEFAULT;
	ret = of_property_read_u32(np, "sample_delay", &sspi->spi_sample_delay);
	if (ret)
		sspi->spi_sample_delay = SUNXI_SPI_SAMP_MODE_DL_DEFAULT;
	if (sspi->spi_sample_mode == SUNXI_SPI_SAMP_MODE_DL_DEFAULT && sspi->spi_sample_delay == SUNXI_SPI_SAMP_MODE_DL_DEFAULT) {
		sspi->bus_sample_mode = SUNXI_SPI_SAMP_MODE_AUTO;
		sspi->bus_sample_type = SUNXI_SPI_SAMP_TYPE_OLD;
		sspi->spi_sample_mode = sspi->spi_sample_delay = 0;
	} else {
		sspi->bus_sample_mode = SUNXI_SPI_SAMP_MODE_MANUAL;
		if ((sspi->bus_mode & SUNXI_SPI_BUS_FLASH) && (sspi->data->quirk_flag & NEW_SAMPLE_MODE)) {
			sspi->bus_sample_type = SUNXI_SPI_SAMP_TYPE_NEW;
			if (sspi->spi_sample_mode > SUNXI_SPI_SAMP_DELAY_CYCLE_NEW) {
				sunxi_warn(sspi->dev, "sample mode %d over new delay cycle\n", sspi->spi_sample_mode);
				sspi->spi_sample_mode = SUNXI_SPI_SAMP_DELAY_CYCLE_0_0;
			}
			if (sspi->spi_sample_delay > SUNXI_SPI_SAMPLE_DELAY_CHAIN_MAX) {
				sunxi_warn(sspi->dev, "sample delay %d over new delay chain\n", sspi->spi_sample_delay);
				sspi->spi_sample_delay = SUNXI_SPI_SAMPLE_DELAY_CHAIN_MIN;
			}
		} else {
			sspi->bus_sample_type = SUNXI_SPI_SAMP_TYPE_OLD;
			if (sspi->spi_sample_mode > SUNXI_SPI_SAMP_DELAY_CYCLE_OLD) {
				sunxi_warn(sspi->dev, "sample mode %d over old delay cycle\n", sspi->spi_sample_mode);
				sspi->spi_sample_mode = SUNXI_SPI_SAMP_DELAY_CYCLE_0_0;
			}
			sspi->spi_sample_delay = 0;
		}
	}

	if (sspi->bus_sample_mode == SUNXI_SPI_SAMP_MODE_MANUAL)
		sunxi_info(sspi->dev, "spi manual set sample type_%d, mode_%d, delay_%d\n",
					sspi->bus_sample_type, sspi->spi_sample_mode, sspi->spi_sample_delay);
}
