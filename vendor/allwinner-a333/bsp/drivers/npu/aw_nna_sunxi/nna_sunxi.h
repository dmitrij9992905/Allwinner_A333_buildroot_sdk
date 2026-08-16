/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef __NNA_SUNXI_H
#define __NNA_SUNXI_H

#ifndef SUNXI_MODNAME
#define SUNXI_MODNAME		"nna"
#endif
#include <sunxi-log.h>

#if IS_ENABLED(CONFIG_AW_LOG_VERBOSE)
#define NNA_ERR(fmt, arg...)	sunxi_err(NULL,   fmt, ##arg)
#define NNA_WARN(fmt, arg...)	sunxi_warn(NULL,  fmt, ##arg)
#define NNA_INFO(fmt, arg...)	sunxi_info(NULL,  fmt, ##arg)
#define NNA_DBG(fmt, arg...)	sunxi_debug(NULL, fmt, ##arg)
#else
#define NNA_ERR(fmt, arg...)	\
	sunxi_err(NULL,   "%d %s(): "fmt, __LINE__, __func__, ## arg)
#define NNA_WARN(fmt, arg...)	\
	sunxi_warn(NULL,  "%d %s(): "fmt, __LINE__, __func__, ## arg)
#define NNA_INFO(fmt, arg...)	\
	sunxi_info(NULL,  "%d %s(): "fmt, __LINE__, __func__, ## arg)
#define NNA_DBG(fmt, arg...)	\
	sunxi_debug(NULL, "%d %s(): "fmt, __LINE__, __func__, ## arg)
#endif

typedef enum clock_freq {
	NNA_CLOCK_300M   = 300,
	NNA_CLOCK_400M   = 400,
	NNA_CLOCK_600M   = 600,
	NNA_CLOCK_1200M  = 1200,
} clock_freq;

struct nna_dma_buf {
	struct list_head i_list;
	int fd;
	dma_addr_t phy_addr;
	struct dma_buf *dma_buf;
	struct dma_buf_attachment *attachment;
	struct sg_table *sgt;
	int p_id;
};

typedef struct nna_context {
	struct device *platform_dev;
	struct cdev *nna_cdev;
	dev_t devid;
	struct class *nna_class;
	struct device *nna_dev;

	/* dependent resource */
	void __iomem *io;
	resource_size_t io_start;
	u32 irq;
	unsigned int nna_irq_status;
	struct clk *clk;
	struct clk *clk_bus;
	struct clk *clk_mbus;
	struct reset_control *clk_rst;

	struct mutex mutex;
	wait_queue_head_t nna_queue;

	struct list_head list;		/* buffer list */
	unsigned int refs_count;
} nna_context;

#endif /* __NNA_SUNXI_H */
