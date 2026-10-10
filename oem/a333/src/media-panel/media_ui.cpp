#include "media_ui.h"
#include "media_backend.h"
#include "media_transition.h"
#include "ui/actions.h"
#include "ui/screens.h"
#include "ui/ui.h"
#include "ui/vars.h"
#include "ui/images.h"

#include <cerrno>
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <ctime>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>
#include <vector>

static media_backend_state_t state;
static char network_info[1024] = "Waiting for network information...";
static double next_status, next_network, next_clock;
static char system_datetime[128];
static char volume_value_text[16];
static char track_time_text[64], cover_source[1100], seek_track[64];
static std::string displayed_cover;
static bool seeking;
static int32_t seek_progress;
static unsigned long network_revision;
static int32_t wifi_selected;
static bool wifi_connected, ethernet_connected, bluetooth_connected, wifi_busy;
static char wifi_status[1024] = "Press Scan to find Wi-Fi networks";
static std::string wifi_password;
static eez::Value wifi_networks;
static lv_style_transition_dsc_t button_transition;
static const lv_style_prop_t button_properties[] = {LV_STYLE_BG_COLOR, 0};

static void command(const char *name)
{
    if (!media_backend_send("/run/a333-media/control.sock", name))
        fprintf(stderr, "media-panel: command '%s': %s\n", name, strerror(errno));
#if MEDIA_PANEL_LOGGING
    else
        fprintf(stderr, "media-panel: command '%s' sent via EEZ Flow\n", name);
#endif
}

#define ACTION(name) void action_##name(lv_event_t *event) { (void)event; command(#name); }
ACTION(previous)
ACTION(toggle)
ACTION(next)
ACTION(radio)
ACTION(music)
ACTION(pair)
ACTION(usb)
ACTION(sd)

const char *get_var_track_position_duration()
{
    int position = static_cast<int>(state.position), duration = static_cast<int>(state.duration);
    if (seeking && duration > 0) position = static_cast<int>(state.duration * seek_progress / 10000.0);
    snprintf(track_time_text, sizeof(track_time_text), "%02d:%02d / %02d:%02d",
             position / 60, position % 60, duration / 60, duration % 60);
    return track_time_text;
}
void set_var_track_position_duration(const char *value) { (void)value; }
int32_t get_var_track_progress()
{
    if (seeking) return seek_progress;
    if (state.duration <= 0) return 0;
    return static_cast<int32_t>(std::min(10000.0, 10000.0 * state.position / state.duration));
}
void set_var_track_progress(int32_t value) { seek_progress = std::max(0, std::min(10000, value)); }
bool get_var_track_seekable() { return state.seekable; }
void set_var_track_seekable(bool value) { (void)value; }
bool get_var_usb_available() { return state.usb_available; }
void set_var_usb_available(bool value) { (void)value; }
bool get_var_sd_available() { return state.sd_available; }
void set_var_sd_available(bool value) { (void)value; }

void action_seek(lv_event_t *event)
{
    (void)event;
    if (seeking && state.seekable && !strcmp(seek_track, state.track_id)) {
        char request[256];
        double position = state.duration * lv_slider_get_value(objects.track_progress_slider) / 10000.0;
        snprintf(request, sizeof(request), "{\"command\":\"seek\",\"track\":\"%s\",\"seconds\":%.3f}",
                 seek_track, position);
        media_backend_request("/run/a333-media/control.sock", request);
    }
    seeking = false;
}

static void seek_pressed(lv_event_t *event)
{
    if (lv_event_get_code(event) == LV_EVENT_PRESSED) {
        seeking = state.seekable;
        snprintf(seek_track, sizeof(seek_track), "%s", state.track_id);
        seek_progress = lv_slider_get_value(objects.track_progress_slider);
    } else {
        seeking = false;
    }
}

static void refresh_cover()
{
    if (displayed_cover == state.cover_path) return;
    displayed_cover = state.cover_path;
    if (displayed_cover.empty()) {
        lv_image_set_src(objects.album_cover_image, &img_material_music_note_2);
    } else {
        snprintf(cover_source, sizeof(cover_source), "A:%s", state.cover_path);
        lv_image_set_src(objects.album_cover_image, cover_source);
    }
}

/* Native read-only values. EEZ evaluates bindings only on the UI thread. */
#define TEXT_VAR(name, field) \
    const char *get_var_##name() { return state.field; } \
    void set_var_##name(const char *value) { (void)value; }
TEXT_VAR(media_source, source)
TEXT_VAR(track_title, title)
TEXT_VAR(track_artist, artist)
TEXT_VAR(track_album, album)
TEXT_VAR(volume_text, volume)
TEXT_VAR(track_filename, filename)
const char *get_var_notify_text() { return state.notification; }
void set_var_notify_text(const char *value) { (void)value; }
bool get_var_notify_visible()
{
    // Volume owns the top banner; the ordinary notice's deadline keeps running.
    return state.notification_visible && !state.volume_visible;
}
void set_var_notify_visible(bool value) { (void)value; }
const char *get_var_system_datetime() { return system_datetime; }
void set_var_system_datetime(const char *value) { (void)value; }
int32_t get_var_volume_value()
{
    const char *number = state.volume;
    while (*number && (*number < '0' || *number > '9')) ++number;
    long value = strtol(number, nullptr, 10);
    return value < 0 ? 0 : value > 100 ? 100 : value;
}
void set_var_volume_value(int32_t value) { (void)value; }
const char *get_var_volume_value_str()
{
    snprintf(volume_value_text, sizeof(volume_value_text), "%d%%", get_var_volume_value());
    return volume_value_text;
}
void set_var_volume_value_str(const char *value) { (void)value; }
bool get_var_volume_visible() { return state.volume_visible; }
void set_var_volume_visible(bool value) { (void)value; }
const char *get_var_network_info() { return network_info; }
void set_var_network_info(const char *value) { (void)value; }

#define FLAG_VAR(name) \
    bool get_var_##name() { return name; } \
    void set_var_##name(bool value) { (void)value; }
FLAG_VAR(wifi_connected)
FLAG_VAR(ethernet_connected)
FLAG_VAR(bluetooth_connected)
FLAG_VAR(wifi_busy)
const char *get_var_wifi_status() { return wifi_status; }
void set_var_wifi_status(const char *value) { (void)value; }
int32_t get_var_wifi_selected() { return wifi_selected; }
void set_var_wifi_selected(int32_t value) { wifi_selected = value; }
const char *get_var_wifi_password() { return wifi_password.c_str(); }
void set_var_wifi_password(const char *value) { wifi_password = value ? value : ""; }

static std::string json_string(const std::string &value)
{
    std::string result = "\"";
    for (unsigned char c : value) {
        if (c == '"' || c == '\\') { result += '\\'; result += c; }
        else if (c < 32) {
            char escape[7];
            snprintf(escape, sizeof(escape), "\\u%04x", c);
            result += escape;
        } else result += c;
    }
    return result + '"';
}

static void wifi_request(const std::string &request)
{
    if (!media_backend_request("/run/a333-media/control.sock", request.c_str()))
        snprintf(wifi_status, sizeof(wifi_status), "Network backend unavailable: %s", strerror(errno));
}

void action_wifi_scan(lv_event_t *event)
{
    (void)event;
    wifi_request("{\"command\":\"wifi_scan\"}");
}

void action_wifi_connect(lv_event_t *event)
{
    (void)event;
    std::string request = "{\"command\":\"wifi_connect\",\"revision\":" +
        std::to_string(network_revision) + ",\"index\":" + std::to_string(wifi_selected) +
        ",\"password\":" + json_string(wifi_password) + "}";
    wifi_request(request);
    // Password lives only in the edit field / one datagram, never UI logs.
    wifi_password.clear();
    lv_textarea_set_text(objects.wifi_password_input, "");
}

static void read_network()
{
    FILE *file = fopen("/run/a333-media/network", "r");
    if (!file)
        return;
    char header[6][1024];
    for (auto &line : header) {
        if (!fgets(line, sizeof(line), file) || !strchr(line, '\n')) {
            fclose(file);
            return;
        }
        line[strcspn(line, "\r\n")] = '\0';
    }
    char *end;
    errno = 0;
    unsigned long revision = strtoul(header[0], &end, 10);
    if (errno || !header[0][0] || *end || header[0][0] == '-') {
        fclose(file);
        return;
    }
    wifi_connected = strcmp(header[1], "1") == 0;
    ethernet_connected = strcmp(header[2], "1") == 0;
    bluetooth_connected = strcmp(header[3], "1") == 0;
    wifi_busy = strcmp(header[4], "1") == 0;
    snprintf(wifi_status, sizeof(wifi_status), "%s", header[5]);
    if (revision != network_revision) {
        std::vector<std::string> labels;
        char line[1024];
        while (labels.size() < 64 && fgets(line, sizeof(line), file)) {
            line[strcspn(line, "\r\n")] = '\0';
            labels.emplace_back(line);
        }
        if (labels.empty()) labels.emplace_back("No Wi-Fi networks found");
        wifi_networks = eez::Value::makeArrayRef(labels.size(), eez::flow::defs_v3::ARRAY_TYPE_STRING, 0);
        for (size_t i = 0; i < labels.size(); ++i)
            wifi_networks.getArray()->values[i] = eez::Value::makeStringRef(labels[i].c_str(), -1, 0);
        eez::flow::setGlobalVariable(FLOW_GLOBAL_VARIABLE_WIFI_NETWORKS, wifi_networks);
        network_revision = revision;
        wifi_selected = 0;
    }
    fclose(file);
}

static void refresh_network()
{
    size_t used = snprintf(network_info, sizeof(network_info), "IPv4 addresses:\n");
    struct ifaddrs *addresses = nullptr;
    if (getifaddrs(&addresses) == 0) {
        for (auto *item = addresses; item; item = item->ifa_next) {
            if (!item->ifa_addr || item->ifa_addr->sa_family != AF_INET ||
                strcmp(item->ifa_name, "lo") == 0)
                continue;
            char address[INET_ADDRSTRLEN];
            auto *ipv4 = reinterpret_cast<struct sockaddr_in *>(item->ifa_addr);
            if (inet_ntop(AF_INET, &ipv4->sin_addr, address, sizeof(address)) &&
                used < sizeof(network_info)) {
                int count = snprintf(network_info + used, sizeof(network_info) - used,
                                     "%s: %s\n", item->ifa_name, address);
                if (count > 0)
                    used += static_cast<size_t>(count);
            }
        }
        freeifaddrs(addresses);
    }
    if (used < sizeof(network_info))
        snprintf(network_info + used, sizeof(network_info) - used,
                 "\nConnect using nmcli or BLE Wi-Fi provisioning.\n"
                 "SSH / SCP: root@<address> (default password: allwinner)");
}

/* Runtime fonts supply Unicode (including Cyrillic) without font duplication.
 * Layout, widgets, bindings and navigation remain generated by EEZ Studio. */
static const lv_font_t *platform_font(const lv_font_t *font, const panel_lvgl_fonts_t *fonts)
{
    // EEZ Skoda descriptors already delegate glyph rendering to FreeType.
    if (font == &lv_font_montserrat_24) return fonts->small;
    if (font == &lv_font_montserrat_32) return fonts->normal;
    if (font == &lv_font_montserrat_40) return fonts->large;
    return font;
}

static void apply_platform_styles(lv_obj_t *obj, const panel_lvgl_fonts_t *fonts)
{
    if (lv_obj_check_type(obj, &lv_label_class)) {
        auto *font = lv_obj_get_style_text_font(obj, LV_PART_MAIN);
        lv_obj_set_style_text_font(obj, platform_font(font, fonts), 0);
    } else if (lv_obj_check_type(obj, &lv_button_class)) {
        lv_obj_set_style_transition(obj, &button_transition, 0);
    } else if (lv_obj_check_type(obj, &lv_dropdown_class) ||
               lv_obj_check_type(obj, &lv_textarea_class) ||
               lv_obj_check_type(obj, &lv_keyboard_class)) {
        auto *font = platform_font(lv_obj_get_style_text_font(obj, LV_PART_MAIN), fonts);
        lv_obj_set_style_text_font(obj, font, 0);
        if (lv_obj_check_type(obj, &lv_dropdown_class))
            lv_obj_set_style_text_font(lv_dropdown_get_list(obj), font, 0);
    } else if (lv_obj_check_type(obj, &lv_tabview_class)) {
        // The generated tabview's internal tab buttons also need Cyrillic.
        auto *buttons = lv_tabview_get_tab_bar(obj);
        auto *font = platform_font(lv_obj_get_style_text_font(buttons, LV_PART_MAIN), fonts);
        lv_obj_set_style_text_font(buttons, font, 0);
    }
    for (uint32_t i = 0; i < lv_obj_get_child_count(obj); ++i)
        apply_platform_styles(lv_obj_get_child(obj, i), fonts);
}

void media_ui_format_datetime(char *text, size_t size, const struct tm *date)
{
    static const char *months[] = {"января", "февраля", "марта", "апреля",
        "мая", "июня", "июля", "августа", "сентября", "октября", "ноября", "декабря"};
    if (!date || date->tm_mon < 0 || date->tm_mon >= 12) {
        snprintf(text, size, "%s", "Время недоступно");
        return;
    }
    snprintf(text, size, "%02d %s %04d, %02d:%02d:%02d", date->tm_mday,
             months[date->tm_mon], date->tm_year + 1900,
             date->tm_hour, date->tm_min, date->tm_sec);
}

static void refresh_clock()
{
    time_t now = time(nullptr);
    struct tm local;
    media_ui_format_datetime(system_datetime, sizeof(system_datetime),
                             localtime_r(&now, &local));
}

void media_ui_init(const panel_lvgl_fonts_t *fonts)
{
    /* Skoda FreeType provides Unicode; Montserrat supplies LV_SYMBOL_* glyphs. */
    fonts->small->fallback = &lv_font_montserrat_24;
    fonts->normal->fallback = &lv_font_montserrat_32;
    fonts->large->fallback = &lv_font_montserrat_40;
    media_backend_init(&state);
    refresh_clock();
    refresh_network();
    ui_init();
    lv_image_set_src(objects.album_cover_image, &img_material_music_note_2);
    lv_image_set_inner_align(objects.album_cover_image, LV_IMAGE_ALIGN_CONTAIN);
    lv_obj_add_event_cb(objects.track_progress_slider, seek_pressed, LV_EVENT_PRESSED, nullptr);
    lv_obj_add_event_cb(objects.track_progress_slider, seek_pressed, LV_EVENT_PRESS_LOST, nullptr);
    media_transition_init();
    wifi_networks = eez::Value::makeArrayRef(1, eez::flow::defs_v3::ARRAY_TYPE_STRING, 0);
    wifi_networks.getArray()->values[0] = eez::Value("Press Scan");
    eez::flow::setGlobalVariable(FLOW_GLOBAL_VARIABLE_WIFI_NETWORKS, wifi_networks);
    lv_style_transition_dsc_init(&button_transition, button_properties,
                                lv_anim_path_ease_out, 120, 0, nullptr);
    apply_platform_styles(objects.main, fonts);
    apply_platform_styles(objects.player_screen, fonts);
    apply_platform_styles(objects.wifi_screen, fonts);
}

void media_ui_tick(double now)
{
    media_transition_tick();
    if (now >= next_clock) {
        refresh_clock();
        next_clock = now + 1.0;
    }
    if (now >= next_status) {
        media_backend_poll(&state, "/run/a333-media/status", now);
        refresh_cover();
        read_network();
        next_status = now + 0.1;
    }
    if (now >= next_network) {
        refresh_network();
        next_network = now + 5.0;
    }
    ui_tick();
}
