// SPDX-License-Identifier: GPL-2.0
/* Copyright(c) 2024 Allwinner Technology Co.,Ltd. All rights reserved. */
#ifndef __SUNXI_DRM_HEAP_H
#define __SUNXI_DRM_HEAP_H

#include <sunxi-smc.h>
#include <uapi/security/sunxi-drm-heap.h>

/* A chunk of DRM mem */
struct sunxi_drm_mem {
	size_t size;	 	/* Size in bytes */
	phys_addr_t paddr;	/* Physical addr */
	unsigned long xaddr;	/* Virtual addr in TA's view */
};

struct sunxi_drm_mem *sunxi_drm_mem_alloc(size_t size);
void sunxi_drm_mem_free(const struct sunxi_drm_mem *mem);

void sunxi_drm_info_get(struct sunxi_drm_info *info);
int  sunxi_drm_copy_from_unsafe(phys_addr_t dst, phys_addr_t src, size_t size);

/*
 * DRM Master Types.
 * This is used as the first arg (@master_types) of
 * sunxi_drm_master_enable_by_type()/sunxi_drm_master_disable_by_type().
 * NOTICE: Keep in sync with '{optee_os-3.7}/lib/libutee/include/tee_sunxi_smc_defs.h'
*/
#define DRM_MASTER_TYPE_NONE		(0x00)
#define DRM_MASTER_TYPE_DE		BIT(0)
#define DRM_MASTER_TYPE_VE_DEC		BIT(1)
#define DRM_MASTER_TYPE_VE_ENC		BIT(2)
#define DRM_MASTER_TYPE_VE		DRM_MASTER_TYPE_VE_DEC  /* Only Decoders accesses DRM mem */
#define DRM_MASTER_TYPE_G2D		BIT(3)
#define DRM_MASTER_TYPE_GPU		BIT(4)
#define DRM_MASTER_TYPE_NPU		BIT(5)
#define DRM_MASTER_TYPE_EINK		BIT(6)
#define DRM_MASTER_TYPE_ISP		BIT(7)
#define DRM_MASTER_TYPE_CSI		BIT(8)
#define DRM_MASTER_TYPE_DI		BIT(9)
#define DRM_MASTER_TYPE_CE		BIT(10)

/*
 * Multiple master_type is supported.
 * e.g: master_types = DRM_MASTER_TYPE_DE | DRM_MASTER_TYPE_VE;
 */
int sunxi_drm_master_enable_by_type(uint32_t master_types);
int sunxi_drm_master_disable_by_type(uint32_t master_types);

int sunxi_drm_query(bool *is_drm_enabled, uint32_t *enabled_master_types, uint32_t *avail_master_types);

#endif  /* __SUNXI_DRM_HEAP_H */
