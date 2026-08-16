/* SPDX-License-Identifier: GPL-2.0-or-later WITH Linux-syscall-note */

#ifndef __SUNXI_NNA_H
#define __SUNXI_NNA_H

#include <linux/ioctl.h>

#define NNA_CMD_IRQ_ENABLE	_IO('A', 0x0)
#define NNA_CMD_IRQ_DISABLE	_IO('A', 0x1)
#define NNA_CMD_IRQ_WAIT	_IOW('A', 0x2, unsigned int)

#define NNA_CMD_SET_FREQ	_IOW('A', 0x10, unsigned int)
#define NNA_CMD_RESET_NNA	_IO('A', 0x11)

#define NNA_CMD_MAP_DMA_FD	_IOWR('A', 0x30, struct nna_dma_buf_param)
#define NNA_CMD_UNMAP_DMA_FD	_IOW('A', 0x31, struct nna_dma_buf_param)
#define NNA_CMD_DMA_SYNC	_IOW('A', 0x32, struct nna_dma_sync_param)

struct nna_dma_buf_param {
	int fd;
	unsigned long phy_addr;
};

struct nna_dma_sync_param {
	unsigned long phy_addr;
	unsigned int size;
	unsigned int dir;	/* 0: dev->cpu, 1: cpu->dev */
};

#endif /* __SUNXI_NNA_H */
