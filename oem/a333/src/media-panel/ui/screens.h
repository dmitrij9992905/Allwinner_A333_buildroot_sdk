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
    lv_obj_t *main_top_bar;
    lv_obj_t *main_brand;
    lv_obj_t *main_wifi_connected_off;
    lv_obj_t *main_wifi_connected_on;
    lv_obj_t *main_ethernet_connected_off;
    lv_obj_t *main_ethernet_connected_on;
    lv_obj_t *main_bluetooth_connected_off;
    lv_obj_t *main_bluetooth_connected_on;
    lv_obj_t *media_source;
    lv_obj_t *track_title;
    lv_obj_t *track_artist;
    lv_obj_t *track_album;
    lv_obj_t *previous;
    lv_obj_t *previous_caption;
    lv_obj_t *toggle;
    lv_obj_t *toggle_caption;
    lv_obj_t *next;
    lv_obj_t *next_caption;
    lv_obj_t *sources;
    lv_obj_t *sources_caption;
    lv_obj_t *network;
    lv_obj_t *network_caption;
    lv_obj_t *main_volume_banner;
    lv_obj_t *main_volume;
    lv_obj_t *playerscreen_top_bar;
    lv_obj_t *playerscreen_brand;
    lv_obj_t *playerscreen_wifi_connected_off;
    lv_obj_t *playerscreen_wifi_connected_on;
    lv_obj_t *playerscreen_ethernet_connected_off;
    lv_obj_t *playerscreen_ethernet_connected_on;
    lv_obj_t *playerscreen_bluetooth_connected_off;
    lv_obj_t *playerscreen_bluetooth_connected_on;
    lv_obj_t *sources_heading;
    lv_obj_t *sources_help;
    lv_obj_t *sources_current;
    lv_obj_t *select_music;
    lv_obj_t *select_music_caption;
    lv_obj_t *select_radio;
    lv_obj_t *select_radio_caption;
    lv_obj_t *select_bluetooth;
    lv_obj_t *select_bluetooth_caption;
    lv_obj_t *sources_back;
    lv_obj_t *sources_back_caption;
    lv_obj_t *playerscreen_volume_banner;
    lv_obj_t *playerscreen_volume;
    lv_obj_t *wifiscreen_top_bar;
    lv_obj_t *wifiscreen_brand;
    lv_obj_t *wifiscreen_wifi_connected_off;
    lv_obj_t *wifiscreen_wifi_connected_on;
    lv_obj_t *wifiscreen_ethernet_connected_off;
    lv_obj_t *wifiscreen_ethernet_connected_on;
    lv_obj_t *wifiscreen_bluetooth_connected_off;
    lv_obj_t *wifiscreen_bluetooth_connected_on;
    lv_obj_t *network_heading;
    lv_obj_t *wifi_status;
    lv_obj_t *wifi_network_list;
    lv_obj_t *wifi_scan;
    lv_obj_t *wifi_scan_caption;
    lv_obj_t *wifi_password_input;
    lv_obj_t *wifi_connect;
    lv_obj_t *wifi_connect_caption;
    lv_obj_t *wifi_keyboard;
    lv_obj_t *network_back;
    lv_obj_t *network_back_caption;
    lv_obj_t *wifiscreen_volume_banner;
    lv_obj_t *wifiscreen_volume;
} objects_t;

extern objects_t objects;

void create_screen_main();
void tick_screen_main();

void create_screen_player_screen();
void tick_screen_player_screen();

void create_screen_wifi_screen();
void tick_screen_wifi_screen();

void tick_screen_by_id(enum ScreensEnum screenId);
void tick_screen(int screen_index);

void create_screens();

#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_SCREENS_H*/