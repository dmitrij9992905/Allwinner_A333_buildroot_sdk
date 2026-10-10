#ifndef EEZ_LVGL_UI_SCREENS_H
#define EEZ_LVGL_UI_SCREENS_H

#include <lvgl/lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

// Screens

enum ScreensEnum {
    _SCREEN_ID_FIRST = 1,
    SCREEN_ID_MAIN = 1,
    SCREEN_ID_PLAYER_SCREEN = 2,
    SCREEN_ID_WIFI_SCREEN = 3,
    _SCREEN_ID_LAST = 3
};

typedef struct _objects_t {
    lv_obj_t *main;
    lv_obj_t *player_screen;
    lv_obj_t *wifi_screen;
    lv_obj_t *main_top_panel;
    lv_obj_t *main_top_panel__top_bar;
    lv_obj_t *main_top_panel__system_datetime_label;
    lv_obj_t *main_top_panel__system_wifi_connected_off;
    lv_obj_t *main_top_panel__system_wifi_connected_on;
    lv_obj_t *main_top_panel__system_ethernet_connected_off;
    lv_obj_t *main_top_panel__system_ethernet_connected_on;
    lv_obj_t *main_top_panel__system_bluetooth_connected_off;
    lv_obj_t *main_top_panel__system_bluetooth_connected_on;
    lv_obj_t *main_top_panel__system_brand;
    lv_obj_t *main_volume_osd;
    lv_obj_t *main_volume_osd__notify_bar_volume;
    lv_obj_t *main_volume_osd__notify_text_volume_value_label;
    lv_obj_t *main_volume_osd__notify_progress_bar_volume;
    lv_obj_t *player_top_panel;
    lv_obj_t *player_top_panel__top_bar;
    lv_obj_t *player_top_panel__system_datetime_label;
    lv_obj_t *player_top_panel__system_wifi_connected_off;
    lv_obj_t *player_top_panel__system_wifi_connected_on;
    lv_obj_t *player_top_panel__system_ethernet_connected_off;
    lv_obj_t *player_top_panel__system_ethernet_connected_on;
    lv_obj_t *player_top_panel__system_bluetooth_connected_off;
    lv_obj_t *player_top_panel__system_bluetooth_connected_on;
    lv_obj_t *player_top_panel__system_brand;
    lv_obj_t *player_volume_osd;
    lv_obj_t *player_volume_osd__notify_bar_volume;
    lv_obj_t *player_volume_osd__notify_text_volume_value_label;
    lv_obj_t *player_volume_osd__notify_progress_bar_volume;
    lv_obj_t *wifi_top_panel;
    lv_obj_t *wifi_top_panel__top_bar;
    lv_obj_t *wifi_top_panel__system_datetime_label;
    lv_obj_t *wifi_top_panel__system_wifi_connected_off;
    lv_obj_t *wifi_top_panel__system_wifi_connected_on;
    lv_obj_t *wifi_top_panel__system_ethernet_connected_off;
    lv_obj_t *wifi_top_panel__system_ethernet_connected_on;
    lv_obj_t *wifi_top_panel__system_bluetooth_connected_off;
    lv_obj_t *wifi_top_panel__system_bluetooth_connected_on;
    lv_obj_t *wifi_top_panel__system_brand;
    lv_obj_t *wifi_volume_osd;
    lv_obj_t *wifi_volume_osd__notify_bar_volume;
    lv_obj_t *wifi_volume_osd__notify_text_volume_value_label;
    lv_obj_t *wifi_volume_osd__notify_progress_bar_volume;
    lv_obj_t *main_contents_container;
    lv_obj_t *main_tabs;
    lv_obj_t *now_playing_tab;
    lv_obj_t *media_source;
    lv_obj_t *track_title;
    lv_obj_t *track_artist;
    lv_obj_t *track_album;
    lv_obj_t *track_filename;
    lv_obj_t *next;
    lv_obj_t *previous;
    lv_obj_t *track_progress_slider;
    lv_obj_t *toggle;
    lv_obj_t *album_cover_image;
    lv_obj_t *track_position_duration_label;
    lv_obj_t *source_tab;
    lv_obj_t *sources_current;
    lv_obj_t *sources_help;
    lv_obj_t *sources_heading;
    lv_obj_t *select_bluetooth;
    lv_obj_t *select_bluetooth_caption;
    lv_obj_t *select_radio;
    lv_obj_t *select_radio_caption;
    lv_obj_t *select_music;
    lv_obj_t *select_music_caption;
    lv_obj_t *select_usb;
    lv_obj_t *select_usb_caption;
    lv_obj_t *select_sd;
    lv_obj_t *select_sd_caption;
    lv_obj_t *wifi_tab;
    lv_obj_t *wifi_keyboard;
    lv_obj_t *wifi_status;
    lv_obj_t *network_heading;
    lv_obj_t *wifi_network_list;
    lv_obj_t *wifi_scan;
    lv_obj_t *wifi_scan_caption;
    lv_obj_t *wifi_password_input;
    lv_obj_t *wifi_connect;
    lv_obj_t *wifi_connect_caption;
    lv_obj_t *sources_back;
    lv_obj_t *sources_back_caption;
    lv_obj_t *network_back;
    lv_obj_t *network_back_caption;
} objects_t;

extern objects_t objects;

void create_screen_main();
void tick_screen_main();

void create_screen_player_screen();
void tick_screen_player_screen();

void create_screen_wifi_screen();
void tick_screen_wifi_screen();

void create_user_widget_top_panel(lv_obj_t *parent_obj, void *flowState, int startWidgetIndex);
void tick_user_widget_top_panel(void *flowState, int startWidgetIndex);

void create_user_widget_notify_panel(lv_obj_t *parent_obj, void *flowState, int startWidgetIndex);
void tick_user_widget_notify_panel(void *flowState, int startWidgetIndex);

void create_user_widget_notify_panel_volume(lv_obj_t *parent_obj, void *flowState, int startWidgetIndex);
void tick_user_widget_notify_panel_volume(void *flowState, int startWidgetIndex);

void tick_screen_by_id(enum ScreensEnum screenId);
void tick_screen(int screen_index);

void create_screens();

#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_SCREENS_H*/