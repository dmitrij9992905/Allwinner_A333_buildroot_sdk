// SPDX-License-Identifier: GPL-2.0-or-later

#define SUNXI_MODNAME		"nna"
#include "nna_sunxi.h"
#include <asm/io.h>
#include <linux/cdev.h>
#include <linux/of_device.h>
#include <linux/of_address.h>
#include <linux/dma-buf.h>
#include <linux/mm.h>
#include <linux/list.h>
#include <linux/clk.h>
#include <linux/reset.h>
#include <linux/interrupt.h>
#include <uapi/linux/sunxi_nna.h>

#define TIMEOUT_MS		3000
#define AWNN_GLB_INTR_STATUS	0xc

static nna_context *nna_priv;

static irqreturn_t nna_interrupt(int irq, void *dev_id)
{
	u32 val;
	(void)irq;
	(void)dev_id;

	val = readl(nna_priv->io + AWNN_GLB_INTR_STATUS);
	writel(val, nna_priv->io + AWNN_GLB_INTR_STATUS);

	nna_priv->nna_irq_status |= val;
	wake_up(&nna_priv->nna_queue);
	return IRQ_HANDLED;
}

static int nna_mmap_dma_fd(struct nna_dma_buf_param *dma_buf_param)
{
	struct nna_dma_buf *nna_buf = NULL;

	nna_buf = (struct nna_dma_buf *)kzalloc(sizeof(struct nna_dma_buf), GFP_KERNEL);
	if (!nna_buf) {
		NNA_ERR("malloc nna_dma_buf failed\n");
		return -EFAULT;
	}

	nna_buf->fd = dma_buf_param->fd;
	nna_buf->dma_buf = dma_buf_get(nna_buf->fd);
	if (IS_ERR_OR_NULL(nna_buf->dma_buf)) {
		NNA_ERR("dma_buf_get failed");
		goto err1;
	}
	nna_buf->attachment = dma_buf_attach(nna_buf->dma_buf, nna_priv->platform_dev);
	if (IS_ERR_OR_NULL(nna_buf->attachment)) {
		NNA_ERR("dma_buf_attach failed");
		goto err2;
	}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(6, 6, 0))
	nna_buf->sgt = dma_buf_map_attachment_unlocked(nna_buf->attachment, DMA_BIDIRECTIONAL);
#else
	nna_buf->sgt = dma_buf_map_attachment(nna_buf->attachment, DMA_BIDIRECTIONAL);
#endif
	if (IS_ERR_OR_NULL(nna_buf->sgt)) {
		NNA_ERR("dma_buf_map_attachment failed\n");
		goto err3;
	}

	nna_buf->fd = dma_buf_param->fd;
	nna_buf->phy_addr = sg_dma_address(nna_buf->sgt->sgl);
	nna_buf->p_id = current->tgid;
	list_add_tail(&nna_buf->i_list, &nna_priv->list);

	dma_buf_param->phy_addr = nna_buf->phy_addr;
	NNA_DBG("fd:%d, phy_addr:%zx, dma_buf:%p, dma_buf_attach:%p, "
		"sg_table:%p, nents:%d, pid:%d\n",
		nna_buf->fd,
		nna_buf->phy_addr,
		nna_buf->dma_buf,
		nna_buf->attachment,
		nna_buf->sgt,
		nna_buf->sgt->nents,
		nna_buf->p_id);
	return 0;
err3:
	dma_buf_detach(nna_buf->dma_buf, nna_buf->attachment);
err2:
	dma_buf_put(nna_buf->dma_buf);
err1:
	if (nna_buf)
		kfree(nna_buf);
	return -1;
}

static int nna_unmap_dma_fd(struct nna_dma_buf_param *dma_buf_param)
{
	struct nna_dma_buf *nna_buf = NULL;
	unsigned int find_nna_dma_buf = 0;

	list_for_each_entry(nna_buf, &nna_priv->list, i_list) {
		if (nna_buf->fd == dma_buf_param->fd && nna_buf->p_id == current->tgid) {
			find_nna_dma_buf = 1;
			break;
		}
	}
	if (!find_nna_dma_buf) {
		NNA_ERR("nna_dma_buf invalid, dma_fd %d, pid %d\n", dma_buf_param->fd, current->tgid);
		return -1;
	}
	NNA_DBG("free: fd:%d, phy_addr:%zx, dma_buf:%p, dma_buf_attach:%p, "
		"sg_table:%p nets:%d, pid:%d\n",
		nna_buf->fd,
		nna_buf->phy_addr,
		nna_buf->dma_buf,
		nna_buf->attachment,
		nna_buf->sgt,
		nna_buf->sgt->nents,
		nna_buf->p_id);

	if (!IS_ERR_OR_NULL(nna_buf->dma_buf)) {
		if (!IS_ERR_OR_NULL(nna_buf->attachment)) {
			if (!IS_ERR_OR_NULL(nna_buf->sgt)) {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(6, 6, 0))
				dma_buf_unmap_attachment_unlocked(nna_buf->attachment,
								  nna_buf->sgt,
								  DMA_BIDIRECTIONAL);
#else
				dma_buf_unmap_attachment(nna_buf->attachment,
							 nna_buf->sgt,
							 DMA_BIDIRECTIONAL);
#endif
			}
			dma_buf_detach(nna_buf->dma_buf, nna_buf->attachment);
		}
		dma_buf_put(nna_buf->dma_buf);
	}
	list_del(&nna_buf->i_list);
	kfree(nna_buf);
	return 0;
}

static long nna_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	int ret = 0;
	s32 restime = 0;
	struct nna_dma_buf_param dma_buf_param;
	struct nna_dma_sync_param dma_sync_param;

	mutex_lock(&nna_priv->mutex);

	switch (cmd) {
	case NNA_CMD_IRQ_ENABLE:
	{
		NNA_DBG("enable nna irq\n");
		enable_irq(nna_priv->irq);
		break;
	}
	case NNA_CMD_IRQ_DISABLE:
	{
		NNA_DBG("disable nna irq\n");
		disable_irq(nna_priv->irq);
		break;
	}
	case NNA_CMD_IRQ_WAIT:
	{
		restime = wait_event_timeout(nna_priv->nna_queue,
					     arg == (arg & nna_priv->nna_irq_status),
					     msecs_to_jiffies(TIMEOUT_MS));
		if (restime <= 0) {
			NNA_ERR("wait status timeout arg 0x%lx, status 0x%x\n",
				arg, nna_priv->nna_irq_status);
			ret = -1;
			nna_priv->nna_irq_status = 0;
		} else {
			nna_priv->nna_irq_status = 0;
			ret = nna_priv->nna_irq_status;
		}
		break;
	}
	case NNA_CMD_SET_FREQ:
	{
		if (arg > NNA_CLOCK_1200M) {
			NNA_WARN("freq set too large:%ld\n", arg);
			ret = -EFAULT;
			break;
		}
		NNA_DBG("old clk freq:%ld\n", clk_get_rate(nna_priv->clk));
		clk_set_rate(nna_priv->clk, arg);
		NNA_DBG("new clk freq:%ld\n", clk_get_rate(nna_priv->clk));
		break;
	}
	case NNA_CMD_RESET_NNA:
	{
		reset_control_assert(nna_priv->clk_rst);
		reset_control_deassert(nna_priv->clk_rst);
		break;
	}
	case NNA_CMD_MAP_DMA_FD:
	{
		if (copy_from_user(&dma_buf_param, (void __user *)arg,
				   sizeof(struct nna_dma_buf_param))) {
			NNA_ERR("NNA_CMD_MAP_DMA_FD copy_from_user failed\n");
			ret = -EFAULT;
			break;
		}
		if (nna_mmap_dma_fd(&dma_buf_param)) {
			NNA_ERR("nna_mmap_dma_fd failed\n");
			ret = -EFAULT;
			break;
		}
		if (copy_to_user((void __user *)arg, &dma_buf_param,
				 sizeof(struct nna_dma_buf_param))) {
			NNA_ERR("NNA_CMD_MAP_DMA_FD copy_to_user failed\n");
			ret = -EFAULT;
			nna_unmap_dma_fd(&dma_buf_param);
			break;
		}
		ret = 0;
		break;
	}
	case NNA_CMD_UNMAP_DMA_FD:
	{
		if (copy_from_user(&dma_buf_param, (void __user *)arg,
				   sizeof(struct nna_dma_buf_param))) {
			NNA_ERR("NNA_CMD_UNMAP_DMA_FD copy_from_user failed\n");
			ret = -EFAULT;
			break;
		}
		if (nna_unmap_dma_fd(&dma_buf_param)) {
			NNA_ERR("nna_unmap_dma_fd failed\n");
			ret = -EFAULT;
		}
		break;
	}
	case NNA_CMD_DMA_SYNC:
	{
		if (copy_from_user(&dma_sync_param, (void __user *)arg,
				   sizeof(struct nna_dma_sync_param))) {
			NNA_ERR("NNA_CMD_DMA_SYNC copy_from_user failed\n");
			ret = -EFAULT;
			break;
		}
		if (dma_sync_param.dir == 1) {
			dma_sync_single_for_device(nna_priv->platform_dev, dma_sync_param.phy_addr,
						   dma_sync_param.size, DMA_TO_DEVICE);
		} else if (dma_sync_param.dir == 0) {
			dma_sync_single_for_cpu(nna_priv->platform_dev, dma_sync_param.phy_addr,
						dma_sync_param.size, DMA_FROM_DEVICE);
		} else {
			ret = -EFAULT;
		}
		break;
	}
	default:
		ret = -EINVAL;
		NNA_ERR("ioctrl cmd invalid %u\n", cmd);
		break;
	}
	mutex_unlock(&nna_priv->mutex);
	return ret;
}

static int nna_mmap(struct file *filp, struct vm_area_struct *vma)
{
	unsigned long temp_pfn;

	if (vma->vm_end - vma->vm_start == 0) {
		NNA_WARN("vma->vm_end is equal vma->vm_start: %lx\n", vma->vm_start);
		return 0;
	}
	if (vma->vm_pgoff > (~0UL >> PAGE_SHIFT)) {
		NNA_WARN("the vma->vm_pgoff is %lx,it is large than the largest page number\n",
			 vma->vm_pgoff);
		return -EINVAL;
	}

	temp_pfn = nna_priv->io_start >> 12;

	/* Set reserved and I/O flag for the area. */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(6, 1, 25))
	vm_flags_set(vma, VM_IO);
#else
	vma->vm_flags |= /* VM_RESERVED | */VM_IO;
#endif
	/* Select uncached access. */
	vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);

	if (io_remap_pfn_range(vma, vma->vm_start, temp_pfn, vma->vm_end - vma->vm_start,
			       vma->vm_page_prot)) {
		NNA_ERR("io_remap_pfn_range failed\n");
		return -EAGAIN;
	}

	return 0;
}

static int nna_open(struct inode *inode, struct file *file)
{
	int ret = 0;

	mutex_lock(&nna_priv->mutex);
	if (nna_priv->refs_count > 0) {
		NNA_DBG("open device refs_count:%d", nna_priv->refs_count);
		nna_priv->refs_count++;
		mutex_unlock(&nna_priv->mutex);
		return 0;
	}
	NNA_DBG("nna_open irqnum = %d\n", nna_priv->irq);
	INIT_LIST_HEAD(&nna_priv->list);
	if (reset_control_reset(nna_priv->clk_rst)) {
		NNA_ERR("reset control deassert  failed!\n");
		ret = -EINVAL;
		goto err1;
	}

	if (nna_priv->clk) {
		if (clk_prepare_enable(nna_priv->clk)) {
			NNA_ERR("enable npu clock failed!\n");
			ret = -EINVAL;
			goto err2;
		}
	}
	if (nna_priv->clk_bus) {
		if (clk_prepare_enable(nna_priv->clk_bus)) {
			NNA_ERR("enable npu bus clock failed!\n");
			ret = -EINVAL;
			goto err3;
		}
	}
	if (nna_priv->clk_mbus) {
		if (clk_prepare_enable(nna_priv->clk_mbus)) {
			NNA_ERR("enable npu mbus clock failed!\n");
			ret = -EINVAL;
			goto err4;
		}
	}

	nna_priv->refs_count++;
	mutex_unlock(&nna_priv->mutex);
	return ret;

err4:
	clk_disable_unprepare(nna_priv->clk_bus);
err3:
	clk_disable_unprepare(nna_priv->clk);
err2:
	reset_control_assert(nna_priv->clk_rst);
err1:
	mutex_unlock(&nna_priv->mutex);
	return ret;

}

static int nna_release(struct inode *inode, struct file *file)
{
	struct list_head *pos, *q;
	struct nna_dma_buf *nna_buf;

	mutex_lock(&nna_priv->mutex);
	if (nna_priv->refs_count > 1) {
		nna_priv->refs_count--;
		NNA_DBG("close device refs_count:%d", nna_priv->refs_count);
		mutex_unlock(&nna_priv->mutex);
		return 0;
	}
	NNA_DBG("nna_release irqnum = %d\n", nna_priv->irq);
	list_for_each_safe(pos, q, &nna_priv->list) {
		nna_buf = list_entry(pos, struct nna_dma_buf, i_list);
		if (nna_buf->p_id == current->tgid)
			continue;
		if (!IS_ERR_OR_NULL(nna_buf->dma_buf)) {
			if (!IS_ERR_OR_NULL(nna_buf->attachment)) {
				if (!IS_ERR_OR_NULL(nna_buf->sgt)) {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(6, 6, 0))
					dma_buf_unmap_attachment_unlocked(nna_buf->attachment,
									  nna_buf->sgt,
									  DMA_BIDIRECTIONAL);
#else
					dma_buf_unmap_attachment(nna_buf->attachment,
								 nna_buf->sgt,
								 DMA_BIDIRECTIONAL);
#endif
				}
				dma_buf_detach(nna_buf->dma_buf, nna_buf->attachment);
			}
			dma_buf_put(nna_buf->dma_buf);
		}
		list_del(&nna_buf->i_list);
		kfree(nna_buf);
	}
	nna_priv->refs_count--;

	clk_disable_unprepare(nna_priv->clk);
	clk_disable_unprepare(nna_priv->clk_bus);
	clk_disable_unprepare(nna_priv->clk_mbus);
	reset_control_assert(nna_priv->clk_rst);

	mutex_unlock(&nna_priv->mutex);
	return 0;
}

static const struct file_operations nna_fops = {
	.owner = THIS_MODULE,
	.open = nna_open,
	.release = nna_release,
	.mmap = nna_mmap,
	.unlocked_ioctl = nna_ioctl,
};

static struct attribute *nna_attributes[] = {
	/* &dev_attr_debug.attr, */
	/* &dev_attr_func_runtime.attr, */
	NULL
};

static struct attribute_group nna_attribute_group = {
	.name = "attr",
	.attrs = nna_attributes
};

static int nna_init(nna_context *devp)
{
	if (alloc_chrdev_region(&devp->devid, 0, 1, "nna_chrdev")) {
		NNA_ERR("alloc_chrdev_region failed\n");
		return -1;
	}

	devp->nna_cdev = cdev_alloc();
	if (devp->nna_cdev == NULL) {
		NNA_ERR("cdev_alloc failed\n");
		return -1;
	}
	cdev_init(devp->nna_cdev, &nna_fops);
	devp->nna_cdev->owner = THIS_MODULE;
	if (cdev_add(devp->nna_cdev, devp->devid, 1)) {
		NNA_ERR("major number %d but add failed\n", MAJOR(devp->devid));
		goto err1;
	}

	devp->nna_class = class_create("nna");
	if (IS_ERR_OR_NULL(devp->nna_class)) {
		NNA_ERR("class_create failed\n");
		goto err2;
	}

	devp->nna_dev = device_create(devp->nna_class, NULL, devp->devid, NULL, "nna");
	if (IS_ERR_OR_NULL(devp->nna_dev)) {
		NNA_ERR("device_create failed\n");
		goto err3;
	}

	if (sysfs_create_group(&devp->nna_dev->kobj, &nna_attribute_group) < 0) {
		NNA_ERR("sysfs_create_file fail!\n");
		goto err4;
	}

	return 0;

err4:
	device_destroy(devp->nna_class, devp->devid);
err3:
	class_destroy(devp->nna_class);
err2:
	cdev_del(devp->nna_cdev);
err1:
	unregister_chrdev_region(devp->devid, 1);
	return -1;
}

static int nna_exit(nna_context *devp)
{
	sysfs_remove_group(&devp->nna_dev->kobj, &nna_attribute_group);
	if (!IS_ERR_OR_NULL(devp->nna_dev))
		device_destroy(devp->nna_class, devp->devid);
	if (!IS_ERR_OR_NULL(devp->nna_class))
		class_destroy(devp->nna_class);
	cdev_del(devp->nna_cdev);
	unregister_chrdev_region(devp->devid, 1);
	return 0;
}

static int nna_probe(struct platform_device *pdev)
{
	int ret = 0;
	struct device *dev = &pdev->dev;
	struct device_node *node = pdev->dev.of_node;
	struct resource res;

	NNA_INFO("start nna_probe\n");

	if (!nna_priv) {
		nna_priv = devm_kzalloc(dev, sizeof(*nna_priv), GFP_KERNEL);
	} else {
		return -EAGAIN;
	}
	if (IS_ERR_OR_NULL(nna_priv)) {
		NNA_ERR("malloc mem for nna device err!\n");
		return -ENOMEM;
	}
	memset(nna_priv, 0, sizeof(nna_context));

	nna_priv->platform_dev = dev;
	if (nna_init(nna_priv) != 0) {
		NNA_ERR("nna init failed!\n");
		goto exit;
	}

	nna_priv->io = devm_platform_ioremap_resource(pdev, 0);
	if (IS_ERR_OR_NULL(nna_priv->io)) {
		NNA_ERR("failed to get npu base addr\n");
		ret = PTR_ERR(nna_priv->io);
		goto exit;
	}
	ret = of_address_to_resource(node, 0, &res);
	if (ret) {
		NNA_ERR("parse device node resource failed\n");
		goto exit;
	}
	nna_priv->io_start = res.start;

	nna_priv->irq = platform_get_irq(pdev, 0);
	if (nna_priv->irq < 0) {
		NNA_ERR("failed to get irq resource\n");
		ret = nna_priv->irq;
		goto exit;
	}

	nna_priv->clk = devm_clk_get(dev, "npu");
	if (IS_ERR_OR_NULL(nna_priv->clk)) {
		NNA_ERR("Fail to get clock npu\n");
		goto exit;
	}
	nna_priv->clk_bus = devm_clk_get(dev, "bus_npu");
	if (IS_ERR_OR_NULL(nna_priv->clk_bus)) {
		NNA_ERR("Fail to get clock bus_npu\n");
		goto exit;
	}
	nna_priv->clk_mbus = devm_clk_get(dev, "mbus_npu");
	if (IS_ERR_OR_NULL(nna_priv->clk_mbus)) {
		NNA_ERR("Fail to get clock mbus_npu\n");
		goto exit;
	}
	nna_priv->clk_rst = devm_reset_control_get_optional(dev, NULL);
	if (IS_ERR_OR_NULL(nna_priv->clk_rst)) {
		NNA_ERR("fail to get rst\n");
		goto exit;
	}

	ret = devm_request_threaded_irq(dev, nna_priv->irq, NULL, nna_interrupt,
					IRQF_EARLY_RESUME | IRQF_ONESHOT,
					dev_name(dev), &nna_priv->devid);
	if (ret) {
		NNA_ERR("failed to request irq resource\n");
		goto exit;
	}
	disable_irq(nna_priv->irq);

	mutex_init(&nna_priv->mutex);
	init_waitqueue_head(&nna_priv->nna_queue);
	nna_priv->nna_irq_status = 0;
	nna_priv->refs_count = 0;
	return 0;

exit:
	if (nna_priv->clk != NULL)
		clk_put(nna_priv->clk);
	if (nna_priv->clk_bus != NULL)
		clk_put(nna_priv->clk_bus);
	if (nna_priv->clk_mbus != NULL)
		clk_put(nna_priv->clk_mbus);
	if (nna_priv->io != NULL)
		devm_iounmap(dev, nna_priv->io);
	if (nna_priv != NULL)
		devm_kfree(dev, nna_priv);
	return ret;
}

static int nna_remove(struct platform_device *pdev)
{
	NNA_INFO("nna_remove\n");
	mutex_destroy(&nna_priv->mutex);
	free_irq(nna_priv->irq, NULL);
	nna_exit(nna_priv);
	return 0;
}

static int nna_suspend(struct platform_device *pdev, pm_message_t state)
{
	NNA_INFO("nna_suspend\n");
	enable_irq_wake(nna_priv->irq);
	return 0;
}

static int nna_resume(struct platform_device *pdev)
{
	NNA_INFO("nna_resume\n");
	disable_irq_wake(nna_priv->irq);
	return 0;
}

static const struct of_device_id sunxi_nna_match[] = {
	{.compatible = "allwinner,aw-nna-sunxi",},
	{},
};

static struct platform_driver nna_driver = {
	.probe	 = nna_probe,
	.remove  = nna_remove,
	.suspend = nna_suspend,
	.resume  = nna_resume,
	.driver  = {
		.name  = "nna",
		.owner = THIS_MODULE,
		.of_match_table = sunxi_nna_match,
	},
};

int __init nna_module_init(void)
{
	int ret;

	NNA_INFO("nna module init\n");
	ret = platform_driver_register(&nna_driver);

	return ret;
}

static void __exit nna_module_exit(void)
{
	NNA_INFO("nna_module_exit\n");
	platform_driver_unregister(&nna_driver);
}

module_init(nna_module_init);
module_exit(nna_module_exit);

MODULE_AUTHOR("dby@allwinnertech.com");
MODULE_LICENSE("GPL");
MODULE_VERSION("0.0.1");
MODULE_DESCRIPTION("aw nna sunxi driver");
