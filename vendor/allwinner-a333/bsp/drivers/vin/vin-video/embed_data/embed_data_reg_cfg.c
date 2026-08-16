/* SPDX-License-Identifier: GPL-2.0 */

 /*
  * embed_data_reg_cfg.c
  *
  * Copyright (c) 2007-2024 Allwinnertech Co., Ltd.
  *
  * Authors:  Zheng ZeQun <zequnzheng@allwinnertech.com>
  *
  * This software is licensed under the terms of the GNU General Public
  * License version 2, as published by the Free Software Foundation, and
  * may be copied, distributed, and modified under those terms.
  *
  * This program is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  * GNU General Public License for more details.
  *
  */
#include <linux/io.h>
#include <linux/string.h>
#include "embed_data_reg.h"
#include "embed_data_reg_cfg.h"

struct embed_data_reg {
	ISP_FRM_FLAG_REG_t *isp_frm_flag;
	unsigned int *tdm_time_base;
	TDM_RX_TIME_OFFSET_REG_t *tdm_rx_time_offset;
	unsigned int *tdm_rx_frame_cnt;

	ISPFE_STAT_EN_REG_t *ispfe_stat_en;
	ISP_VALID_IMAGE_SIZE_REG_t *isp_valid_image_size;

	ISPFE_INTPO_CFG0_REG_t *ispfe_intpo_cfg0;
	ISPFE_INTPO_CFG2_REG_t *ispfe_intpo_cfg2;
	ISPFE_INTPO_CFG3_REG_t *ispfe_intpo_cfg3;
	ISPFE_INTPO_CFG1_REG_t *ispfe_intpo_cfg1;

	VIPP_CORP_START_POS_REG_t *vipp_crop_start_pos;
	VIPP_CORP_SIZE_REG_t *vipp_crop_size;
	VIPP_SC_OUT_SIZE_REG_t *vipp_sc_out_size;

	unsigned char *motion_stat;
	unsigned char *texture_stat;
};

struct embed_data_reg embed_data_reg[VIN_MAX_DEV];

void bsp_embed_data_map_addr(unsigned long id, vin_dma_addr_t base)
{
	embed_data_reg[id].isp_frm_flag = (ISP_FRM_FLAG_REG_t *) (base + TDM_INFO_REG_OFFSET + ISP_FRM_FLAG_REG);
	embed_data_reg[id].tdm_time_base = (unsigned int *) (base + TDM_INFO_REG_OFFSET + TDM_TIME_BASE_REG);
	embed_data_reg[id].tdm_rx_time_offset = (TDM_RX_TIME_OFFSET_REG_t *) (base + TDM_INFO_REG_OFFSET + TDM_RX_TIME_OFFSET_REG);
	embed_data_reg[id].tdm_rx_frame_cnt = (unsigned int *) (base + TDM_INFO_REG_OFFSET + TDM_RX_FRM_CONT_REG);

	embed_data_reg[id].ispfe_stat_en = (ISPFE_STAT_EN_REG_t *) (base + ISP_INFO_REG_OFFSET + ISPFE_STAT_EN_REG);
	embed_data_reg[id].isp_valid_image_size = (ISP_VALID_IMAGE_SIZE_REG_t *) (base + ISP_INFO_REG_OFFSET + ISP_VALID_IMAGE_SIZE_REG);

	embed_data_reg[id].ispfe_intpo_cfg0 = (ISPFE_INTPO_CFG0_REG_t *) (base + ITP_REGS_REG_OFFSET + ISPFE_INTPO_CFG0_REG);
	embed_data_reg[id].ispfe_intpo_cfg2 = (ISPFE_INTPO_CFG2_REG_t *) (base + ITP_REGS_REG_OFFSET + ISPFE_INTPO_CFG2_REG);
	embed_data_reg[id].ispfe_intpo_cfg3 = (ISPFE_INTPO_CFG3_REG_t *) (base + ITP_REGS_REG_OFFSET + ISPFE_INTPO_CFG3_REG);
	embed_data_reg[id].ispfe_intpo_cfg1 = (ISPFE_INTPO_CFG1_REG_t *) (base + ITP_REGS_REG_OFFSET + ISPFE_INTPO_CFG1_REG);

	embed_data_reg[id].vipp_crop_start_pos = (VIPP_CORP_START_POS_REG_t *) (base + VIPP_INFO_REG_OFFSET + VIPP_CORP_START_POS_REG);
	embed_data_reg[id].vipp_crop_size = (VIPP_CORP_SIZE_REG_t *) (base + VIPP_INFO_REG_OFFSET + VIPP_CORP_SIZE_REG);
	embed_data_reg[id].vipp_sc_out_size = (VIPP_SC_OUT_SIZE_REG_t *) (base + VIPP_INFO_REG_OFFSET + VIPP_SC_OUT_SIZE_REG);

	embed_data_reg[id].motion_stat = (unsigned char *)(base + MOTION_REG_OFFSET);
	embed_data_reg[id].texture_stat = (unsigned char *)(base + TEXTURE_REG_OFFSET);
}

unsigned int bsp_emdt_get_isp_frm_error_flag(unsigned long id)
{
	return embed_data_reg[id].isp_frm_flag->bits.isp_frm_error_flag;
}

unsigned int bsp_emdt_get_bk_frm_error_flag(unsigned long id)
{
	return embed_data_reg[id].isp_frm_flag->bits.bk_frm_error_flag;
}

unsigned int bsp_emdt_get_bk_vflip_en(unsigned long id)
{
	return embed_data_reg[id].isp_frm_flag->bits.bk_vflip_en;
}

unsigned int bsp_emdt_get_bk_hflip_en(unsigned long id)
{
	return embed_data_reg[id].isp_frm_flag->bits.bk_hflip_en;
}

unsigned int bsp_emdt_get_tdm_time_base(unsigned long id)
{
	return *embed_data_reg[id].tdm_time_base;
}

unsigned int bsp_emdt_get_tdm_time_cycle(unsigned long id)
{
	return embed_data_reg[id].tdm_rx_time_offset->bits.cycle;
}

unsigned int bsp_emdt_get_tdm_rx_frame_cnt(unsigned long id)
{
	return *embed_data_reg[id].tdm_rx_frame_cnt;
}

void bsp_emdt_set_motion_stat_hflip(unsigned long id)
{
	unsigned char *motion_stat;
	unsigned char i, j, temp;

	motion_stat = embed_data_reg[id].motion_stat;
	for (i = 0; i < ISPFE_MOTION_ROW; i++) {
		for (j = 0; j < ISPFE_MOTION_COL / 2; j++) {
			temp = motion_stat[i * ISPFE_MOTION_COL + j];
			motion_stat[i * ISPFE_MOTION_COL + j] = motion_stat[i * ISPFE_MOTION_COL + ISPFE_MOTION_COL - j - 1];
			motion_stat[i * ISPFE_MOTION_COL + ISPFE_MOTION_COL - j - 1] = temp;
		}
	}

}

void bsp_emdt_set_motion_stat_vflip(unsigned long id)
{
	unsigned char *motion_stat;
	unsigned char i, j, temp;

	motion_stat = embed_data_reg[id].motion_stat;
	for (i = 0; i < ISPFE_MOTION_COL; i++) {
		for (j = 0; j < ISPFE_MOTION_ROW / 2; j++) {
			temp = motion_stat[j * ISPFE_MOTION_COL + i];
			motion_stat[j * ISPFE_MOTION_COL + i] = motion_stat[(ISPFE_MOTION_ROW - j - 1) * ISPFE_MOTION_COL + i];
			motion_stat[(ISPFE_MOTION_ROW - j - 1) * ISPFE_MOTION_COL + i] = temp;
		}
	}
}

void bsp_emdt_set_texture_stat_hflip(unsigned long id)
{
	unsigned char *texture_stat;
	unsigned char i, j, temp;

	texture_stat = embed_data_reg[id].texture_stat;
	for (i = 0; i < ISPFE_TEXTURE_ROW; i++) {
		for (j = 0; j < ISPFE_TEXTURE_COL / 2; j++) {
			temp = texture_stat[i * ISPFE_MOTION_COL + j];
			texture_stat[i * ISPFE_MOTION_COL + j] = texture_stat[i * ISPFE_MOTION_COL + ISPFE_MOTION_COL - j - 1];
			texture_stat[i * ISPFE_MOTION_COL + ISPFE_MOTION_COL - j - 1] = temp;
		}
	}
}

void bsp_emdt_set_texture_stat_vflip(unsigned long id)
{
	unsigned char *texture_stat;
	unsigned char i, j, temp;

	texture_stat = embed_data_reg[id].texture_stat;
	for (i = 0; i < ISPFE_TEXTURE_COL; i++) {
		for (j = 0; j < ISPFE_TEXTURE_ROW / 2; j++) {
			temp = texture_stat[j * ISPFE_MOTION_COL + i];
			texture_stat[j * ISPFE_MOTION_COL + i] = texture_stat[(ISPFE_MOTION_COL - j - 1) * ISPFE_MOTION_COL + i];
			texture_stat[(ISPFE_MOTION_COL - j - 1) * ISPFE_MOTION_COL + i] = temp;
		}
	}
}

void bsp_emdt_set_stat_merge(vin_dma_addr_t base0, vin_dma_addr_t base1)
{
	unsigned char *motion_stat0, *motion_stat1;
	unsigned char *texture_stat0, *texture_stat1;
	unsigned char i, j;

	motion_stat0 = (unsigned char *)(base0 + MOTION_REG_OFFSET);
	texture_stat0 = (unsigned char *)(base0 + TEXTURE_REG_OFFSET);
	motion_stat1 = (unsigned char *)(base1 + MOTION_REG_OFFSET);
	texture_stat1 = (unsigned char *)(base1 + TEXTURE_REG_OFFSET);

	for (j = 0; j < ISPFE_MOTION_ROW; j++) {
		for (i = 0; i < ISPFE_MOTION_COL; i++) {
			if (i < ISPFE_MOTION_COL / 2)
				motion_stat0[j * ISPFE_MOTION_COL + i] = (motion_stat0[j * ISPFE_MOTION_COL + 2 * i] + motion_stat0[j * ISPFE_MOTION_COL + 2 * i + 1]) / 2;
			else
				motion_stat0[j * ISPFE_MOTION_COL + i] = (motion_stat1[j * ISPFE_MOTION_COL + 2 * (i - ISPFE_MOTION_COL / 2)] + motion_stat1[j * ISPFE_MOTION_COL + 2 * (i - ISPFE_MOTION_COL / 2) + 1]) / 2;
		}
	}

	for (j = 0; j < ISPFE_TEXTURE_ROW; j++) {
		for (i = 0; i < ISPFE_TEXTURE_COL; i++) {
			if (i < ISPFE_TEXTURE_COL / 2)
				texture_stat0[j * ISPFE_TEXTURE_COL + i] = (texture_stat0[j * ISPFE_TEXTURE_COL + 2 * i] + texture_stat0[j * ISPFE_TEXTURE_COL + 2 * i + 1]) / 2;
			else
				texture_stat0[j * ISPFE_TEXTURE_COL + i] = (texture_stat1[j * ISPFE_TEXTURE_COL + 2 * (i - ISPFE_TEXTURE_COL / 2)] + texture_stat1[j * ISPFE_TEXTURE_COL + 2 * (i - ISPFE_TEXTURE_COL / 2) + 1]) / 2;
		}
	}
}

#if VIN_FALSE
void bsp_emdt_get_motion_stat(unsigned long id, unsigned char *motion, unsigned int size)
{
	unsigned char *motion_stat;
	int i;

	if (size < MOTION_SIZE)
		return;

	motion_stat = embed_data_reg[id].motion_stat;
	for (i = 0; i < MOTION_SIZE; i++)
		motion[i] = motion_stat[i];
}

void bsp_emdt_set_motion_stat(unsigned long id, unsigned char *motion, unsigned int size)
{
	unsigned char *motion_stat;
	int i;

	if (size < MOTION_SIZE)
		return;

	motion_stat = embed_data_reg[id].motion_stat;
	for (i = 0; i < MOTION_SIZE; i++)
		motion_stat[i] = motion[i];
}

void bsp_emdt_get_texture_stat(unsigned long id, unsigned char *texture, unsigned int size)
{
	unsigned char *texture_stat;
	int i;

	if (size < TEXTURE_SIZE)
		return;

	texture_stat = embed_data_reg[id].texture_stat;
	for (i = 0; i < TEXTURE_SIZE; i++)
		texture[i] = texture_stat[i];
}

void bsp_emdt_set_texture_stat(unsigned long id, unsigned char *texture, unsigned int size)
{
	unsigned char *texture_stat;
	int i;

	if (size < TEXTURE_SIZE)
		return;

	texture_stat = embed_data_reg[id].texture_stat;
	for (i = 0; i < TEXTURE_SIZE; i++)
		texture_stat[i] = texture[i];
}
#endif
