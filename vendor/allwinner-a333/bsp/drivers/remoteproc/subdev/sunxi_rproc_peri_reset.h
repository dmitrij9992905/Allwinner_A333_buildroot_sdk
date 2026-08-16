/* SPDX-License-Identifier: GPL-2.0 */
/*
 * sunxi's rproc peripheral reset
 *
 * Copyright (C) 2023 Allwinnertech - All Rights Reserved
 *
 * Author: shihongfu <shihongfu@allwinnertech.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __SUNXI_RPROC_PERI_RESET_H__
#define __SUNXI_RPROC_PERI_RESET_H__

#include <linux/device.h>
#include <linux/remoteproc.h>

struct platform_device;

typedef struct sunxi_rproc_peri_rst_dev {
	struct rproc_subdev subdev;
	struct rproc *rproc;

	struct reset_control *rst;
	bool skip_crash_check;

} sunxi_rproc_peri_rst_dev_t;

struct sunxi_rproc_peri_rst_dev *sunxi_rproc_peri_rst_probe(struct rproc *rproc,
		struct platform_device *pdev);

void sunxi_rproc_peri_rst_remove(struct sunxi_rproc_peri_rst_dev *peri_rst_dev);

#endif /* __SUNXI_RPROC_PERI_RESET_H__ */
