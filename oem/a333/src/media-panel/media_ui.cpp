#include "media_ui.h"
#include "media_backend.h"
#include "media_transition.h"
#include "ui/actions.h"
#include "ui/screens.h"
#include "ui/ui.h"
#include "ui/vars.h"

#include <cerrno>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>
#include <vector>

static media_backend_state_t state;
static char network_info[1024] = "Waiting for network information...";
static double next_status, next_network;
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

/* Native read-only values. EEZ evaluates bindings only on the UI thread. */
#define TEXT_VAR(name, field) \
    const char *get_var_##name() { return state.field; } \
    void set_var_##name(const char *value) { (void)value; }
TEXT_VAR(media_source, source)
TEXT_VAR(track_title, title)
TEXT_VAR(track_artist, artist)
TEXT_VAR(track_album, album)
TEXT_VAR(volume_text, volume)
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
static void apply_platform_styles(lv_obj_t *obj, const panel_lvgl_fonts_t *fonts)
{
    if (lv_obj_check_type(obj, &lv_label_class)) {
        auto *font = lv_obj_get_style_text_font(obj, LV_PART_MAIN);
        lv_obj_set_style_text_font(obj, font->line_height >= 38 ? fonts->large :
                                  font->line_height >= 30 ? fonts->normal : fonts->small, 0);
    } else if (lv_obj_check_type(obj, &lv_button_class)) {
        lv_obj_set_style_transition(obj, &button_transition, 0);
    } else if (lv_obj_check_type(obj, &lv_dropdown_class) ||
               lv_obj_check_type(obj, &lv_textarea_class) ||
               lv_obj_check_type(obj, &lv_keyboard_class)) {
        lv_obj_set_style_text_font(obj, fonts->small, 0);
        if (lv_obj_check_type(obj, &lv_dropdown_class))
            lv_obj_set_style_text_font(lv_dropdown_get_list(obj), fonts->small, 0);
    }
    for (uint32_t i = 0; i < lv_obj_get_child_count(obj); ++i)
        apply_platform_styles(lv_obj_get_child(obj, i), fonts);
}

void media_ui_init(const panel_lvgl_fonts_t *fonts)
{
    /* Roboto provides Unicode; Montserrat supplies LV_SYMBOL_* glyphs. */
    fonts->small->fallback = &lv_font_montserrat_24;
    fonts->normal->fallback = &lv_font_montserrat_32;
    fonts->large->fallback = &lv_font_montserrat_40;
    media_backend_init(&state);
    refresh_network();
    ui_init();
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
    if (now >= next_status) {
        media_backend_poll(&state, "/run/a333-media/status", now);
        read_network();
        next_status = now + 0.1;
    }
    if (now >= next_network) {
        refresh_network();
        next_network = now + 5.0;
    }
    ui_tick();
}
