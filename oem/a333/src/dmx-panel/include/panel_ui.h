#ifndef DMX_PANEL_UI_H
#define DMX_PANEL_UI_H

#include "dmx_controller.h"
#include "panel_canvas.h"
#include "panel_input.h"

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    PANEL_UI_VIEW_SCENE = 0,
    PANEL_UI_VIEW_RDM,
} panel_ui_view_t;

typedef enum {
    PANEL_UI_ACTION_NONE = 0,
    PANEL_UI_ACTION_SET_SCENE,
    PANEL_UI_ACTION_SET_BLACKOUT,
    PANEL_UI_ACTION_RDM_DISCOVER,
    PANEL_UI_ACTION_RDM_REQUEST_INFO,
    PANEL_UI_ACTION_RDM_SET_IDENTIFY,
    PANEL_UI_ACTION_RDM_SET_ADDRESS,
} panel_ui_action_type_t;

typedef struct {
    panel_ui_action_type_t type;
    union {
        struct {
            uint16_t start_address;
            uint8_t red;
            uint8_t green;
            uint8_t blue;
            uint8_t brightness;
        } scene;
        struct {
            bool enabled;
        } blackout;
        struct {
            rdm_uid_t uid;
        } rdm_info;
        struct {
            rdm_uid_t uid;
            bool enabled;
        } rdm_identify;
        struct {
            rdm_uid_t uid;
            uint16_t start_address;
        } rdm_address;
    } data;
} panel_ui_action_t;

typedef struct panel_ui panel_ui_t;

panel_ui_t *panel_ui_create(void);
void panel_ui_destroy(panel_ui_t *ui);

void panel_ui_set_snapshot(panel_ui_t *ui,
                           const dmx_controller_snapshot_t *snapshot);
void panel_ui_set_view(panel_ui_t *ui, panel_ui_view_t view);
panel_ui_view_t panel_ui_get_view(const panel_ui_t *ui);

/* Renders the complete fixed 720x720 application canvas. */
void panel_ui_render(panel_ui_t *ui, panel_canvas_t *canvas);

/*
 * Applies a pointer event. True means the UI changed and should be redrawn.
 * action may remain PANEL_UI_ACTION_NONE for local navigation/tab changes.
 */
bool panel_ui_handle_pointer(panel_ui_t *ui,
                             const panel_pointer_event_t *event,
                             panel_ui_action_t *action);

#ifdef __cplusplus
}
#endif

#endif
