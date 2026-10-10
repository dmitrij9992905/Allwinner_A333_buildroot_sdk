#include "media_fonts.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

static lv_font_t *runtime_fonts[MEDIA_PANEL_FONT_MAX_SIZE + 1];
static const unsigned sizes[] = {MEDIA_PANEL_FONT_SIZES};

void media_fonts_destroy(void)
{
    for (unsigned i = 0; i < sizeof(sizes) / sizeof(sizes[0]); ++i) {
        unsigned size = sizes[i];
        if (runtime_fonts[size]) lv_freetype_font_delete(runtime_fonts[size]);
        runtime_fonts[size] = NULL;
    }
}

bool media_fonts_init(panel_lvgl_fonts_t *fonts, char *error, size_t error_size)
{
    const char *path = getenv("A333_MEDIA_FONT");
    if (!path || !*path) path = MEDIA_PANEL_FONT_PATH;
    for (unsigned i = 0; i < sizeof(sizes) / sizeof(sizes[0]); ++i) {
        unsigned size = sizes[i];
        runtime_fonts[size] = lv_freetype_font_create(path,
            LV_FREETYPE_FONT_RENDER_MODE_BITMAP, size, LV_FREETYPE_FONT_STYLE_NORMAL);
        if (!runtime_fonts[size]) {
            snprintf(error, error_size, "cannot load FreeType font %s (%u px)", path, size);
            media_fonts_destroy();
            return false;
        }
    }
    fonts->small = runtime_fonts[24];
    fonts->normal = runtime_fonts[32];
    fonts->large = runtime_fonts[40];
    if (error_size) error[0] = '\0';
    return true;
}

bool media_fonts_glyph_dsc(const lv_font_t *font, lv_font_glyph_dsc_t *glyph,
                           uint32_t letter, uint32_t next)
{
    unsigned size = (unsigned)(uintptr_t)font->dsc;
    lv_font_t *actual = size <= MEDIA_PANEL_FONT_MAX_SIZE ? runtime_fonts[size] : NULL;
    if (!actual || !actual->get_glyph_dsc(actual, glyph, letter, next)) return false;
    // Missing TTF symbols go through the generated descriptor's LV_SYMBOL fallback.
    return letter < 32 || glyph->gid.index != 0;
}

const void *media_fonts_glyph_bitmap(lv_font_glyph_dsc_t *glyph, lv_draw_buf_t *buffer)
{
    unsigned size = (unsigned)(uintptr_t)glyph->resolved_font->dsc;
    lv_font_t *actual = size <= MEDIA_PANEL_FONT_MAX_SIZE ? runtime_fonts[size] : NULL;
    if (!actual) return NULL;
    // The FreeType cache must see its own descriptor. Preserve the caller's
    // resolved font so repeated bitmap requests and release use the same proxy.
    const lv_font_t *proxy = glyph->resolved_font;
    glyph->resolved_font = actual;
    const void *bitmap = actual->get_glyph_bitmap(glyph, buffer);
    glyph->resolved_font = proxy;
    return bitmap;
}

void media_fonts_release_glyph(const lv_font_t *font, lv_font_glyph_dsc_t *glyph)
{
    unsigned size = (unsigned)(uintptr_t)font->dsc;
    lv_font_t *actual = size <= MEDIA_PANEL_FONT_MAX_SIZE ? runtime_fonts[size] : NULL;
    if (actual && actual->release_glyph) {
        const lv_font_t *proxy = glyph->resolved_font;
        glyph->resolved_font = actual;
        actual->release_glyph(actual, glyph);
        glyph->resolved_font = proxy;
    }
}
