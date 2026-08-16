/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Copyright(c) 2020 - 2023 Allwinner Technology Co.,Ltd. All rights reserved. */
/*
 * SUNXI SPI Controller Camera Driver
 *
 */

#define SUNXI_MODNAME "spi"
#include <sunxi-log.h>
#include <linux/kthread.h>
#include <linux/scatterlist.h>
#include "spi-sunxi-camera.h"
#include "../spi-sunxi-debug.h"

/* SPI Controller Hardware Register Operation Start */

void sunxi_spi_camera_enable_idlewait(struct sunxi_spi *sspi)
{
	u32 reg_val = readl(sspi->base_addr + SUNXI_SPI_TC_REG);
	reg_val |= SUNXI_SPI_TC_SIWE;
	writel(reg_val, sspi->base_addr + SUNXI_SPI_TC_REG);
}

void sunxi_spi_camera_disable_idlewait(struct sunxi_spi *sspi)
{
	u32 reg_val = readl(sspi->base_addr + SUNXI_SPI_TC_REG);
	reg_val &= ~SUNXI_SPI_TC_SIWE;
	writel(reg_val, sspi->base_addr + SUNXI_SPI_TC_REG);
}

void sunxi_spi_camera_enable_framehead(struct sunxi_spi *sspi)
{
	u32 reg_val = readl(sspi->base_addr + SUNXI_SPI_TC_REG);
	reg_val |= SUNXI_SPI_TC_SFHE;
	writel(reg_val, sspi->base_addr + SUNXI_SPI_TC_REG);
}

void sunxi_spi_camera_disable_framehead(struct sunxi_spi *sspi)
{
	u32 reg_val = readl(sspi->base_addr + SUNXI_SPI_TC_REG);
	reg_val &= ~SUNXI_SPI_TC_SFHE;
	writel(reg_val, sspi->base_addr + SUNXI_SPI_TC_REG);
}

void sunxi_spi_camera_enable_vsync(struct sunxi_spi *sspi)
{
	u32 reg_val = readl(sspi->base_addr + SUNXI_SPI_TC_REG);
	reg_val |= SUNXI_SPI_TC_VIE;
	writel(reg_val, sspi->base_addr + SUNXI_SPI_TC_REG);
}

void sunxi_spi_camera_disable_vsync(struct sunxi_spi *sspi)
{
	u32 reg_val = readl(sspi->base_addr + SUNXI_SPI_TC_REG);
	reg_val &= ~SUNXI_SPI_TC_VIE;
	writel(reg_val, sspi->base_addr + SUNXI_SPI_TC_REG);
}

static void sunxi_spi_camera_vsync_input_select(struct sunxi_spi *sspi, bool edge)
{
	u32 reg_val = readl(sspi->base_addr + SUNXI_SPI_TC_REG);

	if (edge)
		reg_val |= SUNXI_SPI_TC_VIS;
	else
		reg_val &= ~SUNXI_SPI_TC_VIS;

	writel(reg_val, sspi->base_addr + SUNXI_SPI_TC_REG);
}

static void sunxi_spi_camera_set_vsync_freq_relation(struct sunxi_spi *sspi, u32 ahb_clk, u32 sclk)
{
	u32 reg_val = readl(sspi->base_addr + SUNXI_SPI_SVCN_REG);
	u8 val = (ahb_clk / sclk) - 1;

	reg_val &= ~SUNXI_SPI_SVCN_SFT;
	reg_val |= FIELD_PREP(SUNXI_SPI_SVCN_SFT, val);
	writel(reg_val, sspi->base_addr + SUNXI_SPI_SVCN_REG);
}

static void sunxi_spi_camera_set_vsync_cycle_number(struct sunxi_spi *sspi, u32 num)
{
	u32 reg_val = readl(sspi->base_addr + SUNXI_SPI_SVCN_REG);
	reg_val &= ~SUNXI_SPI_SVCN_SVCN;
	/* Convert Byte to Cycle -> 1Byte = 8 Cycle */
	reg_val |= FIELD_PREP(SUNXI_SPI_SVCN_SVCN, (num << 3));
	writel(reg_val, sspi->base_addr + SUNXI_SPI_SVCN_REG);
}

static void sunxi_spi_camera_set_frame_head_number(struct sunxi_spi *sspi, u32 num)
{
	u32 reg_val = readl(sspi->base_addr + SUNXI_SPI_SFHN_REG);
	reg_val &= ~SUNXI_SPI_SFHN;
	reg_val |= FIELD_PREP(SUNXI_SPI_SFHN, num - 1);
	writel(reg_val, sspi->base_addr + SUNXI_SPI_SFHN_REG);
}

static int sunxi_spi_camera_set_frame_head(struct sunxi_spi *sspi, u8 *buf, int len)
{
	u64 reg_val = 0;
	int i;

	if (len > SUNXI_SPI_FRAMEHEAD_MAX)
		return -EINVAL;

	for (i = len; i > 0; i--)
		reg_val |= (buf[len - i] << ((i - 1) * 8));

	writel((reg_val >> 32) & 0xffffffff, sspi->base_addr + SUNXI_SPI_SFHH_REG);
	writel((reg_val >>  0) & 0xffffffff, sspi->base_addr + SUNXI_SPI_SFHL_REG);
	return 0;
}

static int sunxi_spi_camera_get_frame_head(struct sunxi_spi *sspi, u8 *buf, int len)
{
	u64 reg_val = 0;
	int i;

	if (len > SUNXI_SPI_FRAMEHEAD_MAX)
		return -EINVAL;

	reg_val |= readl(sspi->base_addr + SUNXI_SPI_SFHHR_REG);
	reg_val <<= 32;
	reg_val |= readl(sspi->base_addr + SUNXI_SPI_SFHLR_REG);

	for (i = len; i > 0; i--)
		buf[len - i] = (reg_val >> ((i - 1) * 8)) & 0xff;
	return 0;
}

static void sunxi_spi_camera_set_idlewait_enable_value(struct sunxi_spi *sspi, u32 ahb_clk, u32 us)
{
	u32 ahb_time = 1000000000UL / ahb_clk;	/* ns per ahb clk cycle */
	u32 value = DIV_ROUND_UP(us * 1000, ahb_time);
	writel(value, sspi->base_addr + SUNXI_SPI_SIWVE_REG);
}

static void sunxi_spi_camera_set_idlewait_reset_value(struct sunxi_spi *sspi, u32 ahb_clk, u32 mclk, u32 cycle)
{
	u32 reg_val = readl(sspi->base_addr + SUNXI_SPI_SIWVR_REG);
	u32 ahb_time = 1000000000UL / ahb_clk;	/* ns per ahb clk cycle */
	u32 clk_time = 1000000000UL / mclk;		/* ns per spi clk cycle */
	u32 value = DIV_ROUND_UP(clk_time * cycle, ahb_time);

	reg_val &= ~SUNXI_SPI_SIWVR;
	reg_val |= FIELD_PREP(SUNXI_SPI_SIWVR, value);

	writel(reg_val, sspi->base_addr + SUNXI_SPI_SIWVR_REG);
}

/* SPI Controller Hardware Register Operation End */

static int sunxi_spi_camera_map_msg(struct spi_controller *ctlr, struct spi_message *msg)
{
	struct spi_transfer *xfer;
	int ret;

	list_for_each_entry(xfer, &msg->transfers, transfer_list) {
		struct sg_table *sgt = &xfer->rx_sg;
		dma_addr_t buf = xfer->rx_dma;
		size_t len = xfer->len;
		struct scatterlist *sg;

		ret = sg_alloc_table(sgt, 1, GFP_KERNEL);
		if (ret != 0)
			return ret;

		sg = &sgt->sgl[0];
		sg_init_table(sg, 1);
		sg_dma_address(sg) = buf;
		sg_dma_len(sg) = len;
		sgt->nents = 1;
	}

	ctlr->cur_msg_mapped = true;

	return 0;
}

static int sunxi_spi_camera_unmap_msg(struct spi_controller *ctlr, struct spi_message *msg)
{
	struct spi_transfer *xfer;

	if (!ctlr->cur_msg_mapped)
		return 0;

	list_for_each_entry(xfer, &msg->transfers, transfer_list) {
		struct sg_table *sgt = &xfer->rx_sg;

		sgt->nents = 0;
		sg_free_table(sgt);
	}

	ctlr->cur_msg_mapped = false;

	return 0;
}

static int sunxi_spi_camera_transfer_one_message(struct spi_controller *ctlr, struct spi_message *msg)
{
	struct sunxi_spi *sspi = spi_controller_get_devdata(ctlr);
	struct spi_transfer *xfer;
	bool framehead_split = false;
	int ret = 0;

	/* In framehead mode controller will not received  the frist frame head into fifo.
	 * Readback these data from register and put it back into rx_buf.
	 */
	if (sspi->bus_mode == SUNXI_SPI_BUS_CAMERA && sspi->camera_mode == SUNXI_SPI_CAMERA_FRAMEHEAD)
		framehead_split = true;

	while (!sspi->slave_aborted) {
		msg->actual_length = 0;

		list_for_each_entry(xfer, &msg->transfers, transfer_list) {
			if (!xfer->rx_buf || !xfer->rx_dma || !xfer->len) {
				sunxi_err(sspi->dev, "camera transfer xfer error buf:%p len:%d\n", xfer->rx_buf, xfer->len);
				ret = -EINVAL;
				break;
			}

xfer_single_loop:
			if (framehead_split) {
				xfer->len -= sspi->camera_framehead_len;
				sg_dma_len(xfer->rx_sg.sgl) = xfer->len;
			}

			ret = ctlr->transfer_one(ctlr, msg->spi, xfer);

			if (!ret && framehead_split) {
				memmove(xfer->rx_buf + sspi->camera_framehead_len, xfer->rx_buf, xfer->len);
				sunxi_spi_camera_get_frame_head(sspi, xfer->rx_buf, sspi->camera_framehead_len);
				xfer->len += sspi->camera_framehead_len;
				sg_dma_len(xfer->rx_sg.sgl) = xfer->len;
				framehead_split = false;
			}

			if (sspi->slave_aborted)
				msg->status = -EINTR;
			else
				msg->status = ret;

			if (msg->complete)
				msg->complete(msg->context);

			if (msg->status < 0)
				break;

			msg->actual_length += xfer->len;

			if (sspi->camera_xfer_single_loop)
				goto xfer_single_loop;
		}

		if (msg->status < 0) {
			if (msg->status == -ECANCELED)
				msg->status = ret;
			else
				break;
		}
	}

	if (msg->status && ctlr->handle_err)
		ctlr->handle_err(ctlr, msg);

	return 0;
}

static void sunxi_spi_camera_pump_message(struct kthread_work *work)
{
	struct spi_controller *ctlr = container_of(work, struct spi_controller, pump_messages);
	struct spi_message *msg;
	unsigned long flags;
	int ret;

	/* Lock queue */
	spin_lock_irqsave(&ctlr->queue_lock, flags);

	/* Make sure we are not already running a message */
	if (ctlr->cur_msg) {
		dev_err(&ctlr->dev, "cur msg not empty\n");
		spin_unlock_irqrestore(&ctlr->queue_lock, flags);
		return;
	};

	if (list_empty(&ctlr->queue)) {
		dev_err(&ctlr->dev, "queue list is empty\n");
		spin_unlock_irqrestore(&ctlr->queue_lock, flags);
		return ;
	}

	/* Extract head of queue */
	msg = list_first_entry(&ctlr->queue, struct spi_message, queue);
	ctlr->cur_msg = msg;

	list_del_init(&msg->queue);
	ctlr->busy = true;
	spin_unlock_irqrestore(&ctlr->queue_lock, flags);

	mutex_lock(&ctlr->io_mutex);

	ret = sunxi_spi_camera_map_msg(ctlr, msg);
	if (ret) {
		dev_err(&ctlr->dev, "failed to map message %d\n", ret);
		goto out;
	}

	if (ctlr->prepare_message)
		ctlr->prepare_message(ctlr, msg);

	sunxi_spi_camera_transfer_one_message(ctlr, msg);

	if (ctlr->unprepare_message)
		ctlr->unprepare_message(ctlr, msg);

	sunxi_spi_camera_unmap_msg(ctlr, msg);

out:
	mutex_unlock(&ctlr->io_mutex);

	spin_lock_irqsave(&ctlr->queue_lock, flags);
	ctlr->running = false;
	ctlr->cur_msg = NULL;
	msg->status = 0;
	spin_unlock_irqrestore(&ctlr->queue_lock, flags);
}

static int sunxi_spi_camera_transfer(struct spi_device *spi, struct spi_message *msg)
{
	struct spi_controller *ctlr = spi->controller;
	unsigned long flags;
	int ret = 0;

	spin_lock_irqsave(&ctlr->queue_lock, flags);

	if (ctlr->running) {
		ret = -EBUSY;
		goto err;
	}

	if (!msg->is_dma_mapped) {
		ret = -EINVAL;
		goto err;
	}

	ctlr->running = true;
	msg->actual_length = 0;
	msg->status = -EINPROGRESS;

	list_add_tail(&msg->queue, &ctlr->queue);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 9, 0))
	kthread_queue_work(ctlr->kworker, &ctlr->pump_messages);
#else
	kthread_queue_work(&ctlr->kworker, &ctlr->pump_messages);
#endif

err:
	spin_unlock_irqrestore(&ctlr->queue_lock, flags);
	return ret;
}

int sunxi_spi_camera_transfer_init(struct sunxi_spi *sspi)
{
	struct spi_controller *ctlr = sspi->ctlr;

	ctlr->transfer = sunxi_spi_camera_transfer;
	ctlr->running = false;
	ctlr->cur_msg = NULL;
	ctlr->cur_msg_mapped = false;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 9, 0))
	ctlr->kworker = kthread_create_worker(0, dev_name(sspi->dev));
	if (IS_ERR(ctlr->kworker)) {
		sunxi_err(sspi->dev, "failed to create message camera kworker\n");
		return PTR_ERR(ctlr->kworker);
	}
#else
	kthread_init_worker(&ctlr->kworker);
	ctlr->kworker_task = kthread_run(kthread_worker_fn, &ctlr->kworker, "%s", dev_name(&ctlr->dev));
	if (IS_ERR(ctlr->kworker_task)) {
		dev_err(&ctlr->dev, "failed to create message pump task\n");
		return PTR_ERR(ctlr->kworker_task);
	}
#endif

	kthread_init_work(&ctlr->pump_messages, sunxi_spi_camera_pump_message);

	sspi->rx_triglevel = 4;

	return 0;
}

void sunxi_spi_camera_transfer_exit(struct sunxi_spi *sspi)
{
	struct spi_controller *ctlr = sspi->ctlr;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 9, 0))
	kthread_destroy_worker(ctlr->kworker);
#else
	kthread_flush_worker(&ctlr->kworker);
	kthread_stop(ctlr->kworker_task);
#endif
}

int sunxi_spi_camera_set_mode(struct spi_device *spi, enum sunxi_spi_camera_mode mode)
{
	struct sunxi_spi *sspi = spi_controller_get_devdata(spi->controller);

	if (sspi->bus_mode != SUNXI_SPI_BUS_CAMERA) {
		sunxi_err(sspi->dev, "bus mode %#x unsupport camera feature\n", sspi->bus_mode);
		return -EINVAL;
	}

	sspi->camera_mode = mode;

	return 0;
}
EXPORT_SYMBOL_GPL(sunxi_spi_camera_set_mode);

int sunxi_spi_camera_set_vsync(struct spi_device *spi, u32 len)
{
	struct sunxi_spi *sspi = spi_controller_get_devdata(spi->controller);
	int ret = 0;

	if (sspi->bus_mode == SUNXI_SPI_BUS_CAMERA && sspi->camera_mode == SUNXI_SPI_CAMERA_VSYNC) {
#ifdef CONFIG_AW_IC_BOARD
		sunxi_spi_camera_set_vsync_freq_relation(sspi, clk_get_rate(sspi->ahb_clk), clk_get_rate(sspi->mclk));
#else
		sunxi_spi_camera_set_vsync_freq_relation(sspi, 24000000, clk_get_rate(sspi->mclk));
#endif
		sunxi_spi_camera_vsync_input_select(sspi, !(spi->mode & SPI_CS_HIGH));
		sunxi_spi_camera_set_vsync_cycle_number(sspi, len);
	} else {
		sunxi_err(sspi->dev, "vsync set unsupport, bus_%#x camera_%d\n", sspi->bus_mode, sspi->camera_mode);
		ret = -EINVAL;
	}

	return ret;
}
EXPORT_SYMBOL_GPL(sunxi_spi_camera_set_vsync);

int sunxi_spi_camera_set_framehead_flag(struct spi_device *spi, u8 *buf, int len)
{
	struct sunxi_spi *sspi = spi_controller_get_devdata(spi->controller);
	int ret = 0;

	if (sspi->bus_mode == SUNXI_SPI_BUS_CAMERA && sspi->camera_mode == SUNXI_SPI_CAMERA_FRAMEHEAD) {
		if (len > SUNXI_SPI_FRAMEHEAD_MAX) {
			sunxi_err(sspi->dev, "set framehead len %d overflow\n", len);
			ret = -EINVAL;
		} else {
			sspi->camera_framehead_len = len;
			sunxi_spi_camera_set_frame_head_number(sspi, len);
			sunxi_spi_camera_set_frame_head(sspi, buf, len);
		}
	} else {
		sunxi_err(sspi->dev, "framehead set unsupport, bus_%#x camera_%d\n", sspi->bus_mode, sspi->camera_mode);
		ret = -EINVAL;
	}

	return ret;
}
EXPORT_SYMBOL_GPL(sunxi_spi_camera_set_framehead_flag);

int sunxi_spi_camera_get_framehead_flag(struct spi_device *spi, u8 *buf, int len)
{
	struct sunxi_spi *sspi = spi_controller_get_devdata(spi->controller);
	int ret = 0;

	if (sspi->bus_mode == SUNXI_SPI_BUS_CAMERA && sspi->camera_mode == SUNXI_SPI_CAMERA_FRAMEHEAD) {
		if (len > SUNXI_SPI_FRAMEHEAD_MAX || len != sspi->camera_framehead_len) {
			sunxi_err(sspi->dev, "get framehead len %d not correct\n", len);
			ret = -EINVAL;
		} else {
			sunxi_spi_camera_get_frame_head(sspi, buf, len);
		}
	} else {
		sunxi_err(sspi->dev, "framehead get unsupport, bus_%#x camera_%d\n", sspi->bus_mode, sspi->camera_mode);
		ret = -EINVAL;
	}

	return ret;
}
EXPORT_SYMBOL_GPL(sunxi_spi_camera_get_framehead_flag);

int sunxi_spi_camera_set_idlewait_us(struct spi_device *spi, u32 us)
{
	struct sunxi_spi *sspi = spi_controller_get_devdata(spi->controller);
	int ret = 0;

	if (sspi->bus_mode == SUNXI_SPI_BUS_CAMERA && sspi->camera_mode == SUNXI_SPI_CAMERA_IDLEWAIT) {
#if IS_ENABLED(CONFIG_AW_IC_BOARD)
		sunxi_spi_camera_set_idlewait_enable_value(sspi, clk_get_rate(sspi->ahb_clk), us);
		sunxi_spi_camera_set_idlewait_reset_value(sspi, clk_get_rate(sspi->ahb_clk), clk_get_rate(sspi->mclk), 5);
#else
		sunxi_spi_camera_set_idlewait_enable_value(sspi, 24000000, us);
		sunxi_spi_camera_set_idlewait_reset_value(sspi, 24000000, clk_get_rate(sspi->mclk), 5);
#endif
	} else {
		sunxi_err(sspi->dev, "idlewait set unsupport, bus_%#x camera_%d\n", sspi->bus_mode, sspi->camera_mode);
		ret = -EINVAL;
	}

	return ret;
}
EXPORT_SYMBOL_GPL(sunxi_spi_camera_set_idlewait_us);

void sunxi_spi_camera_set_single_transfer_loop(struct spi_device *spi, bool enable)
{
	struct sunxi_spi *sspi = spi_controller_get_devdata(spi->controller);

	sspi->camera_xfer_single_loop = enable;
}
EXPORT_SYMBOL_GPL(sunxi_spi_camera_set_single_transfer_loop);
