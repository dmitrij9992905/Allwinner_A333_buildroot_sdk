/* Hardware-free adapter contract tests. Unknown syscalls fail closed: these
 * wrappers never forward a device operation to the real host kernel. */
#define _POSIX_C_SOURCE 200809L
#include <stdbool.h>
#include <stdint.h>
#include <linux/sunxi-g2d.h>
#include "a333_g2d_uapi.h"
#include "panel_fbdev.h"
#include "panel_g2d.h"

#include <errno.h>
#include <fcntl.h>
#include <linux/dma-buf.h>
#include <linux/dma-heap.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#define SAME_SIZE(local, bsp) \
    _Static_assert(sizeof(local) == sizeof(bsp), "ABI size: " #local)
#define SAME_FIELD(local, bsp, field) \
    _Static_assert(offsetof(local, field) == offsetof(bsp, field), "ABI offset: " #field); \
    _Static_assert(sizeof(((local *)0)->field) == sizeof(((bsp *)0)->field), \
                   "ABI field size: " #field)
#define IMAGE_FIELD(field) SAME_FIELD(struct a333_g2d_image, g2d_image_enh, field)
SAME_SIZE(struct a333_g2d_rect, g2d_rect);
SAME_SIZE(struct a333_g2d_image, g2d_image_enh);
SAME_SIZE(struct a333_g2d_blt, g2d_blt_h);
SAME_FIELD(struct a333_g2d_rect, g2d_rect, x);
SAME_FIELD(struct a333_g2d_rect, g2d_rect, y);
SAME_FIELD(struct a333_g2d_rect, g2d_rect, w);
SAME_FIELD(struct a333_g2d_rect, g2d_rect, h);
IMAGE_FIELD(bbuff);
IMAGE_FIELD(color);
IMAGE_FIELD(format);
IMAGE_FIELD(laddr);
IMAGE_FIELD(haddr);
IMAGE_FIELD(width);
IMAGE_FIELD(height);
IMAGE_FIELD(align);
IMAGE_FIELD(clip_rect);
IMAGE_FIELD(resize);
IMAGE_FIELD(resize.w);
IMAGE_FIELD(resize.h);
IMAGE_FIELD(coor);
IMAGE_FIELD(coor.x);
IMAGE_FIELD(coor.y);
IMAGE_FIELD(gamut);
IMAGE_FIELD(bpremul);
IMAGE_FIELD(alpha);
IMAGE_FIELD(mode);
IMAGE_FIELD(fd);
IMAGE_FIELD(use_phy_addr);
IMAGE_FIELD(color_range);
SAME_FIELD(struct a333_g2d_blt, g2d_blt_h, flag_h);
SAME_FIELD(struct a333_g2d_blt, g2d_blt_h, src_image_h);
SAME_FIELD(struct a333_g2d_blt, g2d_blt_h, dst_image_h);
#define SAME_CONSTANT(local, bsp) \
    _Static_assert((unsigned)(local) == (unsigned)(bsp), "ABI constant: " #local)
SAME_CONSTANT(A333_G2D_ARGB8888, G2D_FORMAT_ARGB8888);
SAME_CONSTANT(A333_G2D_ABGR8888, G2D_FORMAT_ABGR8888);
SAME_CONSTANT(A333_G2D_ROT_0, G2D_ROT_0);
SAME_CONSTANT(A333_G2D_ROT_90, G2D_ROT_90);
SAME_CONSTANT(A333_G2D_ROT_180, G2D_ROT_180);
SAME_CONSTANT(A333_G2D_ROT_270, G2D_ROT_270);
SAME_CONSTANT(A333_G2D_GLOBAL_ALPHA, G2D_GLOBAL_ALPHA);
SAME_CONSTANT(A333_G2D_BITBLT_H, G2D_CMD_BITBLT_H);

#define CHECK(condition) do { \
    if (!(condition)) { \
        fprintf(stderr, "%s:%d: %s (errno=%d)\n", __FILE__, __LINE__, #condition, errno); \
        exit(1); \
    } \
} while (0)

enum kind { UNUSED, ENGINE, HEAP, SOURCE, EXPORT };
enum { FB_FD = 10, FIRST_FD = 100, MAX_FD = 512, MEMORY_SIZE = 2048, GUARD = 16 };
struct descriptor {
    enum kind kind;
    unsigned char *memory, *snapshot;
    size_t size;
    bool mapped, writing, ready, cloexec;
};
static struct descriptor descriptors[MAX_FD];
static int next_fd = FIRST_FD;
static unsigned char framebuffer[MEMORY_SIZE];
static struct fb_fix_screeninfo fixed;
static struct fb_var_screeninfo variable;
static const char *fault = "none";
static bool fb_open, fb_mapped;
static unsigned opens, exports, allocations, heap_opens, allocation_calls;
static unsigned blits, sync_starts, sync_ends;

static bool is_fault(const char *name) { return strcmp(fault, name) == 0; }
static int error(int code) { errno = code; return -1; }

static int descriptor_new(enum kind kind)
{
    CHECK(next_fd < MAX_FD);
    descriptors[next_fd].kind = kind;
    return next_fd++;
}

static struct descriptor *descriptor_get(int fd, enum kind kind)
{
    CHECK(fd >= FIRST_FD && fd < next_fd);
    CHECK(descriptors[fd].kind == kind);
    return &descriptors[fd];
}

static void check_guard(const struct descriptor *d)
{
    for (size_t i = 0; i < GUARD; ++i) {
        CHECK(d->memory[i] == 0xa5);
        CHECK(d->memory[GUARD + d->size + i] == 0xa5);
    }
}

static void check_clean(void)
{
    for (int fd = FIRST_FD; fd < next_fd; ++fd)
        CHECK(descriptors[fd].kind == UNUSED);
    CHECK(!fb_open && !fb_mapped);
}

static void initialize(const char *layout)
{
    memset(&fixed, 0, sizeof(fixed));
    memset(&variable, 0, sizeof(variable));
    memset(framebuffer, 0x5a, sizeof(framebuffer));
    fixed.type = FB_TYPE_PACKED_PIXELS;
    fixed.visual = FB_VISUAL_TRUECOLOR;
    fixed.line_length = 8 * 4;
    fixed.smem_len = 6 * fixed.line_length;
    variable.xres = 3;
    variable.yres = 2;
    variable.xres_virtual = 8;
    variable.yres_virtual = 6;
    variable.xoffset = 2;
    variable.yoffset = 1;
    variable.bits_per_pixel = 32;
    variable.red.length = variable.green.length = variable.blue.length = 8;
    variable.green.offset = 8;
    bool abgr = strcmp(layout, "abgr") == 0 || strcmp(layout, "xbgr") == 0;
    variable.red.offset = abgr ? 0 : 16;
    variable.blue.offset = abgr ? 16 : 0;
    variable.transp.length = layout[0] == 'x' ? 0 : 8;
    variable.transp.offset = 24;
}

int __wrap_open(const char *path, int flags, ...)
{
    CHECK(flags == (O_RDWR | O_CLOEXEC));
    if (strcmp(path, "/dev/fb0") == 0) {
        CHECK(!fb_open);
        fb_open = true;
        return FB_FD;
    }
    if (strcmp(path, "/dev/g2d") == 0) {
        ++opens;
        if (is_fault("g2d-open")) return error(ENOENT);
        return descriptor_new(ENGINE);
    }
    static const char *const heaps[] = {
        "/dev/dma_heap/reserved", "/dev/dma_heap/linux,cma",
        "/dev/dma_heap/default_cma_region", "/dev/dma_heap/system"
    };
    bool known = false;
    for (size_t i = 0; i < sizeof(heaps) / sizeof(heaps[0]); ++i)
        if (strcmp(path, heaps[i]) == 0) known = true;
    CHECK(known);
    ++heap_opens;
    if (is_fault("heap-open") || (is_fault("heap-missing-first") && heap_opens == 1))
        return error(ENOENT);
    return descriptor_new(HEAP);
}

int __wrap_close(int fd)
{
    if (fd == FB_FD) {
        CHECK(fb_open && !fb_mapped);
        fb_open = false;
        return 0;
    }
    CHECK(fd >= FIRST_FD && fd < next_fd);
    struct descriptor *d = &descriptors[fd];
    CHECK(d->kind != UNUSED && !d->mapped);
    if (d->kind == SOURCE) {
        check_guard(d);
        free(d->memory);
        free(d->snapshot);
    }
    memset(d, 0, sizeof(*d));
    errno = EBADF; /* Successful cleanup must not overwrite the reported error. */
    return 0;
}

int __wrap_fcntl(int fd, int command, ...)
{
    struct descriptor *d = descriptor_get(fd, EXPORT);
    CHECK(command == F_SETFD);
    va_list args;
    va_start(args, command);
    int flags = va_arg(args, int);
    va_end(args);
    CHECK(flags == FD_CLOEXEC);
    if (is_fault("fcntl")) return error(EIO);
    d->cloexec = true;
    return 0;
}

off_t __wrap_lseek(int fd, off_t offset, int whence)
{
    struct descriptor *d = descriptor_get(fd, EXPORT);
    CHECK(offset == 0 && whence == SEEK_END && d->cloexec);
    if (is_fault("lseek")) return error(ESPIPE);
    if (is_fault("short-export"))
        return (off_t)(fixed.line_length * (variable.yoffset + variable.yres) - 1);
    return (off_t)fixed.smem_len;
}

void *__wrap_mmap(void *address, size_t length, int prot, int flags, int fd, off_t offset)
{
    CHECK(!address && offset == 0 && prot == (PROT_READ | PROT_WRITE) && flags == MAP_SHARED);
    if (fd == FB_FD) {
        CHECK(fb_open && !fb_mapped && length == fixed.smem_len);
        if (is_fault("fb-mmap")) { errno = ENOMEM; return MAP_FAILED; }
        fb_mapped = true;
        return framebuffer;
    }
    struct descriptor *d = descriptor_get(fd, SOURCE);
    CHECK(length == d->size && !d->mapped);
    if (is_fault("source-mmap")) { errno = ENOMEM; return MAP_FAILED; }
    d->mapped = true;
    return d->memory + GUARD;
}

int __wrap_munmap(void *address, size_t length)
{
    if (address == framebuffer) {
        CHECK(fb_mapped && length == fixed.smem_len);
        fb_mapped = false;
        return 0;
    }
    for (int fd = FIRST_FD; fd < next_fd; ++fd) {
        struct descriptor *d = &descriptors[fd];
        if (d->kind == SOURCE && address == d->memory + GUARD) {
            CHECK(d->mapped && length == d->size);
            check_guard(d);
            d->mapped = false;
            return 0;
        }
    }
    CHECK(false);
    return -1;
}

static size_t round_pitch(size_t width, uint32_t alignment)
{
    if (!alignment) return width; /* BSP G2DALIGN/cal_align zero means no rounding. */
    CHECK((alignment & (alignment - 1)) == 0);
    return (width + alignment - 1) & ~(size_t)(alignment - 1);
}

static size_t validate_image(const struct a333_g2d_image *image, const struct descriptor *d)
{
    CHECK(!image->use_phy_addr && !image->bbuff && !image->bpremul);
    CHECK(image->format == A333_G2D_ARGB8888 || image->format == A333_G2D_ABGR8888);
    CHECK(image->alpha == 255 && image->mode == A333_G2D_GLOBAL_ALPHA);
    CHECK(image->resize.w == 0 && image->resize.h == 0);
    CHECK(image->coor.x == 0 && image->coor.y == 0);
    CHECK(image->clip_rect.x >= 0 && image->clip_rect.y >= 0);
    CHECK((uint64_t)(uint32_t)image->clip_rect.x + image->clip_rect.w <= image->width);
    CHECK((uint64_t)(uint32_t)image->clip_rect.y + image->clip_rect.h <= image->height);
    /* Match RCQ g2d_set_info for RGB: synthesized U/V offsets use aligned
     * PIXEL widths, not RGB byte pitches. These unused addresses are still
     * checked by g2d_rotate. A 3x2 source with zero align produces +6/+12,
     * so checking only the actual 12-byte RGB pitch would miss the bug. */
    uint64_t base = 0x1000u + (uint64_t)(unsigned)image->fd * 0x1000u;
    size_t ysize = round_pitch(image->width, image->align[0]) * image->height;
    size_t usize = round_pitch(image->width, image->align[1]) * image->height;
    size_t pitches[3] = { round_pitch((size_t)image->width * 4, image->align[0]),
                         round_pitch(0, image->align[1]), round_pitch(0, image->align[2]) };
    uint64_t addresses[3] = {
        base + pitches[0] * (uint32_t)image->clip_rect.y + (uint32_t)image->clip_rect.x * 4u,
        base + ysize, base + ysize + usize
    };
    for (unsigned i = 0; i < 3; ++i) {
        CHECK(addresses[i] % 4 == 0);
        CHECK(pitches[i] % (d->kind == EXPORT ? 8u : 4u) == 0);
    }
    for (unsigned i = 0; i < 3; ++i)
        CHECK(image->laddr[i] == 0 && image->haddr[i] == 0 && image->align[i] == 4);
    CHECK(pitches[0] == (size_t)image->width * 4);
    CHECK(pitches[0] * image->height <= d->size);
    /* No access to synthetic U/V: RGB uses only the real byte-pitch plane. */
    return pitches[0];
}

static int emulate_blit(const struct a333_g2d_blt *blit)
{
    const struct a333_g2d_image *s = &blit->src_image_h, *t = &blit->dst_image_h;
    struct descriptor *source = descriptor_get(s->fd, SOURCE);
    struct descriptor *destination = descriptor_get(t->fd, EXPORT);
    CHECK(source->mapped && source->ready && !source->writing && destination->cloexec);
    CHECK(memcmp(source->memory + GUARD, source->snapshot, source->size) == 0);
    size_t source_pitch = validate_image(s, source);
    size_t destination_pitch = validate_image(t, destination);
    CHECK(destination_pitch == fixed.line_length);
    CHECK(s->format == t->format && s->clip_rect.x == 0 && s->clip_rect.y == 0);
    CHECK(s->format == (variable.red.offset == 16 ? A333_G2D_ARGB8888 : A333_G2D_ABGR8888));
    CHECK(s->width == s->clip_rect.w && s->height == s->clip_rect.h);
    CHECK((uint64_t)s->width * s->height * 4 == source->size);
    CHECK(t->width == fixed.line_length / 4 && t->height == variable.yoffset + variable.yres);
    CHECK(t->clip_rect.x == (int32_t)variable.xoffset && t->clip_rect.y == (int32_t)variable.yoffset);
    CHECK(t->clip_rect.w == variable.xres && t->clip_rect.h == variable.yres);
    bool swapped = blit->flag_h == A333_G2D_ROT_90 || blit->flag_h == A333_G2D_ROT_270;
    CHECK(t->clip_rect.w == (swapped ? s->height : s->width));
    CHECK(t->clip_rect.h == (swapped ? s->width : s->height));
    ++blits;
    if (is_fault("blit")) return error(EIO);
    if (is_fault("blit-eintr")) return error(EINTR);
    /* Forward rotation selected ONLY by the submitted ioctl flag. Expected
     * output below uses an independent inverse-coordinate oracle. */
    for (uint32_t y = 0; y < s->height; ++y) {
        for (uint32_t x = 0; x < s->width; ++x) {
            uint32_t dx = x, dy = y;
            switch (blit->flag_h) {
            case A333_G2D_ROT_0: break;
            case A333_G2D_ROT_90: dx = s->height - 1 - y; dy = x; break;
            case A333_G2D_ROT_180: dx = s->width - 1 - x; dy = s->height - 1 - y; break;
            case A333_G2D_ROT_270: dx = y; dy = s->width - 1 - x; break;
            default: CHECK(false);
            }
            size_t index = (size_t)(dy + (uint32_t)t->clip_rect.y) * destination_pitch +
                           (dx + (uint32_t)t->clip_rect.x) * 4;
            CHECK(index + 4 <= destination->size);
            memcpy(framebuffer + index, source->memory + GUARD + (size_t)y * source_pitch + x * 4, 4);
        }
    }
    check_guard(source);
    return 0; /* Writes complete before the ioctl returns. */
}

int __wrap_ioctl(int fd, unsigned long request, ...)
{
    va_list args;
    va_start(args, request);
    if (request == FBIOBLANK) {
        CHECK(fd == FB_FD && va_arg(args, int) == FB_BLANK_UNBLANK);
        va_end(args);
        return 0;
    }
    void *argument = va_arg(args, void *);
    va_end(args);
    CHECK(argument);
    if (request == FBIOGET_FSCREENINFO || request == FBIOGET_VSCREENINFO) {
        CHECK(fd == FB_FD && fb_open);
        if (request == FBIOGET_FSCREENINFO) memcpy(argument, &fixed, sizeof(fixed));
        else memcpy(argument, &variable, sizeof(variable));
        return 0;
    }
    if (request == A333_FBIOGET_DMABUF) {
        CHECK(fd == FB_FD);
        ++exports;
        if (is_fault("export")) return error(EIO);
        struct a333_fb_dmabuf_export *result = argument;
        CHECK(result->fd == -1);
        if (is_fault("export-fd")) { result->fd = -1; return 0; }
        result->fd = descriptor_new(EXPORT);
        result->flags = 0; /* Vendor flags are output-only. */
        descriptors[result->fd].size = fixed.smem_len;
        if (is_fault("export-error-fd")) return error(EIO);
        return 0;
    }
    if (request == DMA_HEAP_IOCTL_ALLOC) {
        (void)descriptor_get(fd, HEAP);
        struct dma_heap_allocation_data *a = argument;
        CHECK(a->len == (uint64_t)variable.xres * variable.yres * 4 && a->heap_flags == 0);
        CHECK(a->len <= MEMORY_SIZE);
        CHECK(a->fd_flags == (O_RDWR | O_CLOEXEC));
        ++allocation_calls;
        if (is_fault("allocate") || (is_fault("allocate-first") && allocation_calls == 1))
            return error(ENOMEM);
        a->fd = (uint32_t)descriptor_new(SOURCE);
        struct descriptor *d = &descriptors[a->fd];
        d->size = (size_t)a->len;
        d->memory = malloc(d->size + 2 * GUARD);
        d->snapshot = malloc(d->size);
        CHECK(d->memory && d->snapshot);
        memset(d->memory, 0xa5, d->size + 2 * GUARD);
        memset(d->snapshot, 0xa5, d->size);
        ++allocations;
        return 0;
    }
    if (request == DMA_BUF_IOCTL_SYNC) {
        struct descriptor *d = descriptor_get(fd, SOURCE);
        CHECK(d->mapped);
        uint64_t flags = ((struct dma_buf_sync *)argument)->flags;
        if (flags == (DMA_BUF_SYNC_START | DMA_BUF_SYNC_WRITE)) {
            ++sync_starts;
            if (is_fault("sync-start")) return error(EIO);
            if (is_fault("sync-start-eintr") && sync_starts == 1) return error(EINTR);
            CHECK(!d->writing);
            CHECK(memcmp(d->memory + GUARD, d->snapshot, d->size) == 0);
            d->writing = true;
            d->ready = false;
        } else {
            CHECK(flags == (DMA_BUF_SYNC_END | DMA_BUF_SYNC_WRITE) && d->writing);
            ++sync_ends;
            if (is_fault("sync-end")) return error(EIO);
            if (is_fault("sync-end-eintr") && sync_ends == 1) return error(EINTR);
            d->writing = false;
            d->ready = true;
            memcpy(d->snapshot, d->memory + GUARD, d->size);
        }
        return 0;
    }
    CHECK(request == A333_G2D_BITBLT_H);
    (void)descriptor_get(fd, ENGINE);
    return emulate_blit(argument);
}

static panel_g2d_t *open_adapter(void)
{
    char reason[192] = "not cleared";
    panel_g2d_t *g2d = panel_g2d_open(FB_FD, &fixed, &variable, reason, sizeof(reason));
    CHECK(g2d && reason[0] == '\0');
    return g2d;
}

static panel_canvas_t make_canvas(unsigned rotation, uint32_t *pixels)
{
    bool swapped = rotation == 90 || rotation == 270;
    panel_canvas_t c = { .width = swapped ? 2 : 3, .height = swapped ? 3 : 2,
                         .stride = 5, .pixels = pixels };
    /* Transparent row padding must not be copied or counted as visible alpha. */
    for (size_t i = 0; i < 32; ++i) pixels[i] = 0x00123456;
    for (int y = 0; y < c.height; ++y)
        for (int x = 0; x < c.width; ++x)
            pixels[(size_t)y * c.stride + (size_t)x] =
                PANEL_RGB(17 + 31 * x + 3 * y, 7 + 9 * x + 41 * y, 193 - 23 * x - 11 * y);
    return c;
}

static uint32_t encode_expected(uint32_t color, bool hardware)
{
    uint32_t a = color >> 24;
    uint32_t r = (color >> 16) & 255, g = (color >> 8) & 255, b = color & 255;
    if (!hardware) {
        r = (r * a + 127) / 255;
        g = (g * a + 127) / 255;
        b = (b * a + 127) / 255;
    }
    return (variable.transp.length || hardware ? 0xff000000u : 0) |
           (r << variable.red.offset) | (g << variable.green.offset) | (b << variable.blue.offset);
}

static size_t scaled(unsigned position, unsigned extent, int source)
{
    return extent <= 1 || source <= 1 ? 0 : (size_t)position * (size_t)(source - 1) / (extent - 1);
}

static void verify_frame(const panel_canvas_t *c, unsigned rotation, bool hardware)
{
    unsigned char expected[MEMORY_SIZE];
    memset(expected, 0x5a, sizeof(expected));
    for (unsigned y = 0; y < variable.yres; ++y) {
        for (unsigned x = 0; x < variable.xres; ++x) {
            size_t sx, sy;
            switch (rotation) {
            case 0: sx = scaled(x, variable.xres, c->width); sy = scaled(y, variable.yres, c->height); break;
            case 90: sx = scaled(y, variable.yres, c->width); sy = scaled(variable.xres - 1 - x, variable.xres, c->height); break;
            case 180: sx = scaled(variable.xres - 1 - x, variable.xres, c->width); sy = scaled(variable.yres - 1 - y, variable.yres, c->height); break;
            case 270: sx = scaled(variable.yres - 1 - y, variable.yres, c->width); sy = scaled(x, variable.xres, c->height); break;
            default: CHECK(false); return;
            }
            uint32_t value = encode_expected(c->pixels[sy * c->stride + sx], hardware);
            size_t offset = (size_t)(variable.yoffset + y) * fixed.line_length +
                            (size_t)(variable.xoffset + x) * 4;
            memcpy(expected + offset, &value, 4);
        }
    }
    /* Includes left/right row padding, top/bottom rows, and unused memory. */
    CHECK(memcmp(framebuffer, expected, sizeof(framebuffer)) == 0);
}

static void verify_untouched(void)
{
    for (size_t i = 0; i < sizeof(framebuffer); ++i) CHECK(framebuffer[i] == 0x5a);
}

static void geometry(unsigned rotation)
{
    uint32_t pixels[32], original[32];
    panel_canvas_t c = make_canvas(rotation, pixels);
    memcpy(original, pixels, sizeof(pixels));
    panel_g2d_t *g2d = open_adapter();
    char reason[192] = "not cleared";
    CHECK(panel_g2d_present(g2d, &c, rotation, reason, sizeof(reason)) == 0);
    CHECK(reason[0] == '\0' && blits == 1 && allocations == 1);
    CHECK(sync_starts == (is_fault("sync-start-eintr") ? 2u : 1u));
    CHECK(sync_ends == (is_fault("sync-end-eintr") ? 2u : 1u));
    if (is_fault("heap-missing-first") || is_fault("allocate-first")) CHECK(heap_opens == 2);
    CHECK(memcmp(pixels, original, sizeof(pixels)) == 0);
    verify_frame(&c, rotation, true);
    panel_g2d_close(g2d);
    panel_g2d_close(NULL);
    check_clean();
}

static void bad_layout(const char *name)
{
    const struct fb_fix_screeninfo *f = &fixed;
    const struct fb_var_screeninfo *v = &variable;
    int fd = FB_FD;
    if (!strcmp(name, "null-fixed")) f = NULL;
    else if (!strcmp(name, "null-variable")) v = NULL;
    else if (!strcmp(name, "bad-fd")) fd = -1;
    else if (!strcmp(name, "type")) fixed.type = FB_TYPE_PLANES;
    else if (!strcmp(name, "visual")) fixed.visual = FB_VISUAL_DIRECTCOLOR;
    else if (!strcmp(name, "bpp")) variable.bits_per_pixel = 16;
    else if (!strcmp(name, "red-length")) variable.red.length = 5;
    else if (!strcmp(name, "green-length")) variable.green.length = 6;
    else if (!strcmp(name, "blue-length")) variable.blue.length = 5;
    else if (!strcmp(name, "green-offset")) variable.green.offset = 16;
    else if (!strcmp(name, "red-offset")) variable.red.offset = 8;
    else if (!strcmp(name, "blue-offset")) variable.blue.offset = 8;
    else if (!strcmp(name, "red-msb")) variable.red.msb_right = 1;
    else if (!strcmp(name, "green-msb")) variable.green.msb_right = 1;
    else if (!strcmp(name, "blue-msb")) variable.blue.msb_right = 1;
    else if (!strcmp(name, "alpha-msb")) variable.transp.msb_right = 1;
    else if (!strcmp(name, "alpha-length")) variable.transp.length = 4;
    else if (!strcmp(name, "alpha-offset")) variable.transp.offset = 0;
    else if (!strcmp(name, "zero-stride")) fixed.line_length = 0;
    else if (!strcmp(name, "unaligned-stride")) fixed.line_length = 28;
    else if (!strcmp(name, "zero-width")) variable.xres = 0;
    else if (!strcmp(name, "zero-height")) variable.yres = 0;
    else if (!strcmp(name, "wide")) variable.xres = 8193;
    else if (!strcmp(name, "tall")) variable.yres = 8193;
    else if (!strcmp(name, "xoffset")) variable.xoffset = 6;
    else if (!strcmp(name, "yoffset")) variable.yoffset = 8191;
    else if (!strcmp(name, "short-smem")) fixed.smem_len = fixed.line_length * 3 - 1;
    else if (!strcmp(name, "overflow-x")) variable.xoffset = UINT32_MAX;
    else if (!strcmp(name, "overflow-y")) variable.yoffset = UINT32_MAX;
    else if (!strcmp(name, "wide-stride")) {
        /* Isolate the 8192-pixel pitch limit, not alignment or smem limits. */
        fixed.line_length = 8194 * 4;
        fixed.smem_len = fixed.line_length * 6;
        variable.xres_virtual = 8194;
    }
    else if (!strcmp(name, "nonstd")) variable.nonstd = 1;
    else if (!strcmp(name, "grayscale")) variable.grayscale = 1;
    else CHECK(false);
    char reason[192] = "";
    CHECK(panel_g2d_open(fd, f, v, reason, sizeof(reason)) == NULL);
    CHECK(errno == ENOTSUP && strstr(reason, "layout") && opens == 0);
    check_clean();
}

static void bad_canvas(const char *name)
{
    panel_g2d_t *g2d = open_adapter();
    uint32_t pixels[32];
    unsigned rotation = 0;
    if (!strncmp(name, "scale-", 6)) rotation = (unsigned)atoi(name + 6);
    panel_canvas_t c = make_canvas(rotation, pixels);
    const panel_canvas_t *canvas = &c;
    panel_g2d_t *target = g2d;
    int expected = EINVAL;
    if (!strcmp(name, "null-g2d")) target = NULL;
    else if (!strcmp(name, "null-canvas")) canvas = NULL;
    else if (!strcmp(name, "null-pixels")) c.pixels = NULL;
    else if (!strcmp(name, "zero-width")) c.width = 0;
    else if (!strcmp(name, "negative-width")) c.width = -1;
    else if (!strcmp(name, "zero-height")) c.height = 0;
    else if (!strcmp(name, "negative-height")) c.height = -1;
    else if (!strcmp(name, "stride")) c.stride = (size_t)c.width - 1;
    else if (!strcmp(name, "rotation")) rotation = 45;
    else if (!strncmp(name, "scale-", 6)) { --c.width; expected = ENOTSUP; }
    else CHECK(false);
    char reason[192] = "";
    CHECK(panel_g2d_present(target, canvas, rotation, reason, sizeof(reason)) < 0);
    CHECK(errno == expected && reason[0] && allocations == 0 && blits == 0);
    verify_untouched();
    panel_g2d_close(g2d);
    check_clean();
}

static bool open_failure(void)
{
    return is_fault("g2d-open") || is_fault("export") || is_fault("export-error-fd") || is_fault("export-fd") ||
           is_fault("fcntl") || is_fault("lseek") || is_fault("short-export");
}

static void device_error(bool recover)
{
    uint32_t pixels[32], original[32];
    panel_canvas_t c = make_canvas(0, pixels);
    memcpy(original, pixels, sizeof(pixels));
    char reason[192] = "";
    panel_g2d_t *g2d = panel_g2d_open(FB_FD, &fixed, &variable, reason, sizeof(reason));
    if (open_failure()) {
        CHECK(!g2d && reason[0]);
        int expected = is_fault("g2d-open") ? ENOENT : is_fault("export-fd") ? ENODEV :
                       is_fault("lseek") ? ESPIPE : is_fault("short-export") ? EOVERFLOW : EIO;
        CHECK(errno == expected);
        check_clean();
        return;
    }
    CHECK(g2d);
    CHECK(panel_g2d_present(g2d, &c, 0, reason, sizeof(reason)) < 0);
    int expected = is_fault("heap-open") ? ENOENT :
                   (is_fault("allocate") || is_fault("source-mmap")) ? ENOMEM :
                   is_fault("blit-eintr") ? EINTR : EIO;
    CHECK(reason[0] && errno == expected);
    CHECK(blits == (is_fault("blit") || is_fault("blit-eintr") ? 1u : 0u));
    if (is_fault("sync-start")) CHECK(sync_starts == 1 && sync_ends == 0);
    if (is_fault("sync-end")) CHECK(sync_starts == 1 && sync_ends == 1);
    if (is_fault("allocate") || is_fault("heap-open")) CHECK(heap_opens == 4);
    verify_untouched();
    CHECK(memcmp(pixels, original, sizeof(pixels)) == 0);
    if (recover) {
        fault = "none";
        CHECK(panel_g2d_present(g2d, &c, 0, reason, sizeof(reason)) == 0);
        CHECK(reason[0] == '\0');
        verify_frame(&c, 0, true);
    }
    panel_g2d_close(g2d);
    check_clean();
}

static void reuse(void)
{
    panel_g2d_t *g2d = open_adapter();
    uint32_t pixels[32];
    const unsigned rotations[] = { 0, 180, 90, 270, 0 };
    const unsigned expected_allocations[] = { 1, 1, 2, 2, 3 };
    for (size_t i = 0; i < sizeof(rotations) / sizeof(rotations[0]); ++i) {
        panel_canvas_t c = make_canvas(rotations[i], pixels);
        memset(framebuffer, 0x5a, sizeof(framebuffer));
        /* No reason buffer is a supported diagnostic-free call. */
        CHECK(panel_g2d_present(g2d, &c, rotations[i], NULL, 0) == 0);
        CHECK(allocations == expected_allocations[i]);
        verify_frame(&c, rotations[i], true);
    }
    panel_g2d_close(g2d);
    check_clean();
}

static void resize_map_retry(unsigned rotation)
{
    panel_g2d_t *g2d = open_adapter();
    uint32_t old_pixels[32], resized_pixels[32], original[32];
    panel_canvas_t old = make_canvas(rotation, old_pixels);
    unsigned resized_rotation = (rotation + 90) % 360;
    panel_canvas_t resized = make_canvas(resized_rotation, resized_pixels);
    memcpy(original, old_pixels, sizeof(original));
    char reason[192] = "";
    CHECK(panel_g2d_present(g2d, &old, rotation, reason, sizeof(reason)) == 0);
    CHECK(allocations == 1 && blits == 1);
    verify_frame(&old, rotation, true);

    /* This is a geometry CHANGE after a successful mapping, not merely an
     * initial mmap failure followed by a retry of the same failed geometry. */
    memset(framebuffer, 0x5a, sizeof(framebuffer));
    fault = "source-mmap";
    CHECK(panel_g2d_present(g2d, &resized, resized_rotation, reason, sizeof(reason)) < 0);
    CHECK(errno == ENOMEM && strstr(reason, "map G2D source"));
    CHECK(allocations == 2 && blits == 1 && sync_starts == 1 && sync_ends == 1);
    verify_untouched();
    /* Failed mmap must release the NEW fd immediately (and the old source
     * was already released); ENGINE/EXPORT remain owned by the live adapter. */
    for (int fd = FIRST_FD; fd < next_fd; ++fd)
        CHECK(descriptors[fd].kind != SOURCE);

    fault = "none";
    CHECK(panel_g2d_present(g2d, &old, rotation, reason, sizeof(reason)) == 0);
    CHECK(!reason[0] && allocations == 3 && blits == 2);
    CHECK(memcmp(old_pixels, original, sizeof(original)) == 0);
    verify_frame(&old, rotation, true);
    /* The recovered old geometry should also be cached and reusable. */
    CHECK(panel_g2d_present(g2d, &old, rotation, reason, sizeof(reason)) == 0);
    CHECK(allocations == 3 && blits == 3);
    verify_frame(&old, rotation, true);
    panel_g2d_close(g2d);
    check_clean();
}

static void alpha_rejection(unsigned alpha)
{
    panel_g2d_t *g2d = open_adapter();
    uint32_t pixels[32], original[32];
    panel_canvas_t c = make_canvas(90, pixels);
    CHECK(panel_g2d_present(g2d, &c, 90, NULL, 0) == 0);
    memset(framebuffer, 0x5a, sizeof(framebuffer));
    /* Last visible pixel catches implementations checking only the first row. */
    size_t last = (size_t)(c.height - 1) * c.stride + (size_t)c.width - 1;
    pixels[last] = (pixels[last] & 0xffffffu) | (alpha << 24);
    memcpy(original, pixels, sizeof(pixels));
    char reason[192] = "";
    CHECK(panel_g2d_present(g2d, &c, 90, reason, sizeof(reason)) < 0);
    CHECK(errno == ENOTSUP && reason[0] && blits == 1);
    CHECK(memcmp(pixels, original, sizeof(pixels)) == 0);
    verify_untouched();
    /* Rejection must still finish CPU synchronization and leave the adapter
     * usable, even though fbdev's auto policy itself falls back permanently. */
    pixels[last] |= 0xff000000u;
    CHECK(panel_g2d_present(g2d, &c, 90, reason, sizeof(reason)) == 0);
    CHECK(reason[0] == '\0' && blits == 2 && allocations == 1);
    verify_frame(&c, 90, true);
    panel_g2d_close(g2d);
    check_clean();
}

static void integration(const char *renderer, unsigned rotation)
{
    if (strcmp(renderer, "default")) CHECK(setenv("A333_PANEL_RENDERER", renderer, 1) == 0);
    if (is_fault("layout")) fixed.visual = FB_VISUAL_DIRECTCOLOR;
    char reason[192] = "not cleared";
    panel_fbdev_t *display = panel_fbdev_open(NULL, reason, sizeof(reason));
    bool strict = !strcmp(renderer, "g2d");
    if (!strcmp(renderer, "invalid") || is_fault("fb-mmap") || (strict && !PANEL_ENABLE_G2D)) {
        CHECK(!display && reason[0]);
        CHECK(opens == 0);
        check_clean();
        return;
    }
    CHECK(display && !reason[0]);
    CHECK(panel_fbdev_width(display) == 3 && panel_fbdev_height(display) == 2);
    CHECK(panel_fbdev_bits_per_pixel(display) == 32 && !strcmp(panel_fbdev_path(display), "/dev/fb0"));
    uint32_t pixels[32], original[32];
    panel_canvas_t c = make_canvas(rotation, pixels);
    if (is_fault("non1to1")) --c.width;
    if (is_fault("alpha")) pixels[0] = PANEL_ARGB(127, 219, 77, 31);
    memcpy(original, pixels, sizeof(pixels));
    bool hardware = PANEL_ENABLE_G2D && strcmp(renderer, "software") && is_fault("none");
    bool fails = strict && !hardware;
    for (int frame = 0; frame < 2; ++frame) {
        memset(framebuffer, 0x5a, sizeof(framebuffer));
        unsigned previous_opens = opens, previous_blits = blits, previous_heaps = heap_opens;
        int result = panel_fbdev_present(display, &c, rotation);
        if (fails) {
            CHECK(result < 0 && errno != 0);
            verify_untouched();
        } else {
            CHECK(result == 0);
            verify_frame(&c, rotation, hardware);
        }
        CHECK(memcmp(pixels, original, sizeof(pixels)) == 0);
        if (frame == 1 && !hardware)
            CHECK(opens == previous_opens && blits == previous_blits && heap_opens == previous_heaps);
    }
    if (!PANEL_ENABLE_G2D || !strcmp(renderer, "software")) CHECK(opens == 0 && blits == 0);
    if (hardware) CHECK(opens == 1 && exports == 1 && allocations == 1 && blits == 2);
    if (is_fault("alpha") || is_fault("layout") || is_fault("non1to1")) CHECK(blits == 0);
    panel_fbdev_close(display);
    panel_fbdev_close(NULL);
    check_clean();
}

int main(int argc, char **argv)
{
    CHECK(argc >= 2);
    initialize("argb");
    if (!strcmp(argv[1], "abi")) { /* Compile-time assertions above. */ }
    else if (!strcmp(argv[1], "geometry")) {
        CHECK(argc == 4 || argc == 5);
        initialize(argv[3]);
        if (argc == 5) fault = argv[4];
        geometry((unsigned)atoi(argv[2]));
    } else if (!strcmp(argv[1], "layout")) {
        CHECK(argc == 3); bad_layout(argv[2]);
    } else if (!strcmp(argv[1], "canvas")) {
        CHECK(argc == 3); bad_canvas(argv[2]);
    } else if (!strcmp(argv[1], "error") || !strcmp(argv[1], "recover")) {
        CHECK(argc == 3); fault = argv[2]; device_error(!strcmp(argv[1], "recover"));
    } else if (!strcmp(argv[1], "reuse")) reuse();
    else if (!strcmp(argv[1], "resize-map-retry")) {
        CHECK(argc == 4); initialize(argv[3]); resize_map_retry((unsigned)atoi(argv[2]));
    }
    else if (!strcmp(argv[1], "alpha")) {
        CHECK(argc == 4); initialize(argv[2]); alpha_rejection((unsigned)atoi(argv[3]));
    } else if (!strcmp(argv[1], "integration")) {
        CHECK(argc == 6); initialize(argv[5]); fault = argv[3];
        integration(argv[2], (unsigned)atoi(argv[4]));
    } else CHECK(false);
    puts("PASS");
    return 0;
}
