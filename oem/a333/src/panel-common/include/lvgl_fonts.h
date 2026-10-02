#ifndef PANEL_COMMON_LVGL_FONTS_H
#define PANEL_COMMON_LVGL_FONTS_H

#include <lvgl.h>

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Font pointers shared by applications, with no dependency on either UI. */
typedef struct {
    const lv_font_t *small;
    const lv_font_t *normal;
    const lv_font_t *large;
} panel_lvgl_font_set_t;

typedef struct {
    lv_font_t *small;
    lv_font_t *normal;
    lv_font_t *large;
} panel_lvgl_fonts_t;

/*
 * Creates the three UI sizes from the Roboto TTF embedded in the executable.
 * Call this after lv_init() and keep the object alive until the UI is gone.
 */
bool panel_lvgl_fonts_init(panel_lvgl_fonts_t *fonts,
                         char *error,
                         size_t error_size);

/* Applications with generated layouts can preserve their design font sizes. */
bool panel_lvgl_fonts_init_sizes(panel_lvgl_fonts_t *fonts,
                                uint16_t small, uint16_t normal, uint16_t large,
                                char *error, size_t error_size);

panel_lvgl_font_set_t panel_lvgl_fonts_ui(const panel_lvgl_fonts_t *fonts);
void panel_lvgl_fonts_destroy(panel_lvgl_fonts_t *fonts);

#ifdef __cplusplus
}
#endif

#endif
