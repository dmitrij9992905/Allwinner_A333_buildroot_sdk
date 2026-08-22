#define _POSIX_C_SOURCE 200809L

#include "dmx_controller.h"
#include "lvgl_fonts.h"
#include "lvgl_port.h"
#include "lvgl_ui.h"

#include <errno.h>
#include <fcntl.h>
#include <linux/kd.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <time.h>
#include <unistd.h>

#ifndef DMX_PANEL_VERSION
#define DMX_PANEL_VERSION "development"
#endif

#define UI_REFRESH_INTERVAL_MS UINT64_C(100)
#define UI_MAX_SLEEP_MS 20u

typedef struct {
    const char *serial_path;
    const char *framebuffer_path;
    const char *input_path;
    rdm_uid_t controller_uid;
    dmx_rs485_mode_t rs485_mode;
    dmx_break_mode_t break_mode;
    bool simulate;
    bool swap_xy;
    bool invert_x;
    bool invert_y;
    bool allow_missing_input;
} application_options_t;

typedef struct {
    int fd;
    int previous_mode;
    bool changed;
} console_guard_t;

static volatile sig_atomic_t stop_requested;

static void signal_handler(int signal_number)
{
    (void)signal_number;
    stop_requested = 1;
}

static uint64_t monotonic_milliseconds(void)
{
    struct timespec time_value;

    if (clock_gettime(CLOCK_MONOTONIC, &time_value) != 0)
        return 0;
    return (uint64_t)time_value.tv_sec * UINT64_C(1000) +
           (uint64_t)time_value.tv_nsec / UINT64_C(1000000);
}

static void sleep_milliseconds(uint32_t milliseconds)
{
    struct timespec delay;

    if (milliseconds == 0)
        milliseconds = 1;
    if (milliseconds > UI_MAX_SLEEP_MS)
        milliseconds = UI_MAX_SLEEP_MS;
    delay.tv_sec = (time_t)(milliseconds / 1000u);
    delay.tv_nsec = (long)(milliseconds % 1000u) * 1000000L;
    while (nanosleep(&delay, &delay) < 0 && errno == EINTR && !stop_requested) {
        /* Continue the remaining delay unless a termination signal arrived. */
    }
}

static void print_usage(FILE *stream, const char *program)
{
    fprintf(stream,
            "Usage: %s [options]\n"
            "\n"
            "Allwinner A333 LVGL DMX512/RDM touch controller.\n"
            "\n"
            "Options:\n"
            "  --serial PATH          DMX/RDM UART (default /dev/ttyS2)\n"
            "  --framebuffer PATH     framebuffer (default /dev/fb0)\n"
            "  --input PATH           evdev node; 'auto' finds touchscreen (default)\n"
            "  --controller-uid UID   RDM UID as MMMM:DDDDDDDD\n"
            "                         (default 7FF0:4C465831, prototype only)\n"
            "  --rs485 auto|rts       external auto-direction or kernel RTS\n"
            "                         (default auto for the Panel 86 circuit)\n"
            "  --break ioctl|baud     DMX/RDM break generator (default ioctl)\n"
            "  --swap-xy              swap touchscreen axes\n"
            "  --invert-x             invert touchscreen X axis\n"
            "  --invert-y             invert touchscreen Y axis\n"
            "  --allow-missing-input  continue when no touchscreen is present\n"
            "  --simulate             use two virtual RDM fixtures, no UART\n"
            "  --help                 show this help\n",
            program);
}

static bool take_value(int argc,
                       char **argv,
                       int *index,
                       const char *option,
                       const char **value)
{
    if (*index + 1 >= argc) {
        fprintf(stderr, "%s requires a value\n", option);
        return false;
    }
    ++*index;
    *value = argv[*index];
    return true;
}

/* Returns 1 for --help, zero for success, and -1 for invalid arguments. */
static int parse_options(int argc,
                         char **argv,
                         application_options_t *options)
{
    int index;

    memset(options, 0, sizeof(*options));
    options->serial_path = "/dev/ttyS2";
    options->framebuffer_path = "/dev/fb0";
    options->controller_uid =
        rdm_uid_from_u64(UINT64_C(0x7ff04c465831));
    options->rs485_mode = DMX_RS485_AUTO_DIRECTION;
    options->break_mode = DMX_BREAK_IOCTL;

    for (index = 1; index < argc; ++index) {
        const char *argument = argv[index];
        const char *value;

        if (strcmp(argument, "--help") == 0 || strcmp(argument, "-h") == 0)
            return 1;
        if (strcmp(argument, "--simulate") == 0) {
            options->simulate = true;
            continue;
        }
        if (strcmp(argument, "--swap-xy") == 0) {
            options->swap_xy = true;
            continue;
        }
        if (strcmp(argument, "--invert-x") == 0) {
            options->invert_x = true;
            continue;
        }
        if (strcmp(argument, "--invert-y") == 0) {
            options->invert_y = true;
            continue;
        }
        if (strcmp(argument, "--allow-missing-input") == 0) {
            options->allow_missing_input = true;
            continue;
        }
        if (strcmp(argument, "--serial") == 0 ||
            strcmp(argument, "--framebuffer") == 0 ||
            strcmp(argument, "--input") == 0) {
            if (!take_value(argc, argv, &index, argument, &value))
                return -1;
            if (strcmp(argument, "--serial") == 0)
                options->serial_path = value;
            else if (strcmp(argument, "--framebuffer") == 0)
                options->framebuffer_path = value;
            else
                options->input_path = strcmp(value, "auto") == 0 ? NULL : value;
            continue;
        }
        if (strcmp(argument, "--controller-uid") == 0) {
            if (!take_value(argc, argv, &index, argument, &value))
                return -1;
            if (!rdm_uid_parse(value, &options->controller_uid)) {
                fprintf(stderr,
                        "invalid controller UID '%s' (expected MMMM:DDDDDDDD)\n",
                        value);
                return -1;
            }
            continue;
        }
        if (strcmp(argument, "--rs485") == 0) {
            if (!take_value(argc, argv, &index, argument, &value))
                return -1;
            if (strcmp(value, "auto") == 0)
                options->rs485_mode = DMX_RS485_AUTO_DIRECTION;
            else if (strcmp(value, "rts") == 0)
                options->rs485_mode = DMX_RS485_KERNEL_RTS;
            else {
                fprintf(stderr, "invalid --rs485 value '%s'\n", value);
                return -1;
            }
            continue;
        }
        if (strcmp(argument, "--break") == 0) {
            if (!take_value(argc, argv, &index, argument, &value))
                return -1;
            if (strcmp(value, "ioctl") == 0)
                options->break_mode = DMX_BREAK_IOCTL;
            else if (strcmp(value, "baud") == 0)
                options->break_mode = DMX_BREAK_BAUD;
            else {
                fprintf(stderr, "invalid --break value '%s'\n", value);
                return -1;
            }
            continue;
        }

        fprintf(stderr, "unknown option '%s'\n", argument);
        return -1;
    }
    return 0;
}

static console_guard_t enter_graphics_mode(void)
{
    console_guard_t guard = {-1, KD_TEXT, false};

    guard.fd = open("/dev/tty0", O_RDWR | O_CLOEXEC);
    if (guard.fd < 0)
        return guard;
    if (ioctl(guard.fd, KDGETMODE, &guard.previous_mode) < 0) {
        (void)close(guard.fd);
        guard.fd = -1;
        return guard;
    }
    if (guard.previous_mode != KD_GRAPHICS) {
        if (ioctl(guard.fd, KDSETMODE, KD_GRAPHICS) < 0) {
            (void)close(guard.fd);
            guard.fd = -1;
            return guard;
        }
        guard.changed = true;
    }
    return guard;
}

static void leave_graphics_mode(console_guard_t *guard)
{
    if (guard->fd < 0)
        return;
    if (guard->changed)
        (void)ioctl(guard->fd, KDSETMODE, guard->previous_mode);
    (void)close(guard->fd);
    guard->fd = -1;
}

static void install_signal_handlers(void)
{
    struct sigaction action;

    memset(&action, 0, sizeof(action));
    action.sa_handler = signal_handler;
    (void)sigemptyset(&action.sa_mask);
    (void)sigaction(SIGINT, &action, NULL);
    (void)sigaction(SIGTERM, &action, NULL);
    (void)sigaction(SIGHUP, &action, NULL);
}

static void dispatch_action(dmx_controller_t *controller,
                            const panel_ui_action_t *action)
{
    bool accepted = true;

    switch (action->type) {
    case PANEL_UI_ACTION_NONE:
        return;
    case PANEL_UI_ACTION_SET_SCENE:
        dmx_controller_set_scene(controller,
                                 action->data.scene.start_address,
                                 action->data.scene.red,
                                 action->data.scene.green,
                                 action->data.scene.blue,
                                 action->data.scene.brightness);
        return;
    case PANEL_UI_ACTION_SET_BLACKOUT:
        dmx_controller_set_blackout(controller,
                                    action->data.blackout.enabled);
        return;
    case PANEL_UI_ACTION_RDM_DISCOVER:
        accepted = dmx_controller_discover(controller);
        break;
    case PANEL_UI_ACTION_RDM_REQUEST_INFO:
        accepted = dmx_controller_request_device_info(
            controller, &action->data.rdm_info.uid);
        break;
    case PANEL_UI_ACTION_RDM_SET_IDENTIFY:
        accepted = dmx_controller_set_identify(
            controller,
            &action->data.rdm_identify.uid,
            action->data.rdm_identify.enabled);
        break;
    case PANEL_UI_ACTION_RDM_SET_ADDRESS:
        accepted = dmx_controller_set_start_address(
            controller,
            &action->data.rdm_address.uid,
            action->data.rdm_address.start_address);
        break;
    }

    if (!accepted)
        fprintf(stderr, "RDM command was not queued (controller busy/stopped)\n");
}

static int run_event_loop(lvgl_port_t *port,
                          dmx_lvgl_ui_t *ui,
                          dmx_controller_t *controller)
{
    uint64_t rendered_generation = UINT64_MAX;
    uint64_t last_snapshot_ms = 0;

    while (!stop_requested) {
        dmx_controller_snapshot_t snapshot;
        panel_ui_action_t action;
        lvgl_port_info_t port_info;
        uint64_t now = monotonic_milliseconds();
        uint32_t wait_ms;

        dmx_controller_get_snapshot(controller, &snapshot);
        if (snapshot.generation != rendered_generation &&
            (last_snapshot_ms == 0 ||
             now - last_snapshot_ms >= UI_REFRESH_INTERVAL_MS)) {
            dmx_lvgl_ui_set_snapshot(ui, &snapshot);
            rendered_generation = snapshot.generation;
            last_snapshot_ms = now;
        }

        wait_ms = lvgl_port_process(port);
        while (dmx_lvgl_ui_pop_action(ui, &action))
            dispatch_action(controller, &action);

        lvgl_port_get_info(port, &port_info);
        if (port_info.last_display_error != 0) {
            fprintf(stderr,
                    "present %s: %s\n",
                    port_info.framebuffer_path,
                    strerror(port_info.last_display_error));
            return -1;
        }
        if (port_info.input_available && port_info.last_input_error != 0) {
            fprintf(stderr,
                    "read touchscreen %s: %s\n",
                    port_info.input_path,
                    strerror(port_info.last_input_error));
            return -1;
        }
        sleep_milliseconds(wait_ms);
    }
    return 0;
}

int main(int argc, char **argv)
{
    application_options_t options;
    dmx_controller_config_t controller_config;
    lvgl_port_config_t port_config;
    dmx_lvgl_ui_config_t ui_config;
    dmx_controller_snapshot_t snapshot;
    dmx_lvgl_fonts_t fonts = {0};
    dmx_lvgl_ui_fonts_t ui_fonts;
    lvgl_port_info_t port_info;
    char device_error[256];
    char uid_text[14];
    lvgl_port_t *port = NULL;
    dmx_lvgl_ui_t *ui = NULL;
    dmx_controller_t *controller = NULL;
    console_guard_t console = {-1, KD_TEXT, false};
    int parse_result;
    int result = EXIT_FAILURE;

    if (argc == 2 && strcmp(argv[1], "--version") == 0) {
        printf("dmx-panel %s\n", DMX_PANEL_VERSION);
        return EXIT_SUCCESS;
    }

    parse_result = parse_options(argc, argv, &options);
    if (parse_result != 0) {
        print_usage(parse_result > 0 ? stdout : stderr, argv[0]);
        return parse_result > 0 ? EXIT_SUCCESS : EXIT_FAILURE;
    }

    install_signal_handlers();
    console = enter_graphics_mode();

    lvgl_port_config_init(&port_config);
    port_config.framebuffer_path = options.framebuffer_path;
    port_config.input_path = options.input_path;
    port_config.input_swap_xy = options.swap_xy;
    port_config.input_invert_x = options.invert_x;
    port_config.input_invert_y = options.invert_y;
    port_config.allow_missing_input = options.allow_missing_input;
    port = lvgl_port_init(&port_config, device_error, sizeof(device_error));
    if (port == NULL) {
        fprintf(stderr, "%s\n", device_error);
        goto cleanup;
    }
    if (!dmx_lvgl_fonts_init(&fonts, device_error, sizeof(device_error))) {
        fprintf(stderr, "%s\n", device_error);
        goto cleanup;
    }

    ui_fonts = dmx_lvgl_fonts_ui(&fonts);
    memset(&ui_config, 0, sizeof(ui_config));
    ui_config.display = lvgl_port_display(port);
    ui_config.fonts = ui_fonts;
    ui_config.install_theme = true;
    ui = dmx_lvgl_ui_create(&ui_config);
    if (ui == NULL) {
        fprintf(stderr, "LVGL UI initialization failed\n");
        goto cleanup;
    }

    memset(&controller_config, 0, sizeof(controller_config));
    controller_config.serial_path = options.serial_path;
    controller_config.controller_uid = options.controller_uid;
    controller_config.rs485_mode = options.rs485_mode;
    controller_config.break_mode = options.break_mode;
    controller_config.simulate = options.simulate;
    controller = dmx_controller_create(&controller_config);
    if (controller == NULL) {
        fprintf(stderr, "controller: out of memory\n");
        goto cleanup;
    }
    if (dmx_controller_start(controller) != 0) {
        dmx_controller_get_snapshot(controller, &snapshot);
        fprintf(stderr, "%s\n", snapshot.status);
        goto cleanup;
    }

    lvgl_port_get_info(port, &port_info);
    rdm_uid_format(&options.controller_uid, uid_text);
    printf("dmx-panel version: %s\n", DMX_PANEL_VERSION);
    printf("LVGL framebuffer: %s, %dx%dx%d, rotation %u, %u draw lines\n",
           port_info.framebuffer_path,
           port_info.framebuffer_width,
           port_info.framebuffer_height,
           port_info.framebuffer_bits_per_pixel,
           port_info.display_rotation,
           port_info.draw_buffer_lines);
    printf("touchscreen: %s (%s)\n",
           port_info.input_path,
           port_info.input_name);
    printf("font: embedded Roboto Regular 2.138\n");
    printf("DMX/RDM: %s, controller UID %s%s\n",
           options.simulate ? "simulation" : options.serial_path,
           uid_text,
           options.simulate ? "" : ", press Ctrl-C to stop");
    (void)fflush(stdout);

    if (run_event_loop(port, ui, controller) == 0)
        result = EXIT_SUCCESS;

cleanup:
    dmx_controller_destroy(controller);
    dmx_lvgl_ui_destroy(ui);
    dmx_lvgl_fonts_destroy(&fonts);
    lvgl_port_deinit(port);
    leave_graphics_mode(&console);
    return result;
}
