#include "panel_canvas.h"
#include "panel_ui.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void populate_preview(dmx_controller_snapshot_t *snapshot)
{
    memset(snapshot, 0, sizeof(*snapshot));
    snapshot->worker_running = true;
    snapshot->serial_open = true;
    snapshot->red = 80;
    snapshot->green = 145;
    snapshot->blue = 255;
    snapshot->brightness = 204;
    snapshot->scene_address = 1;
    snapshot->dmx_frames_sent = 48231;
    snapshot->rdm_transactions = 17;
    snapshot->device_count = 2;
    (void)snprintf(snapshot->status,
                   sizeof(snapshot->status),
                   "DMX RUNNING ON /DEV/TTYS4 - 2 RDM DEVICES");

    snapshot->devices[0].uid =
        (rdm_uid_t){{0x7a, 0x70, 0x10, 0x20, 0x30, 0x40}};
    snapshot->devices[0].information_valid = true;
    snapshot->devices[0].identify_known = true;
    snapshot->devices[0].protocol_version = 0x0100;
    snapshot->devices[0].device_model_id = 0x0102;
    snapshot->devices[0].product_category = 0x0101;
    snapshot->devices[0].dmx_footprint = 8;
    snapshot->devices[0].current_personality = 1;
    snapshot->devices[0].personality_count = 2;
    snapshot->devices[0].dmx_start_address = 1;
    (void)snprintf(snapshot->devices[0].manufacturer_label,
                   sizeof(snapshot->devices[0].manufacturer_label),
                   "Example Lighting");
    (void)snprintf(snapshot->devices[0].model_description,
                   sizeof(snapshot->devices[0].model_description),
                   "RGBW Wash 8");
    (void)snprintf(snapshot->devices[0].device_label,
                   sizeof(snapshot->devices[0].device_label),
                   "Front Wash");
    (void)snprintf(snapshot->devices[0].software_version_label,
                   sizeof(snapshot->devices[0].software_version_label),
                   "2.4.1");

    snapshot->devices[1] = snapshot->devices[0];
    snapshot->devices[1].uid =
        (rdm_uid_t){{0x7a, 0x70, 0x10, 0x20, 0x30, 0x41}};
    snapshot->devices[1].dmx_start_address = 101;
    (void)snprintf(snapshot->devices[1].device_label,
                   sizeof(snapshot->devices[1].device_label),
                   "Back Wash");
}

int main(int argc, char **argv)
{
    const char *scene_path = argc > 1 ? argv[1] : "scene.ppm";
    const char *rdm_path = argc > 2 ? argv[2] : "rdm.ppm";
    dmx_controller_snapshot_t snapshot;
    panel_canvas_t canvas;
    panel_ui_t *ui;

    if (panel_canvas_init(&canvas,
                          PANEL_CANVAS_WIDTH,
                          PANEL_CANVAS_HEIGHT) < 0) {
        fprintf(stderr, "canvas: %s\n", strerror(errno));
        return EXIT_FAILURE;
    }
    ui = panel_ui_create();
    if (ui == NULL) {
        fprintf(stderr, "ui: %s\n", strerror(errno));
        panel_canvas_destroy(&canvas);
        return EXIT_FAILURE;
    }

    populate_preview(&snapshot);
    panel_ui_set_snapshot(ui, &snapshot);
    panel_ui_set_view(ui, PANEL_UI_VIEW_SCENE);
    panel_ui_render(ui, &canvas);
    if (panel_canvas_write_ppm(&canvas, scene_path) < 0) {
        fprintf(stderr, "%s: %s\n", scene_path, strerror(errno));
        panel_ui_destroy(ui);
        panel_canvas_destroy(&canvas);
        return EXIT_FAILURE;
    }

    panel_ui_set_view(ui, PANEL_UI_VIEW_RDM);
    panel_ui_render(ui, &canvas);
    if (panel_canvas_write_ppm(&canvas, rdm_path) < 0) {
        fprintf(stderr, "%s: %s\n", rdm_path, strerror(errno));
        panel_ui_destroy(ui);
        panel_canvas_destroy(&canvas);
        return EXIT_FAILURE;
    }

    printf("wrote %s and %s\n", scene_path, rdm_path);
    panel_ui_destroy(ui);
    panel_canvas_destroy(&canvas);
    return EXIT_SUCCESS;
}
