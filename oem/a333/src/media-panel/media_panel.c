#define _POSIX_C_SOURCE 200809L

#include "lvgl_fonts.h"
#include "lvgl_port.h"

#include <fcntl.h>
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

static volatile sig_atomic_t stopped;
static lv_obj_t *source_label;
static lv_obj_t *title_label;
static lv_obj_t *artist_label;
static lv_obj_t *album_label;
static lv_obj_t *volume_banner;
static lv_obj_t *volume_label;
static unsigned long volume_sequence;
static double volume_notice_until;

static double monotonic_seconds(void)
{
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return (double)now.tv_sec + (double)now.tv_nsec / 1000000000.0;
}
static lv_style_transition_dsc_t button_transition;
static const lv_style_prop_t button_transition_props[] = {
    LV_STYLE_BG_COLOR, 0
};

static void stop_signal(int signum)
{
    (void)signum;
    stopped = 1;
}

static void send_command(lv_event_t *event)
{
    const char *command = lv_event_get_user_data(event);
    struct sockaddr_un address = {.sun_family = AF_UNIX};
    int sock = socket(AF_UNIX, SOCK_DGRAM | SOCK_CLOEXEC, 0);
    if (sock < 0)
        return;
    (void)snprintf(address.sun_path, sizeof(address.sun_path), "%s", CONTROL_SOCKET);
    (void)sendto(sock, command, strlen(command), 0,
                 (struct sockaddr *)&address, sizeof(address));
    close(sock);
}

static lv_obj_t *make_button(lv_obj_t *parent, const char *caption,
                              const char *command, lv_font_t *font)
{
    lv_obj_t *button = lv_button_create(parent);
    lv_obj_set_size(button, 170, 78);
    lv_obj_set_style_bg_color(button, lv_color_hex(0x245578), 0);
    lv_obj_set_style_bg_color(button, lv_color_hex(0x4094c8), LV_STATE_PRESSED);
    lv_obj_set_style_transition(button, &button_transition, 0);
    lv_obj_add_event_cb(button, send_command, LV_EVENT_CLICKED, (void *)command);
    lv_obj_t *label = lv_label_create(button);
    lv_label_set_text(label, caption);
    lv_obj_set_style_text_font(label, font, 0);
    lv_obj_center(label);
    return button;
}

static void read_status(void)
{
    char buffer[6][256] = {{0}};
    lv_obj_t *labels[] = {source_label, title_label, artist_label, album_label};
    FILE *file = fopen(STATUS_FILE, "r");
    if (file == NULL)
        return;
    for (size_t i = 0; i < 4; ++i) {
        if (fgets(buffer[i], sizeof(buffer[i]), file) == NULL)
            break;
        buffer[i][strcspn(buffer[i], "\r\n")] = '\0';
        if (strcmp(lv_label_get_text(labels[i]), buffer[i]) != 0)
            lv_label_set_text(labels[i], buffer[i]);
    }
    unsigned long sequence = volume_sequence;
    if (fgets(buffer[4], sizeof(buffer[4]), file) != NULL) {
        buffer[4][strcspn(buffer[4], "\r\n")] = '\0';
    }
    if (fgets(buffer[5], sizeof(buffer[5]), file) != NULL) {
        sequence = strtoul(buffer[5], NULL, 10);
    }
    if (buffer[4][0] != '\0' && sequence != volume_sequence) {
        volume_notice_until = monotonic_seconds() + 5.0;
        lv_label_set_text(volume_label, buffer[4]);
        lv_obj_remove_flag(volume_banner, LV_OBJ_FLAG_HIDDEN);
    } else if (buffer[4][0] == '\0' && monotonic_seconds() >= volume_notice_until) {
        if (lv_label_get_text(volume_label)[0] != '\0')
            lv_label_set_text(volume_label, "");
        lv_obj_add_flag(volume_banner, LV_OBJ_FLAG_HIDDEN);
    } else if (buffer[4][0] != '\0' &&
               strcmp(lv_label_get_text(volume_label), buffer[4]) != 0) {
        lv_label_set_text(volume_label, buffer[4]);
    }
    volume_sequence = sequence;
    fclose(file);
}

int main(void)
{
    lvgl_port_config_t config;
    dmx_lvgl_fonts_t fonts = {0};
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
    if (!dmx_lvgl_fonts_init(&fonts, error, sizeof(error))) {
        fprintf(stderr, "media-panel: %s\n", error);
        lvgl_port_deinit(port);
        return 1;
    }

    lv_obj_t *screen = lv_screen_active();
    lv_style_transition_dsc_init(&button_transition, button_transition_props,
                                 lv_anim_path_ease_out, 120, 0, NULL);
    lv_obj_set_style_bg_color(screen, lv_color_hex(0x101820), 0);
    lv_obj_set_style_text_color(screen, lv_color_hex(0xf5f8ff), 0);
    source_label = lv_label_create(screen);
    lv_obj_set_style_text_font(source_label, fonts.normal, 0);
    lv_obj_set_pos(source_label, 50, 35);
    lv_label_set_text(source_label, "A333 Media");
    title_label = lv_label_create(screen);
    lv_obj_set_style_text_font(title_label, fonts.large, 0);
    lv_obj_set_width(title_label, 1120);
    lv_label_set_long_mode(title_label, LV_LABEL_LONG_MODE_DOTS);
    lv_obj_set_pos(title_label, 50, 150);
    lv_label_set_text(title_label, "Ready");
    artist_label = lv_label_create(screen);
    lv_obj_set_style_text_font(artist_label, fonts.normal, 0);
    lv_obj_set_pos(artist_label, 50, 235);
    lv_label_set_text(artist_label, "Pair a phone or select music");
    album_label = lv_label_create(screen);
    lv_obj_set_style_text_font(album_label, fonts.normal, 0);
    lv_obj_set_pos(album_label, 50, 285);
    lv_label_set_text(album_label, "");

    volume_banner = lv_obj_create(screen);
    lv_obj_set_size(volume_banner, 420, 66);
    lv_obj_align(volume_banner, LV_ALIGN_TOP_MID, 0, 12);
    lv_obj_set_style_bg_color(volume_banner, lv_color_hex(0x245578), 0);
    lv_obj_set_style_bg_opa(volume_banner, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(volume_banner, 0, 0);
    lv_obj_set_style_radius(volume_banner, 18, 0);
    lv_obj_set_style_pad_all(volume_banner, 8, 0);
    lv_obj_remove_flag(volume_banner, LV_OBJ_FLAG_SCROLLABLE);
    volume_label = lv_label_create(volume_banner);
    lv_obj_set_style_text_font(volume_label, fonts.normal, 0);
    lv_obj_center(volume_label);
    lv_label_set_text(volume_label, "");
    lv_obj_add_flag(volume_banner, LV_OBJ_FLAG_HIDDEN);

    lv_obj_t *controls = lv_obj_create(screen);
    lv_obj_set_size(controls, 1150, 140);
    lv_obj_set_pos(controls, 50, 570);
    lv_obj_set_flex_flow(controls, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(controls, LV_FLEX_ALIGN_SPACE_EVENLY,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    make_button(controls, "Previous", "previous", fonts.normal);
    make_button(controls, "Play / Pause", "toggle", fonts.normal);
    make_button(controls, "Next", "next", fonts.normal);
    make_button(controls, "Radio", "radio", fonts.normal);
    make_button(controls, "Music", "music", fonts.normal);
    make_button(controls, "Pair BT", "pair", fonts.normal);

    unsigned int iterations = 0;
    while (!stopped) {
        if (++iterations % 20 == 0)
            read_status();
        uint32_t delay_ms = lvgl_port_process(port);
        if (delay_ms < 5)
            delay_ms = 5;
        if (delay_ms > 10)
            delay_ms = 10;
        struct timespec pause = {.tv_sec = 0,
                                 .tv_nsec = (long)delay_ms * 1000000L};
        nanosleep(&pause, NULL);
    }
    dmx_lvgl_fonts_destroy(&fonts);
    lvgl_port_deinit(port);
    if (tty >= 0) {
        (void)ioctl(tty, KDSETMODE, KD_TEXT);
        close(tty);
    }
    return 0;
}
