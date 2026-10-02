#ifndef EEZ_LVGL_UI_EVENTS_H
#define EEZ_LVGL_UI_EVENTS_H

#include <lvgl/lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

extern void action_previous(lv_event_t * e);
extern void action_toggle(lv_event_t * e);
extern void action_next(lv_event_t * e);
extern void action_radio(lv_event_t * e);
extern void action_music(lv_event_t * e);
extern void action_pair(lv_event_t * e);
extern void action_wifi_scan(lv_event_t * e);
extern void action_wifi_connect(lv_event_t * e);

#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_EVENTS_H*/