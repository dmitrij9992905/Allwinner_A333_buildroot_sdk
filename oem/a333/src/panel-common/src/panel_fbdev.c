#define _POSIX_C_SOURCE 200809L

#include "panel_fbdev.h"
#include "panel_g2d.h"

#include <errno.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>

#ifndef PANEL_ENABLE_G2D
#define PANEL_ENABLE_G2D 1
#endif

struct panel_fbdev {
    int fd;
    char path[128];
    struct fb_fix_screeninfo fixed;
    struct fb_var_screeninfo variable;
    uint8_t *memory;
    size_t memory_size;
    unsigned int bytes_per_pixel;
    uint32_t red_table[256];
    uint32_t green_table[256];
    uint32_t blue_table[256];
    uint32_t opaque_bits;
    panel_g2d_t *g2d;
    int use_g2d, require_g2d, g2d_attempted, g2d_reported;
    int profile;
    uint64_t profile_started, frame_count, hardware_count, total_us, max_us;
};

static void initialize_pixel_tables(panel_fbdev_t *display);

static void set_error(char *buffer, size_t size, const char *format, ...)
{
    va_list args;

    if (buffer == NULL || size == 0)
        return;
    va_start(args, format);
    (void)vsnprintf(buffer, size, format, args);
    va_end(args);
}

static int validate_bitfield(const struct fb_bitfield *field,
                             unsigned int bits_per_pixel)
{
    if (field->length == 0)
        return 0;
    if (field->length > 32 || field->offset >= bits_per_pixel ||
        field->length > bits_per_pixel - field->offset)
        return -1;
    return 0;
}

panel_fbdev_t *panel_fbdev_open(const char *path,
                               char *error_text,
                               size_t error_text_size)
{
    const char *device_path = path != NULL ? path : "/dev/fb0";
    panel_fbdev_t *display;
    uint64_t visible_end;

    display = calloc(1, sizeof(*display));
    if (display == NULL) {
        set_error(error_text, error_text_size, "out of memory");
        return NULL;
    }
    display->fd = -1;
    display->memory = MAP_FAILED;
    (void)snprintf(display->path, sizeof(display->path), "%s", device_path);

    display->fd = open(device_path, O_RDWR | O_CLOEXEC);
    if (display->fd < 0) {
        set_error(error_text,
                  error_text_size,
                  "open %s: %s",
                  device_path,
                  strerror(errno));
        panel_fbdev_close(display);
        return NULL;
    }
    if (ioctl(display->fd, FBIOGET_FSCREENINFO, &display->fixed) < 0 ||
        ioctl(display->fd, FBIOGET_VSCREENINFO, &display->variable) < 0) {
        set_error(error_text,
                  error_text_size,
                  "query %s: %s",
                  device_path,
                  strerror(errno));
        panel_fbdev_close(display);
        return NULL;
    }

    if (display->fixed.type != FB_TYPE_PACKED_PIXELS ||
        (display->fixed.visual != FB_VISUAL_TRUECOLOR &&
         display->fixed.visual != FB_VISUAL_DIRECTCOLOR)) {
        errno = ENOTSUP;
        set_error(error_text,
                  error_text_size,
                  "%s is not a true/direct-color packed framebuffer",
                  device_path);
        panel_fbdev_close(display);
        return NULL;
    }
    if (display->variable.xres == 0 || display->variable.yres == 0 ||
        display->variable.bits_per_pixel < 8 ||
        display->variable.bits_per_pixel > 32 ||
        display->variable.bits_per_pixel % 8 != 0) {
        errno = ENOTSUP;
        set_error(error_text,
                  error_text_size,
                  "%s has unsupported %u bpp geometry",
                  device_path,
                  display->variable.bits_per_pixel);
        panel_fbdev_close(display);
        return NULL;
    }
    if (validate_bitfield(&display->variable.red,
                          display->variable.bits_per_pixel) < 0 ||
        validate_bitfield(&display->variable.green,
                          display->variable.bits_per_pixel) < 0 ||
        validate_bitfield(&display->variable.blue,
                          display->variable.bits_per_pixel) < 0 ||
        validate_bitfield(&display->variable.transp,
                          display->variable.bits_per_pixel) < 0) {
        errno = ENOTSUP;
        set_error(error_text,
                  error_text_size,
                  "%s reports invalid color bitfields",
                  device_path);
        panel_fbdev_close(display);
        return NULL;
    }

    display->bytes_per_pixel = display->variable.bits_per_pixel / 8u;
    display->memory_size = display->fixed.smem_len;
    if ((uint64_t)(display->variable.xoffset + display->variable.xres) *
            display->bytes_per_pixel >
        display->fixed.line_length) {
        errno = EOVERFLOW;
        set_error(error_text,
                  error_text_size,
                  "%s viewport exceeds its reported line stride",
                  device_path);
        panel_fbdev_close(display);
        return NULL;
    }
    visible_end =
        (uint64_t)(display->variable.yoffset + display->variable.yres - 1u) *
            display->fixed.line_length +
        (uint64_t)(display->variable.xoffset + display->variable.xres) *
            display->bytes_per_pixel;
    if (display->memory_size == 0 || visible_end > display->memory_size) {
        errno = EOVERFLOW;
        set_error(error_text,
                  error_text_size,
                  "%s framebuffer memory is smaller than its viewport",
                  device_path);
        panel_fbdev_close(display);
        return NULL;
    }

    display->memory = mmap(NULL,
                           display->memory_size,
                           PROT_READ | PROT_WRITE,
                           MAP_SHARED,
                           display->fd,
                           0);
    if (display->memory == MAP_FAILED) {
        set_error(error_text,
                  error_text_size,
                  "mmap %s: %s",
                  device_path,
                  strerror(errno));
        panel_fbdev_close(display);
        return NULL;
    }

    initialize_pixel_tables(display);
    const char *renderer = getenv("A333_PANEL_RENDERER");
    if (renderer && strcmp(renderer, "auto") != 0 &&
        strcmp(renderer, "software") != 0 && strcmp(renderer, "g2d") != 0) {
        errno = EINVAL;
        set_error(error_text, error_text_size,
                  "A333_PANEL_RENDERER must be auto, software or g2d");
        panel_fbdev_close(display);
        return NULL;
    }
    display->require_g2d = renderer && strcmp(renderer, "g2d") == 0;
    display->use_g2d = PANEL_ENABLE_G2D && (!renderer || strcmp(renderer, "software") != 0);
    display->profile = getenv("A333_PANEL_PROFILE") &&
                       strcmp(getenv("A333_PANEL_PROFILE"), "1") == 0;
    if (display->require_g2d && !PANEL_ENABLE_G2D) {
        errno = ENOTSUP;
        set_error(error_text, error_text_size, "G2D support was disabled at build time");
        panel_fbdev_close(display);
        return NULL;
    }
    (void)ioctl(display->fd, FBIOBLANK, FB_BLANK_UNBLANK);
    if (error_text != NULL && error_text_size != 0)
        error_text[0] = '\0';
    return display;
}

void panel_fbdev_close(panel_fbdev_t *display)
{
    if (display == NULL)
        return;
    panel_g2d_close(display->g2d);
    if (display->memory != MAP_FAILED)
        (void)munmap(display->memory, display->memory_size);
    if (display->fd >= 0)
        (void)close(display->fd);
    free(display);
}

int panel_fbdev_width(const panel_fbdev_t *display)
{
    return display != NULL ? (int)display->variable.xres : 0;
}

int panel_fbdev_height(const panel_fbdev_t *display)
{
    return display != NULL ? (int)display->variable.yres : 0;
}

int panel_fbdev_bits_per_pixel(const panel_fbdev_t *display)
{
    return display != NULL ? (int)display->variable.bits_per_pixel : 0;
}

const char *panel_fbdev_path(const panel_fbdev_t *display)
{
    return display != NULL ? display->path : "";
}

static uint32_t reverse_bits(uint32_t value, unsigned int length)
{
    uint32_t result = 0;
    unsigned int i;

    for (i = 0; i < length; ++i) {
        result = (result << 1) | (value & 1u);
        value >>= 1;
    }
    return result;
}

static uint32_t encode_component(uint8_t value,
                                 const struct fb_bitfield *field)
{
    uint64_t maximum;
    uint32_t encoded;

    if (field->length == 0)
        return 0;
    maximum = field->length == 32 ? UINT32_MAX
                                  : (UINT64_C(1) << field->length) - 1u;
    encoded = (uint32_t)(((uint64_t)value * maximum + 127u) / 255u);
    if (field->msb_right != 0)
        encoded = reverse_bits(encoded, field->length);
    return encoded << field->offset;
}

static uint32_t encode_pixel(const panel_fbdev_t *display,
                             panel_color_t source)
{
    uint8_t alpha = (uint8_t)(source >> 24);
    uint8_t red = (uint8_t)(source >> 16);
    uint8_t green = (uint8_t)(source >> 8);
    uint8_t blue = (uint8_t)source;

    if (alpha != 255u) {
        red = (uint8_t)(((unsigned int)red * alpha + 127u) / 255u);
        green = (uint8_t)(((unsigned int)green * alpha + 127u) / 255u);
        blue = (uint8_t)(((unsigned int)blue * alpha + 127u) / 255u);
    }
    return display->red_table[red] | display->green_table[green] |
           display->blue_table[blue] | display->opaque_bits;
}

static void initialize_pixel_tables(panel_fbdev_t *display)
{
    unsigned int value;

    for (value = 0; value <= 255u; ++value) {
        display->red_table[value] =
            encode_component((uint8_t)value, &display->variable.red);
        display->green_table[value] =
            encode_component((uint8_t)value, &display->variable.green);
        display->blue_table[value] =
            encode_component((uint8_t)value, &display->variable.blue);
    }
    display->opaque_bits =
        encode_component(255u, &display->variable.transp);
}

static size_t scale_coordinate(unsigned int coordinate,
                               unsigned int destination_extent,
                               size_t source_extent)
{
    if (destination_extent <= 1u || source_extent <= 1u)
        return 0;
    return (size_t)((uint64_t)coordinate * (source_extent - 1u) /
                    (destination_extent - 1u));
}

static int software_present(panel_fbdev_t *display,
                             const panel_canvas_t *canvas,
                             unsigned int rotation)
{
    unsigned int destination_y;

    if (display == NULL || display->memory == MAP_FAILED || canvas == NULL ||
        canvas->pixels == NULL || canvas->width <= 0 || canvas->height <= 0) {
        errno = EINVAL;
        return -1;
    }
    if (rotation != 0 && rotation != 90 &&
        rotation != 180 && rotation != 270) {
        errno = EINVAL;
        return -1;
    }

    for (destination_y = 0; destination_y < display->variable.yres;
         ++destination_y) {
        uint8_t *destination =
            display->memory +
            (size_t)(display->variable.yoffset + destination_y) *
                display->fixed.line_length +
            (size_t)display->variable.xoffset * display->bytes_per_pixel;
        unsigned int destination_x;

        for (destination_x = 0; destination_x < display->variable.xres;
             ++destination_x) {
            size_t source_x;
            size_t source_y;
            uint32_t encoded;

            switch (rotation) {
            case 90:
                source_x = scale_coordinate(destination_y,
                                            display->variable.yres,
                                            (size_t)canvas->width);
                source_y = scale_coordinate(display->variable.xres - 1u -
                                                destination_x,
                                            display->variable.xres,
                                            (size_t)canvas->height);
                break;
            case 180:
                source_x = scale_coordinate(display->variable.xres - 1u -
                                                destination_x,
                                            display->variable.xres,
                                            (size_t)canvas->width);
                source_y = scale_coordinate(display->variable.yres - 1u -
                                                destination_y,
                                            display->variable.yres,
                                            (size_t)canvas->height);
                break;
            case 270:
                source_x = scale_coordinate(display->variable.yres - 1u -
                                                destination_y,
                                            display->variable.yres,
                                            (size_t)canvas->width);
                source_y = scale_coordinate(destination_x,
                                            display->variable.xres,
                                            (size_t)canvas->height);
                break;
            case 0:
            default:
                source_x = scale_coordinate(destination_x,
                                            display->variable.xres,
                                            (size_t)canvas->width);
                source_y = scale_coordinate(destination_y,
                                            display->variable.yres,
                                            (size_t)canvas->height);
                break;
            }
            encoded = encode_pixel(
                display,
                canvas->pixels[source_y * canvas->stride + source_x]);

            memcpy(destination +
                       (size_t)destination_x * display->bytes_per_pixel,
                   &encoded,
                   display->bytes_per_pixel);
        }
    }
    return 0;
}

static uint64_t monotonic_us(void)
{
    struct timespec now;
    if (clock_gettime(CLOCK_MONOTONIC, &now) < 0) return 0;
    return (uint64_t)now.tv_sec * 1000000u + (uint64_t)now.tv_nsec / 1000u;
}

int panel_fbdev_present(panel_fbdev_t *display,
                        const panel_canvas_t *canvas,
                        unsigned int rotation)
{
    char reason[192] = "";
    uint64_t start = 0;
    int result = -1, hardware = 0;
    if (!display || !canvas || !canvas->pixels || canvas->width <= 0 ||
        canvas->height <= 0 || canvas->stride < (size_t)canvas->width ||
        (rotation != 0 && rotation != 90 && rotation != 180 && rotation != 270)) {
        errno = EINVAL;
        return -1;
    }
    if (display->profile) start = monotonic_us();
    if (display->use_g2d) {
        if (!display->g2d_attempted) {
            display->g2d_attempted = 1;
            display->g2d = panel_g2d_open(display->fd, &display->fixed,
                                          &display->variable, reason, sizeof(reason));
        }
        if (display->g2d)
            result = panel_g2d_present(display->g2d, canvas, rotation, reason, sizeof(reason));
        if (result == 0) {
            hardware = 1;
            if (!display->g2d_reported) {
                fprintf(stderr, "panel: G2D enabled, synchronous DMA-BUF rotation=%u, %dx%d -> %ux%u\n",
                        rotation, canvas->width, canvas->height,
                        display->variable.xres, display->variable.yres);
                display->g2d_reported = 1;
            }
        } else {
            int saved = errno;
            fprintf(stderr, "panel: G2D unavailable (%s); %s\n", reason,
                    display->require_g2d ? "forced G2D output failed" : "using software output");
            panel_g2d_close(display->g2d);
            display->g2d = NULL;
            display->use_g2d = 0; /* No repeated failing ioctls/log spam. */
            errno = saved;
        }
    }
    if (!hardware && !display->require_g2d)
        result = software_present(display, canvas, rotation);
    else if (!hardware && reason[0] == '\0') {
        /* Keep strict failure meaningful on subsequent frames too. */
        errno = ENODEV;
        result = -1;
    }
    if (display->profile && result == 0) {
        uint64_t end = monotonic_us();
        uint64_t elapsed = end >= start ? end - start : 0;
        if (!display->profile_started) display->profile_started = start;
        display->frame_count++;
        display->hardware_count += (uint64_t)hardware;
        display->total_us += elapsed;
        if (elapsed > display->max_us) display->max_us = elapsed;
        if (end - display->profile_started >= 5000000u) {
            fprintf(stderr, "panel: present frames=%llu, g2d=%llu, mean=%llu us, max=%llu us (includes source copy)\n",
                    (unsigned long long)display->frame_count,
                    (unsigned long long)display->hardware_count,
                    (unsigned long long)(display->total_us / display->frame_count),
                    (unsigned long long)display->max_us);
            display->profile_started = end;
            display->frame_count = display->hardware_count = display->total_us = display->max_us = 0;
        }
    }
    return result;
}
