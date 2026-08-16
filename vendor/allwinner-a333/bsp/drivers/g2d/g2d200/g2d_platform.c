/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Copyright(c) 2020 - 2023 Allwinner Technology Co.,Ltd. All rights reserved. */
/*
 * Copyright (c) 2007-2019 Allwinnertech Co., Ltd.
 * Author: zhengxiaobin <zhengxiaobin@allwinnertech.com>
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
#include <linux/list.h>
#include "g2d_platform.h"
#include "g2d_driver_i.h"
#include "g2d_top.h"
#include "g2d_mixer.h"
#include "g2d_thread.h"
#include "../g2d_buf_cache.h"

#define G2D_BYTE_ALIGN(x) (((x + (4*1024-1)) >> 12) << 12)
static struct device *g2d_dmabuf_dev;
static LIST_HEAD(g2d_task_list);
static DEFINE_IDA(g2d_task_ida);

static struct g2d_format_attr fmt_attr_tbl[] = {
/*
format  bits hor_rsample(u,v) ver_rsample(u,v) uvc interleave factor div
*/
	{ G2D_FORMAT_ARGB8888, 8,  1, 1, 1, 1, 0, 1, 4, 1},
	{ G2D_FORMAT_ABGR8888, 8,  1, 1, 1, 1, 0, 1, 4, 1},
	{ G2D_FORMAT_RGBA8888, 8,  1, 1, 1, 1, 0, 1, 4, 1},
	{ G2D_FORMAT_BGRA8888, 8,  1, 1, 1, 1, 0, 1, 4, 1},
	{ G2D_FORMAT_XRGB8888, 8,  1, 1, 1, 1, 0, 1, 4, 1},
	{ G2D_FORMAT_XBGR8888, 8,  1, 1, 1, 1, 0, 1, 4, 1},
	{ G2D_FORMAT_RGBX8888, 8,  1, 1, 1, 1, 0, 1, 4, 1},
	{ G2D_FORMAT_BGRX8888, 8,  1, 1, 1, 1, 0, 1, 4, 1},
	{ G2D_FORMAT_RGB888, 8,  1, 1, 1, 1, 0, 1, 3, 1},
	{ G2D_FORMAT_BGR888, 8,  1, 1, 1, 1, 0, 1, 3, 1},
	{ G2D_FORMAT_RGB565, 8,  1, 1, 1, 1, 0, 1, 2, 1},
	{ G2D_FORMAT_BGR565, 8,  1, 1, 1, 1, 0, 1, 2, 1},
	{ G2D_FORMAT_ARGB4444, 8,  1, 1, 1, 1, 0, 1, 2, 1},
	{ G2D_FORMAT_ABGR4444, 8,  1, 1, 1, 1, 0, 1, 2, 1},
	{ G2D_FORMAT_RGBA4444, 8,  1, 1, 1, 1, 0, 1, 2, 1},
	{ G2D_FORMAT_BGRA4444, 8,  1, 1, 1, 1, 0, 1, 2, 1},
	{ G2D_FORMAT_ARGB1555, 8,  1, 1, 1, 1, 0, 1, 2, 1},
	{ G2D_FORMAT_ABGR1555, 8,  1, 1, 1, 1, 0, 1, 2, 1},
	{ G2D_FORMAT_RGBA5551, 8,  1, 1, 1, 1, 0, 1, 2, 1},
	{ G2D_FORMAT_BGRA5551, 8,  1, 1, 1, 1, 0, 1, 2, 1},
	{ G2D_FORMAT_ARGB2101010, 10, 1, 1, 1, 1, 0, 1, 4, 1},
	{ G2D_FORMAT_ABGR2101010, 10, 1, 1, 1, 1, 0, 1, 4, 1},
	{ G2D_FORMAT_RGBA1010102, 10, 1, 1, 1, 1, 0, 1, 4, 1},
	{ G2D_FORMAT_BGRA1010102, 10, 1, 1, 1, 1, 0, 1, 4, 1},
	{ G2D_FORMAT_IYUV422_V0Y1U0Y0, 8,  1, 1, 1, 1, 0, 1, 2, 1},
	{ G2D_FORMAT_IYUV422_Y1V0Y0U0, 8,  1, 1, 1, 1, 0, 1, 2, 1},
	{ G2D_FORMAT_IYUV422_U0Y1V0Y0, 8,  1, 1, 1, 1, 0, 1, 2, 1},
	{ G2D_FORMAT_IYUV422_Y1U0Y0V0, 8,  1, 1, 1, 1, 0, 1, 2, 1},
	{ G2D_FORMAT_YUV422_PLANAR, 8,  2, 2, 1, 1, 0, 0, 2, 1},
	{ G2D_FORMAT_YUV420_PLANAR, 8,  2, 2, 2, 2, 0, 0, 3, 2},
	{ G2D_FORMAT_YUV411_PLANAR, 8,  4, 4, 1, 1, 0, 0, 3, 2},
	{ G2D_FORMAT_YUV422UVC_U1V1U0V0, 8,  2, 2, 1, 1, 1, 0, 2, 1},
	{ G2D_FORMAT_YUV422UVC_V1U1V0U0, 8,  2, 2, 1, 1, 1, 0, 2, 1},
	{ G2D_FORMAT_YUV420UVC_U1V1U0V0, 8,  2, 2, 2, 2, 1, 0, 3, 2},
	{ G2D_FORMAT_YUV420UVC_V1U1V0U0, 8,  2, 2, 2, 2, 1, 0, 3, 2},
	{ G2D_FORMAT_YUV411UVC_U1V1U0V0, 8,  4, 4, 1, 1, 1, 0, 3, 2},
	{ G2D_FORMAT_YUV411UVC_V1U1V0U0, 8,  4, 4, 1, 1, 1, 0, 3, 2},
	{ G2D_FORMAT_Y8, 8,  1, 1, 1, 1, 0, 0, 1, 1},
	{ G2D_FORMAT_YVU10_444, 10, 1, 1, 1, 1, 0, 1, 4, 1},
	{ G2D_FORMAT_YVU10_P210, 10, 2, 2, 1, 1, 0, 0, 4, 1},
	{ G2D_FORMAT_YVU10_P010, 10, 2, 2, 2, 2, 0, 0, 3, 1},
};

void g2d_set_dmabuf_dev(struct device *dmabuf_dev)
{
	g2d_dmabuf_dev = dmabuf_dev;
}

#if IS_ENABLED(CONFIG_G2D_BUF_CACHED)

int g2d_dma_map(int fd, struct dmabuf_item *item)
{
	int ret = 0;

	if (fd < 0 || !item) {
		G2D_ERR("g2d_dma_map error, invalid dma-buf fd or param\n");
		return -EINVAL;
	}

	ret = g2d_buf_cached_map(fd, &item->dma_addr);
	if (ret == 0) {
		item->fd = fd;
		item->buf = NULL;
		item->sgt = NULL;
		item->attachment = NULL;
		return 0;
	}

	G2D_ERR("g2d_buf_cached_map error, ret=%d\n", ret);
	return ret;
}

void g2d_dma_unmap(struct dmabuf_item *item)
{
	int ret = 0;

	if (!item || item->fd < 0) {
		G2D_ERR("g2d_dma_unmap error, invalid dma-buf fd or param\n");
		return;
	}

	ret = g2d_buf_cached_unmap(item->fd, item->dma_addr);
	if (ret != 0) {
		G2D_ERR("g2d_buf_cached_unmap error, ret=%d\n", ret);
	}

	return;
}

#else

int g2d_dma_map(int fd, struct dmabuf_item *item)
{
	struct dma_buf *dmabuf;
	struct dma_buf_attachment *attachment;
	struct sg_table *sgt;
	int ret = -1;
	struct g2d_time_info *g2d_time_inf = get_g2d_time_inf();

	ktime_get_real_ts64(&(g2d_time_inf->dma_map_start_ts));
	if (fd < 0) {
		G2D_WARN("dma_buf_id %d is invalid\n", fd);
		goto exit;
	}
	dmabuf = dma_buf_get(fd);
	if (IS_ERR(dmabuf)) {
		G2D_WARN("dma_buf_get failed, fd=%d\n", fd);
		goto exit;
	}

	attachment = dma_buf_attach(dmabuf, g2d_dmabuf_dev);
	if (IS_ERR(attachment)) {
		G2D_WARN("dma_buf_attach failed\n");
		goto err_buf_put;
	}
	sgt = dma_buf_map_attachment(attachment, DMA_BIDIRECTIONAL);
	if (IS_ERR_OR_NULL(sgt)) {
		G2D_WARN("dma_buf_map_attachment failed\n");
		goto err_buf_detach;
	}

	item->fd = fd;
	item->buf = dmabuf;
	item->sgt = sgt;
	item->attachment = attachment;
	item->dma_addr = sg_dma_address(sgt->sgl);
	ret = 0;
	goto exit;

err_buf_detach:
	dma_buf_detach(dmabuf, attachment);
err_buf_put:
	dma_buf_put(dmabuf);
exit:
	ktime_get_real_ts64(&(g2d_time_inf->dma_map_end_ts));
	return ret;
}

void g2d_dma_unmap(struct dmabuf_item *item)
{
	struct g2d_time_info *g2d_time_inf = get_g2d_time_inf();
	ktime_get_real_ts64(&(g2d_time_inf->dma_unmap_start_ts));
	dma_buf_unmap_attachment(item->attachment, item->sgt, DMA_BIDIRECTIONAL);
	dma_buf_detach(item->buf, item->attachment);
	dma_buf_put(item->buf);
	ktime_get_real_ts64(&(g2d_time_inf->dma_unmap_end_ts));
}

#endif // CONFIG_G2D_BUF_CACHED

__s32 g2d_set_info(g2d_image_enh *g2d_img, struct dmabuf_item *item)
{
	__s32 ret = -1;
	__u32 i = 0;
	__u32 len = ARRAY_SIZE(fmt_attr_tbl);
	__u32 y_width, y_height, u_width, u_height;
	__u32 y_pitch, u_pitch;
	__u32 y_size, u_size;

	g2d_img->laddr[0] = item->dma_addr;

	if (g2d_img->format >= G2D_FORMAT_MAX) {
		G2D_WARN("format 0x%x is out of range\n", g2d_img->format);
		goto exit;
	}

	for (i = 0; i < len; ++i) {

		if (fmt_attr_tbl[i].format == g2d_img->format) {
			y_width = g2d_img->width;
			y_height = g2d_img->height;
			u_width = y_width/fmt_attr_tbl[i].hor_rsample_u;
			u_height = y_height/fmt_attr_tbl[i].ver_rsample_u;

			y_pitch = G2DALIGN(y_width, g2d_img->align[0]);
			u_pitch = G2DALIGN(u_width * (fmt_attr_tbl[i].uvc + 1),
					g2d_img->align[1]);

			y_size = y_pitch * y_height;
			u_size = u_pitch * u_height;
			g2d_img->laddr[1] = g2d_img->laddr[0] + y_size;
			g2d_img->laddr[2] = g2d_img->laddr[0] + y_size + u_size;

			if (g2d_img->format == G2D_FORMAT_YUV420_PLANAR) {
				/* v */
				g2d_img->laddr[1] = g2d_img->laddr[0] + y_size + u_size;
				g2d_img->laddr[2] = g2d_img->laddr[0] + y_size; /* u */
			}

			ret = 0;
			break;
		}
	}
	if (ret != 0)
		G2D_WARN("format 0x%x is invalid\n", g2d_img->format);
exit:
	return ret;

}

int g2d_set_image_addr(struct dmabuf_item **p_item, g2d_image_enh *p_img)
{
	int ret = -1;

	if (!p_item || !p_img)
		goto OUT;

	if (!p_img->use_phy_addr) {
		*p_item = kmalloc(sizeof(**p_item),
				  GFP_KERNEL | __GFP_ZERO);
		if (!*p_item)
			goto OUT;
		if (g2d_dma_map(p_img->fd, *p_item)) {
			kfree(*p_item);
			*p_item = NULL;
			G2D_WARN("map dst fail\n");
			goto OUT;
		}
		ret = g2d_set_info(p_img, *p_item);
	} else
		ret = 0;
OUT:
	return ret;
}

__s32 g2d_byte_cal(__u32 format, __u32 *ycnt, __u32 *ucnt, __u32 *vcnt)
{
	*ycnt = 0;
	*ucnt = 0;
	*vcnt = 0;
	if (format <= G2D_FORMAT_BGRX8888)
		*ycnt = 4;

	else if (format <= G2D_FORMAT_BGR888)
		*ycnt = 3;

	else if (format <= G2D_FORMAT_BGRA5551)
		*ycnt = 2;

	else if (format <= G2D_FORMAT_BGRA1010102)
		*ycnt = 4;

	else if (format <= 0x23) {
		*ycnt = 2;
	}

	else if (format <= 0x25) {
		*ycnt = 1;
		*ucnt = 2;
	}

	else if (format == 0x26) {
		*ycnt = 1;
		*ucnt = 1;
		*vcnt = 1;
	}

	else if (format <= 0x29) {
		*ycnt = 1;
		*ucnt = 2;
	}

	else if (format == 0x2a) {
		*ycnt = 1;
		*ucnt = 1;
		*vcnt = 1;
	}

	else if (format <= 0x2d) {
		*ycnt = 1;
		*ucnt = 2;
	}

	else if (format == 0x2e) {
		*ycnt = 1;
		*ucnt = 1;
		*vcnt = 1;
	}

	else if (format == 0x30)
		*ycnt = 1;

	else if (format <= 0x36) {
		*ycnt = 2;
		*ucnt = 4;
	}

	else if (format <= 0x39)
		*ycnt = 6;
	return 0;
}


/*
 */
__u32 cal_align(__u32 width, __u32 align)
{
	int slide = 0;
	int number = 2;

	switch (align) {
	case 0:
		return width;
	case 2:
		return (width + 1) >> 1 << 1;
	case 4:
		return (width + 3) >> 2 << 2;
	case 8:
		return (width + 7) >> 3 << 3;
	case 16:
		return (width + 15) >> 4 << 4;
	case 32:
		return (width + 31) >> 5 << 5;
	case 64:
		return (width + 63) >> 6 << 6;
	case 128:
		return (width + 127) >> 7 << 7;
	default:
		while (number <= align) {
			number = number << 1;
			slide  = slide + 1;
		}
		return ((width + align - 1) >> slide << slide);
	}
}

__s32 g2d_image_check(g2d_image_enh *p_image)
{
	__s32 ret = -EINVAL;

	if (!p_image) {
		G2D_WARN("NUll pointer\n");
		goto OUT;
	}

	if (((p_image->clip_rect.x < 0) &&
	     ((-p_image->clip_rect.x) > p_image->clip_rect.w)) ||
	    ((p_image->clip_rect.y < 0) &&
	     ((-p_image->clip_rect.y) > p_image->clip_rect.h)) ||
	    ((p_image->clip_rect.x > 0) &&
	     (p_image->clip_rect.x > p_image->width - 1)) ||
	    ((p_image->clip_rect.y > 0) &&
	     (p_image->clip_rect.y > p_image->height - 1))) {
		G2D_WARN("Invalid imager parameter setting\n");
		goto OUT;
	}

	if (((p_image->clip_rect.x < 0) &&
				((-p_image->clip_rect.x) <
				 p_image->clip_rect.w))) {
		p_image->clip_rect.w =
			p_image->clip_rect.w +
			p_image->clip_rect.x;
		p_image->clip_rect.x = 0;
	} else if ((p_image->clip_rect.x +
				p_image->clip_rect.w)
			> p_image->width) {
		p_image->clip_rect.w =
			p_image->width -
			p_image->clip_rect.x;
	}
	if (((p_image->clip_rect.y < 0) &&
				((-p_image->clip_rect.y) <
				 p_image->clip_rect.h))) {
		p_image->clip_rect.h =
			p_image->clip_rect.h +
			p_image->clip_rect.y;
		p_image->clip_rect.y = 0;
	} else if ((p_image->clip_rect.y +
				p_image->clip_rect.h)
			> p_image->height) {
		p_image->clip_rect.h =
			p_image->height -
			p_image->clip_rect.y;
	}

	p_image->bpremul = 0;

	ret = 0;
OUT:
	return ret;

}

void g2d_bsp_set_base(unsigned long base)
{
#if IS_ENABLED(CONFIG_G2D200)
	g2d_thread_set_base(base);
#else
	g2d_top_set_base(base);
#endif
}

__s32 g2d_bsp_open(int thread_id)
{
	int ret = 0;
#if IS_ENABLED(CONFIG_G2D200)
	g2d_thread_open();
	g2d_threadn_open(thread_id);
#else
	g2d_top_open();
#endif
	return ret;
}

__s32 g2d_bsp_close(int thread_id)
{
	int ret = 0;
#if IS_ENABLED(CONFIG_G2D200)
	g2d_thread_close();
	g2d_threadn_close(thread_id);
#else
	g2d_top_close();
#endif
	return ret;
}

__s32 g2d_bsp_reset(int thread_id)
{
	int ret = 0;
#if IS_ENABLED(CONFIG_G2D200)
	g2d_thread_reset();
	g2d_threadn_reset(thread_id);
#else
	g2d_top_reset();
#endif
	return ret;
}

void g2d_bsp_handle_irq(__g2d_drv_t *g2d_ext_hd)
{
#if IS_ENABLED(CONFIG_G2D200)
	if (g2d_threadn_task_end_irq_query(g2d_ext_hd->current_thread_id)) {
		g2d_threadn_reset(g2d_ext_hd->current_thread_id);
		g2d_ext_hd->finish_flag = 1;
		wake_up(&(g2d_ext_hd->queue));
		return;
	}
#else

#if IS_ENABLED(CONFIG_G2D_MIXER)
#if G2D_MIXER_RCQ_USED == 1
	if (g2d_top_rcq_task_irq_query()) {
		g2d_top_mixer_reset();
		g2d_ext_hd->finish_flag = 1;
		wake_up(&(g2d_ext_hd->queue));
		return;
	}
#else
	if (g2d_mixer_irq_query()) {
		g2d_top_mixer_reset();
		g2d_ext_hd->finish_flag = 1;
		wake_up(&(g2d_ext_hd->queue));
		return;
	}
#endif
#endif

#if IS_ENABLED(CONFIG_G2D_ROTATE)
	if (g2d_rot_irq_query()) {
		g2d_top_rot_reset();
		g2d_ext_hd->finish_flag = 1;
		wake_up(&(g2d_ext_hd->queue));
		return;
	}
#endif

#endif
}

static __s32 g2d_task_mem_setup(struct g2d_task *p_task,
				 struct mixer_para *p_para)
{
	__u32 i = 0, frame_index = 0;
	__u32 rcq_reg_mem_size = 0;
	__s32 ret = -1;
	struct g2d_reg_block **p_reg_blks;
	struct g2d_rcq_head *rcq_hd = NULL;

	if (!p_task->p_rcq_info)
		goto OUT;

	p_task->p_rcq_info->block_num_per_frame =
	    p_task->frame[0].frame_get_reg_block_num(&p_task->frame[0]);

	p_task->p_rcq_info->alloc_num_per_frame =
	    G2D_RCQ_HEADER_ALIGN(p_task->p_rcq_info->block_num_per_frame);
	/* The frame length must be 32 bytes aligned,
	 * which means the number of blocks in each frame must be an even number. */

	p_task->p_rcq_info->rcq_header_len =
	    p_task->p_rcq_info->alloc_num_per_frame *
	    sizeof(*(p_task->p_rcq_info->vir_addr));

	/* real block num */
	p_task->p_rcq_info->cur_num =
	    p_task->p_rcq_info->block_num_per_frame * p_task->frame_cnt;

	/* block num that need to be alloced */
	p_task->p_rcq_info->alloc_num =
	    p_task->p_rcq_info->alloc_num_per_frame * p_task->frame_cnt;

	/* regblocks + rcq header */
	/* size of reg_values restored in dram */
	rcq_reg_mem_size =
	    p_task->frame[0].frame_get_rcq_mem_size(&p_task->frame[0]) *
	    p_task->frame_cnt;

	/* size of reg_blk_header restored in dram */
	rcq_reg_mem_size += sizeof(*(p_task->p_rcq_info->vir_addr)) *
			    p_task->p_rcq_info->alloc_num;

	p_task->p_rcq_info->rcq_reg_mem_size = rcq_reg_mem_size;
	if (g2d_top_mem_pool_alloc(p_task->p_rcq_info)) {
		G2D_WARN("g2d_top_mem_pool_alloc fail\n");
		goto OUT;
	}

	/* malloc memory for rcq queue */

	if (!p_task->p_rcq_info->vir_addr) {
		G2D_WARN("Malloc rcq queue memory fail\n");
		goto OUT;
	}

	p_task->p_rcq_info->reg_blk =
	    kmalloc(sizeof(*(p_task->p_rcq_info->reg_blk)) *
			p_task->p_rcq_info->cur_num,
		    GFP_KERNEL | __GFP_ZERO);

	if (p_task->p_rcq_info->reg_blk == NULL) {
		G2D_WARN("kalloc for g2d_reg_block failed\n");
		goto OUT;
	}

	p_reg_blks = p_task->p_rcq_info->reg_blk;
	for (i = 0; i < p_task->frame_cnt; ++i) {
		if (p_task->frame[i].frame_mem_setup(
			&p_task->frame[i], &p_para[i], p_task)) {
			G2D_WARN("Frame:%d setupt fail\n", i);
			goto OUT;
		}
		p_task->frame[i].core->get_reg_block(p_task->frame[i].core,
						p_reg_blks);
#if IS_ENABLED(CONFIG_G2D200)
		p_task->frame[i].core->get_reg_block(p_task->frame[i].core,
					   p_reg_blks);
		p_reg_blks +=
			p_task->frame[i].core->get_reg_block_num(p_task->frame[i].core);
#endif
		if (p_task->frame[i].type == MIXER_FRAME) {
			/* overlay video */
			p_task->frame[i].ovl_v->get_reg_block(p_task->frame[i].ovl_v,
								  p_reg_blks);
			p_reg_blks += p_task->frame[i].ovl_v->get_reg_block_num(
				p_task->frame[i].ovl_v);

			/* overlay ui */
			p_task->frame[i].ovl_u->get_reg_block(p_task->frame[i].ovl_u,
								  p_reg_blks);

			p_reg_blks += p_task->frame[i].ovl_u->get_reg_block_num(
				p_task->frame[i].ovl_u);

			/* scaler */
			p_task->frame[i].scal->get_reg_block(p_task->frame[i].scal,
								 p_reg_blks);
			p_reg_blks += p_task->frame[i].scal->get_reg_block_num(
				p_task->frame[i].scal);

			/* blender */
			p_task->frame[i].bld->get_reg_block(p_task->frame[i].bld,
								p_reg_blks);
			p_reg_blks += p_task->frame[i].bld->get_reg_block_num(
				p_task->frame[i].bld);

			/* write back */
			p_task->frame[i].wb->get_reg_block(p_task->frame[i].wb,
							   p_reg_blks);
			p_reg_blks +=
				p_task->frame[i].wb->get_reg_block_num(p_task->frame[i].wb);
		} else if (p_task->frame[i].type == ROTATE_FRAME) {
			/* rot */
			p_task->frame[i].rot->get_reg_block(p_task->frame[i].rot,
							   p_reg_blks);
			p_reg_blks +=
				p_task->frame[i].rot->get_reg_block_num(p_task->frame[i].rot);
		}
	}

	p_reg_blks = p_task->p_rcq_info->reg_blk;
	rcq_hd = p_task->p_rcq_info->vir_addr;
	for (frame_index = 0; frame_index < p_task->frame_cnt; ++frame_index) {
		for (i = 0; i < p_task->p_rcq_info->alloc_num_per_frame; ++i) {
			struct g2d_reg_block *reg_blk = *p_reg_blks;

			if (p_task->p_rcq_info->alloc_num_per_frame >
				p_task->p_rcq_info->block_num_per_frame &&
			    i == p_task->p_rcq_info->block_num_per_frame) {
				if (frame_index == p_task->frame_cnt - 1)
					rcq_hd->dirty.bits.n_header_len = 0;
				else
					rcq_hd->dirty.bits.n_header_len =
					    p_task->p_rcq_info->rcq_header_len;
				++rcq_hd;
			} else {
				rcq_hd->low_addr =
				    (__u32)((uintptr_t)(reg_blk->phy_addr) & 0xffffffff);
#if IS_ENABLED(CONFIG_ARM64)
				rcq_hd->dw0.bits.high_addr =
				    (u8)((__u64)(reg_blk->phy_addr) >> 32);
#else
				rcq_hd->dw0.bits.high_addr =
				    (u8)((__u64)(__u32)(reg_blk->phy_addr) >> 32);
#endif
				rcq_hd->dw0.bits.len = reg_blk->size;
				rcq_hd->dirty.bits.dirty = reg_blk->dirty;
				rcq_hd->reg_offset = (__u32)(__u64)(
				    reg_blk->reg_addr -
				    (u8 __iomem *)p_task->p_g2d_info->io);
				reg_blk->rcq_hd = rcq_hd;

				/* last frame's next frame len should be zero */
				if (frame_index == p_task->frame_cnt - 1)
					rcq_hd->dirty.bits.n_header_len = 0;
				else
					rcq_hd->dirty.bits.n_header_len =
					    p_task->p_rcq_info->rcq_header_len;

				++rcq_hd;
				++p_reg_blks;
			}
		}
	}

	ret = 0;
OUT:
	return ret;
}

#if IS_ENABLED(CONFIG_G2D200)
static __u32 get_core_frame_rcq_mem_size(struct g2d_frame *p_frame)
{
	int rcq_mem_size = 0;
	rcq_mem_size += p_frame->core->get_rcq_mem_size(p_frame->core);
	return rcq_mem_size;
}
#endif

static __u32 get_rot_frame_rcq_mem_size(struct g2d_frame *p_frame)
{
	int rcq_mem_size = 0;
	rcq_mem_size += p_frame->rot->get_rcq_mem_size(p_frame->rot);
	return rcq_mem_size;
}

static __u32 get_mixer_frame_rcq_mem_size(struct g2d_frame *p_frame)
{
	int rcq_mem_size = 0;
	rcq_mem_size += p_frame->wb->get_rcq_mem_size(p_frame->wb);
	rcq_mem_size += p_frame->ovl_v->get_rcq_mem_size(p_frame->ovl_v);
	rcq_mem_size += p_frame->ovl_u->get_rcq_mem_size(p_frame->ovl_u);
	rcq_mem_size += p_frame->scal->get_rcq_mem_size(p_frame->scal);
	rcq_mem_size += p_frame->bld->get_rcq_mem_size(p_frame->bld);
	return rcq_mem_size;
}

static __u32 g2d_get_frame_rcq_mem_size(struct g2d_frame *p_frame)
{
	__u32 rcq_mem_size = 0;
#if IS_ENABLED(CONFIG_G2D200)
	rcq_mem_size += get_core_frame_rcq_mem_size(p_frame);
#endif
	if (p_frame->type == MIXER_FRAME) {
		rcq_mem_size += get_mixer_frame_rcq_mem_size(p_frame);
	} else if (p_frame->type == ROTATE_FRAME) {
		rcq_mem_size += get_rot_frame_rcq_mem_size(p_frame);
	}
	return rcq_mem_size;
}

#if IS_ENABLED(CONFIG_G2D200)
static __s32 core_frame_mem_setup(struct g2d_frame *p_frame,
				 struct g2d_task *p_task)
{
	int ret;
	ret = p_frame->core->rcq_setup(p_frame->core, p_frame->g2d_base,
				p_task->p_rcq_info);
	return ret;
}
#endif

static __s32 mixer_frame_mem_setup(struct g2d_frame *p_frame,
				 struct g2d_task *p_task)
{
	int ret;
	ret = p_frame->wb->rcq_setup(p_frame->wb, p_frame->g2d_base,
				p_task->p_rcq_info);
	if (ret)
		goto OUT;
	ret = p_frame->ovl_v->rcq_setup(p_frame->ovl_v, p_frame->g2d_base,
				p_task->p_rcq_info);
	if (ret)
		goto OUT;
	ret = p_frame->ovl_u->rcq_setup(p_frame->ovl_u, p_frame->g2d_base,
				p_task->p_rcq_info);
	if (ret)
		goto OUT;
	ret = p_frame->bld->rcq_setup(p_frame->bld, p_frame->g2d_base,
				p_task->p_rcq_info);
	if (ret)
		goto OUT;
	ret = p_frame->scal->rcq_setup(p_frame->scal, p_frame->g2d_base,
				p_task->p_rcq_info);
OUT:
	return ret;
}

static __s32 rot_frame_mem_setup(struct g2d_frame *p_frame,
				 struct g2d_task *p_task)
{
	int ret;
	ret = p_frame->rot->rcq_setup(p_frame->rot, p_frame->g2d_base,
					  p_task->p_rcq_info);
	return ret;
}

static __s32 g2d_frame_mem_setup(struct g2d_frame *p_frame,
				 struct mixer_para *p_para,
				 struct g2d_task *p_task)
{
	__s32 ret = -1;

#if IS_ENABLED(CONFIG_G2D200)
	ret = core_frame_mem_setup(p_frame, p_task);
	if (ret)
		goto OUT;
#endif

	if (p_frame->type == MIXER_FRAME) {
		ret = mixer_frame_mem_setup(p_frame, p_task);
	} else if (p_frame->type == ROTATE_FRAME) {
		ret = rot_frame_mem_setup(p_frame, p_task);
	}
	if (ret)
		goto OUT;

	if (p_para[0].op_flag & OP_SPLIT_MEM)
		goto OUT;

	/* we will free & dma unmap them in frame->destory function */
	ret = g2d_set_image_addr(&p_frame->dst_item, &p_para->dst_image_h);
	if (ret)
		goto OUT;

	if (p_para->op_flag > OP_FILLRECT
		&& p_para->src_image_h.bbuff == 1) {
		ret = g2d_set_image_addr(&p_frame->src_item,
					 &p_para->src_image_h);
		if (ret)
			goto OUT;
	}

	if (p_para->op_flag & OP_BLEND
		&& p_para->ptn_image_h.bbuff == 1) {
		ret = g2d_set_image_addr(&p_frame->ptn_item,
					 &p_para->ptn_image_h);
		if (ret)
			goto OUT;
	}

	if (p_para->op_flag & OP_MASK
		&& p_para->ptn_image_h.bbuff == 1) {
		ret = g2d_set_image_addr(&p_frame->ptn_item,
					 &p_para->ptn_image_h);
		if (ret)
			goto OUT;
		ret = g2d_set_image_addr(&p_frame->mask_item,
					 &p_para->mask_image_h);
		if (ret)
			goto OUT;
	}

OUT:
	return ret;
}

__s32 g2d_frame_apply(struct g2d_frame *p_frame,
			    struct mixer_para *p_para)
{
	__s32 ret = -1;

	if (g2d_image_check(&p_para->dst_image_h))
		goto OUT;
	if ((p_para->op_flag & OP_BITBLT)
		|| (p_para->op_flag & OP_BLEND)
		|| (p_para->op_flag & OP_ROTATE)) {
		if (g2d_image_check(&p_para->src_image_h))
			goto OUT;
		if (p_para->op_flag & OP_BLEND) {
			/* actually is use as src2 */
			if (g2d_image_check(&p_para->ptn_image_h))
				goto OUT;
		}
	} else if ((p_para->op_flag & OP_MASK)) {
		p_para->dst_image_h.bbuff = 1;
		p_para->src_image_h.bbuff = 1;
		p_para->ptn_image_h.bbuff = 1;
		p_para->mask_image_h.bbuff = 1;
	}

	ret = 0;

#if IS_ENABLED(CONFIG_G2D_MIXER)
	if (p_para->op_flag & OP_BITBLT) {
		ret = g2d_bsp_bitblt(p_frame, &p_para->src_image_h,
				     &p_para->dst_image_h, p_para->flag_h);
	} else if (p_para->op_flag & OP_BLEND) {
		ret = g2d_bsp_bld(p_frame, &p_para->src_image_h, &p_para->ptn_image_h,
				  &p_para->dst_image_h, p_para->bld_cmd,
				  &p_para->ck_para);
	} else if (p_para->op_flag & OP_FILLRECT) {
		ret = g2d_fillrectangle(p_frame, &p_para->dst_image_h,
					p_para->dst_image_h.color);
	} else if (p_para->op_flag & OP_MASK) {
		ret = g2d_bsp_maskblt(
		    p_frame, &p_para->src_image_h, &p_para->ptn_image_h,
		    &p_para->mask_image_h, &p_para->dst_image_h,
		    p_para->back_flag, p_para->fore_flag);
	}
#endif
#if IS_ENABLED(CONFIG_G2D_ROTATE)
	if (p_para->op_flag & OP_ROTATE) {
		ret = g2d_rot_set(p_frame->rot, &p_para->src_image_h,
			 &p_para->dst_image_h, p_para->flag_h,
			 p_para->lbc_para.is_lbc, p_para->lbc_para.lbc_cmp_ratio,
			 p_para->lbc_para.enc_is_lossy, p_para->lbc_para.dec_is_lossy);
	}
#endif

#if IS_ENABLED(CONFIG_G2D200)
	if (p_frame->type == MIXER_FRAME) {
		core_ctl_set_mixer_en(p_frame->core, 1);
	} else if (p_frame->type == ROTATE_FRAME) {
		core_ctl_set_rotate_en(p_frame->core, 1);
	}
#endif



OUT:
	return ret;
}

static __s32 g2d_frame_destroy(struct g2d_frame *p_frame)
{
	__s32 ret = 0;

#if IS_ENABLED(CONFIG_G2D200)
	ret += p_frame->core->destory(p_frame->core);
#endif

	if (p_frame->type == MIXER_FRAME) {
		ret += p_frame->wb->destory(p_frame->wb);
		ret += p_frame->ovl_v->destory(p_frame->ovl_v);
		ret += p_frame->ovl_u->destory(p_frame->ovl_u);
		ret += p_frame->bld->destory(p_frame->bld);
		ret += p_frame->scal->destory(p_frame->scal);
	} else if (p_frame->type == ROTATE_FRAME) {
		ret += p_frame->rot->destory(p_frame->rot);
	}

	if (p_frame->dst_item) {
		g2d_dma_unmap(p_frame->dst_item);
		kfree(p_frame->dst_item);
		p_frame->dst_item = NULL;
	}
	if (p_frame->src_item) {
		g2d_dma_unmap(p_frame->src_item);
		kfree(p_frame->src_item);
		p_frame->src_item = NULL;
	}
	if (p_frame->ptn_item) {
		g2d_dma_unmap(p_frame->ptn_item);
		kfree(p_frame->ptn_item);
		p_frame->ptn_item = NULL;
	}
	if (p_frame->mask_item) {
		g2d_dma_unmap(p_frame->mask_item);
		kfree(p_frame->mask_item);
		p_frame->mask_item = NULL;
	}


	return ret;
}

static __u32 g2d_frame_get_reg_block_num(struct g2d_frame *p_frame)
{
	__u32 block_num = 0;
#if IS_ENABLED(CONFIG_G2D200)
	block_num += p_frame->core->get_reg_block_num(p_frame->core);
#endif
	if (p_frame->type == MIXER_FRAME) {
		block_num += p_frame->ovl_u->get_reg_block_num(p_frame->ovl_u);
		block_num += p_frame->ovl_v->get_reg_block_num(p_frame->ovl_v);
		block_num += p_frame->scal->get_reg_block_num(p_frame->scal);
		block_num += p_frame->bld->get_reg_block_num(p_frame->bld);
		block_num += p_frame->wb->get_reg_block_num(p_frame->wb);
	} else if (p_frame->type == ROTATE_FRAME) {
		block_num += p_frame->rot->get_reg_block_num(p_frame->rot);
	}

	return block_num;
}

#if IS_ENABLED(CONFIG_G2D200)
static int core_frame_submodule_setup(struct g2d_frame *p_frame)
{
	p_frame->core = g2d_core_submodule_setup(p_frame);
	if (!p_frame->core) {
		G2D_WARN("rot submodule setup fail\n");
		goto FREE;
	}
	return 0;
FREE:
	kfree(p_frame->core);
	return -1;
}
#endif

#if IS_ENABLED(CONFIG_G2D_ROTATE)
static int rot_frame_submodule_setup(struct g2d_frame *p_frame)
{
	p_frame->rot = g2d_rot_submodule_setup(p_frame);
	if (!p_frame->rot) {
		G2D_WARN("rot submodule setup fail\n");
		goto FREE;
	}
	return 0;
FREE:
	kfree(p_frame->rot);
	return -1;
}
#endif

#if IS_ENABLED(CONFIG_G2D_MIXER)
static int mixer_frame_submodule_setup(struct g2d_frame *p_frame)
{
	p_frame->wb = g2d_wb_submodule_setup(p_frame);
	if (!p_frame->wb) {
		G2D_WARN("Write back submodule setup fail\n");
		goto FREE;
	}

	p_frame->ovl_v = g2d_ovl_v_submodule_setup(p_frame);
	if (!p_frame->ovl_v) {
		G2D_WARN("ovl v submodule setup fail\n");
		goto FREE;
	}

	p_frame->ovl_u = g2d_ovl_u_submodule_setup(p_frame);
	if (!p_frame->ovl_u) {
		G2D_WARN("ovl u submodule setup fail\n");
		goto FREE;
	}

	p_frame->bld = g2d_bld_submodule_setup(p_frame);
	if (!p_frame->bld) {
		G2D_WARN("bld submodule setup fail\n");
		goto FREE;
	}

	p_frame->scal = g2d_scaler_submodule_setup(p_frame);
	if (!p_frame->scal) {
		G2D_WARN("scaler submodule setup fail\n");
		goto FREE;
	}

	return 0;
FREE:
	kfree(p_frame->wb);
	kfree(p_frame->ovl_v);
	kfree(p_frame->ovl_u);
	kfree(p_frame->bld);
	kfree(p_frame->scal);
	return -1;
}
#endif

static int g2d_frame_setup(struct g2d_frame *p_frame, unsigned int index, enum frame_type type)
{
	int ret = -1;

	if (!p_frame) {
		G2D_WARN("Null pointer\n");
		return ret;
	}

	p_frame->apply = g2d_frame_apply;
	p_frame->frame_id = index;
	p_frame->destory = g2d_frame_destroy;
	p_frame->frame_get_reg_block_num = g2d_frame_get_reg_block_num;
	p_frame->frame_mem_setup = g2d_frame_mem_setup;
	p_frame->frame_get_rcq_mem_size = g2d_get_frame_rcq_mem_size;
	p_frame->type = type;

#if IS_ENABLED(CONFIG_G2D_MIXER)
	ret = core_frame_submodule_setup(p_frame);
#endif
#if IS_ENABLED(CONFIG_G2D_MIXER)
	if (p_frame->type == MIXER_FRAME)
		ret = mixer_frame_submodule_setup(p_frame);
#endif
#if IS_ENABLED(CONFIG_G2D_ROTATE)
	if (p_frame->type == ROTATE_FRAME)
		ret = rot_frame_submodule_setup(p_frame);
#endif

	return ret;
}

#if defined(G2D_MIXER_RCQ_USED) && defined(G2D_ROT_AHB_USED)
static int g2d_update_regs_by_ahb(struct g2d_task *p_task)
{
	int ret = -1;
#if IS_ENABLED(CONFIG_G2D200)
	if (p_task->frame[0].type == ROTATE_FRAME) {
		g2d_threadn_set_task_end_irq_en(p_task->thread_id, 1);
		core_ctl_set_rotate_en(p_task->frame[0]->core, 1);
	} else if (p_task->frame[0].type == MIXER_FRAME) {
		g2d_threadn_set_task_end_irq_en(p_task->thread_id, 1);
		core_ctl_set_mixer_en(p_task->frame[0]->core, 1);
	}
	g2d_thread_start();
#else
	if (p_task->frame[0].type == ROTATE_FRAME) {
		g2d_rot_irq_en();
		g2d_rot_start();
	} else if (p_task->frame[0].type == MIXER_FRAME) {
		g2d_mixer_irq_en(1);
		g2d_mixer_start(1);
	}
#endif
	return ret;
}

static int g2d_update_regs_by_rcq(struct g2d_task *p_task)
{
	int ret = -1;
#if IS_ENABLED(CONFIG_ARM64)
	g2d_top_set_rcq_head((__u64)p_task->p_rcq_info->phy_addr,
			     p_task->p_rcq_info->rcq_header_len);
#else
	g2d_top_set_rcq_head((__u64)(__u32)p_task->p_rcq_info->phy_addr,
			     p_task->p_rcq_info->rcq_header_len);
#endif
	g2d_top_rcq_irq_en(1);
	g2d_top_rcq_update_en(1);
	return ret;
}
#endif

#if IS_ENABLED(CONFIG_G2D200)
static int g2d_update_regs_by_hyper_thread_rcq(struct g2d_task *p_task)
{
	int ret = -1;
	g2d_thread_enable_master_mode(1);
	g2d_threadn_set_rcq_accept_irq_en(p_task->thread_id, 0);
	g2d_threadn_set_rcq_task_end_irq_gen(p_task->thread_id, 1);
#if IS_ENABLED(CONFIG_ARM64)
	g2d_threadn_set_rcq_head(p_task->thread_id, (__u64)p_task->p_rcq_info->phy_addr,
			     p_task->p_rcq_info->rcq_header_len);
#else
	g2d_threadn_set_rcq_head(p_task->thread_id, (__u32)p_task->p_rcq_info->phy_addr,
			     p_task->p_rcq_info->rcq_header_len);
#endif
	g2d_threadn_set_rcq_task_cmd_num(p_task->thread_id, p_task->frame_cnt);
	g2d_threadn_set_task_end_irq_en(p_task->thread_id, 1);
	g2d_threadn_set_rcq_update_en(p_task->thread_id, 1);
	return ret;
}
#endif

static __s32 g2d_task_apply(struct g2d_task *p_task)
{
	__s32 ret = -1;

#if IS_ENABLED(CONFIG_G2D200)
	g2d_update_regs_by_hyper_thread_rcq(p_task);
#elif IS_ENABLED(G2D_MIXER_RCQ_USED) && !IS_ENABLED(G2D_ROT_RCQ_USED)
	if (p_task->frame[0].type == ROTATE_FRAME) {
		g2d_update_regs_by_ahb(p_task);
	} else if (p_task->frame[0].type == MIXER_FRAME) {
		g2d_update_regs_by_rcq(p_task);
	}
#else
		g2d_update_regs_by_ahb(p_task);
#endif
	ret = g2d_wait_cmd_finish(WAIT_CMD_TIME_MS * (p_task->frame_cnt));

	return ret;
}

static __s32 g2d_task_destroy(struct g2d_task *p_task)
{
	int i;

	for (i = 0; i < p_task->frame_cnt; ++i) {
		if (p_task->frame[i].destory(&p_task->frame[i]))
			G2D_WARN("Frame:%d destory fail\n", i);
	}
	g2d_top_mem_pool_free(p_task->p_rcq_info);

	if (p_task->p_rcq_info) {
		kfree(p_task->p_rcq_info->reg_blk);
		kfree(p_task->p_rcq_info);
	}

	list_del(&p_task->list);
	ida_simple_remove(&g2d_task_ida, p_task->task_id);
	kfree(p_task->frame);
	kfree(p_task);

	return 0;
}

/*
 * @name       :g2d_task_create
 * @brief      :create mixer task instance include memory allocate
 * @param[IN]  :p_g2d_info:pointer of hardware resource
 * @param[IN]  :p_para:mixer task parameter
 * @param[IN]  :frame_len:number of frame
 * @return     :task_id >= 1, else fail
 */
__u32 g2d_task_create(__g2d_info_t *p_g2d_info, struct mixer_para *p_para,
			 unsigned int frame_len)
{
	__s32 ret = -1, i = 0;
	struct g2d_task *task = NULL;

	if (!p_g2d_info || !frame_len) {
		G2D_WARN("Null pointer\n");
		goto OUT;
	}

	task = kzalloc(sizeof(*task), GFP_KERNEL | __GFP_ZERO);
	if (!task) {
		G2D_WARN("kmalloc g2d_task fail\n");
		goto OUT;
	}

	task->frame_cnt = frame_len;
	task->frame = kmalloc_array(frame_len, sizeof(*(task->frame)),
				    GFP_KERNEL | __GFP_ZERO);

	task->p_rcq_info =
	    kmalloc(sizeof(*(task->p_rcq_info)), GFP_KERNEL | __GFP_ZERO);

	if (!task->frame || !task->p_rcq_info) {
		G2D_WARN("Kmalloc fail\n");
		goto FREE;
	}

	task->destory = g2d_task_destroy;
	task->apply = g2d_task_apply;
	task->mem_setup = g2d_task_mem_setup;
	task->p_g2d_info = p_g2d_info;
	task->task_id = ida_simple_get(&g2d_task_ida, 1, 0, GFP_KERNEL);
	if (task->task_id < 0) {
		goto FREE;
	}

	for (i = 0; i < frame_len; ++i) {
		enum frame_type type;
		if (p_para[i].op_flag & OP_ROTATE) {
			type = ROTATE_FRAME;
		} else {
			type = MIXER_FRAME;
		}
		if (g2d_frame_setup(&task->frame[i], i, type)) {
			G2D_WARN("frame %d g2d_frame_setup fail\n", i);
			goto IDA_REMOVE;
		}
		task->frame[i].g2d_base = (u8 __iomem *)p_g2d_info->io;
	}

	if (task->mem_setup(task, p_para)) {
		task->destory(task);
		goto OUT;
	}

	list_add_tail(&task->list, &g2d_task_list);

	for (i = 0; i < frame_len; ++i) {
		ret = task->frame[i].apply(&task->frame[i],
						 &(p_para[i]));
		if (ret)
			G2D_WARN("rcq_info %d apply failed\n", i);
	}

	return task->task_id;
IDA_REMOVE:
	ida_simple_remove(&g2d_task_ida, task->task_id);
FREE:
	kfree(task->frame);
	kfree(task->p_rcq_info);
	kfree(task);
OUT:
	return 0;
}

/*
 * @name       :g2d_task_get_by_id
 * @brief      :get task instance of specified task id
 * @param[IN]  :id: task id
 * @return     :pointer of mixer task or NULL if fail
 */
struct g2d_task *g2d_task_get_by_id(__u32 id)
{
	struct g2d_task *p_task = NULL;

	list_for_each_entry(p_task, &g2d_task_list, list) {
		if (p_task->task_id == id)
			return p_task;
	}
	return NULL;
}

__s32 g2d_task_process(__g2d_info_t *p_g2d_info, struct mixer_para *p_para,
			 unsigned int frame_len)
{
	__s32 ret = -1;
	struct g2d_task *p_task = NULL;
	__u32 id = 0;

	id = g2d_task_create(p_g2d_info, p_para, frame_len);
	p_task = g2d_task_get_by_id(id);
	if (!p_task)
		goto OUT;

	ret = p_task->apply(p_task);

	p_task->destory(p_task);

OUT:
	return ret;
}

__s32 g2d_lbc_rot_set_para(__g2d_info_t *p_g2d_info, g2d_lbc_rot *para)
{
	int ret;
	struct mixer_para *p_mixer_para = (struct mixer_para *)
		kzalloc(sizeof(struct mixer_para), GFP_KERNEL);
	/* mixer module */
	memcpy(&(p_mixer_para->dst_image_h),
		   &(para->blt.dst_image_h), sizeof(g2d_image_enh));
	memcpy(&(p_mixer_para->src_image_h),
		   &(para->blt.src_image_h), sizeof(g2d_image_enh));
	p_mixer_para->lbc_para.is_lbc = true;
	p_mixer_para->lbc_para.lbc_cmp_ratio = para->lbc_cmp_ratio;
	p_mixer_para->lbc_para.enc_is_lossy = para->enc_is_lossy;
	p_mixer_para->lbc_para.dec_is_lossy = para->dec_is_lossy;
	p_mixer_para->flag_h = para->blt.flag_h;
	p_mixer_para->op_flag = OP_ROTATE;
	ret = g2d_task_process(p_g2d_info, p_mixer_para, 1);
	kfree(p_mixer_para);
	return ret;
}

__s32 g2d_rotate_set_para(__g2d_info_t *p_g2d_info, g2d_image_enh *src,
						  g2d_image_enh *dst, __u32 flag)
{
	int ret;
	struct mixer_para *p_mixer_para = (struct mixer_para *)
		kzalloc(sizeof(struct mixer_para), GFP_KERNEL);
	/* mixer module */
	memcpy(&(p_mixer_para->dst_image_h),
		   dst, sizeof(g2d_image_enh));
	memcpy(&(p_mixer_para->src_image_h),
		   src, sizeof(g2d_image_enh));
	p_mixer_para->flag_h = flag;
	p_mixer_para->op_flag = OP_ROTATE;
	ret = g2d_task_process(p_g2d_info, p_mixer_para, 1);
	kfree(p_mixer_para);
	return ret;
}
