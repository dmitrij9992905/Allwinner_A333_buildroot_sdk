/* SPDX-License-Identifier: GPL-2.0 */

 /*
  * embed_data_reg_cfg.h
  *
sum_weight_max  * Copyright (c) 2007-2024 Allwinnertech Co., Ltd.
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
#ifndef _EMBED_DATA_REG_CFG_H_
#define _EMBED_DATA_REG_CFG_H_

#include "../../platform/platform_cfg.h"
#include "embed_data_reg.h"

#define EMBED_DATA_SIZE (TDM_INFO_SIZE + ISP_INFO_SIZE + ITP_REGS_SIZE + VIPP_INFO_SIZE + MOTION_SIZE + TEXTURE_SIZE)

#define ISPFE_MOTION_ROW	24
#define ISPFE_MOTION_COL	32

#define ISPFE_TEXTURE_ROW	24
#define ISPFE_TEXTURE_COL	32

void bsp_embed_data_map_addr(unsigned long id, vin_dma_addr_t base);
unsigned int bsp_emdt_get_isp_frm_error_flag(unsigned long id);
unsigned int bsp_emdt_get_bk_frm_error_flag(unsigned long id);
unsigned int bsp_emdt_get_bk_vflip_en(unsigned long id);
unsigned int bsp_emdt_get_bk_hflip_en(unsigned long id);
unsigned int bsp_emdt_get_tdm_time_base(unsigned long id);
unsigned int bsp_emdt_get_tdm_time_cycle(unsigned long id);
unsigned int bsp_emdt_get_tdm_rx_frame_cnt(unsigned long id);
void bsp_emdt_set_motion_stat_hflip(unsigned long id);
void bsp_emdt_set_motion_stat_vflip(unsigned long id);
void bsp_emdt_set_texture_stat_hflip(unsigned long id);
void bsp_emdt_set_texture_stat_vflip(unsigned long id);
void bsp_emdt_set_stat_merge(vin_dma_addr_t base0, vin_dma_addr_t base1);
#if VIN_FALSE
void bsp_emdt_get_motion_stat(unsigned long id, unsigned char *motion, unsigned int size);
void bsp_emdt_set_motion_stat(unsigned long id, unsigned char *motion, unsigned int size);
void bsp_emdt_get_texture_stat(unsigned long id, unsigned char *texture, unsigned int size);
void bsp_emdt_set_texture_stat(unsigned long id, unsigned char *texture, unsigned int size);
#endif

#endif /* _EMBED_DATA_REG_CFG_H_ */
