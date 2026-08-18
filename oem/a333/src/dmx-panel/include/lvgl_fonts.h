#ifndef DMX_PANEL_LVGL_FONTS_H
#define DMX_PANEL_LVGL_FONTS_H

#include "lvgl_ui.h"

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    lv_font_t *small;
    lv_font_t *normal;
    lv_font_t *large;
} dmx_lvgl_fonts_t;

/*
 * Creates the three UI sizes from the Roboto TTF embedded in the executable.
 * Call this after lv_init() and keep the object alive until the UI is gone.
 */
bool dmx_lvgl_fonts_init(dmx_lvgl_fonts_t *fonts,
                         char *error,
                         size_t error_size);

dmx_lvgl_ui_fonts_t dmx_lvgl_fonts_ui(const dmx_lvgl_fonts_t *fonts);
void dmx_lvgl_fonts_destroy(dmx_lvgl_fonts_t *fonts);

#ifdef __cplusplus
}
#endif

#endif
