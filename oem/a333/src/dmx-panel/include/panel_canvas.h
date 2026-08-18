#ifndef DMX_PANEL_CANVAS_H
#define DMX_PANEL_CANVAS_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PANEL_CANVAS_WIDTH 720
#define PANEL_CANVAS_HEIGHT 720

/* Pixels are stored as 0xAARRGGBB, independently of the framebuffer format. */
typedef uint32_t panel_color_t;

typedef struct {
    int width;
    int height;
    size_t stride;
    panel_color_t *pixels;
} panel_canvas_t;

#define PANEL_ARGB(a, r, g, b)                                                \
    ((((uint32_t)(a) & 0xffu) << 24) | (((uint32_t)(r) & 0xffu) << 16) |      \
     (((uint32_t)(g) & 0xffu) << 8) | ((uint32_t)(b) & 0xffu))
#define PANEL_RGB(r, g, b) PANEL_ARGB(0xffu, (r), (g), (b))

int panel_canvas_init(panel_canvas_t *canvas, int width, int height);
void panel_canvas_destroy(panel_canvas_t *canvas);

void panel_canvas_clear(panel_canvas_t *canvas, panel_color_t color);
void panel_canvas_put_pixel(panel_canvas_t *canvas,
                            int x,
                            int y,
                            panel_color_t color);

/* Writes a binary PPM (P6) image. Alpha is composited over black. */
int panel_canvas_write_ppm(const panel_canvas_t *canvas, const char *path);

#ifdef __cplusplus
}
#endif

#endif
