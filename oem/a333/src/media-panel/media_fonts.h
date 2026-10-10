#ifndef A333_MEDIA_FONTS_H
#define A333_MEDIA_FONTS_H
#include "lvgl_fonts.h"
#ifdef __cplusplus
extern "C" {
#endif
bool media_fonts_init(panel_lvgl_fonts_t *fonts, char *error, size_t error_size);
void media_fonts_destroy(void);
bool media_fonts_glyph_dsc(const lv_font_t *font, lv_font_glyph_dsc_t *glyph,
                           uint32_t letter, uint32_t next);
const void *media_fonts_glyph_bitmap(lv_font_glyph_dsc_t *glyph, lv_draw_buf_t *buffer);
void media_fonts_release_glyph(const lv_font_t *font, lv_font_glyph_dsc_t *glyph);
#ifdef __cplusplus
}
#endif
#endif
