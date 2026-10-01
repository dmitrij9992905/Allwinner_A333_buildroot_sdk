#define _POSIX_C_SOURCE 200809L
#include "panel_g2d.h"
#include "a333_g2d_uapi.h"

#include <errno.h>
#include <fcntl.h>
#include <linux/dma-buf.h>
#include <linux/dma-heap.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

struct panel_g2d {
    int fd, framebuffer_dma_fd, source_fd;
    void *source;
    size_t source_size;
    uint32_t source_width, source_height;
    struct fb_var_screeninfo variable;
    uint32_t destination_stride_pixels;
    uint32_t format;
};

static int fail(char *reason, size_t size, const char *operation, int code)
{
    if (reason && size)
        (void)snprintf(reason, size, "%s: %s", operation, strerror(code));
    errno = code;
    return -1;
}

static int sync_buffer(int fd, uint64_t flags)
{
    struct dma_buf_sync sync = { .flags = flags };
    int result;
    do {
        result = ioctl(fd, DMA_BUF_IOCTL_SYNC, &sync);
    } while (result < 0 && errno == EINTR);
    return result;
}

static int allocate_source(size_t length)
{
    /* Linux's default CMA heap is named after the CMA area. Prefer physically
     * contiguous CMA; system buffers are mapped through the A333 IOMMU.
     * Only known non-secure, CPU-mappable heaps are tried.
     */
    static const char *const heaps[] = {
        "/dev/dma_heap/reserved", "/dev/dma_heap/linux,cma",
        "/dev/dma_heap/default_cma_region", "/dev/dma_heap/system"
    };
    int last_error = ENOENT;
    size_t i;
    for (i = 0; i < sizeof(heaps) / sizeof(heaps[0]); ++i) {
        struct dma_heap_allocation_data allocation = {
            .len = length, .fd_flags = O_RDWR | O_CLOEXEC
        };
        int heap = open(heaps[i], O_RDWR | O_CLOEXEC);
        if (heap < 0) {
            if (errno != ENOENT) last_error = errno;
            continue;
        }
        int result = ioctl(heap, DMA_HEAP_IOCTL_ALLOC, &allocation);
        int saved_error = errno;
        (void)close(heap);
        if (result == 0)
            return (int)allocation.fd;
        last_error = saved_error;
    }
    errno = last_error;
    return -1;
}

static void release_source(panel_g2d_t *g2d)
{
    if (g2d->source != MAP_FAILED)
        (void)munmap(g2d->source, g2d->source_size);
    if (g2d->source_fd >= 0)
        (void)close(g2d->source_fd);
    g2d->source = MAP_FAILED;
    g2d->source_fd = -1;
    g2d->source_size = 0;
    g2d->source_width = g2d->source_height = 0;
}

void panel_g2d_close(panel_g2d_t *g2d)
{
    if (!g2d) return;
    release_source(g2d);
    if (g2d->framebuffer_dma_fd >= 0) (void)close(g2d->framebuffer_dma_fd);
    if (g2d->fd >= 0) (void)close(g2d->fd);
    free(g2d);
}

panel_g2d_t *panel_g2d_open(int framebuffer_fd,
                           const struct fb_fix_screeninfo *fixed,
                           const struct fb_var_screeninfo *variable,
                           char *reason, size_t reason_size)
{
    struct a333_fb_dmabuf_export export = { .fd = -1 };
    panel_g2d_t *g2d;
    /* RCQ's rotate engine preserves format and cannot scale. Be conservative
     * about fb layouts; unsupported formats remain on the existing CPU path.
     */
    if (!fixed || !variable || framebuffer_fd < 0 ||
        fixed->type != FB_TYPE_PACKED_PIXELS || fixed->visual != FB_VISUAL_TRUECOLOR ||
        variable->bits_per_pixel != 32 || variable->red.length != 8 ||
        variable->green.length != 8 || variable->blue.length != 8 ||
        variable->green.offset != 8 || variable->red.msb_right ||
        variable->green.msb_right || variable->blue.msb_right ||
        variable->transp.msb_right ||
        (variable->transp.length != 0 &&
         (variable->transp.length != 8 || variable->transp.offset != 24)) ||
        !((variable->red.offset == 16 && variable->blue.offset == 0) ||
          (variable->red.offset == 0 && variable->blue.offset == 16)) ||
        fixed->line_length == 0 || fixed->line_length % 8 != 0 ||
        fixed->line_length / 4 > 8192 || variable->nonstd || variable->grayscale ||
        variable->xres == 0 || variable->yres == 0 ||
        variable->xres > 8192 || variable->yres > 8192 ||
        (uint64_t)variable->xoffset + variable->xres > fixed->line_length / 4 ||
        (uint64_t)fixed->line_length * (variable->yoffset + (uint64_t)variable->yres) >
            fixed->smem_len ||
        (uint64_t)variable->yoffset + variable->yres > 8192) {
        fail(reason, reason_size, "G2D framebuffer layout unsupported", ENOTSUP);
        return NULL;
    }
    g2d = calloc(1, sizeof(*g2d));
    if (!g2d) {
        fail(reason, reason_size, "G2D allocation", ENOMEM);
        return NULL;
    }
    g2d->fd = g2d->framebuffer_dma_fd = g2d->source_fd = -1;
    g2d->source = MAP_FAILED;
    g2d->variable = *variable;
    g2d->destination_stride_pixels = fixed->line_length / 4;
    g2d->format = variable->red.offset == 16 ? A333_G2D_ARGB8888 : A333_G2D_ABGR8888;
    g2d->fd = open("/dev/g2d", O_RDWR | O_CLOEXEC);
    if (g2d->fd < 0) {
        fail(reason, reason_size, "open /dev/g2d", errno);
        goto error;
    }
    int exported = ioctl(framebuffer_fd, A333_FBIOGET_DMABUF, &export);
    if (exported < 0 || export.fd < 0) {
        int code = exported < 0 ? errno : ENODEV;
        if (export.fd >= 0) (void)close(export.fd);
        fail(reason, reason_size, "export framebuffer DMA-BUF", code);
        goto error;
    }
    g2d->framebuffer_dma_fd = export.fd;
    /* Vendor export flags are output-only. Guarantee close-on-exec ourselves. */
    if (fcntl(export.fd, F_SETFD, FD_CLOEXEC) < 0) {
        fail(reason, reason_size, "framebuffer DMA-BUF close-on-exec", errno);
        goto error;
    }
    off_t size = lseek(export.fd, 0, SEEK_END);
    uint64_t end = (uint64_t)fixed->line_length *
                   (variable->yoffset + (uint64_t)variable->yres);
    if (size < 0 || (uint64_t)size < end) {
        fail(reason, reason_size, "framebuffer DMA-BUF size", size < 0 ? errno : EOVERFLOW);
        goto error;
    }
    if (reason && reason_size) reason[0] = '\0';
    return g2d;
error:
    {
        int saved = errno;
        panel_g2d_close(g2d);
        errno = saved;
        return NULL;
    }
}

int panel_g2d_present(panel_g2d_t *g2d, const panel_canvas_t *canvas,
                      unsigned int rotation, char *reason, size_t reason_size)
{
    struct a333_g2d_blt blit = { 0 };
    uint32_t width, height;
    if (!g2d || !canvas || !canvas->pixels || canvas->width <= 0 ||
        canvas->height <= 0 || canvas->stride < (size_t)canvas->width ||
        (rotation != 0 && rotation != 90 && rotation != 180 && rotation != 270))
        return fail(reason, reason_size, "G2D canvas", EINVAL);
    width = (uint32_t)canvas->width;
    height = (uint32_t)canvas->height;
    if ((rotation == 90 || rotation == 270) ?
        (width != g2d->variable.yres || height != g2d->variable.xres) :
        (width != g2d->variable.xres || height != g2d->variable.yres))
        return fail(reason, reason_size, "G2D rotation requires a 1:1 canvas", ENOTSUP);
    if (g2d->source_fd < 0 || width != g2d->source_width || height != g2d->source_height) {
        release_source(g2d);
        g2d->source_size = (size_t)width * height * sizeof(panel_color_t);
        g2d->source_fd = allocate_source(g2d->source_size);
        if (g2d->source_fd < 0)
            return fail(reason, reason_size, "allocate G2D DMA heap buffer", errno);
        g2d->source = mmap(NULL, g2d->source_size, PROT_READ | PROT_WRITE,
                           MAP_SHARED, g2d->source_fd, 0);
        if (g2d->source == MAP_FAILED) {
            int saved = errno;
            release_source(g2d);
            return fail(reason, reason_size, "map G2D source", saved);
        }
        g2d->source_width = width;
        g2d->source_height = height;
    }
    if (sync_buffer(g2d->source_fd, DMA_BUF_SYNC_START | DMA_BUF_SYNC_WRITE) < 0)
        return fail(reason, reason_size, "G2D CPU-write begin", errno);
    /* Keep the original canvas for reliable software fallback. This is one
     * sequential copy, replacing the full CPU rotate/scale/per-pixel encoding.
     * LVGL's canvas is always opaque. For ABGR swap R/B while copying.
     */
    int opaque = 1;
    for (uint32_t y = 0; y < height; ++y) {
        const uint32_t *source = canvas->pixels + y * canvas->stride;
        uint32_t *destination = (uint32_t *)g2d->source + (size_t)y * width;
        for (uint32_t x = 0; x < width; ++x)
            if ((source[x] >> 24) != 255u) opaque = 0;
        if (g2d->format == A333_G2D_ARGB8888)
            memcpy(destination, source, (size_t)width * 4);
        else
            for (uint32_t x = 0; x < width; ++x) {
                uint32_t color = source[x];
                destination[x] = (color & 0xff00ff00u) |
                                 ((color & 0xffu) << 16) | ((color >> 16) & 0xffu);
            }
    }
    if (sync_buffer(g2d->source_fd, DMA_BUF_SYNC_END | DMA_BUF_SYNC_WRITE) < 0)
        return fail(reason, reason_size, "G2D CPU-write end", errno);
    if (!opaque)
        return fail(reason, reason_size, "G2D requires an opaque canvas", ENOTSUP);
    /* Even zero rotation uses ROT_0 (0x400), not NONE: NONE selects the
     * mixer's different path, with different alpha/resize semantics.
     */
    blit.flag_h = rotation == 90 ? A333_G2D_ROT_90 : rotation == 180 ? A333_G2D_ROT_180 :
                  rotation == 270 ? A333_G2D_ROT_270 : A333_G2D_ROT_0;
    blit.src_image_h.format = blit.dst_image_h.format = g2d->format;
    /* RCQ g2d_set_info synthesizes unused U/V addresses for RGB too, then
     * validates their four-byte alignment. Its y_size is aligned PIXEL
     * width * height, not the RGB byte pitch. Align all planes to 4 so odd
     * dimensions cannot silently skip the job with a successful return.
     * For 32-bit RGB this leaves the real byte pitch (4 * width) unchanged.
     */
    for (unsigned int plane = 0; plane < 3; ++plane)
        blit.src_image_h.align[plane] = blit.dst_image_h.align[plane] = 4;
    blit.src_image_h.width = width;
    blit.src_image_h.height = height;
    blit.src_image_h.clip_rect.w = width;
    blit.src_image_h.clip_rect.h = height;
    blit.src_image_h.fd = g2d->source_fd;
    blit.dst_image_h.width = g2d->destination_stride_pixels;
    blit.dst_image_h.height = g2d->variable.yoffset + g2d->variable.yres;
    blit.dst_image_h.clip_rect.x = (int32_t)g2d->variable.xoffset;
    blit.dst_image_h.clip_rect.y = (int32_t)g2d->variable.yoffset;
    blit.dst_image_h.clip_rect.w = g2d->variable.xres;
    blit.dst_image_h.clip_rect.h = g2d->variable.yres;
    blit.dst_image_h.fd = g2d->framebuffer_dma_fd;
    blit.src_image_h.alpha = blit.dst_image_h.alpha = 255;
    blit.src_image_h.mode = blit.dst_image_h.mode = A333_G2D_GLOBAL_ALPHA;
    /* G2D RCQ waits for its completion IRQ before returning from BITBLT_H.
     * Don't retry EINTR here: the hardware may already have run the job.
     */
    if (ioctl(g2d->fd, A333_G2D_BITBLT_H, &blit) < 0)
        return fail(reason, reason_size, "G2D synchronous rotation", errno);
    if (reason && reason_size) reason[0] = '\0';
    return 0;
}
