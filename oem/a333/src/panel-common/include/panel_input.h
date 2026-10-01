#ifndef DMX_PANEL_INPUT_H
#define DMX_PANEL_INPUT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    PANEL_POINTER_DOWN = 0,
    PANEL_POINTER_MOVE,
    PANEL_POINTER_UP,
} panel_pointer_event_type_t;

typedef struct {
    panel_pointer_event_type_t type;
    int x;
    int y;
    int pressure;
    uint64_t timestamp_ms;
} panel_pointer_event_t;

typedef struct {
    /* NULL means auto-discover a direct absolute touchscreen evdev node. */
    const char *device_path;
    int canvas_width;
    int canvas_height;
    unsigned int display_rotation;
    bool swap_xy;
    bool invert_x;
    bool invert_y;
} panel_input_config_t;

typedef struct panel_input panel_input_t;

int panel_input_find_goodix(char *path, size_t path_size);
panel_input_t *panel_input_open(const panel_input_config_t *config,
                               char *error_text,
                               size_t error_text_size);
void panel_input_close(panel_input_t *input);

int panel_input_fd(const panel_input_t *input);
const char *panel_input_path(const panel_input_t *input);
const char *panel_input_name(const panel_input_t *input);

/*
 * Drains available evdev records without blocking. Returns pointer-event
 * count, or -errno. DOWN/MOVE/UP are emitted only at EV_SYN/SYN_REPORT.
 */
int panel_input_read(panel_input_t *input,
                     panel_pointer_event_t *events,
                     size_t capacity);

#ifdef __cplusplus
}
#endif

#endif
