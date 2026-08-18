#ifndef DMX_PANEL_FBDEV_H
#define DMX_PANEL_FBDEV_H

#include "panel_canvas.h"

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct panel_fbdev panel_fbdev_t;

/*
 * Opens a packed-pixel Linux framebuffer (normally /dev/fb0) and mmaps it.
 * The returned object discovers resolution, stride and RGB bitfields through
 * FBIOGET_*SCREENINFO; no RGB565/XRGB8888 assumption is made.
 */
panel_fbdev_t *panel_fbdev_open(const char *path,
                               char *error_text,
                               size_t error_text_size);
void panel_fbdev_close(panel_fbdev_t *display);

int panel_fbdev_width(const panel_fbdev_t *display);
int panel_fbdev_height(const panel_fbdev_t *display);
int panel_fbdev_bits_per_pixel(const panel_fbdev_t *display);
const char *panel_fbdev_path(const panel_fbdev_t *display);

/* Nearest-neighbour scales the canvas to the active framebuffer viewport. */
int panel_fbdev_present(panel_fbdev_t *display, const panel_canvas_t *canvas);

#ifdef __cplusplus
}
#endif

#endif
