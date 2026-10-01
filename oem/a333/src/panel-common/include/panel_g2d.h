#ifndef PANEL_G2D_H
#define PANEL_G2D_H

#include "panel_canvas.h"
#include <linux/fb.h>

typedef struct panel_g2d panel_g2d_t;

/* Synchronous DMA-BUF-only output adapter, not an LVGL/NXP draw unit.
 * Success means hardware has finished writing before the caller resumes.
 * No raw physical addresses and no /dev/mem access are used.
 */
panel_g2d_t *panel_g2d_open(int framebuffer_fd,
                           const struct fb_fix_screeninfo *fixed,
                           const struct fb_var_screeninfo *variable,
                           char *reason, size_t reason_size);
int panel_g2d_present(panel_g2d_t *g2d, const panel_canvas_t *canvas,
                      unsigned int rotation, char *reason, size_t reason_size);
void panel_g2d_close(panel_g2d_t *g2d);

#endif
