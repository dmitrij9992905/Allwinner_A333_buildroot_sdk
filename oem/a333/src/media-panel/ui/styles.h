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

// Style: SkodaStyleTopBar
lv_style_t *get_style_skoda_style_top_bar_MAIN_DEFAULT();
void add_style_skoda_style_top_bar(lv_obj_t *obj);
void remove_style_skoda_style_top_bar(lv_obj_t *obj);

// Style: SkodaStyleNotifyPanel
lv_style_t *get_style_skoda_style_notify_panel_MAIN_DEFAULT();
void add_style_skoda_style_notify_panel(lv_obj_t *obj);
void remove_style_skoda_style_notify_panel(lv_obj_t *obj);

// Style: SkodaStyleTabView
lv_style_t *get_style_skoda_style_tab_view_MAIN_DEFAULT();
void add_style_skoda_style_tab_view(lv_obj_t *obj);
void remove_style_skoda_style_tab_view(lv_obj_t *obj);

// Style: SkodaStyleTabBar
lv_style_t *get_style_skoda_style_tab_bar_MAIN_DEFAULT();
lv_style_t *get_style_skoda_style_tab_bar_ITEMS_DEFAULT();
lv_style_t *get_style_skoda_style_tab_bar_ITEMS_CHECKED();
void add_style_skoda_style_tab_bar(lv_obj_t *obj);
void remove_style_skoda_style_tab_bar(lv_obj_t *obj);

// Style: SkodaStyleImageButtonPlayback
lv_style_t *get_style_skoda_style_image_button_playback_MAIN_DEFAULT();
lv_style_t *get_style_skoda_style_image_button_playback_MAIN_PRESSED();
void add_style_skoda_style_image_button_playback(lv_obj_t *obj);
void remove_style_skoda_style_image_button_playback(lv_obj_t *obj);

// Style: SkodaStyleTabs
void add_style_skoda_style_tabs(lv_obj_t *obj);
void remove_style_skoda_style_tabs(lv_obj_t *obj);

// Style: SkodaStyleProgressBar
lv_style_t *get_style_skoda_style_progress_bar_MAIN_DEFAULT();
lv_style_t *get_style_skoda_style_progress_bar_INDICATOR_DEFAULT();
void add_style_skoda_style_progress_bar(lv_obj_t *obj);
void remove_style_skoda_style_progress_bar(lv_obj_t *obj);

// Style: SkodaStyleSlider
lv_style_t *get_style_skoda_style_slider_MAIN_DEFAULT();
lv_style_t *get_style_skoda_style_slider_INDICATOR_DEFAULT();
void add_style_skoda_style_slider(lv_obj_t *obj);
void remove_style_skoda_style_slider(lv_obj_t *obj);

#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_STYLES_H*/