#ifndef EEZ_LVGL_UI_STYLES_H
#define EEZ_LVGL_UI_STYLES_H

#include <lvgl/lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

// Style: SkodaStyleScreen
lv_style_t *get_style_skoda_style_screen_MAIN_DEFAULT();
void add_style_skoda_style_screen(lv_obj_t *obj);
void remove_style_skoda_style_screen(lv_obj_t *obj);

#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_STYLES_H*/