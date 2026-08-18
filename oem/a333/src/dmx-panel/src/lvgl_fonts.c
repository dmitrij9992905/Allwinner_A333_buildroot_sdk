#define _POSIX_C_SOURCE 200809L

#include "lvgl_fonts.h"
#include "roboto_font_data.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#if LVGL_VERSION_MAJOR != 9 || LVGL_VERSION_MINOR != 5
#error "dmx_panel requires LVGL 9.5.x"
#endif

enum {
    DMX_FONT_SMALL_SIZE = 16,
    DMX_FONT_NORMAL_SIZE = 20,
    DMX_FONT_LARGE_SIZE = 28,
    DMX_FONT_CACHE_GLYPHS = 64
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
    *font = lv_tiny_ttf_create_data_ex(dmx_roboto_regular_ttf,
                                       dmx_roboto_regular_ttf_size(),
                                       size,
                                       LV_FONT_KERNING_NORMAL,
                                       DMX_FONT_CACHE_GLYPHS);
    return *font != NULL;
}

bool dmx_lvgl_fonts_init(dmx_lvgl_fonts_t *fonts,
                         char *error,
                         size_t error_size)
{
    if (fonts == NULL) {
        set_error(error, error_size, "Roboto font destination is null");
        return false;
    }
    memset(fonts, 0, sizeof(*fonts));

    if (dmx_roboto_regular_ttf_size() == 0u) {
        set_error(error, error_size, "embedded Roboto font is empty");
        return false;
    }

    if (!create_font(DMX_FONT_SMALL_SIZE, &fonts->small) ||
        !create_font(DMX_FONT_NORMAL_SIZE, &fonts->normal) ||
        !create_font(DMX_FONT_LARGE_SIZE, &fonts->large)) {
        set_error(error, error_size,
                  "cannot create LVGL fonts from embedded Roboto");
        dmx_lvgl_fonts_destroy(fonts);
        return false;
    }
    if (error != NULL && error_size != 0)
        error[0] = '\0';
    return true;
}

dmx_lvgl_ui_fonts_t dmx_lvgl_fonts_ui(const dmx_lvgl_fonts_t *fonts)
{
    dmx_lvgl_ui_fonts_t result = {0};

    if (fonts != NULL) {
        result.small = fonts->small;
        result.normal = fonts->normal;
        result.large = fonts->large;
    }
    return result;
}

void dmx_lvgl_fonts_destroy(dmx_lvgl_fonts_t *fonts)
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
