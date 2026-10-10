#ifndef A333_MEDIA_UI_H
#define A333_MEDIA_UI_H

#include "lvgl_fonts.h"
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

void media_ui_init(const panel_lvgl_fonts_t *fonts);
void media_ui_tick(double now);
/* Locale-independent Russian date; local timezone is selected by the caller. */
void media_ui_format_datetime(char *text, size_t size, const struct tm *date);

#ifdef __cplusplus
}
#endif
#endif
