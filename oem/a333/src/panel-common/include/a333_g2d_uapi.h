/* SPDX-License-Identifier: GPL-2.0-or-later WITH Linux-syscall-note */
/* Copyright(c) 2020 - 2023 Allwinner Technology Co.,Ltd. All rights reserved. */
#ifndef A333_G2D_UAPI_H
#define A333_G2D_UAPI_H

/* ABI subset copied from this SDK's BSP include/uapi/linux/sunxi-g2d.h.
 * Keep resize/coor fields: older online sunxi-g2d headers omit them and
 * silently change the ioctl layout. No external SDK headers are required.
 * Fixed-width enum storage matches the vendor kernel's 32-bit C enums.
 */
#include <stddef.h>
#include <stdint.h>
#include <sys/ioctl.h>

enum {
    A333_G2D_ARGB8888 = 0,
    A333_G2D_ABGR8888 = 1,
    A333_G2D_ROT_90 = 0x100,
    A333_G2D_ROT_180 = 0x200,
    A333_G2D_ROT_270 = 0x300,
    A333_G2D_ROT_0 = 0x400,
    A333_G2D_GLOBAL_ALPHA = 1,
    A333_G2D_BITBLT_H = 0x55
};

struct a333_g2d_rect {
    int32_t x, y;
    uint32_t w, h;
};

struct a333_g2d_image {
    int32_t bbuff;
    uint32_t color, format;
    uint32_t laddr[3], haddr[3];
    uint32_t width, height, align[3];
    struct a333_g2d_rect clip_rect;
    struct { uint32_t w, h; } resize;
    struct { uint32_t x, y; } coor;
    uint32_t gamut;
    int32_t bpremul;
    uint8_t alpha;
    uint32_t mode;
    int32_t fd;
    uint32_t use_phy_addr, color_range;
};

struct a333_g2d_blt {
    uint32_t flag_h;
    struct a333_g2d_image src_image_h, dst_image_h;
};

struct a333_fb_dmabuf_export { int32_t fd; uint32_t flags; };
#define A333_FBIOGET_DMABUF _IOR('F', 0x21, struct a333_fb_dmabuf_export)

_Static_assert(sizeof(struct a333_g2d_image) == 116, "G2D image ABI");
_Static_assert(offsetof(struct a333_g2d_image, fd) == 104, "G2D fd ABI");
_Static_assert(sizeof(struct a333_g2d_blt) == 236, "G2D blit ABI");
#endif
