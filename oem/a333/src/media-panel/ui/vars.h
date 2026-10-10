#ifndef EEZ_LVGL_UI_VARS_H
#define EEZ_LVGL_UI_VARS_H

#include <stdint.h>
#include <stdbool.h>
#ifdef __cplusplus
#include "eez-flow.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

// enum declarations

// Flow global variables

enum FlowGlobalVariables {
    FLOW_GLOBAL_VARIABLE_WIFI_NETWORKS = 0
};

// Native global variables

extern const char *get_var_media_source();
extern void set_var_media_source(const char *value);
extern const char *get_var_track_title();
extern void set_var_track_title(const char *value);
extern const char *get_var_track_artist();
extern void set_var_track_artist(const char *value);
extern const char *get_var_track_album();
extern void set_var_track_album(const char *value);
extern const char *get_var_volume_text();
extern void set_var_volume_text(const char *value);
extern bool get_var_volume_visible();
extern void set_var_volume_visible(bool value);
extern const char *get_var_network_info();
extern void set_var_network_info(const char *value);
extern bool get_var_wifi_connected();
extern void set_var_wifi_connected(bool value);
extern bool get_var_ethernet_connected();
extern void set_var_ethernet_connected(bool value);
extern bool get_var_bluetooth_connected();
extern void set_var_bluetooth_connected(bool value);
extern bool get_var_wifi_busy();
extern void set_var_wifi_busy(bool value);
extern const char *get_var_wifi_status();
extern void set_var_wifi_status(const char *value);
extern int32_t get_var_wifi_selected();
extern void set_var_wifi_selected(int32_t value);
extern const char *get_var_wifi_password();
extern void set_var_wifi_password(const char *value);
extern const char *get_var_system_datetime();
extern void set_var_system_datetime(const char *value);
extern const char *get_var_notify_text();
extern void set_var_notify_text(const char *value);
extern const char *get_var_volume_value_str();
extern void set_var_volume_value_str(const char *value);
extern const char *get_var_track_filename();
extern void set_var_track_filename(const char *value);
extern int32_t get_var_volume_value();
extern void set_var_volume_value(int32_t value);
extern const char *get_var_track_position_duration();
extern void set_var_track_position_duration(const char *value);
extern int32_t get_var_track_progress();
extern void set_var_track_progress(int32_t value);
extern bool get_var_track_seekable();
extern void set_var_track_seekable(bool value);
extern bool get_var_usb_available();
extern void set_var_usb_available(bool value);
extern bool get_var_sd_available();
extern void set_var_sd_available(bool value);
extern bool get_var_notify_visible();
extern void set_var_notify_visible(bool value);

#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_VARS_H*/