#ifndef A333_MEDIA_UI_H
#define A333_MEDIA_UI_H

#include "lvgl_fonts.h"

#ifdef __cplusplus
extern "C" {
#endif

void media_ui_init(const panel_lvgl_fonts_t *fonts);
void media_ui_tick(double now);

#ifdef __cplusplus
}
#endif
#endif
