#define _POSIX_C_SOURCE 200809L

#include "lvgl_port.h"

#include "panel_canvas.h"
#include "panel_fbdev.h"
#include "panel_input.h"

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#if LVGL_VERSION_MAJOR != 9 || LVGL_VERSION_MINOR != 5
#error "dmx_panel LVGL port requires LVGL 9.5"
#endif

#if LV_COLOR_DEPTH != 32
#error "dmx_panel LVGL port requires LV_COLOR_DEPTH == 32"
#endif

#define INPUT_EVENT_QUEUE_CAPACITY 32u

struct lvgl_port {
    panel_fbdev_t *framebuffer;
    panel_input_t *panel_input;
    panel_canvas_t canvas;
    uint8_t *draw_buffer_pixels;
    size_t draw_buffer_size;
    uint32_t draw_buffer_lines;

    lv_display_t *display;
    lv_indev_t *input;

    panel_pointer_event_t input_events[INPUT_EVENT_QUEUE_CAPACITY];
    size_t input_event_index;
    size_t input_event_count;
    int pointer_x;
    int pointer_y;
    lv_indev_state_t pointer_state;

    bool owns_lvgl;
    uint64_t last_tick_ms;
    lvgl_port_info_t info;
};

_Static_assert(sizeof(lv_color32_t) == sizeof(panel_color_t),
               "LVGL XRGB8888 and panel canvas pixels must both be 32 bit");

static void set_error(char *buffer, size_t size, const char *format, ...)
{
    va_list arguments;

    if (buffer == NULL || size == 0)
        return;
    va_start(arguments, format);
    (void)vsnprintf(buffer, size, format, arguments);
    va_end(arguments);
}

static void copy_text(char *destination,
                      size_t destination_size,
                      const char *source)
{
    if (destination == NULL || destination_size == 0)
        return;
    (void)snprintf(destination,
                   destination_size,
                   "%s",
                   source != NULL ? source : "");
}

static uint64_t monotonic_milliseconds(void)
{
    struct timespec value;

    if (clock_gettime(CLOCK_MONOTONIC, &value) < 0)
        return 0;
    return (uint64_t)value.tv_sec * 1000u +
           (uint64_t)value.tv_nsec / 1000000u;
}

static void display_flush(lv_display_t *display,
                          const lv_area_t *area,
                          uint8_t *pixel_map)
{
    lvgl_port_t *port =
        display != NULL ? lv_display_get_user_data(display) : NULL;

    if (port != NULL && area != NULL && pixel_map != NULL) {
        int source_width = (int)area->x2 - (int)area->x1 + 1;
        int source_height = (int)area->y2 - (int)area->y1 + 1;

        ++port->info.flush_count;
        if (source_width > 0 && source_height > 0) {
            uint32_t source_stride =
                lv_draw_buf_width_to_stride((uint32_t)source_width,
                                            LV_COLOR_FORMAT_XRGB8888);
            int left = area->x1 < 0 ? 0 : area->x1;
            int top = area->y1 < 0 ? 0 : area->y1;
            int right = area->x2 >= port->canvas.width
                            ? port->canvas.width - 1
                            : area->x2;
            int bottom = area->y2 >= port->canvas.height
                             ? port->canvas.height - 1
                             : area->y2;
            int y;

            for (y = top; left <= right && top <= bottom && y <= bottom;
                 ++y) {
                const uint32_t *source = (const uint32_t *)(
                    pixel_map + (size_t)(y - (int)area->y1) *
                                    source_stride +
                    (size_t)(left - (int)area->x1) * sizeof(*source));
                panel_color_t *destination =
                    port->canvas.pixels +
                    (size_t)y * port->canvas.stride + (size_t)left;
                int x;

                /* XRGB8888 and panel_color_t are B,G,R,X/A in memory on this
                 * little-endian target. Make the unused byte opaque because
                 * panel_fbdev treats it as alpha during format conversion. */
                for (x = left; x <= right; ++x)
                    *destination++ = *source++ | UINT32_C(0xff000000);
            }
        }

        if (lv_display_flush_is_last(display)) {
            if (panel_fbdev_present(port->framebuffer, &port->canvas) < 0)
                port->info.last_display_error = errno != 0 ? errno : EIO;
            else {
                port->info.last_display_error = 0;
                ++port->info.presented_frame_count;
            }
        }
    }
    if (display != NULL)
        lv_display_flush_ready(display);
}

static bool refill_input_events(lvgl_port_t *port)
{
    int count;

    if (port->panel_input == NULL)
        return false;
    count = panel_input_read(port->panel_input,
                             port->input_events,
                             INPUT_EVENT_QUEUE_CAPACITY);
    if (count < 0) {
        port->info.last_input_error = -count;
        port->input_event_count = 0;
        port->input_event_index = 0;
        port->pointer_state = LV_INDEV_STATE_RELEASED;
        return false;
    }
    port->info.last_input_error = 0;
    port->input_event_index = 0;
    port->input_event_count = (size_t)count;
    return count > 0;
}

static void input_read(lv_indev_t *input, lv_indev_data_t *data)
{
    lvgl_port_t *port =
        input != NULL ? lv_indev_get_user_data(input) : NULL;

    if (data == NULL)
        return;
    if (port == NULL) {
        data->point.x = 0;
        data->point.y = 0;
        data->state = LV_INDEV_STATE_RELEASED;
        data->continue_reading = false;
        return;
    }

    if (port->input_event_index >= port->input_event_count)
        (void)refill_input_events(port);
    if (port->input_event_index < port->input_event_count) {
        const panel_pointer_event_t *event =
            &port->input_events[port->input_event_index++];

        port->pointer_x = event->x;
        port->pointer_y = event->y;
        port->pointer_state = event->type == PANEL_POINTER_UP
                                  ? LV_INDEV_STATE_RELEASED
                                  : LV_INDEV_STATE_PRESSED;
        ++port->info.input_event_count;
    }

    data->point.x = port->pointer_x;
    data->point.y = port->pointer_y;
    data->state = port->pointer_state;
    data->continue_reading =
        port->input_event_index < port->input_event_count;
}

void lvgl_port_config_init(lvgl_port_config_t *config)
{
    if (config == NULL)
        return;
    memset(config, 0, sizeof(*config));
    config->draw_buffer_lines = LVGL_PORT_DEFAULT_DRAW_BUFFER_LINES;
}

static int initialize_devices(lvgl_port_t *port,
                              const lvgl_port_config_t *config,
                              char *error_text,
                              size_t error_text_size)
{
    panel_input_config_t input_config;
    char detail[256];
    int saved_errno;

    port->framebuffer = panel_fbdev_open(config->framebuffer_path,
                                         detail,
                                         sizeof(detail));
    if (port->framebuffer == NULL) {
        saved_errno = errno != 0 ? errno : EIO;
        set_error(error_text, error_text_size, "%s", detail);
        errno = saved_errno;
        return -1;
    }

    memset(&input_config, 0, sizeof(input_config));
    input_config.device_path = config->input_path;
    input_config.canvas_width = LVGL_PORT_HORIZONTAL_RESOLUTION;
    input_config.canvas_height = LVGL_PORT_VERTICAL_RESOLUTION;
    input_config.swap_xy = config->input_swap_xy;
    input_config.invert_x = config->input_invert_x;
    input_config.invert_y = config->input_invert_y;
    port->panel_input = panel_input_open(&input_config,
                                         detail,
                                         sizeof(detail));
    if (port->panel_input == NULL) {
        saved_errno = errno != 0 ? errno : EIO;
        port->info.last_input_error = saved_errno;
        if (!config->allow_missing_input) {
            set_error(error_text, error_text_size, "%s", detail);
            errno = saved_errno;
            return -1;
        }
    }
    return 0;
}

static int initialize_buffers(lvgl_port_t *port,
                              uint32_t requested_lines,
                              char *error_text,
                              size_t error_text_size)
{
    uint32_t stride;

    port->draw_buffer_lines = requested_lines != 0
                                  ? requested_lines
                                  : LVGL_PORT_DEFAULT_DRAW_BUFFER_LINES;
    if (port->draw_buffer_lines > LVGL_PORT_VERTICAL_RESOLUTION)
        port->draw_buffer_lines = LVGL_PORT_VERTICAL_RESOLUTION;
    stride = lv_draw_buf_width_to_stride(LVGL_PORT_HORIZONTAL_RESOLUTION,
                                         LV_COLOR_FORMAT_XRGB8888);
    if (stride == 0 ||
        (size_t)port->draw_buffer_lines > SIZE_MAX / (size_t)stride ||
        (size_t)port->draw_buffer_lines * (size_t)stride > UINT32_MAX) {
        errno = EOVERFLOW;
        set_error(error_text,
                  error_text_size,
                  "invalid LVGL draw-buffer geometry");
        return -1;
    }
    port->draw_buffer_size =
        (size_t)port->draw_buffer_lines * (size_t)stride;
    if (panel_canvas_init(&port->canvas,
                          LVGL_PORT_HORIZONTAL_RESOLUTION,
                          LVGL_PORT_VERTICAL_RESOLUTION) < 0) {
        int saved_errno = errno != 0 ? errno : ENOMEM;

        set_error(error_text,
                  error_text_size,
                  "allocate %dx%d display canvas: %s",
                  LVGL_PORT_HORIZONTAL_RESOLUTION,
                  LVGL_PORT_VERTICAL_RESOLUTION,
                  strerror(saved_errno));
        errno = saved_errno;
        return -1;
    }
    port->draw_buffer_pixels = calloc(1, port->draw_buffer_size);
    if (port->draw_buffer_pixels == NULL) {
        int saved_errno = errno != 0 ? errno : ENOMEM;

        set_error(error_text,
                  error_text_size,
                  "allocate LVGL draw buffer: %s",
                  strerror(saved_errno));
        errno = saved_errno;
        return -1;
    }
    return 0;
}

static int register_lvgl_devices(lvgl_port_t *port,
                                 char *error_text,
                                 size_t error_text_size)
{
    port->display = lv_display_create(LVGL_PORT_HORIZONTAL_RESOLUTION,
                                      LVGL_PORT_VERTICAL_RESOLUTION);
    if (port->display == NULL) {
        errno = ENOMEM;
        set_error(error_text,
                  error_text_size,
                  "LVGL display registration failed");
        return -1;
    }
    lv_display_set_user_data(port->display, port);
    lv_display_set_color_format(port->display, LV_COLOR_FORMAT_XRGB8888);
    lv_display_set_buffers(port->display,
                           port->draw_buffer_pixels,
                           NULL,
                           (uint32_t)port->draw_buffer_size,
                           LV_DISPLAY_RENDER_MODE_PARTIAL);
    lv_display_set_flush_cb(port->display, display_flush);

    if (port->panel_input != NULL) {
        port->input = lv_indev_create();
        if (port->input == NULL) {
            errno = ENOMEM;
            set_error(error_text,
                      error_text_size,
                      "LVGL input registration failed");
            return -1;
        }
        lv_indev_set_user_data(port->input, port);
        lv_indev_set_type(port->input, LV_INDEV_TYPE_POINTER);
        lv_indev_set_display(port->input, port->display);
        lv_indev_set_read_cb(port->input, input_read);
    }
    return 0;
}

static void fill_static_info(lvgl_port_t *port)
{
    port->info.logical_width = LVGL_PORT_HORIZONTAL_RESOLUTION;
    port->info.logical_height = LVGL_PORT_VERTICAL_RESOLUTION;
    port->info.framebuffer_width = panel_fbdev_width(port->framebuffer);
    port->info.framebuffer_height = panel_fbdev_height(port->framebuffer);
    port->info.framebuffer_bits_per_pixel =
        panel_fbdev_bits_per_pixel(port->framebuffer);
    port->info.draw_buffer_lines = port->draw_buffer_lines;
    port->info.input_available = port->panel_input != NULL;
    port->info.owns_lvgl = port->owns_lvgl;
    copy_text(port->info.framebuffer_path,
              sizeof(port->info.framebuffer_path),
              panel_fbdev_path(port->framebuffer));
    if (port->panel_input != NULL) {
        copy_text(port->info.input_path,
                  sizeof(port->info.input_path),
                  panel_input_path(port->panel_input));
        copy_text(port->info.input_name,
                  sizeof(port->info.input_name),
                  panel_input_name(port->panel_input));
    }
}

lvgl_port_t *lvgl_port_init(const lvgl_port_config_t *config,
                            char *error_text,
                            size_t error_text_size)
{
    lvgl_port_config_t defaults;
    const lvgl_port_config_t *effective;
    lvgl_port_t *port;

    lvgl_port_config_init(&defaults);
    effective = config != NULL ? config : &defaults;
    port = calloc(1, sizeof(*port));
    if (port == NULL) {
        set_error(error_text, error_text_size, "out of memory");
        return NULL;
    }
    port->pointer_state = LV_INDEV_STATE_RELEASED;

    if (initialize_devices(port,
                           effective,
                           error_text,
                           error_text_size) < 0) {
        int saved_errno = errno;

        lvgl_port_deinit(port);
        errno = saved_errno;
        return NULL;
    }

    port->owns_lvgl = !lv_is_initialized();
    if (port->owns_lvgl)
        lv_init();
    if (initialize_buffers(port,
                           effective->draw_buffer_lines,
                           error_text,
                           error_text_size) < 0) {
        int saved_errno = errno;

        lvgl_port_deinit(port);
        errno = saved_errno;
        return NULL;
    }
    if (register_lvgl_devices(port, error_text, error_text_size) < 0) {
        int saved_errno = errno;

        lvgl_port_deinit(port);
        errno = saved_errno;
        return NULL;
    }

    port->last_tick_ms = monotonic_milliseconds();
    fill_static_info(port);
    if (error_text != NULL && error_text_size != 0)
        error_text[0] = '\0';
    return port;
}

void lvgl_port_deinit(lvgl_port_t *port)
{
    bool owns_lvgl;

    if (port == NULL)
        return;
    owns_lvgl = port->owns_lvgl;
    if (port->input != NULL)
        lv_indev_delete(port->input);
    if (port->display != NULL)
        lv_display_delete(port->display);
    panel_input_close(port->panel_input);
    panel_fbdev_close(port->framebuffer);
    free(port->draw_buffer_pixels);
    panel_canvas_destroy(&port->canvas);
    free(port);

    if (owns_lvgl && lv_is_initialized())
        lv_deinit();
}

uint32_t lvgl_port_process(lvgl_port_t *port)
{
    uint64_t now;
    uint64_t elapsed;

    if (port == NULL) {
        errno = EINVAL;
        return UINT32_MAX;
    }
    now = monotonic_milliseconds();
    if (now != 0 && port->last_tick_ms != 0 && now >= port->last_tick_ms) {
        elapsed = now - port->last_tick_ms;
        while (elapsed > UINT32_MAX) {
            lv_tick_inc(UINT32_MAX);
            elapsed -= UINT32_MAX;
        }
        if (elapsed != 0)
            lv_tick_inc((uint32_t)elapsed);
        port->last_tick_ms = now;
    } else if (now != 0) {
        port->last_tick_ms = now;
    }
    return lv_timer_handler();
}

void lvgl_port_get_info(const lvgl_port_t *port, lvgl_port_info_t *info)
{
    if (info == NULL)
        return;
    if (port == NULL) {
        memset(info, 0, sizeof(*info));
        return;
    }
    *info = port->info;
}

lv_display_t *lvgl_port_display(const lvgl_port_t *port)
{
    return port != NULL ? port->display : NULL;
}

lv_indev_t *lvgl_port_input(const lvgl_port_t *port)
{
    return port != NULL ? port->input : NULL;
}
