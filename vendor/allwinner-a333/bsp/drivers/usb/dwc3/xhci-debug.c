// SPDX-License-Identifier: GPL-2.0
/* Copyright(c) 2020 - 2024 Allwinner Technology Co.,Ltd. All rights reserved. */
/*
 * xhci-debug.c - xHCI debugfs interface for allwinner platform
 *
 * Copyright (c) 2024 Allwinner Technology Co., Ltd.
 *
 * xhci-debugfs.c - xHCI debugfs interface
 *
 * Copyright (C) 2017 Intel Corporation
 *
 * Author: Lu Baolu <baolu.lu@linux.intel.com>
 */

#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/iopoll.h>

#include "xhci.h"
#include "xhci-debugfs.h"

#define	TEST_J		1
#define	TEST_K		2
#define	TEST_SE0_NAK	3
#define	TEST_PACKET	4
#define	TEST_FORCE_EN	5

static int xhci_portsc_show(struct seq_file *s, void *unused)
{
	struct xhci_port	*port = s->private;
	u32			portsc;
	char			str[XHCI_MSG_MAX];

	portsc = readl(port->addr);
	seq_printf(s, "%s\n", xhci_decode_portsc(str, portsc));

	return 0;
}

static int xhci_port_open(struct inode *inode, struct file *file)
{
	return single_open(file, xhci_portsc_show, inode->i_private);
}

static ssize_t xhci_port_write(struct file *file,  const char __user *ubuf,
			       size_t count, loff_t *ppos)
{
	struct seq_file		*s = file->private_data;
	struct xhci_port	*port = s->private;
	struct xhci_hcd		*xhci = hcd_to_xhci(port->rhub->hcd);
	char			buf[32];
	u32			portsc;
	unsigned long		flags;
	u32			command;
	u32			state;

	if (copy_from_user(&buf, ubuf, min_t(size_t, sizeof(buf) - 1, count)))
		return -EFAULT;

	if (!strncmp(buf, "reset", 5)) {
		spin_lock_irqsave(&xhci->lock, flags);
		sunxi_info(NULL, "RESET XHCI Controller\n");
		state = readl(&xhci->op_regs->status);
		sunxi_info(NULL, "state:0x%08x\n", state);

		sunxi_info(NULL, "Stop HCD\n");
		command = readl(&xhci->op_regs->command);
		sunxi_info(NULL, "command:0x%08x\n", command);
		command &= ~CMD_RUN;
		writel(command, &xhci->op_regs->command);
		sunxi_info(NULL, "command:0x%08x\n", readl(&xhci->op_regs->command));

		state = readl(&xhci->op_regs->status);
		sunxi_info(NULL, "state:0x%08x\n", state);

		sunxi_info(NULL, "Resetting HCD\n");
		command = readl(&xhci->op_regs->command);
		sunxi_info(NULL, "command:0x%08x\n", command);
		command |= CMD_RESET;
		writel(command, &xhci->op_regs->command);
		sunxi_info(NULL, "command:0x%08x\n", readl(&xhci->op_regs->command));

		while (1) {
			command = readl(&xhci->op_regs->command);
			if (!(command & BIT(1))) {
				sunxi_info(NULL, "command:0x%08x\n", command);
				break;
			}
		}
		sunxi_info(NULL, "Reset complete\n");

		portsc = readl(port->addr);
		sunxi_info(NULL, "portsc:0x%08x\n", portsc);
		portsc &= ~PORT_POWER;
		writel(portsc, port->addr);
		sunxi_info(NULL, "portsc:0x%08x\n", readl(port->addr));
		sunxi_info(NULL, "RESET XHCI Controller finished\n");
		spin_unlock_irqrestore(&xhci->lock, flags);
	} else {
		return -EINVAL;
	}
	return count;
}

static const struct file_operations port_fops = {
	.open			= xhci_port_open,
	.write			= xhci_port_write,
	.read			= seq_read,
	.llseek			= seq_lseek,
	.release		= single_release,
};

static void xhci_debugfs_create_port(struct xhci_hcd *xhci,
				      struct dentry *parent)
{
	unsigned int		num_ports;
	char			port_name[8];
	struct xhci_port	*port;
	struct dentry		*dir;

	num_ports = HCS_MAX_PORTS(xhci->hcs_params1);

	parent = debugfs_lookup("ports", parent);

	while (num_ports--) {
		scnprintf(port_name, sizeof(port_name), "port%02d",
			  num_ports + 1);
		dir = debugfs_lookup(port_name, parent);
		port = &xhci->hw_ports[num_ports];
		debugfs_create_file("port", 0644, dir, port, &port_fops);
	}
}

static void xhci_debugfs_destroy_port(struct xhci_hcd *xhci,
				      struct dentry *parent)
{
	unsigned int		num_ports;
	char			port_name[8];
	struct dentry		*dir;

	num_ports = HCS_MAX_PORTS(xhci->hcs_params1);

	parent = debugfs_lookup("ports", parent);

	while (num_ports--) {
		scnprintf(port_name, sizeof(port_name), "port%02d",
			  num_ports + 1);
		dir = debugfs_lookup(port_name, parent);
		debugfs_lookup_and_remove("port", dir);
	}
}

void xhci_debug_init(struct dwc3 *dwc)
{
	struct platform_device *pdev = dwc->xhci;
	struct usb_hcd *hcd = NULL;
	struct xhci_hcd *xhci = NULL;

	if (!pdev)
		return;

	hcd = platform_get_drvdata(pdev);
	if (!hcd)
		return;

	xhci = hcd_to_xhci(hcd);

	xhci_debugfs_create_port(xhci, xhci->debugfs_root);
}

void xhci_debug_exit(struct dwc3 *dwc)
{
	struct platform_device *pdev = dwc->xhci;
	struct usb_hcd *hcd = NULL;
	struct xhci_hcd *xhci = NULL;

	if (pdev == NULL)
		return;

	hcd = platform_get_drvdata(pdev);
	if (!hcd)
		return;

	xhci = hcd_to_xhci(hcd);

	xhci_debugfs_destroy_port(xhci, xhci->debugfs_root);
}

static int xhci_host_u2_test_mode(struct xhci_port *port, int param)
{
	struct xhci_hcd *xhci = hcd_to_xhci(port->rhub->hcd);
	u32 reg_value;
	int ret;

	switch (param) {
	case TEST_J:
		sunxi_info(NULL, "%s: TEST_J\n", __func__);
		break;
	case TEST_K:
		sunxi_info(NULL, "%s: TEST_K\n", __func__);
		break;
	case TEST_SE0_NAK:
		sunxi_info(NULL, "%s: TEST_SE0_NAK\n", __func__);
		break;
	case TEST_PACKET:
		sunxi_info(NULL, "%s: TEST_PACKET\n", __func__);
		break;
	case TEST_FORCE_EN:
		sunxi_info(NULL, "%s: TEST_FORCE_EN\n", __func__);
		break;
	default:
		sunxi_info(NULL, "not support test mode(%d)\n", param);
		return -1;
	}

	/* disabled port power */
	reg_value = readl(port->addr);
	reg_value &= ~PORT_POWER;
	writel(reg_value, port->addr);
	msleep(20);
	sunxi_info(NULL, "portsc: %#x\n", readl(port->addr));

	/* stop hci */
	reg_value = readl(&xhci->op_regs->command);
	reg_value &= ~CMD_RUN;
	writel(reg_value, &xhci->op_regs->command);
	msleep(20);

	/* wait for hci halted */
	ret = readl_poll_timeout_atomic(&xhci->op_regs->status, reg_value, (reg_value & STS_HALT), 5, 200000);
	if (ret) {
		sunxi_err(NULL, "timeout for waiting hci halted (%#x)\n", reg_value);
		return -EINVAL;
	}
	sunxi_info(NULL, "halted: %#x, param: %d\n", readl(&xhci->op_regs->status), param);

	/* set test mode */
	reg_value = readl(port->addr + 1);
	reg_value &= ~(0xf << PORT_TEST_MODE_SHIFT);
	reg_value |= (param << PORT_TEST_MODE_SHIFT);
	writel(reg_value, port->addr + 1);
	msleep(20);
	sunxi_info(NULL, "testmode: %#x\n", readl(port->addr + 1));

	return 0;
}


static int xhci_ed_test_show(struct seq_file *s, void *unused)
{
	char			buf[XHCI_MSG_MAX];

	sprintf(buf, "\n USB2.0 host test mode:\n"
				"echo:\ntest_j_state\ntest_k_state\ntest_se0_nak\n"
				"test_pack\ntest_force_enable\n\n");
	sunxi_info(NULL, "%s", buf);

	return 0;
}

static int xhci_ed_test_open(struct inode *inode, struct file *file)
{
	return single_open(file, xhci_ed_test_show, inode->i_private);
}

static ssize_t xhci_ed_test_write(struct file *file,  const char __user *ubuf,
				  size_t count, loff_t *ppos)
{
	struct seq_file		*s = file->private_data;
	struct xhci_port	*port = s->private;
	char			buf[32];
	u32 testmode = 0;

	if (copy_from_user(&buf, ubuf, min_t(size_t, sizeof(buf) - 1, count)))
		return -EFAULT;

	/* USB2.0 test mode */
	if (!strncmp(buf, "test_j_state", 12))
		testmode = TEST_J;
	else if (!strncmp(buf, "test_k_state", 12))
		testmode = TEST_K;
	else if (!strncmp(buf, "test_se0_nak", 12))
		testmode = TEST_SE0_NAK;
	else if (!strncmp(buf, "test_pack", 9))
		testmode = TEST_PACKET;
	else if (!strncmp(buf, "test_force_enable", 17))
		testmode = TEST_FORCE_EN;
	else
		testmode = 0;

	xhci_host_u2_test_mode(port, testmode);

	return count;
}

static const struct file_operations ed_test_fops = {
	.open			= xhci_ed_test_open,
	.write			= xhci_ed_test_write,
	.read			= seq_read,
	.llseek			= seq_lseek,
	.release		= single_release,
};

static void xhci_sunxi_debugfs_create_ed_test(struct xhci_hcd *xhci,
					      struct dentry *parent)
{
	struct xhci_port	*port;
	struct dentry		*dir;

	parent = debugfs_lookup("ports", parent);

	dir = debugfs_lookup("port01", parent);
	port = &xhci->hw_ports[0];
	debugfs_create_file("ed_test", 0644, dir, port, &ed_test_fops);
}

static void xhci_sunxi_debugfs_destroy_ed_test(struct xhci_hcd *xhci,
					       struct dentry *parent)
{
	struct dentry		*dir;

	parent = debugfs_lookup("ports", parent);
	dir = debugfs_lookup("port01", parent);
	debugfs_lookup_and_remove("ed_test", dir);
}

void xhci_sunxi_debug_init(struct dwc3 *dwc)
{
	struct platform_device *pdev = dwc->xhci;
	struct usb_hcd *hcd = NULL;
	struct xhci_hcd *xhci = NULL;

	if (!pdev)
		return;

	hcd = platform_get_drvdata(pdev);
	if (!hcd)
		return;

	xhci = hcd_to_xhci(hcd);

	xhci_sunxi_debugfs_create_ed_test(xhci, xhci->debugfs_root);
}

void xhci_sunxi_debug_exit(struct dwc3 *dwc)
{
	struct platform_device *pdev = dwc->xhci;
	struct usb_hcd *hcd = NULL;
	struct xhci_hcd *xhci = NULL;

	if (!pdev)
		return;

	hcd = platform_get_drvdata(pdev);
	if (!hcd)
		return;

	xhci = hcd_to_xhci(hcd);

	xhci_sunxi_debugfs_destroy_ed_test(xhci, xhci->debugfs_root);
}
