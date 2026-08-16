/*
 * (C) Copyright 2013-2016
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 *
 * wangwei <wangwei@allwinnertech.com>
 * SPDX-License-Identifier:     GPL-2.0+
 */

#ifndef __SID_SUN252IW1_H__
#define __SID_SUN252IW1_H__

#include <linux/types.h>
#include <asm/arch/cpu.h>

/* Efuse power control */
#define EFUSE_HV_SWITCH			(IOMEM_ADDR(SUNXI_RTC_BASE) + 0x204)

/* SID registers */
#define SID_PRCTL               	(IOMEM_ADDR(SUNXI_SID_BASE) + 0x40)
#define SID_OP_LOCK			(0xAC)
#define SID_PRKEY               	(IOMEM_ADDR(SUNXI_SID_BASE) + 0x50)
#define SID_RDKEY               	(IOMEM_ADDR(SUNXI_SID_BASE) + 0x60)
#define SID_ROTPK_VALUE(n)              (IOMEM_ADDR(SUNXI_SID_BASE) + 0x120 + (n * 4))
#define SID_ROTPK_CTRL                  (IOMEM_ADDR(SUNXI_SID_BASE) + 0x140)
#define SID_ROTPK_EFUSED_BIT            (1)     /* Bit offset of 'ROTPK_EFUSED' */
#define SID_ROTPK_CMP_RET_BIT           (0)     /* Bit offset of 'ROTPK_COMP_RESULT' */


/* SID SRAM */
#define SID_EFUSE               	(IOMEM_ADDR(SUNXI_SID_BASE) + 0x200)
#define SID_SRAM_SIZE_BYTES		(0x100)

/* Efuse Mapping Table */
#define EFUSE_CHIPID            	(0x0)
#define SID_CHIPID_SIZE			(128)

#define EFUSE_BROM_CONF			(0x30)
#define SID_BROM_CONF_SIZE		(64)
#define EFUSE_ANTI_BRUSH		EFUSE_BROM_CONF
#define ANTI_BRUSH_BIT_OFFSET		(0)
#define ANTI_BRUSH_MODE			(SID_EFUSE + EFUSE_ANTI_BRUSH)

#define EFUSE_ROTPK			(0x80)
#define SCC_ROTPK_BURNED_FLAG		(25)

#define EFUSE_OEM_PROGRAM		(0xA4)
#define SID_OEM_PROGRAM_SIZE		(256)

#define EFUSE_WRITE_PROTECT		(0x3C)
#define EFUSE_READ_PROTECT		(0x3C)

#endif    /*  #ifndef __SID_SUN252IW1_H__  */
