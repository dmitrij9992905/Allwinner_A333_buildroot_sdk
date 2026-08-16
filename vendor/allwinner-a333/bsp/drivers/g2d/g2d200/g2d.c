/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Copyright(c) 2020 - 2023 Allwinner Technology Co.,Ltd. All rights reserved. */
/*
 * Copyright (c) 2007-2019 Allwinnertech Co., Ltd.
 * Author: zhengxiaobin <zhengxiaobin@allwinnertech.com>
 *
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#include "g2d_driver_i.h"
#include "g2d_platform.h"
#include "g2d_top.h"
#include "g2d_debug.h"
#if IS_ENABLED(CONFIG_G2D_MIXER)
#include "g2d_mixer.h"
#endif
#if IS_ENABLED(CONFIG_G2D_ROTATE)
#include "g2d_rotate.h"
#endif
#include "linux/pm_runtime.h"
#include "linux/pm_domain.h"
#include "linux/hwspinlock.h"
#include "../g2d_buf_cache.h"

static u64 sunxi_g2d_dma_mask = DMA_BIT_MASK(32);

#if IS_ENABLED(CONFIG_G2D_SYNCFENCE)
extern int syncfence_init(void);
extern void syncfence_exit(void);
#endif

/* alloc based on 4K byte */
#if IS_ENABLED(CONFIG_G2D_USE_HWSPINLOCK)
static __g2d_hwspinlock g2d_drv_hwspinlock;
#endif

static __g2d_drv_t g2d_ext_hd;
static __g2d_info_t para;
unsigned int loglevel;

static ssize_t g2d_loglevel_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t count)
{
	loglevel = simple_strtoul(buf, NULL, 0);

	return count;
}

static ssize_t g2d_loglevel_show(struct device *dev,
				struct device_attribute *attr,
				char *buf)
{
	u32 count = 0;
	if (!loglevel)
		count += sprintf(buf + count, "0:NONE  1:G2D_DRV  2:IOCTL_PARA  4:G2D_REG\n");
	else
		count += sprintf(buf + count, "loglevel = %d\n", loglevel);

	return count;
}

static ssize_t g2d_time_info_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	static char time_info[1024];
	char *ptr = time_info;
	struct g2d_time_info *g2d_time_inf = get_g2d_time_inf();
	if (g2d_time_inf->dump_time_info_en  == 1) {
		ptr += sprintf(ptr, "g2d use            %u us\n",
			__get_ts_diff(g2d_time_inf->ctr_end_ts, g2d_time_inf->ctr_start_ts));
		ptr += sprintf(ptr, "g2d unlock use     %u us\n",
			__get_ts_diff(g2d_time_inf->acq_lock_ts, g2d_time_inf->ctr_start_ts));
		ptr += sprintf(ptr, "g2d dma_map use    %u us\n",
			__get_ts_diff(g2d_time_inf->dma_map_end_ts, g2d_time_inf->dma_map_start_ts));
		ptr += sprintf(ptr, "g2d dma_unmap use  %u us\n",
			__get_ts_diff(g2d_time_inf->dma_unmap_end_ts, g2d_time_inf->dma_unmap_start_ts));
		ptr += sprintf(ptr, "g2d hw_proc use    %u us\n",
			__get_ts_diff(g2d_time_inf->hw_proc_end_ts, g2d_time_inf->hw_proc_start_ts));
		return sprintf(buf, "time_info:\n%s", time_info);
	}

	return sprintf(buf, "please echo 1 > time_info and run again\
		before dumping time_info\n");
}

static ssize_t g2d_time_info_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t count)
{
	struct g2d_time_info *g2d_time_inf = get_g2d_time_inf();
	if (strncasecmp(buf, "1", 1) == 0)
		g2d_time_inf->dump_time_info_en = 1;
	else if (strncasecmp(buf, "0", 1) == 0)
		g2d_time_inf->dump_time_info_en = 0;
	else
		G2D_WARN("Error input\n");

	return count;
}

#if IS_ENABLED(CONFIG_G2D_BUF_CACHED)
static ssize_t g2d_buf_cache_info_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	ssize_t wc = 0;
	wc += g2d_buf_cache_debug_show(buf + wc);
	return wc;
}

static ssize_t g2d_buf_cache_info_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t count)
{
	return count;
}

static DEVICE_ATTR(buf_cache_dbg, 0660,
		g2d_buf_cache_info_show, g2d_buf_cache_info_store);
#endif

static DEVICE_ATTR(loglevel, 0660,
		g2d_loglevel_show, g2d_loglevel_store);
static DEVICE_ATTR(func_runtime, 0660,
		g2d_time_info_show, g2d_time_info_store);

static struct attribute *g2d_attributes[] = {
	&dev_attr_loglevel.attr,
	&dev_attr_func_runtime.attr,
#if IS_ENABLED(CONFIG_G2D_BUF_CACHED)
	&dev_attr_buf_cache_dbg.attr,
#endif
	NULL
};

static struct attribute_group g2d_attribute_group = {
	.name = "attr",
	.attrs = g2d_attributes
};

int g2d_blit_h(g2d_blt_h *blit_para)
{
	int ret = -1;
#if IS_ENABLED(CONFIG_G2D_ROTATE)
	ret = g2d_rotate_set_para(&para, &blit_para->src_image_h,
			    &blit_para->dst_image_h,
			    blit_para->flag_h);
#else
	G2D_ERR("Please enable CONFIG_G2D_ROTATE\n");
#endif
	return ret;
}
EXPORT_SYMBOL_GPL(g2d_blit_h);

/**
 * g2d_bsp_blit_h
 * @info: g2d_blt_h
 *
 * DESCRIPTION:
 * The current interface is provided for internal calls of driver,
 * and the power and clock can be turned on and off without using ioctl.
 * eg: Framebuffer rotation driver.
 *
 */
int g2d_bsp_blit_h(g2d_blt_h *info)
{
	__s32 ret = 0;

#if IS_ENABLED(CONFIG_PM_GENERIC_DOMAINS)
	pm_runtime_get_sync(para.dev);
#endif

	ret = g2d_blit_h(info);

#if IS_ENABLED(CONFIG_PM_GENERIC_DOMAINS)
	pm_runtime_put_sync(para.dev);
#endif

	return ret;
}
EXPORT_SYMBOL(g2d_bsp_blit_h);


void *g2d_malloc(__u32 bytes_num, __u32 *phy_addr)
{
	void *address = NULL;

#ifdef ALLOC_USING_DMA
	u32 actual_bytes;

	if (bytes_num != 0) {
		actual_bytes = G2D_BYTE_ALIGN(bytes_num);

		address = dma_alloc_coherent(para.dev, actual_bytes,
					     (dma_addr_t *) phy_addr,
					     GFP_KERNEL);
		if (address) {
			return address;
		}
		G2D_WARN("dma_alloc_coherent fail, size=0x%x\n", bytes_num);
		return NULL;
	}
	G2D_WARN("reuquet memory size is zero\n");
#else
	unsigned int map_size = 0;
	struct page *page;

	if (bytes_num != 0) {
		map_size = PAGE_ALIGN(bytes_num);
		page = alloc_pages(GFP_KERNEL, get_order(map_size));
		if (page != NULL) {
			address = page_address(page);
			if (address == NULL) {
				free_pages((unsigned long)(page),
					   get_order(map_size));
				G2D_WARN("page_address fail\n");
				return NULL;
			}
			*phy_addr = virt_to_phys(address);
			return address;
		}
		G2D_WARN("alloc_pages fail\n");
		return NULL;
	}
	G2D_WARN("size is zero\n");
#endif

	return NULL;
}

void g2d_free(void *virt_addr, uintptr_t phy_addr, unsigned int size)
{
#ifdef ALLOC_USING_DMA
	u32 actual_bytes;

	actual_bytes = PAGE_ALIGN(size);
	if (phy_addr && virt_addr)
		dma_free_coherent(para.dev, actual_bytes, virt_addr,
				  (dma_addr_t) phy_addr);
#else
	unsigned int map_size = PAGE_ALIGN(size);
	unsigned int page_size = map_size;

	if (virt_addr == NULL)
		return;

	free_pages((unsigned long)virt_addr, get_order(page_size));
#endif
}

static int g2d_clock_prepare(const __g2d_info_t *info)
{
	int ret = 0;
	if (info->bus_clk) {
		ret |=  clk_prepare(info->bus_clk);
	}
	if (info->clk) {
		if (info->clk_parent) {
			clk_set_parent(info->clk, info->clk_parent);
		}
		ret |= clk_prepare(info->clk);
	}
	if (info->mbus_clk) {
		ret |= clk_prepare(info->mbus_clk);
	}
	if (info->ahb_clk) {
		ret |= clk_prepare(info->ahb_clk);
	}
	if (info->mbus_vo_clk) {
		ret |= clk_prepare(info->mbus_vo_clk);
	}
	if (info->mbus_desys_clk) {
		ret |= clk_prepare(info->mbus_desys_clk);
	}
	if (info->ahb_de_clk) {
		ret |= clk_prepare(info->ahb_de_clk);
	}
	if (info->vo_clk) {
		ret |= clk_prepare(info->vo_clk);
	}
	if (info->hb_clk) {
		ret |= clk_prepare(info->hb_clk);
	}
	if (ret != 0)
		G2D_ERR("clock prepare error\n");

	return ret;
}

static int g2d_clock_enable(__g2d_info_t *info)
{
	int ret = 0;

	if (info->vo_reset) {
		ret |= reset_control_deassert(info->vo_reset);
		if (ret != 0) {
			G2D_ERR("deassert vo_reset error\n");
			return ret;
		}
	}
	if (info->reset) {
		ret = reset_control_deassert(info->reset);
		if (ret != 0) {
			G2D_ERR("deassert error\n");
			return ret;
		}
	}
	if (info->desys_reset) {
		ret |= reset_control_deassert(info->desys_reset);
		if (ret != 0) {
			G2D_ERR("deassert desys_reset error\n");
			return ret;
		}
	}

	if (info->mbus_vo_clk)
		ret |= clk_enable(info->mbus_vo_clk);
	if (info->vo_clk)
		ret |= clk_enable(info->vo_clk);
	if (info->mbus_desys_clk)
		ret |= clk_enable(info->mbus_desys_clk);
	if (info->ahb_de_clk)
		ret |= clk_enable(info->ahb_de_clk);
	if (info->ahb_clk)
		ret |= clk_enable(info->ahb_clk);
	if (info->hb_clk)
		ret |= clk_enable(info->hb_clk);
	if (info->mbus_clk)
		ret |= clk_enable(info->mbus_clk);
	if (info->bus_clk)
		ret |=  clk_enable(info->bus_clk);
	if (info->clk)
		ret |= clk_enable(info->clk);
	if (ret != 0)
		G2D_ERR("clock enable error\n");

	return ret;
}

static int g2d_clock_unprepare(const __g2d_info_t *info)
{
	if (info->clk)
		clk_unprepare(info->clk);
	if (info->bus_clk)
		clk_unprepare(info->bus_clk);
	if (info->mbus_clk)
		clk_unprepare(info->mbus_clk);
	if (info->hb_clk)
		clk_unprepare(info->hb_clk);
	if (info->ahb_clk)
		clk_unprepare(info->ahb_clk);
	if (info->vo_clk)
		clk_unprepare(info->vo_clk);
	if (info->mbus_vo_clk)
		clk_unprepare(info->mbus_vo_clk);
	if (info->mbus_desys_clk)
		clk_unprepare(info->mbus_desys_clk);
	if (info->ahb_de_clk)
		clk_unprepare(info->ahb_de_clk);
	return 0;
}

static int g2d_clock_disable(const __g2d_info_t *info)
{
	if (info->clk)
		clk_disable(info->clk);
	if (info->bus_clk)
		clk_disable(info->bus_clk);
	if (info->mbus_clk)
		clk_disable(info->mbus_clk);
	if (info->hb_clk)
		clk_disable(info->hb_clk);
	if (info->ahb_clk)
		clk_disable(info->ahb_clk);
	if (info->vo_clk)
		clk_disable(info->vo_clk);
	if (info->mbus_vo_clk)
		clk_disable(info->mbus_vo_clk);
	if (info->mbus_desys_clk)
		clk_disable(info->mbus_desys_clk);
	if (info->ahb_de_clk)
		clk_disable(info->ahb_de_clk);

	if (info->reset)
		reset_control_assert(info->reset);
	if (info->vo_reset)
		reset_control_assert(info->vo_reset);
	if (info->desys_reset)
		reset_control_assert(info->desys_reset);
	return 0;
}

int g2d_open(struct inode *inode, struct file *file)
{
	mutex_lock(&para.mutex);
	para.user_cnt++;
	if (para.user_cnt == 1) {
		g2d_clock_prepare(&para);
#ifndef CONFIG_PM_GENERIC_DOMAINS
		g2d_clock_enable(&para);
#endif
		para.opened = true;
#ifndef CONFIG_PM_GENERIC_DOMAINS
		g2d_bsp_open(g2d_ext_hd.current_thread_id);
#endif
	}

	mutex_unlock(&para.mutex);
	return 0;
}
EXPORT_SYMBOL_GPL(g2d_open);

int g2d_release(struct inode *inode, struct file *file)
{
	mutex_lock(&para.mutex);
	para.user_cnt--;
	if (para.user_cnt == 0) {
#ifndef CONFIG_PM_GENERIC_DOMAINS
		g2d_clock_disable(&para);
#endif
		g2d_clock_unprepare(&para);
		para.opened = false;
#ifndef CONFIG_PM_GENERIC_DOMAINS
		g2d_bsp_close(g2d_ext_hd.current_thread_id);
#endif
	}

	mutex_unlock(&para.mutex);

	return 0;
}
EXPORT_SYMBOL_GPL(g2d_release);

int g2d_mmap(struct file *file, struct vm_area_struct *vma)
{
	unsigned long mypfn = vma->vm_pgoff;
	unsigned long vmsize = vma->vm_end - vma->vm_start;

	vma->vm_pgoff = 0;

	vma->vm_page_prot = pgprot_writecombine(vma->vm_page_prot);
	if (remap_pfn_range(vma, vma->vm_start, mypfn,
			    vmsize, vma->vm_page_prot))
		return -EAGAIN;

	return 0;
}

irqreturn_t g2d_handle_irq(int irq, void *g2d_ext_hd)
{
	G2D_DRV_DBG("g2d_handle_irq\n");
	g2d_dump_reg(para.mem->start, para.io);
	g2d_bsp_handle_irq((__g2d_drv_t *)g2d_ext_hd);
	return IRQ_HANDLED;
}

#if IS_ENABLED(CONFIG_AW_IOMMU) && (IS_ENABLED(CONFIG_ARCH_SUN8IW20) || IS_ENABLED(CONFIG_ARCH_SUN20IW1))
extern void sunxi_reset_device_iommu(unsigned int master_id);
#endif

int g2d_wait_cmd_finish(unsigned int timeout)
{
	struct g2d_time_info *g2d_time_inf = get_g2d_time_inf();
	ktime_get_real_ts64(&(g2d_time_inf->hw_proc_start_ts));
	timeout = wait_event_timeout(g2d_ext_hd.queue,
				     g2d_ext_hd.finish_flag == 1,
				     msecs_to_jiffies(timeout));
	ktime_get_real_ts64(&(g2d_time_inf->hw_proc_end_ts));
	if (timeout == 0) {
		g2d_dump_reg(para.mem->start, para.io);
		g2d_bsp_reset(g2d_ext_hd.current_thread_id);
		G2D_ERR("G2D irq pending flag timeout\n");

		/* reset iommu */
#if IS_ENABLED(CONFIG_AW_IOMMU) && (IS_ENABLED(CONFIG_ARCH_SUN8IW20) || IS_ENABLED(CONFIG_ARCH_SUN20IW1))
		sunxi_reset_device_iommu(G2D_IOMMU_MASTER_ID);
#endif
		g2d_ext_hd.finish_flag = 1;
		wake_up(&g2d_ext_hd.queue);
		return -1;
	}
	g2d_ext_hd.finish_flag = 0;

	return 0;
}

int g2d_get_layout_version(void)
{
#ifdef G2D_V2X_SUPPORT
	return 2;
#else
	return 1;
#endif
}
EXPORT_SYMBOL(g2d_get_layout_version);

void g2d_query_hardware_version(struct g2d_hardware_version *v)
{

#if IS_ENABLED(CONFIG_ARCH_SUN50IW10P1)
#define SYS_CFG_BASE 0x03000000
#define VER_REG_OFFS 0x00000024
	void __iomem *io = NULL;
	io = ioremap(SYS_CFG_BASE, 0x100);
	if (io == NULL) {
		G2D_WARN("ioremap of sys_cfg register failed\n");
		return;
	}
	v->chip_version = readl(io + VER_REG_OFFS);
	iounmap(io);
#else
	v->chip_version = 0;
#endif

	v->g2d_version = g2d_ip_version();
	G2D_INFO("g2d version: %08x chip version: %08x", v->g2d_version, v->chip_version);
}

int g2d_ioctl_mutex_lock(void)
{
#if IS_ENABLED(CONFIG_G2D_USE_HWSPINLOCK)
	int ret;
	int i;
	if (hwlock) {
		for (i = 0; i < 200; i++) {
			ret =  __hwspin_trylock(hwlock, HWLOCK_RAW, &hwspinlock_flag);
			if (ret != 0) {
				msleep(3);
				continue;
			} else
				break;
		}
		if (ret != 0) {
			G2D_ERR("try to get hwspinlock filed 200 times\n");
			return -1;
		}
	}
	enable_irq(para.irq);
#endif
	if (!mutex_trylock(&para.mutex))
		mutex_lock(&para.mutex);
	return 0;
}
EXPORT_SYMBOL_GPL(g2d_ioctl_mutex_lock);

int g2d_ioctl_mutex_unlock(void)
{
#if IS_ENABLED(CONFIG_G2D_USE_HWSPINLOCK)
	disable_irq(para.irq);
	if (hwlock)
		__hwspin_unlock(hwlock, HWLOCK_RAW, &hwspinlock_flag);
#endif
	mutex_unlock(&para.mutex);
	return 0;
}
EXPORT_SYMBOL_GPL(g2d_ioctl_mutex_unlock);

long g2d_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	int ret = -1;
	struct g2d_time_info *g2d_time_inf = get_g2d_time_inf();

	ktime_get_real_ts64(&(g2d_time_inf->ctr_start_ts));

	ret = g2d_ioctl_mutex_lock();
	if (ret < 0)
		return -EFAULT;

	ktime_get_real_ts64(&(g2d_time_inf->acq_lock_ts));

	g2d_ext_hd.finish_flag = 0;

#if IS_ENABLED(CONFIG_PM_GENERIC_DOMAINS)
	pm_runtime_get_sync(para.dev);
#endif

	switch (cmd) {
	case G2D_CMD_MIXER_TASK:
		{
#if IS_ENABLED(CONFIG_G2D_MIXER)
		int i;
		struct mixer_para *p_mixer_para = NULL;
		unsigned long karg[2];
		unsigned long ubuffer[2] = { 0 };

		if (copy_from_user((void *)karg, (void __user *)arg,
				   sizeof(unsigned long) * 2)) {
			ret = -EFAULT;
			goto err_noput;
		}
		ubuffer[0] = *(unsigned long *)karg;
		ubuffer[1] = (*(unsigned long *)(karg + 1));

		p_mixer_para = kmalloc(sizeof(*p_mixer_para) * ubuffer[1],
			       GFP_KERNEL | __GFP_ZERO);
		if (!p_mixer_para)
			goto err_noput;
		if (copy_from_user(p_mixer_para, (void __user *)ubuffer[0],
				   sizeof(*p_mixer_para) * ubuffer[1])) {
			ret = -EFAULT;
			goto err_noput;
		}
		for (i = 0; i < ubuffer[1]; i++) {
			dump_mixer_para_info(&(p_mixer_para[i]));
		}
		ret  = g2d_task_process(&para, p_mixer_para, ubuffer[1]);
		kfree(p_mixer_para);
#endif
		break;
		}
	case G2D_CMD_CREATE_TASK:
		{
#if IS_ENABLED(CONFIG_G2D_MIXER)
			int i;
			struct mixer_para *p_mixer_para = NULL;
			unsigned long karg[2];
			unsigned long ubuffer[2] = { 0 };

			if (copy_from_user((void *)karg, (void __user *)arg,
					   sizeof(unsigned long) * 2)) {
				ret = -EFAULT;
				goto err_noput;
			}
			ubuffer[0] = *(unsigned long *)karg;
			ubuffer[1] = (*(unsigned long *)(karg + 1));
			p_mixer_para = kmalloc(sizeof(*p_mixer_para) * ubuffer[1],
					       GFP_KERNEL | __GFP_ZERO);
			if (!p_mixer_para)
				goto err_noput;
			if (copy_from_user(p_mixer_para, (void __user *)ubuffer[0],
					   sizeof(*p_mixer_para) * ubuffer[1])) {
				ret = -EFAULT;
				goto err_noput;
			}
			for (i = 0; i < ubuffer[1]; i++) {
				dump_mixer_para_info(&(p_mixer_para[i]));
			}
			ret = g2d_task_create(&para, p_mixer_para, ubuffer[1]);
			if (copy_to_user((void __user *)ubuffer[0], p_mixer_para,
					 sizeof(*p_mixer_para) * ubuffer[1])) {
				G2D_WARN("copy_to_user fail\n");
				return  -EFAULT;
			}
			kfree(p_mixer_para);
#endif
			break;
		}
	case G2D_CMD_TASK_APPLY:
		{
#if IS_ENABLED(CONFIG_G2D_MIXER)
			unsigned long karg[1];
			unsigned long ubuffer[1] = { 0 };
			struct g2d_task *p_task = NULL;

			if (copy_from_user((void *)karg, (void __user *)arg,
					   sizeof(unsigned long))) {
				ret = -EFAULT;
				goto err_noput;
			}
			ubuffer[0] = *(unsigned long *)karg;
			p_task = g2d_task_get_by_id(ubuffer[0]);
			if (!p_task) {
				ret = -EFAULT;
				goto err_noput;
			}
			ret = p_task->apply(p_task);
#endif
			break;
		}
	case G2D_CMD_TASK_DESTROY:
		{
#if IS_ENABLED(CONFIG_G2D_MIXER)
			unsigned long karg[1];
			unsigned long ubuffer[1] = { 0 };
			struct g2d_task *p_task = NULL;

			if (copy_from_user((void *)karg, (void __user *)arg,
					   sizeof(unsigned long))) {
				ret = -EFAULT;
				goto err_noput;
			}
			ubuffer[0] = *(unsigned long *)karg;
			p_task = g2d_task_get_by_id(ubuffer[0]);

			if (!p_task) {
				ret = -EFAULT;
				G2D_WARN("Fail to find mixer task inst:%lu\n", ubuffer[0]);
				goto err_noput;
			}

			ret = p_task->destory(p_task);
#endif
			break;
		}
	case G2D_CMD_BITBLT_H:
		{
		g2d_blt_h *blit_para = (g2d_blt_h *)kmalloc(sizeof(g2d_blt_h),
							    GFP_KERNEL);
		if (!blit_para) {
			G2D_WARN("blit_para kmalloc failed\n");
			ret = -EFAULT;
			goto err_noput;
		}
		if (copy_from_user(blit_para, (g2d_blt_h *) arg,
				   sizeof(g2d_blt_h))) {
			G2D_WARN("BITBLT copy from user failed\n");
			ret = -EFAULT;
			kfree(blit_para);
			goto err_noput;
		}
		dump_g2d_blt_h_info(blit_para);
		if (blit_para->flag_h & 0xff00) {
			ret = g2d_blit_h(blit_para);
		}
#if IS_ENABLED(CONFIG_G2D_MIXER)
		else {
			struct mixer_para *mixer_blit_para = (struct mixer_para *)
				kmalloc(sizeof(struct mixer_para), GFP_KERNEL);
			/* mixer module */
			memset(mixer_blit_para, 0, sizeof(*mixer_blit_para));
			memcpy(&(mixer_blit_para->dst_image_h),
			       &(blit_para->dst_image_h), sizeof(g2d_image_enh));
			memcpy(&(mixer_blit_para->src_image_h),
			       &(blit_para->src_image_h), sizeof(g2d_image_enh));
			mixer_blit_para->flag_h = blit_para->flag_h;
			mixer_blit_para->op_flag = OP_BITBLT;
			ret = g2d_task_process(&para, mixer_blit_para, 1);
			kfree(mixer_blit_para);
		}

#endif
		kfree(blit_para);
		break;
		}
	case G2D_CMD_LBC_ROT:
		{
			g2d_lbc_rot lbc_para;

			if (copy_from_user(&lbc_para, (g2d_lbc_rot *)arg,
						sizeof(g2d_lbc_rot))) {
				ret = -EFAULT;
				goto err_noput;
			}
			dump_g2d_lbc_rot_info(&lbc_para);

			ret = g2d_lbc_rot_set_para(&para, &lbc_para);
			break;
		}

	case G2D_CMD_BLD_H:{
#if IS_ENABLED(CONFIG_G2D_MIXER)
			g2d_bld *bld_para = NULL;
			struct mixer_para *mixer_bld_para = NULL;

			bld_para = kmalloc(sizeof(*bld_para), GFP_KERNEL);
			if (!bld_para) {
				G2D_WARN("bld_para kmalloc failed\n");
				ret = -EFAULT;
				goto err_noput;
			}
			mixer_bld_para = kmalloc(sizeof(*mixer_bld_para), GFP_KERNEL);
			if (!mixer_bld_para) {
				G2D_WARN("mixer_bld_para kmalloc failed\n");
				ret = -EFAULT;
				kfree(bld_para);
				goto err_noput;
			}
			if (copy_from_user(bld_para, (g2d_bld *) arg,
					   sizeof(g2d_bld))) {
				ret = -EFAULT;
				kfree(bld_para);
				kfree(mixer_bld_para);
				goto err_noput;
			}
			dump_g2d_bld_info(bld_para);
			memset(mixer_bld_para, 0, sizeof(*mixer_bld_para));
			memcpy(&mixer_bld_para->dst_image_h,
			       &bld_para->dst_image, sizeof(g2d_image_enh));
			memcpy(&mixer_bld_para->src_image_h,
			       &bld_para->src_image[0], sizeof(g2d_image_enh));
			/* ptn use as src */
			memcpy(&mixer_bld_para->ptn_image_h,
			       &bld_para->src_image[1], sizeof(g2d_image_enh));
			memcpy(&mixer_bld_para->ck_para, &bld_para->ck_para,
			       sizeof(g2d_ck));
			mixer_bld_para->bld_cmd = bld_para->bld_cmd;
			mixer_bld_para->op_flag = OP_BLEND;

			ret  = g2d_task_process(&para, mixer_bld_para, 1);
			kfree(bld_para);
			kfree(mixer_bld_para);
#endif
			break;
		}
	case G2D_CMD_FILLRECT_H:{
#if IS_ENABLED(CONFIG_G2D_MIXER)
		g2d_fillrect_h fill_para;
		struct mixer_para mixer_fill_para;

		if (copy_from_user(&fill_para, (g2d_fillrect_h *) arg,
				   sizeof(g2d_fillrect_h))) {
			ret = -EFAULT;
			goto err_noput;
		}
		dump_g2d_fillrect_h_info(&fill_para);
		memset(&mixer_fill_para, 0, sizeof(mixer_fill_para));
		memcpy(&mixer_fill_para.dst_image_h,
		       &fill_para.dst_image_h, sizeof(g2d_image_enh));
		mixer_fill_para.op_flag = OP_FILLRECT;

		ret  = g2d_task_process(&para, &mixer_fill_para, 1);
#endif
		break;
	}
	case G2D_CMD_MASK_H:{
#if IS_ENABLED(CONFIG_G2D_MIXER)
			g2d_maskblt mask_para;
			struct mixer_para mixer_mask_para;

			if (copy_from_user(&mask_para, (g2d_maskblt *) arg,
					   sizeof(g2d_maskblt))) {
				ret = -EFAULT;
				goto err_noput;
			}
			dump_g2d_maskblt_info(&mask_para);
			memset(&mixer_mask_para, 0, sizeof(mixer_mask_para));
			memcpy(&mixer_mask_para.ptn_image_h,
			       &mask_para.ptn_image_h, sizeof(g2d_image_enh));
			memcpy(&mixer_mask_para.mask_image_h,
			       &mask_para.mask_image_h, sizeof(g2d_image_enh));
			memcpy(&mixer_mask_para.dst_image_h,
			       &mask_para.dst_image_h, sizeof(g2d_image_enh));
			memcpy(&mixer_mask_para.src_image_h,
			       &mask_para.src_image_h, sizeof(g2d_image_enh));
			mixer_mask_para.back_flag = mask_para.back_flag;
			mixer_mask_para.fore_flag = mask_para.fore_flag;
			mixer_mask_para.op_flag = OP_MASK;

			ret  = g2d_task_process(&para, &mixer_mask_para, 1);
#endif
			break;
		}
	case G2D_CMD_QUERY_VERSION:
		{
			struct g2d_hardware_version version;
			g2d_query_hardware_version(&version);

			if (copy_to_user((struct g2d_hardware_version *)arg, &version,
						sizeof(struct g2d_hardware_version))) {
				ret = -EFAULT;
				goto err_noput;
			}

			break;
		}

	default:
		goto err_noput;
		break;
	}

err_noput:

#if IS_ENABLED(CONFIG_PM_GENERIC_DOMAINS)
	pm_runtime_put_sync(para.dev);
#endif

	g2d_ioctl_mutex_unlock();
	ktime_get_real_ts64(&(g2d_time_inf->ctr_end_ts));

	return ret;
}

__s32 drv_g2d_init(void)
{
	g2d_set_dmabuf_dev(para.dmabuf_dev);
	init_waitqueue_head(&g2d_ext_hd.queue);
	g2d_bsp_set_base((unsigned long) para.io);
#if IS_ENABLED(CONFIG_G2D_BUF_CACHED)
	g2d_buf_cache_init(para.dmabuf_dev, 32);
#endif
	return 0;
}

__s32 drv_g2d_exit(void)
{
#if IS_ENABLED(CONFIG_G2D_BUF_CACHED)
	g2d_buf_cache_exit();
#endif
	return 0;
}

#if IS_ENABLED(CONFIG_PM_GENERIC_DOMAINS)
static int g2d_attach_pd(struct device *dev, const char *values_of_power_domain_names[], int array_size)
{
	int i;
	struct device_link *link;
	struct device *pd_dev;

	if (dev->pm_domain)
		return 0;
	for (i = 0; i < array_size; i++) {
		if (values_of_power_domain_names[i] == NULL) {
			break;
		}
		pd_dev = dev_pm_domain_attach_by_name(dev, values_of_power_domain_names[i]);
		if (IS_ERR(pd_dev))
			return PTR_ERR(pd_dev);

		if (!pd_dev)
			return 0;
		link = device_link_add(dev, pd_dev,
				DL_FLAG_STATELESS |
				DL_FLAG_PM_RUNTIME);
		if (!link) {
			G2D_ERR("Failed to add device_link to %s\n", values_of_power_domain_names[i]);
			return -EINVAL;
		}
	}
	return 0;
}
#endif

static int g2d_suspend(struct device *dev)
{
	int ret = 0;
	mutex_lock(&para.mutex);
	if (para.opened) {
		ret = pm_runtime_force_suspend(para.dev);
	}
	mutex_unlock(&para.mutex);
	G2D_INFO("g2d_suspend succesfully\n");

	return ret;
}

static int g2d_resume(struct device *dev)
{
	int ret = 0;
	mutex_lock(&para.mutex);
	if (para.opened) {
		ret = pm_runtime_force_resume(para.dev);
	}
	mutex_unlock(&para.mutex);
	G2D_INFO("g2d_resume succesfully\n");

	return ret;
}

#if IS_ENABLED(CONFIG_PM_GENERIC_DOMAINS)
static int g2d_runtime_resume(struct device *dev)
{

	g2d_clock_enable(&para);

	g2d_bsp_open(g2d_ext_hd.current_thread_id);

	return 0;
}

static int g2d_runtime_suspend(struct device *dev)
{

	g2d_clock_disable(&para);

	g2d_bsp_close(g2d_ext_hd.current_thread_id);

	return 0;
}
#endif

static int g2d_probe(struct platform_device *pdev)
{
#if IS_ENABLED(CONFIG_PM_GENERIC_DOMAINS)
	const char *values_of_power_domain_names[PM_ARRAY_SIZE];
#endif
	int ret = 0;

	para.dev = &pdev->dev;
	para.dmabuf_dev = &pdev->dev;
	para.dmabuf_dev->dma_mask = &sunxi_g2d_dma_mask;
	para.dmabuf_dev->coherent_dma_mask = DMA_BIT_MASK(32);
	platform_set_drvdata(pdev, &para);
	memset(&g2d_ext_hd, 0, sizeof(__g2d_drv_t));

#if IS_ENABLED(CONFIG_PM_GENERIC_DOMAINS)
	ret = of_property_read_string(pdev->dev.of_node, "power-domain-names", values_of_power_domain_names);
	ret = g2d_attach_pd(para.dev, values_of_power_domain_names, PM_ARRAY_SIZE);
	pm_runtime_enable(para.dev);
#endif

#if IS_ENABLED(CONFIG_G2D_USE_HWSPINLOCK)
	hwlock = hwspin_lock_request_specific(g2d_hwspinlock_id);
	if (!hwlock) {
		G2D_ERR("G2D: Hwspinlock request is failed!\n");
	}
#endif

	para.mem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!para.mem) {
		dev_err(&pdev->dev, "Failed to get MEM resource\n");
		return -ENODEV;
	}

	para.io = of_iomap(pdev->dev.of_node, 0);
	if (para.io == NULL) {
		G2D_ERR("iormap() of register failed\n");
		ret = -ENXIO;
		goto dealloc_fb;
	}

	para.irq = irq_of_parse_and_map(pdev->dev.of_node, 0);
	if (!para.irq) {
		G2D_ERR("irq_of_parse_and_map irq fail for transform\n");
		ret = -ENXIO;
		goto release_regs;
	}

#if IS_ENABLED(CONFIG_G2D_USE_HWSPINLOCK)
	if (hwlock) {
		ret =  __hwspin_lock_timeout(hwlock, 1000, HWLOCK_RAW, &hwspinlock_flag);
		if (ret != 0) {
			G2D_ERR("G2D: Hwspinlock is already taken \n");
			hwspin_lock_free(hwlock);
			return -1;
		} else {
			/* request the irq */
			ret = request_irq(para.irq, g2d_handle_irq, 0,
					  dev_name(&pdev->dev), &g2d_ext_hd);
			if (ret) {
				G2D_ERR("failed to install irq resource\n");
				goto release_regs;
			}
			disable_irq(para.irq);
			__hwspin_unlock(hwlock, HWLOCK_RAW, &hwspinlock_flag);
		}
	}
#endif

#if !IS_ENABLED(CONFIG_G2D_USE_HWSPINLOCK)
	/* request the irq */
	ret = request_irq(para.irq, g2d_handle_irq, 0,
			  dev_name(&pdev->dev), &g2d_ext_hd);
	if (ret) {
		G2D_ERR("failed to install irq resource\n");
		goto release_regs;
	}
#endif
	/* clk init */
	para.clk = devm_clk_get(&pdev->dev, "g2d");
	if (IS_ERR(para.clk)) {
		G2D_ERR("fail to get clk\n");
		ret = -ENXIO;
		goto out_dispose_mapping;
	} else {
		para.clk_parent = clk_get_parent(para.clk);
		para.bus_clk = devm_clk_get(&pdev->dev, "bus");
		if (IS_ERR(para.bus_clk))
			para.bus_clk = NULL;
		para.mbus_clk = devm_clk_get(&pdev->dev, "mbus_g2d");
		if (IS_ERR(para.mbus_clk))
			para.mbus_clk = NULL;
		para.ahb_clk = devm_clk_get(&pdev->dev, "ahb_g2d");
		if (IS_ERR(para.ahb_clk))
			para.ahb_clk = NULL;
		para.vo_clk = devm_clk_get(&pdev->dev, "vo");
		if (IS_ERR(para.vo_clk))
			para.vo_clk = NULL;
		para.mbus_vo_clk = devm_clk_get(&pdev->dev, "mbus_vo");
		if (IS_ERR(para.mbus_vo_clk))
			para.mbus_vo_clk = NULL;
		para.mbus_desys_clk = devm_clk_get(&pdev->dev, "mbus_desys");
		if (IS_ERR(para.mbus_desys_clk))
			para.mbus_desys_clk = NULL;
		para.ahb_de_clk = devm_clk_get(&pdev->dev, "ahb_de");
		if (IS_ERR(para.ahb_de_clk))
			para.ahb_de_clk = NULL;
		para.hb_clk = devm_clk_get(&pdev->dev, "g2d_hb");
		if (IS_ERR(para.hb_clk))
			para.hb_clk = NULL;
		para.reset = devm_reset_control_get(&pdev->dev, NULL);
		if (IS_ERR(para.reset)) {
			G2D_WARN("reset get failed\n");
			para.reset = NULL;
		}
		para.vo_reset = devm_reset_control_get_optional_shared(&pdev->dev, "rst_bus_vo");
		if (IS_ERR(para.vo_reset)) {
			G2D_WARN("vo_reset get failed\n");
			para.vo_reset = NULL;
		}
		para.desys_reset = devm_reset_control_get_optional_shared(&pdev->dev, "rst_bus_desys");
		if (IS_ERR(para.desys_reset)) {
			G2D_WARN("desys_reset get failed\n");
			para.desys_reset = NULL;
		}
	}

	ret = of_property_read_u32(pdev->dev.of_node, "thread-id",
		&(g2d_ext_hd.current_thread_id));
	if (ret != 0)
		g2d_ext_hd.current_thread_id = -1;
	G2D_INFO("current_thread_id=%d\n", g2d_ext_hd.current_thread_id);

	drv_g2d_init();
	mutex_init(&(para.mutex));

	ret = sysfs_create_group(&(para.g2d_dev->kobj), &g2d_attribute_group);
	if (ret < 0)
		G2D_ERR("sysfs_create_file fail\n");

#if (IS_ENABLED(CONFIG_G2D_USE_HWSPINLOCK) && !IS_ENABLED(CONFIG_PM_GENERIC_DOMAINS))
	/* to avoid g2d used by rv but no power */
	pm_runtime_get_sync(para.dev);
#endif
	return 0;

out_dispose_mapping:
	irq_dispose_mapping(para.irq);
release_regs:
	iounmap(para.io);
dealloc_fb:
	platform_set_drvdata(pdev, NULL);

	return ret;
}

static int g2d_remove(struct platform_device *pdev)
{
#if (IS_ENABLED(CONFIG_G2D_USE_HWSPINLOCK) && !IS_ENABLED(CONFIG_PM_GENERIC_DOMAINS))
	pm_runtime_put_sync(para.dev);
#endif
#if IS_ENABLED(CONFIG_G2D_USE_HWSPINLOCK)
	if (hwlock)
		hwspin_lock_free(hwlock);
#endif

#if IS_ENABLED(CONFIG_PM_GENERIC_DOMAINS)
	pm_runtime_disable(para.dev);
#endif

	free_irq(para.irq, NULL);

	drv_g2d_exit();

	platform_set_drvdata(pdev, NULL);

	sysfs_remove_group(&(para.g2d_dev->kobj), &g2d_attribute_group);

	G2D_INFO("Driver unloaded succesfully\n");
	return 0;
}

static const struct dev_pm_ops g2d_pm_ops = {
	.suspend = g2d_suspend,
	.resume = g2d_resume,
#if IS_ENABLED(CONFIG_PM_GENERIC_DOMAINS)
	.runtime_suspend = g2d_runtime_suspend,
	.runtime_resume = g2d_runtime_resume,
#endif
};

static const struct of_device_id sunxi_g2d_match[] = {
	{.compatible = "allwinner,sunxi-g2d",},
	{},
};

static const struct file_operations g2d_fops = {
	.owner = THIS_MODULE,
	.open = g2d_open,
	.release = g2d_release,
	.unlocked_ioctl = g2d_ioctl,
	.mmap = g2d_mmap,
};

static struct platform_driver g2d_driver = {
	.probe = g2d_probe,
	.remove = g2d_remove,
	.driver = {
		.owner = THIS_MODULE,
		.name = "g2d",
		.pm   = &g2d_pm_ops,
		.of_match_table = sunxi_g2d_match,
	},
};

int __init g2d_module_init(void)
{
	int ret = 0, err;

	alloc_chrdev_region(&(para.devid), 0, 1, "g2d_chrdev");
	if (para.g2d_cdev) {
		kfree(para.g2d_cdev);
		para.g2d_cdev = NULL;
	}
	para.g2d_cdev = cdev_alloc();
	cdev_init(para.g2d_cdev, &g2d_fops);
	para.g2d_cdev->owner = THIS_MODULE;
	err = cdev_add(para.g2d_cdev, para.devid, 1);
	if (err) {
		G2D_ERR("I was assigned major number %d\n", MAJOR(para.devid));
		return -1;
	}
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 6, 0)
	para.g2d_class = class_create("g2d");
#else
	para.g2d_class = class_create(THIS_MODULE, "g2d");
#endif
	if (IS_ERR(para.g2d_class)) {
		G2D_ERR("create class error\n");
		return -1;
	}

	para.g2d_dev = device_create(para.g2d_class, NULL, para.devid, NULL, "g2d");
	if (ret == 0)
		ret = platform_driver_register(&g2d_driver);

#if IS_ENABLED(CONFIG_G2D_SYNCFENCE)
	syncfence_init();
#endif

	G2D_INFO("rcq version initialized.major:%d\n", MAJOR(para.devid));
	G2D_INFO("g2d_module_init\n");
	return ret;
}

static void __exit g2d_module_exit(void)
{

#if IS_ENABLED(CONFIG_G2D_SYNCFENCE)
	syncfence_exit();
#endif

	platform_driver_unregister(&g2d_driver);
	device_destroy(para.g2d_class, para.devid);
	class_destroy(para.g2d_class);

	cdev_del(para.g2d_cdev);
	G2D_INFO("g2d_module_exit\n");
}

/*subsys_initcall_sync(g2d_module_init);*/
module_init(g2d_module_init);
module_exit(g2d_module_exit);

MODULE_AUTHOR("zxb <zhengxiaobin@allwinnertech.com>");
MODULE_DESCRIPTION("g2d(rcq) driver");
MODULE_VERSION("1.0.0");
MODULE_LICENSE("GPL");
MODULE_IMPORT_NS(DMA_BUF);
