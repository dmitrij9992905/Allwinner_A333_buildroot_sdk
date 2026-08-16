// SPDX-License-Identifier: GPL-2.0
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

#include "sunxi_rproc_peri_reset.h"

#include <linux/reset.h>
#include <linux/slab.h>
#include <linux/platform_device.h>

#define AW_RPROC_PERI_RESET_DEV_NODE_NAME "peri_reset"

#define PERI_RESET_SKIP_CRASH_CHECK_PROPERTY_NAME "skip-crash-check"

#define subdev_to_peri_rst_dev(_subdev)	container_of(_subdev, sunxi_rproc_peri_rst_dev_t, subdev)

static void rproc_subdev_peri_rst_stop(struct rproc_subdev *subdev, bool crashed)
{
	int ret;
	struct device *dev;
	sunxi_rproc_peri_rst_dev_t *peri_rst_dev;

	peri_rst_dev = subdev_to_peri_rst_dev(subdev);

	if (!peri_rst_dev->skip_crash_check && !crashed)
		return;

	dev = &peri_rst_dev->rproc->dev;

	ret = reset_control_assert(peri_rst_dev->rst);
	if (ret) {
		dev_err(dev, "peripheral reset assert failed, ret: %d\n", ret);
		return;
	}

	dev_dbg(dev, "peripheral reset assert success!\n");
}

struct sunxi_rproc_peri_rst_dev *sunxi_rproc_peri_rst_probe(struct rproc *rproc,
		struct platform_device *pdev)
{
	struct device *dev;
	struct sunxi_rproc_peri_rst_dev *peri_rst_dev;
	struct device_node *rproc_np, *rst_dev_np;
	const char *node_name;
	struct reset_control *rst;

	dev = &rproc->dev;

	peri_rst_dev = kzalloc(sizeof(*peri_rst_dev), GFP_KERNEL);
	if (!peri_rst_dev) {
		dev_err(dev, "alloc memory for peripheral reset dev failed!\n");
		return NULL;
	}

	node_name = AW_RPROC_PERI_RESET_DEV_NODE_NAME;
	rproc_np = pdev->dev.of_node;
	of_node_get(rproc_np);
	rst_dev_np = of_find_node_by_name(rproc_np, node_name);
	if (!rst_dev_np) {
		dev_warn(dev, "sub node('%s') not found!\n", node_name);
		goto err_out;
	}

	rst = of_reset_control_array_get_exclusive(rst_dev_np);
	if (IS_ERR_OR_NULL(rst)) {
		dev_err(dev, "get peripheral reset info failed, ret: %ld\n", PTR_ERR(rst));
		goto err_out;
	}

	peri_rst_dev->skip_crash_check = of_property_read_bool(rst_dev_np, PERI_RESET_SKIP_CRASH_CHECK_PROPERTY_NAME);

	peri_rst_dev->rst = rst;
	peri_rst_dev->rproc = rproc;
	peri_rst_dev->subdev.stop = rproc_subdev_peri_rst_stop;

	rproc_add_subdev(rproc, &peri_rst_dev->subdev);

	dev_info(dev, "peripheral reset subdev probe success!\n");
	return peri_rst_dev;

err_out:
	kfree(peri_rst_dev);
	return NULL;
}

void sunxi_rproc_peri_rst_remove(struct sunxi_rproc_peri_rst_dev *peri_rst_dev)
{
	if (!peri_rst_dev)
		return;

	rproc_remove_subdev(peri_rst_dev->rproc, &peri_rst_dev->subdev);

	if (peri_rst_dev->rst)
		reset_control_put(peri_rst_dev->rst);

	kfree(peri_rst_dev);
}
