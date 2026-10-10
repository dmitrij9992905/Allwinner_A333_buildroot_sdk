#ifndef EEZ_LVGL_UI_IMAGES_H
#define EEZ_LVGL_UI_IMAGES_H

#include <lvgl/lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

extern const lv_img_dsc_t img_bg_1280_800;
extern const lv_img_dsc_t img_material_wifi;
extern const lv_img_dsc_t img_material_settings_ethernet;
extern const lv_img_dsc_t img_material_bluetooth;
extern const lv_img_dsc_t img_material_music_note;
extern const lv_img_dsc_t img_material_music_note_2;
extern const lv_img_dsc_t img_material_music_note_2_72dp;
extern const lv_img_dsc_t img_material_skip_next_256dp;
extern const lv_img_dsc_t img_material_skip_prev_256dp;
extern const lv_img_dsc_t img_material_skip_prev_144dp;
extern const lv_img_dsc_t img_material_play_pause_144dp;
extern const lv_img_dsc_t img_material_skip_next_144dp;

#ifndef EXT_IMG_DESC_T
#define EXT_IMG_DESC_T
typedef struct _ext_img_desc_t {
    const char *name;
    const lv_img_dsc_t *img_dsc;
} ext_img_desc_t;
#endif

extern const ext_img_desc_t images[12];

#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_IMAGES_H*/