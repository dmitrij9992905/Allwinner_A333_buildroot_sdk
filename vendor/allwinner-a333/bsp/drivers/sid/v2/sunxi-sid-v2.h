/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Copyright(c) 2020 - 2023 Allwinner Technology Co.,Ltd. All rights reserved. */

#ifndef _SUNXI_SID_V2_H_
#define _SUNXI_SID_V2_H_

#include <sunxi-sid.h>

struct sunxi_sid_hw_data {
	u32 markid_mask; /* for efuse markid mask */
	u32 rotpk_status_offset; /* for rotpk status offset */
	u32 rotpk_status_mask; /* for rotpk status mask */
	u32 secure_status_offset; /* for secure status offset */ //TODO:If not configured here, use the secbit configuration from the dtsi.
};

struct sunxi_sid_key {
	char name[64];
	u32 offset;
	u32 size;
	u32 shift;
	u32 mask;
};

struct sunxi_sid_dts {
	struct sunxi_sid_key *key_info;
	void __iomem *soc_ver_base;
	void __iomem *key_base;
	u32 soc_ver_reg;
	u32 soc_ver_mask;
	u32 sid_ver_offset;
	u32 key_nonsec_offset_max; /* non secure key max range, like 0x50 in A537 */
	u32 key_nonsec_adress_offset;
	u32 key_size_max; /* non secure key max range, like 0x50 in A537 */
	u32 key_num;
};

struct sunxi_sid_dev_info {
	struct miscdevice miscdev;
	struct sunxi_sid_chip *chip;
};

struct sunxi_sid_chip {
	struct device *dev;
	struct device_node *of_node;
	struct platform_device *pdev;
	struct resource *res;
	void __iomem *base;
	struct sunxi_sid_hw_data *data;
	struct mutex mutex;
	struct regulator *regulator; /* efuse power */
	struct sunxi_sid_dts dts;
	sunxi_efuse_key_info_t user_key;
	char *user_data;
	struct sunxi_sid_dev_info *dev_info;
};

#define SUNXI_EFUSE_READ	_IO('V', 1)
#define SUNXI_EFUSE_WRITE	_IO('V', 2)

#define SUNXI_DIE_INFO_MASK	0xFFFF0000
#define SUNXI_DIE_ENABLE_OFFSET	15

#endif /* _SUNXI_SID_V2_H_ */
