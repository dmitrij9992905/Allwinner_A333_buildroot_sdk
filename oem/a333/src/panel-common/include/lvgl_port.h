#ifndef PANEL_COMMON_LVGL_PORT_H
#define PANEL_COMMON_LVGL_PORT_H

#include "lvgl.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef PANEL_DISPLAY_ROTATION
#define PANEL_DISPLAY_ROTATION 90
#endif

#if PANEL_DISPLAY_ROTATION == 0 || PANEL_DISPLAY_ROTATION == 180
#define LVGL_PORT_HORIZONTAL_RESOLUTION 800
#define LVGL_PORT_VERTICAL_RESOLUTION 1280
#elif PANEL_DISPLAY_ROTATION == 90 || PANEL_DISPLAY_ROTATION == 270
#define LVGL_PORT_HORIZONTAL_RESOLUTION 1280
#define LVGL_PORT_VERTICAL_RESOLUTION 800
#else
#error "PANEL_DISPLAY_ROTATION must be 0, 90, 180 or 270"
#endif
#define LVGL_PORT_DEFAULT_DRAW_BUFFER_LINES 48u
#define LVGL_PORT_PATH_SIZE 128u
#define LVGL_PORT_INPUT_NAME_SIZE 128u

typedef struct lvgl_port lvgl_port_t;

typedef struct {
    /* NULL selects /dev/fb0. */
    const char *framebuffer_path;

    /* NULL discovers a direct absolute touchscreen evdev device. */
    const char *input_path;
    bool input_swap_xy;
    bool input_invert_x;
    bool input_invert_y;

    /* Zero selects LVGL_PORT_DEFAULT_DRAW_BUFFER_LINES. */
    uint32_t draw_buffer_lines;

    /* Useful for display-only diagnostics over SSH. False by default. */
    bool allow_missing_input;
} lvgl_port_config_t;

typedef struct {
    int logical_width;
    int logical_height;
    int framebuffer_width;
    int framebuffer_height;
    int framebuffer_bits_per_pixel;
    unsigned int display_rotation;
    uint32_t draw_buffer_lines;
    bool input_available;
    bool owns_lvgl;

    char framebuffer_path[LVGL_PORT_PATH_SIZE];
    char input_path[LVGL_PORT_PATH_SIZE];
    char input_name[LVGL_PORT_INPUT_NAME_SIZE];

    uint64_t flush_count;
    uint64_t presented_frame_count;
    uint64_t input_event_count;
    int last_display_error;
    int last_input_error;
} lvgl_port_info_t;

/* Fill a configuration with the documented defaults. */
void lvgl_port_config_init(lvgl_port_config_t *config);

/*
 * Initialize LVGL when necessary, then register an orientation-aware
 * XRGB8888 display and a pointer input device. This port targets the bundled
 * LVGL 9.5.
 *
 * All LVGL and lvgl_port calls must be made from the same thread.
 */
lvgl_port_t *lvgl_port_init(const lvgl_port_config_t *config,
                            char *error_text,
                            size_t error_text_size);
void lvgl_port_deinit(lvgl_port_t *port);

/*
 * Advance LVGL's monotonic tick and run its timers once. Return value is the
 * number of milliseconds LVGL recommends waiting before the next call.
 */
uint32_t lvgl_port_process(lvgl_port_t *port);

void lvgl_port_get_info(const lvgl_port_t *port, lvgl_port_info_t *info);
lv_display_t *lvgl_port_display(const lvgl_port_t *port);
lv_indev_t *lvgl_port_input(const lvgl_port_t *port);

#ifdef __cplusplus
}
#endif

#endif
