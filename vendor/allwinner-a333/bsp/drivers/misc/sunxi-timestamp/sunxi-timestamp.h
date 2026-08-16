/* SPDX-License-Identifier: GPL-2.0-or-later */
#ifndef __SUNXI_TIMESTAMP_H__
#define __SUNXI_TIMESTAMP_H__

struct sunxi_timestamp_dev {
	void __iomem *base;
	struct device *dev;
	u32 hosc_fre;
};

#endif /* __SUNXI_TIMESTAMP_H__  */
