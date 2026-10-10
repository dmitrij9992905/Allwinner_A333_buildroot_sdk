#ifndef EEZ_LVGL_UI_FONTS_H
#define EEZ_LVGL_UI_FONTS_H

#include <lvgl/lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

extern const lv_font_t ui_font_skoda_pro_14;
extern const lv_font_t ui_font_skoda_pro_16;
extern const lv_font_t ui_font_skoda_pro_medium_14;
extern const lv_font_t ui_font_skoda_pro_medium_16;
extern const lv_font_t ui_font_skoda_pro_medium_18;
extern const lv_font_t ui_font_skoda_pro_medium_20;
extern const lv_font_t ui_font_skoda_pro_medium_22;
extern const lv_font_t ui_font_skoda_pro_medium_24;
extern const lv_font_t ui_font_skoda_pro_medium_26;
extern const lv_font_t ui_font_skoda_pro_medium_28;
extern const lv_font_t ui_font_skoda_pro_medium_40;
extern const lv_font_t ui_font_skoda_pro_medium_32;

#ifndef EXT_FONT_DESC_T
#define EXT_FONT_DESC_T
typedef struct _ext_font_desc_t {
    const char *name;
    const void *font_ptr;
} ext_font_desc_t;
#endif

extern ext_font_desc_t fonts[];

#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_FONTS_H*/