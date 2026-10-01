#define _POSIX_C_SOURCE 200809L

#include "lvgl_fonts.h"
#include "roboto_font_data.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#if LVGL_VERSION_MAJOR != 9 || LVGL_VERSION_MINOR != 5
#error "panel-common requires LVGL 9.5.x"
#endif

enum {
    PANEL_FONT_SMALL_SIZE = 16,
    PANEL_FONT_NORMAL_SIZE = 20,
    PANEL_FONT_LARGE_SIZE = 28,
    PANEL_FONT_CACHE_GLYPHS = 64
};

static void set_error(char *buffer, size_t size, const char *format, ...)
{
    va_list arguments;

    if (buffer == NULL || size == 0)
        return;
    va_start(arguments, format);
    (void)vsnprintf(buffer, size, format, arguments);
    va_end(arguments);
}

static bool create_font(uint16_t size, lv_font_t **font)
{
    *font = lv_tiny_ttf_create_data_ex(panel_roboto_regular_ttf,
                                       panel_roboto_regular_ttf_size(),
                                       size,
                                       LV_FONT_KERNING_NORMAL,
                                       PANEL_FONT_CACHE_GLYPHS);
    return *font != NULL;
}

bool panel_lvgl_fonts_init(panel_lvgl_fonts_t *fonts,
                         char *error,
                         size_t error_size)
{
    if (fonts == NULL) {
        set_error(error, error_size, "Roboto font destination is null");
        return false;
    }
    memset(fonts, 0, sizeof(*fonts));

    if (panel_roboto_regular_ttf_size() == 0u) {
        set_error(error, error_size, "embedded Roboto font is empty");
        return false;
    }

    if (!create_font(PANEL_FONT_SMALL_SIZE, &fonts->small) ||
        !create_font(PANEL_FONT_NORMAL_SIZE, &fonts->normal) ||
        !create_font(PANEL_FONT_LARGE_SIZE, &fonts->large)) {
        set_error(error, error_size,
                  "cannot create LVGL fonts from embedded Roboto");
        panel_lvgl_fonts_destroy(fonts);
        return false;
    }
    if (error != NULL && error_size != 0)
        error[0] = '\0';
    return true;
}

panel_lvgl_font_set_t panel_lvgl_fonts_ui(const panel_lvgl_fonts_t *fonts)
{
    panel_lvgl_font_set_t result = {0};

    if (fonts != NULL) {
        result.small = fonts->small;
        result.normal = fonts->normal;
        result.large = fonts->large;
    }
    return result;
}

void panel_lvgl_fonts_destroy(panel_lvgl_fonts_t *fonts)
{
    if (fonts == NULL)
        return;
    if (fonts->large != NULL)
        lv_tiny_ttf_destroy(fonts->large);
    if (fonts->normal != NULL)
        lv_tiny_ttf_destroy(fonts->normal);
    if (fonts->small != NULL)
        lv_tiny_ttf_destroy(fonts->small);
    memset(fonts, 0, sizeof(*fonts));
}
