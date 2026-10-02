#define _POSIX_C_SOURCE 200809L

#include "lvgl_fonts.h"
#include "lvgl_port.h"
#include "media_ui.h"

#include <fcntl.h>
#include <errno.h>
#include <linux/kd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <time.h>
#include <unistd.h>

#define STATUS_FILE "/run/a333-media/status"
#define CONTROL_SOCKET "/run/a333-media/control.sock"

#ifndef MEDIA_PANEL_LOGGING
#define MEDIA_PANEL_LOGGING 1
#endif
#if MEDIA_PANEL_LOGGING
#define MEDIA_LOG(...) do { fprintf(stderr, "media-panel: " __VA_ARGS__); } while (0)
#else
#define MEDIA_LOG(...) ((void)0)
#endif

static volatile sig_atomic_t stopped;
static double monotonic_seconds(void)
{
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return (double)now.tv_sec + (double)now.tv_nsec / 1000000000.0;
}

static void stop_signal(int signum)
{
    (void)signum;
    stopped = 1;
}

int main(void)
{
    setvbuf(stderr, NULL, _IONBF, 0);
    setvbuf(stdout, NULL, _IOLBF, 0);
    MEDIA_LOG("starting (rotation=%d, status=%s, control=%s)\n",
              PANEL_DISPLAY_ROTATION, STATUS_FILE, CONTROL_SOCKET);
    lvgl_port_config_t config;
    panel_lvgl_fonts_t fonts = {0};
    char error[256];
    int tty = open("/dev/tty0", O_RDWR | O_CLOEXEC);
    if (tty >= 0)
        (void)ioctl(tty, KDSETMODE, KD_GRAPHICS);
    signal(SIGINT, stop_signal);
    signal(SIGTERM, stop_signal);
    lvgl_port_config_init(&config);
    config.allow_missing_input = true;
    lvgl_port_t *port = lvgl_port_init(&config, error, sizeof(error));
    if (port == NULL) {
        fprintf(stderr, "media-panel: %s\n", error);
        return 1;
    }
    if (!panel_lvgl_fonts_init_sizes(&fonts, 24, 32, 40, error, sizeof(error))) {
        fprintf(stderr, "media-panel: %s\n", error);
        lvgl_port_deinit(port);
        return 1;
    }
    lvgl_port_info_t info;
    lvgl_port_get_info(port, &info);
    MEDIA_LOG("display=%s %dx%d, logical=%dx%d; input=%s (%s)\n",
              info.framebuffer_path, info.framebuffer_width, info.framebuffer_height,
              info.logical_width, info.logical_height,
              info.input_available ? info.input_path : "unavailable", info.input_name);

    media_ui_init(&fonts);
    while (!stopped) {
        media_ui_tick(monotonic_seconds());
        uint32_t delay_ms = lvgl_port_process(port);
        if (delay_ms < 5)
            delay_ms = 5;
        if (delay_ms > 10)
            delay_ms = 10;
        struct timespec pause = {.tv_sec = 0,
                                 .tv_nsec = (long)delay_ms * 1000000L};
        nanosleep(&pause, NULL);
    }
    panel_lvgl_fonts_destroy(&fonts);
    lvgl_port_deinit(port);
    if (tty >= 0) {
        (void)ioctl(tty, KDSETMODE, KD_TEXT);
        close(tty);
    }
    MEDIA_LOG("stopped\n");
    return 0;
}
