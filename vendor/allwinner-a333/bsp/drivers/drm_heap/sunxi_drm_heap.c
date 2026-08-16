// SPDX-License-Identifier: GPL-2.0
/* Copyright(c) 2020 - 2023 Allwinner Technology Co.,Ltd. All rights reserved. */
//#define DEBUG
#include <linux/dma-buf.h>
#include <linux/dma-mapping.h>
#include <linux/err.h>
#include <linux/highmem.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/scatterlist.h>
#include <linux/slab.h>
#include <linux/sched/signal.h>
#include <linux/genalloc.h>
#include <linux/platform_device.h>
#include <linux/of_address.h>
#include <asm/page.h>
#include <sunxi-smc.h>
#include <sunxi-log.h>
#include <sunxi-drm-heap.h>
#include <uapi/security/sunxi-drm-heap.h>

/*
 * google gonna use these source in GKI 2.0 (suppose
 * to lunch in 2021), so these source may encounter
 * namespace conflict in less than one year, just
 * include them with all function static, so no conflict
 * with other sources
 * 							--ouyangkun 2020.09.12
 */
#include "third-party/heap.c"
#include "third-party/heap-helpers.c"

#define DRM_HEAP_DEV_NAME	"sunxi_drm_heap"

MODULE_IMPORT_NS(DMA_BUF);

static struct sunxi_drm_info sunxi_drm_info;  /* FIXME: Lock this variable */

static struct gen_pool *drm_pool;  /* NULL means module is not ready (Load failed or to be loaded) */

/* Kernel API */
struct sunxi_drm_mem *sunxi_drm_mem_alloc(size_t size)
{
	struct sunxi_drm_mem *mem;
	pgoff_t pagecount;
	unsigned long xaddr;
	phys_addr_t paddr;

	if (!drm_pool) {
		sunxi_err(NULL, "Module is not ready for mem_alloc\n");
		goto err0;
	}

	mem = kzalloc(sizeof(*mem), GFP_KERNEL);
	if (!mem) {
		sunxi_err(NULL, "kzalloc(*mem) failed. size = 0x%zx\n", size);
		goto err0;
	}

	/*
	 * Allocations from all heaps have to begin
	 * and end on page boundaries.
	 */
	size = PAGE_ALIGN(size);
	if (!size) {
		sunxi_err(NULL, "size 0x%zx is not page aligned\n", size);
		goto err1;
	}
	pagecount = size / PAGE_SIZE;

	// TODO: Use 'gen_pool_dma_alloc()' instead of 'gen_pool_alloc() + gen_pool_virt_to_phys()' for optimization?
	/* @xaddr is the virt start addr on TA's view  */
	xaddr = gen_pool_alloc(drm_pool, pagecount * PAGE_SIZE);
	if (!xaddr) {
		sunxi_err(NULL, "gen_pool_alloc() failed. size = 0x%zx\n", size);
		goto err1;
	}

	paddr = gen_pool_virt_to_phys(drm_pool, xaddr);

	mem->paddr = paddr;
	mem->size = size;
	mem->xaddr = xaddr;

	sunxi_debug(NULL, "paddr = %pa, size = 0x%zx\n", &mem->paddr, mem->size);
	return mem;
/*
err2:
	gen_pool_free(drm_pool, (unsigned long)mem->xaddr, mem->size);
*/
err1:
	kfree(mem);
err0:
	return NULL;
}
EXPORT_SYMBOL_GPL(sunxi_drm_mem_alloc);

/* Kernel API */
void sunxi_drm_mem_free(const struct sunxi_drm_mem *mem)
{
	if (!drm_pool) {
		sunxi_err(NULL, "Module is not ready for mem_free\n");
		return;
	}

	sunxi_debug(NULL, " paddr = %pa, size = 0x%zx\n", &mem->paddr, mem->size);

	gen_pool_free(drm_pool, (unsigned long)mem->xaddr, mem->size);
	kfree(mem);
}
EXPORT_SYMBOL_GPL(sunxi_drm_mem_free);

/* Kernel API */
void sunxi_drm_info_get(struct sunxi_drm_info *info)
{
	if (!drm_pool) {
		sunxi_err(NULL, "Module is not ready for info_get\n");
		memset(info, 0, sizeof(*info));
		return;
	}

	/* Update 'avail_size' whenever a caller accesses 'sunxi_drm_info' */
	sunxi_drm_info.avail_size = gen_pool_avail(drm_pool);
	*info = sunxi_drm_info;
}
EXPORT_SYMBOL_GPL(sunxi_drm_info_get);

/* Kernel API */
int sunxi_drm_copy_from_unsafe(phys_addr_t dst, phys_addr_t src, size_t size)
{
	return smccc_sunxi_drm_copy_from_unsafe(dst, src, size);
}
EXPORT_SYMBOL_GPL(sunxi_drm_copy_from_unsafe);

/* Kernel API */
int sunxi_drm_master_enable_by_type(uint32_t master_types)
{
	return smccc_sunxi_drm_master_enable_by_type(master_types, true);
}
EXPORT_SYMBOL_GPL(sunxi_drm_master_enable_by_type);

/* Kernel API */
int sunxi_drm_master_disable_by_type(uint32_t master_types)
{
	return smccc_sunxi_drm_master_enable_by_type(master_types, false);
}
EXPORT_SYMBOL_GPL(sunxi_drm_master_disable_by_type);

/* Kernel API */
int sunxi_drm_query(bool *is_drm_enabled, uint32_t *enabled_master_types, uint32_t *avail_master_types)
{
	return smccc_sunxi_drm_query(is_drm_enabled, enabled_master_types, avail_master_types);
}
EXPORT_SYMBOL_GPL(sunxi_drm_query);

/* dma-buf free callback */
static void sunxi_drm_heap_callback_free(struct heap_helper_buffer *hb)
{
	sunxi_debug(NULL, "len = 0x%zx, xaddr = 0x%px\n",
		    hb->size, hb->xaddr);

	/*
	 * No need to check `if (!drm_pool)` here.
	 * `ioctl()` already worked here, means the module is already loaded.
	 */

	kfree(hb->pages);
	gen_pool_free(drm_pool, (unsigned long)hb->xaddr, hb->size);
	kfree(hb);
}

/* ioctl(DMA_HEAP_IOC_ALLOC) */
static int sunxi_drm_heap_ioctl_allocate(struct dma_heap *heap, unsigned long len,
				   unsigned long fd_flags,
				   unsigned long heap_flags)
{
	struct heap_helper_buffer *hb;
	struct dma_buf *dmabuf;
	int ret = -ENOMEM;
	pgoff_t pg;
	unsigned long xaddr;
	struct page *allocated_page;
	phys_addr_t paddr;

	sunxi_debug(NULL, "len = 0x%lx\n", len);

	hb = kzalloc(sizeof(*hb), GFP_KERNEL);
	if (!hb) {
		sunxi_err(NULL, "kzalloc() failed\n");
		return -ENOMEM;
	}

	init_heap_helper_buffer(hb, sunxi_drm_heap_callback_free);
	hb->heap = heap;
	hb->size = len;

	hb->pagecount = len / PAGE_SIZE;

	/* @xaddr is the virt start addr on TA's view  */
	xaddr = gen_pool_alloc(drm_pool, hb->pagecount * PAGE_SIZE);
	if (!xaddr) {
		sunxi_err(NULL, "gen_pool_alloc() failed\n");
		ret = -ENOMEM;
		goto err0;
	}

	hb->xaddr = (void *)xaddr;
	paddr = gen_pool_virt_to_phys(drm_pool, xaddr);
	sunxi_debug(NULL, "allocated at 0x%px (phy 0x%pa) with size 0x%zx\n",
	       hb->xaddr,
	       &paddr, hb->size);

	hb->pages = kmalloc_array(hb->pagecount,
			       sizeof(*hb->pages), GFP_KERNEL);
	if (!hb->pages) {
		sunxi_err(NULL, "kmalloc_array() failed: pagecount = 0x%lx\n", hb->pagecount);
		ret = -ENOMEM;
		goto err1;
	}

	allocated_page = phys_to_page(gen_pool_virt_to_phys(drm_pool, xaddr));
	for (pg = 0; pg < hb->pagecount; pg++)
		hb->pages[pg] = &allocated_page[pg];

	/* create the dmabuf, also attach @hb to dmabuf->priv */
	dmabuf = heap_helper_export_dmabuf(hb, fd_flags);
	if (IS_ERR(dmabuf)) {
		sunxi_err(NULL, "heap_helper_export_dmabuf() failed\n");
		ret = PTR_ERR(dmabuf);
		goto err2;
	}

	hb->dmabuf = dmabuf;

	ret = dma_buf_fd(dmabuf, fd_flags);
	if (ret < 0) {
		sunxi_err(NULL, "dma_buf_fd() failed\n");
		dma_buf_put(dmabuf);
		/* just return, as put will call release and that will free */
		return ret;
	}

	sunxi_debug(NULL, "done\n");
	return ret;

err2:
	kfree(hb->pages);
err1:
	gen_pool_free(drm_pool, (unsigned long)hb->xaddr, hb->size);
err0:
	kfree(hb);
	return ret;
}

/* ioctl(DMA_HEAP_GET_ADDR) */
int sunxi_drm_heap_ioctl_phys(struct dma_heap *heap, int dmabuf_fd,
			unsigned int *tee_addr, unsigned int *phy_addr, unsigned int *len)
{
	struct dma_buf *dma_buf;
	struct heap_helper_buffer *helper;

	(void)heap;
	dma_buf = dma_buf_get(dmabuf_fd);
	if (IS_ERR_OR_NULL(dma_buf)) {
		sunxi_err(NULL, "Invalid dmabuf_fd = 0x%x\n", dmabuf_fd);
		return PTR_ERR(dma_buf);
	}

	helper = (struct heap_helper_buffer *)dma_buf->priv;
	*phy_addr = page_to_phys(helper->pages[0]);
	*tee_addr = *phy_addr - sunxi_drm_info.drm_base + sunxi_drm_info.tee_base;
	*len = helper->size;

	dma_buf_put(dma_buf);
	return 0;
}

/* ioctl() ops */
static const struct dma_heap_ops sunxi_drm_heap_ops = {
	.allocate = sunxi_drm_heap_ioctl_allocate,
	.phys     = sunxi_drm_heap_ioctl_phys,
};

/* probe() and remove() */

static int sunxi_drm_heap_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	sunxi_debug(dev, "Probe done\n");
	return 0;
}

static int sunxi_drm_heap_remove(struct platform_device *pdev)
{
	return 0;
}

static const struct of_device_id sunxi_drm_heap_dt_ids[] = {
	{ .compatible = "allwinner,sunxi-drm-heap-v2" },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, sunxi_drm_heap_dt_ids);

static struct platform_driver sunxi_drm_heap_driver = {
	.probe  = sunxi_drm_heap_probe,
	.remove = sunxi_drm_heap_remove,
	.driver = {
		.name = "sunxi-drm-heap-v2",
		.of_match_table	= sunxi_drm_heap_dt_ids,
	},
};

/* init() and exit() */

static __init int sunxi_drm_heap_init(void)
{
	struct dma_heap_export_info exp_info;
	struct dma_heap *heap;
	int ret = 0;

	sunxi_debug(NULL, "Init begin\n");
	ret = optee_probe_drm_configure(&sunxi_drm_info.drm_base,
					&sunxi_drm_info.drm_size,
					&sunxi_drm_info.tee_base);
	if (ret) {
		sunxi_err(NULL, "optee_probe_drm_configure() failed\n");
		memset(&sunxi_drm_info, 0, sizeof(sunxi_drm_info));
		return 0; /* compatible with the non-secure + no-optee product */
	}

	drm_pool = gen_pool_create(PAGE_SHIFT, -1);
	if (!drm_pool) {
		sunxi_err(NULL, "gen_pool_create() failed\n");
		return -ENOMEM;
	}

	gen_pool_set_algo(drm_pool, gen_pool_best_fit, NULL);
	ret = gen_pool_add_virt(drm_pool,  /* Add the **whole** DRM region to @drm_pool */
				sunxi_drm_info.tee_base,  /* Virt start addr on TA's view. Allocated by `heap_helper_buffer.xaddr` */
				sunxi_drm_info.drm_base,  /* Phys start addr */
				sunxi_drm_info.drm_size,  /* Size */
				-1);
	if (ret) {
		sunxi_err(NULL, "gen_pool_add_virt() failed\n");
		goto exit;
	}

	ret = dma_heap_init();
	if (ret) {
		sunxi_err(NULL, "dma_heap_init() failed\n");
		goto exit;
	}

	exp_info.name = DRM_HEAP_DEV_NAME;
	exp_info.ops = &sunxi_drm_heap_ops;
	exp_info.priv = NULL;

	heap = dma_heap_add(&exp_info);
	if (IS_ERR(heap)) {
		ret = PTR_ERR(heap);
		sunxi_err(NULL, "dma_heap_add() failed\n");
		goto exit;
	}

	ret = platform_driver_register(&sunxi_drm_heap_driver);
	if (ret) {
		sunxi_err(NULL, "platform_driver_register() failed\n");
		goto exit;
	}

	sunxi_debug(NULL, "Init done\n");
	return 0;

exit:
	gen_pool_destroy(drm_pool);
	drm_pool = NULL;
	memset(&sunxi_drm_info, 0, sizeof(sunxi_drm_info));
	return ret;
}

/* FIXME: After `rmmod && insmod`, error occurs due to `heap.c`. */
static __exit void sunxi_drm_heap_exit(void)
{
	platform_driver_unregister(&sunxi_drm_heap_driver);
	gen_pool_destroy(drm_pool);
	drm_pool = NULL;
	memset(&sunxi_drm_info, 0, sizeof(sunxi_drm_info));
}

subsys_initcall(sunxi_drm_heap_init);
module_exit(sunxi_drm_heap_exit);

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("weidonghui <weidonghui@allwinnertech.com>");
MODULE_AUTHOR("Martin <wuyan@allwinnertech.com>");
MODULE_VERSION("V2.2.2");
MODULE_DESCRIPTION("sunxi DRM heap driver");
