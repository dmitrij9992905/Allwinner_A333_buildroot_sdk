#include "panel_canvas.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int panel_canvas_init(panel_canvas_t *canvas, int width, int height)
{
    size_t pixel_count;

    if (canvas == NULL) {
        errno = EINVAL;
        return -1;
    }
    memset(canvas, 0, sizeof(*canvas));
    if (width <= 0 || height <= 0) {
        errno = EINVAL;
        return -1;
    }
    if ((size_t)width > SIZE_MAX / (size_t)height) {
        errno = EOVERFLOW;
        return -1;
    }
    pixel_count = (size_t)width * (size_t)height;
    if (pixel_count > SIZE_MAX / sizeof(*canvas->pixels)) {
        errno = EOVERFLOW;
        return -1;
    }

    canvas->pixels = calloc(pixel_count, sizeof(*canvas->pixels));
    if (canvas->pixels == NULL)
        return -1;
    canvas->width = width;
    canvas->height = height;
    canvas->stride = (size_t)width;
    return 0;
}

void panel_canvas_destroy(panel_canvas_t *canvas)
{
    if (canvas == NULL)
        return;
    free(canvas->pixels);
    memset(canvas, 0, sizeof(*canvas));
}

void panel_canvas_clear(panel_canvas_t *canvas, panel_color_t color)
{
    size_t pixel_count;
    size_t i;

    if (canvas == NULL || canvas->pixels == NULL || canvas->width <= 0 ||
        canvas->height <= 0)
        return;
    pixel_count = canvas->stride * (size_t)canvas->height;
    for (i = 0; i < pixel_count; ++i)
        canvas->pixels[i] = color;
}

void panel_canvas_put_pixel(panel_canvas_t *canvas,
                            int x,
                            int y,
                            panel_color_t color)
{
    if (canvas == NULL || canvas->pixels == NULL || x < 0 || y < 0 ||
        x >= canvas->width || y >= canvas->height)
        return;
    canvas->pixels[(size_t)y * canvas->stride + (size_t)x] = color;
}

static uint8_t composite_channel(uint8_t channel, uint8_t alpha)
{
    return (uint8_t)(((unsigned int)channel * alpha + 127u) / 255u);
}

int panel_canvas_write_ppm(const panel_canvas_t *canvas, const char *path)
{
    FILE *file;
    int y;

    if (canvas == NULL || canvas->pixels == NULL || canvas->width <= 0 ||
        canvas->height <= 0 || path == NULL) {
        errno = EINVAL;
        return -1;
    }

    file = fopen(path, "wb");
    if (file == NULL)
        return -1;
    if (fprintf(file, "P6\n%d %d\n255\n", canvas->width, canvas->height) < 0) {
        int saved_errno = errno;
        (void)fclose(file);
        errno = saved_errno != 0 ? saved_errno : EIO;
        return -1;
    }

    for (y = 0; y < canvas->height; ++y) {
        int x;
        const panel_color_t *row =
            canvas->pixels + (size_t)y * canvas->stride;

        for (x = 0; x < canvas->width; ++x) {
            panel_color_t pixel = row[x];
            uint8_t alpha = (uint8_t)(pixel >> 24);
            uint8_t rgb[3];

            rgb[0] = composite_channel((uint8_t)(pixel >> 16), alpha);
            rgb[1] = composite_channel((uint8_t)(pixel >> 8), alpha);
            rgb[2] = composite_channel((uint8_t)pixel, alpha);
            if (fwrite(rgb, 1, sizeof(rgb), file) != sizeof(rgb)) {
                int saved_errno = errno;
                (void)fclose(file);
                errno = saved_errno != 0 ? saved_errno : EIO;
                return -1;
            }
        }
    }

    if (fclose(file) != 0)
        return -1;
    return 0;
}
