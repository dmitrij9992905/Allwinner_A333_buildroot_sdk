#ifndef DMX_PANEL_LVGL_UI_H
#define DMX_PANEL_LVGL_UI_H

#include "panel_ui.h"

#include <lvgl.h>

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Fonts are supplied by the application as lv_font_t pointers. The target
 * application creates them from its embedded Roboto TTF. */

typedef struct {
    const lv_font_t *small;
    const lv_font_t *normal;
    const lv_font_t *large;
} dmx_lvgl_ui_fonts_t;

typedef struct {
    /* Parent container/screen. NULL selects lv_screen_active(). */
    lv_obj_t *parent;
    /* Display whose theme is changed when install_theme is true. */
    lv_display_t *display;
    dmx_lvgl_ui_fonts_t fonts;
    bool install_theme;
} dmx_lvgl_ui_config_t;

typedef struct dmx_lvgl_ui dmx_lvgl_ui_t;

/*
 * Installs LVGL 9.5's dark default theme using the normal Roboto font. The
 * interface applies the small and large Roboto pointers explicitly to its
 * widgets. All font pointers are mandatory.
 */
bool dmx_lvgl_ui_apply_roboto_theme(lv_display_t *display,
                                    const dmx_lvgl_ui_fonts_t *fonts);

/* Creates the complete Scene/RDM interface. The 720x720 control surface is
 * centered on the A333's 1280x800 display. All three fonts are mandatory. */
dmx_lvgl_ui_t *dmx_lvgl_ui_create(const dmx_lvgl_ui_config_t *config);
void dmx_lvgl_ui_destroy(dmx_lvgl_ui_t *ui);

/*
 * Update widgets from a controller snapshot. Call only from the LVGL thread;
 * the function copies the snapshot and never keeps the caller's pointer.
 */
void dmx_lvgl_ui_set_snapshot(dmx_lvgl_ui_t *ui,
                              const dmx_controller_snapshot_t *snapshot);

/*
 * Controller commands are queued by widget callbacks and consumed here. This
 * keeps LVGL event handling independent from serial/RDM controller work. The
 * action type is the existing panel_ui_action_t used by the fbdev UI.
 */
bool dmx_lvgl_ui_pop_action(dmx_lvgl_ui_t *ui, panel_ui_action_t *action);
size_t dmx_lvgl_ui_pending_actions(const dmx_lvgl_ui_t *ui);

/* Root object, useful to show/hide the complete interface from a port. */
lv_obj_t *dmx_lvgl_ui_root(dmx_lvgl_ui_t *ui);

#ifdef __cplusplus
}
#endif

#endif
