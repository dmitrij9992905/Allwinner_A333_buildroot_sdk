#include "panel_canvas.h"
#include "panel_ui.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static panel_ui_action_t press(panel_ui_t *ui, int x, int y)
{
    const panel_pointer_event_t event = {PANEL_POINTER_DOWN, x, y, 1, 0};
    panel_ui_action_t action;

    assert(panel_ui_handle_pointer(ui, &event, &action));
    return action;
}

int main(void)
{
    const rdm_uid_t expected_uid = {{0x12, 0x34, 0, 0, 0, 1}};
    dmx_controller_snapshot_t snapshot;
    panel_ui_action_t action;
    panel_canvas_t canvas;
    panel_ui_t *ui;

    memset(&snapshot, 0, sizeof(snapshot));
    snapshot.red = 255;
    snapshot.brightness = 255;
    snapshot.scene_address = 1;
    snapshot.device_count = 1;
    snapshot.devices[0].uid = expected_uid;
    snapshot.devices[0].dmx_start_address = 10;
    (void)snprintf(snapshot.devices[0].device_label,
                   sizeof(snapshot.devices[0].device_label),
                   "Test fixture");

    ui = panel_ui_create();
    assert(ui != NULL);
    panel_ui_set_snapshot(ui, &snapshot);
    assert(panel_canvas_init(&canvas,
                             PANEL_CANVAS_WIDTH,
                             PANEL_CANVAS_HEIGHT) == 0);

    action = press(ui, 580, 290);
    assert(action.type == PANEL_UI_ACTION_SET_SCENE);
    assert(action.data.scene.brightness >= 126 &&
           action.data.scene.brightness <= 129);

    action = press(ui, 650, 400);
    assert(action.type == PANEL_UI_ACTION_SET_SCENE);
    assert(action.data.scene.start_address == 2);

    action = press(ui, 580, 500);
    assert(action.type == PANEL_UI_ACTION_SET_BLACKOUT);
    assert(action.data.blackout.enabled);

    action = press(ui, 220, 70);
    assert(action.type == PANEL_UI_ACTION_NONE);
    assert(panel_ui_get_view(ui) == PANEL_UI_VIEW_RDM);

    action = press(ui, 100, 190);
    assert(action.type == PANEL_UI_ACTION_RDM_REQUEST_INFO);
    assert(rdm_uid_equal(&action.data.rdm_info.uid, &expected_uid));

    action = press(ui, 600, 410);
    assert(action.type == PANEL_UI_ACTION_RDM_SET_IDENTIFY);
    assert(action.data.rdm_identify.enabled);

    action = press(ui, 560, 520);
    assert(action.type == PANEL_UI_ACTION_NONE);
    action = press(ui, 640, 520);
    assert(action.type == PANEL_UI_ACTION_RDM_SET_ADDRESS);
    assert(action.data.rdm_address.start_address == 11);

    panel_ui_render(ui, &canvas);
    assert(canvas.pixels[0] != 0);

    panel_canvas_destroy(&canvas);
    panel_ui_destroy(ui);
    puts("panel_ui: all tests passed");
    return 0;
}
