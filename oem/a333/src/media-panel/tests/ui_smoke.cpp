#include "media_ui.h"
#include "media_backend.h"
#include "ui/screens.h"
#include "ui/vars.h"
#include "ui/fonts.h"
#include <src/draw/lv_image_decoder_private.h>
#include <cassert>
#include <cstdio>
#include <cstring>
#include <vector>
#include <string>
#include <cstdlib>
#include <unistd.h>

static std::vector<uint32_t> frame(1280 * 800);
static media_backend_state_t sample;
static bool sample_available;
static std::string last_request;
extern "C" bool __wrap_media_backend_request(const char *, const char *request)
{
    last_request = request;
    return true;
}
extern "C" bool __wrap_media_backend_poll(media_backend_state_t *state, const char *, double now)
{
    if (!sample_available) return false;
    *state = sample;
    state->volume_visible = state->volume[0] && now < state->volume_until;
    return true;
}
static void flush(lv_display_t *display, const lv_area_t *, uint8_t *)
{
    lv_display_flush_ready(display);
}
static void tick(int milliseconds = 400)
{
    for (int i = 0; i < milliseconds / 10; ++i) {
        lv_tick_inc(10);
        media_ui_tick(lv_tick_get() / 1000.0);
        lv_timer_handler();
    }
    // Position setters invalidate layout; inactive roots need an explicit
    // update before asserting their coordinates after SCREEN_LOADED cleanup.
    for (auto *screen : {objects.main, objects.player_screen, objects.wifi_screen})
        lv_obj_update_layout(screen);
}
static void snapshot(const char *prefix, const char *screen)
{
    if (!prefix) return;
    char path[1024];
    snprintf(path, sizeof(path), "%s-%s.ppm", prefix, screen);
    FILE *file = fopen(path, "wb");
    assert(file);
    fprintf(file, "P6\n1280 800\n255\n");
    for (auto pixel : frame) {
        unsigned char rgb[] = {static_cast<unsigned char>(pixel >> 16),
                               static_cast<unsigned char>(pixel >> 8),
                               static_cast<unsigned char>(pixel)};
        fwrite(rgb, 1, 3, file);
    }
    fclose(file);
}
int main(int argc, char **argv)
{
    lv_init();
    auto *display = lv_display_create(1280, 800);
    lv_display_set_color_format(display, LV_COLOR_FORMAT_XRGB8888);
    lv_display_set_buffers(display, frame.data(), nullptr, frame.size() * 4,
                           LV_DISPLAY_RENDER_MODE_DIRECT);
    lv_display_set_flush_cb(display, flush);
    panel_lvgl_fonts_t fonts;
    char error[256];
    assert(panel_lvgl_fonts_init_sizes(&fonts, 24, 32, 40, error, sizeof(error)));
    media_ui_init(&fonts);
    tick();
    assert(lv_screen_active() == objects.main);
    assert(!lv_obj_has_flag(objects.main_top_panel__system_wifi_connected_off, LV_OBJ_FLAG_HIDDEN));
    assert(lv_obj_has_flag(objects.main_top_panel__system_wifi_connected_on, LV_OBJ_FLAG_HIDDEN));
    assert(lv_obj_has_flag(objects.main_volume_osd, LV_OBJ_FLAG_HIDDEN));
    assert(lv_color_eq(lv_obj_get_style_image_recolor(objects.main_top_panel__system_wifi_connected_on, LV_PART_MAIN),
                       lv_color_hex(0xf5f8ff)));
    assert(lv_obj_get_style_text_font(objects.main_top_panel__system_datetime_label, LV_PART_MAIN) ==
           &ui_font_skoda_pro_medium_24);
    assert(lv_obj_get_style_text_font(lv_tabview_get_tab_bar(objects.main_tabs), LV_PART_MAIN) ==
           &ui_font_skoda_pro_medium_28);
    struct tm date = {};
    date.tm_year = 126; date.tm_mon = 9; date.tm_mday = 8;
    date.tm_hour = 14; date.tm_min = 35; date.tm_sec = 20;
    char formatted[128];
    media_ui_format_datetime(formatted, sizeof(formatted), &date);
    assert(strcmp(formatted, "08 октября 2026, 14:35:20") == 0);
    static const char *months[] = {"января", "февраля", "марта", "апреля", "мая", "июня",
        "июля", "августа", "сентября", "октября", "ноября", "декабря"};
    for (int month = 0; month < 12; ++month) {
        date.tm_mon = month;
        media_ui_format_datetime(formatted, sizeof(formatted), &date);
        assert(strstr(formatted, months[month]));
    }
    media_ui_format_datetime(formatted, sizeof(formatted), nullptr);
    assert(strcmp(formatted, "Время недоступно") == 0);
    assert(strlen(get_var_system_datetime()) > 20);
    assert(lv_tabview_get_tab_count(objects.main_tabs) == 3);
    assert(lv_obj_get_parent(objects.track_position_duration_label) == objects.now_playing_tab);
    snapshot(argc > 1 ? argv[1] : nullptr, "main");
    lv_tabview_set_active(objects.main_tabs, 1, LV_ANIM_ON);
    tick();
    assert(lv_tabview_get_tab_active(objects.main_tabs) == 1);
    lv_area_t radio_bounds, usb_bounds, sd_bounds;
    lv_obj_get_coords(objects.select_radio, &radio_bounds);
    lv_obj_get_coords(objects.select_usb, &usb_bounds);
    lv_obj_get_coords(objects.select_sd, &sd_bounds);
    assert(radio_bounds.y2 < usb_bounds.y1);
    assert(usb_bounds.x2 < sd_bounds.x1);
    snapshot(argc > 1 ? argv[1] : nullptr, "sources-tab");
    eez_flow_set_screen(SCREEN_ID_PLAYER_SCREEN, LV_SCREEN_LOAD_ANIM_OVER_LEFT, 300, 0);
    tick(60);
    assert(lv_obj_get_x(objects.player_screen) > 0);
    assert(lv_obj_get_x(objects.player_screen) < 64);
    assert(lv_obj_get_x(objects.main) < 0);
    assert(lv_obj_get_style_opa(objects.main, LV_PART_MAIN) < LV_OPA_COVER);
    assert(lv_obj_get_style_opa(objects.player_screen, LV_PART_MAIN) < LV_OPA_COVER);
    snapshot(argc > 1 ? argv[1] : nullptr, "forward-transition");
    tick(90);
    assert(lv_obj_get_style_opa(objects.main, LV_PART_MAIN) == LV_OPA_TRANSP);
    assert(lv_obj_get_style_opa(objects.player_screen, LV_PART_MAIN) > LV_OPA_TRANSP);
    assert(lv_obj_get_style_opa(objects.player_screen, LV_PART_MAIN) < LV_OPA_COVER);
    snapshot(argc > 1 ? argv[1] : nullptr, "forward-fade-in");
    tick();
    assert(lv_screen_active() == objects.player_screen);
    assert(lv_obj_get_x(objects.main) == 0);
    assert(lv_obj_get_style_opa(objects.main, LV_PART_MAIN) == LV_OPA_COVER);
    snapshot(argc > 1 ? argv[1] : nullptr, "sources");
    lv_obj_send_event(objects.sources_back, LV_EVENT_CLICKED, nullptr);
    tick(60);
    assert(lv_obj_get_x(objects.main) < 0);
    assert(lv_obj_get_x(objects.player_screen) > 0);
    snapshot(argc > 1 ? argv[1] : nullptr, "back-transition");
    tick();
    assert(lv_screen_active() == objects.main);
    lv_tabview_set_active(objects.main_tabs, 2, LV_ANIM_ON);
    tick();
    assert(lv_screen_active() == objects.main);
    assert(lv_tabview_get_tab_active(objects.main_tabs) == 2);
    assert(lv_keyboard_get_textarea(objects.wifi_keyboard) == objects.wifi_password_input);
    lv_area_t bounds;
    lv_obj_get_coords(objects.wifi_keyboard, &bounds);
    assert(bounds.y1 >= 0 && bounds.y2 < 800 && bounds.x1 >= 0 && bounds.x2 < 1280);
    assert(lv_textarea_get_password_mode(objects.wifi_password_input));
    lv_textarea_set_text(objects.wifi_password_input, "secret\\\"42");
    tick();
    assert(strcmp(get_var_wifi_password(), "secret\\\"42") == 0);
    lv_buttonmatrix_set_selected_button(objects.wifi_keyboard, 1); // q
    lv_obj_send_event(objects.wifi_keyboard, LV_EVENT_VALUE_CHANGED, nullptr);
    tick();
    assert(strcmp(get_var_wifi_password(), "secret\\\"42q") == 0);
    auto networks = eez::Value::makeArrayRef(2, eez::flow::defs_v3::ARRAY_TYPE_STRING, 0);
    networks.getArray()->values[0] = eez::Value("Home Wi-Fi");
    networks.getArray()->values[1] = eez::Value("Guest Wi-Fi");
    eez::flow::setGlobalVariable(FLOW_GLOBAL_VARIABLE_WIFI_NETWORKS, networks);
    tick();
    assert(lv_dropdown_get_option_count(objects.wifi_network_list) == 2);
    lv_dropdown_set_selected(objects.wifi_network_list, 1);
    lv_obj_send_event(objects.wifi_network_list, LV_EVENT_VALUE_CHANGED, nullptr);
    tick();
    assert(get_var_wifi_selected() == 1);
    snapshot(argc > 1 ? argv[1] : nullptr, "wifi");
    lv_tabview_set_active(objects.main_tabs, 0, LV_ANIM_ON);
    tick();
    assert(lv_screen_active() == objects.main);
    // Rapid navigation while an animation is in progress must be queued;
    // all reused roots must end at x=0 and full opacity.
    eez_flow_set_screen(SCREEN_ID_PLAYER_SCREEN, LV_SCREEN_LOAD_ANIM_OVER_LEFT, 300, 0);
    tick(40);
    eez_flow_set_screen(SCREEN_ID_WIFI_SCREEN, LV_SCREEN_LOAD_ANIM_OVER_LEFT, 300, 0);
    eez_flow_set_screen(SCREEN_ID_MAIN, LV_SCREEN_LOAD_ANIM_OVER_RIGHT, 300, 0);
    tick(900);
    assert(lv_screen_active() == objects.main);
    for (auto *screen : {objects.main, objects.player_screen, objects.wifi_screen}) {
        assert(lv_obj_get_x(screen) == 0);
        assert(lv_obj_get_style_opa(screen, LV_PART_MAIN) == LV_OPA_COVER);
    }
    lv_tabview_set_active(objects.main_tabs, 0, LV_ANIM_OFF);
    snprintf(sample.source, sizeof(sample.source), "Music");
    snprintf(sample.title, sizeof(sample.title), "Тестовая музыка");
    snprintf(sample.filename, sizeof(sample.filename), "Альбом/песня.flac");
    snprintf(sample.volume, sizeof(sample.volume), "Volume: 65%%");
    // A real PNG file exercises the same LVGL decoder/filesystem as artwork.
    static const unsigned char png[] = {
        0x89,0x50,0x4e,0x47,0x0d,0x0a,0x1a,0x0a,0,0,0,0x0d,0x49,0x48,0x44,0x52,
        0,0,0,2,0,0,0,1,8,2,0,0,0,0x7b,0x40,0xe8,0xdd,0,0,0,0x0f,0x49,0x44,
        0x41,0x54,0x78,0x9c,0x63,0xfc,0xcf,0xc0,0xc0,0xc0,0xc0,0,0,6,8,1,1,
        0xcb,0x47,0x76,0x75,0,0,0,0,0x49,0x45,0x4e,0x44,0xae,0x42,0x60,0x82
    };
    char art_path[] = "/tmp/media-panel-art-XXXXXX";
    int art_fd = mkstemp(art_path);
    assert(art_fd >= 0);
    assert(write(art_fd, png, sizeof(png)) == sizeof(png));
    close(art_fd);
    snprintf(sample.cover_path, sizeof(sample.cover_path), "%s", art_path);
    sample.volume_until = lv_tick_get() / 1000.0 + 5;
    sample_available = true;
    sample.position = 30;
    sample.duration = 120;
    sample.seekable = true;
    sample.usb_available = true;
    snprintf(sample.track_id, sizeof(sample.track_id), "7");
    tick();
    assert(lv_image_get_inner_align(objects.album_cover_image) == LV_IMAGE_ALIGN_CONTAIN);
    const char *art_source = static_cast<const char *>(lv_image_get_src(objects.album_cover_image));
    assert(!strncmp(art_source, "A:/tmp/media-panel-art-", 22));
    lv_image_decoder_dsc_t decoded = {};
    assert(lv_image_decoder_open(&decoded, art_source, nullptr) == LV_RESULT_OK);
    assert(decoded.header.w == 2 && decoded.header.h == 1);
    lv_image_decoder_close(&decoded);
    assert(lv_slider_get_value(objects.track_progress_slider) == 2500);
    assert(strcmp(get_var_track_position_duration(), "00:30 / 02:00") == 0);
    assert(!lv_obj_has_state(objects.track_progress_slider, LV_STATE_DISABLED));
    assert(!lv_obj_has_state(objects.select_usb, LV_STATE_DISABLED));
    assert(lv_obj_has_state(objects.select_sd, LV_STATE_DISABLED));
    lv_obj_send_event(objects.track_progress_slider, LV_EVENT_PRESSED, nullptr);
    lv_slider_set_value(objects.track_progress_slider, 5000, LV_ANIM_OFF);
    lv_obj_send_event(objects.track_progress_slider, LV_EVENT_VALUE_CHANGED, nullptr);
    tick(40);
    assert(lv_slider_get_value(objects.track_progress_slider) == 5000);
    lv_obj_send_event(objects.track_progress_slider, LV_EVENT_RELEASED, nullptr);
    tick(40);
    assert(last_request.find("\"command\":\"seek\"") != std::string::npos);
    assert(last_request.find("\"seconds\":60.000") != std::string::npos);
    last_request.clear();
    lv_obj_send_event(objects.track_progress_slider, LV_EVENT_PRESSED, nullptr);
    snprintf(sample.track_id, sizeof(sample.track_id), "8");
    tick(150); // Allow the 100 ms state-poll interval to observe the new ID.
    lv_obj_send_event(objects.track_progress_slider, LV_EVENT_RELEASED, nullptr);
    tick(40);
    assert(last_request.empty()); // A drag must not seek a newly started track.
    sample.seekable = false;
    tick(150);
    assert(lv_obj_has_state(objects.track_progress_slider, LV_STATE_DISABLED));
    assert(strcmp(lv_label_get_text(objects.track_filename), "Альбом/песня.flac") == 0);
    assert(!lv_obj_has_flag(objects.main_volume_osd, LV_OBJ_FLAG_HIDDEN));
    assert(lv_bar_get_value(objects.main_volume_osd__notify_progress_bar_volume) == 65);
    assert(strcmp(lv_label_get_text(objects.main_volume_osd__notify_text_volume_value_label), "65%") == 0);
    snapshot(argc > 1 ? argv[1] : nullptr, "volume");
    sample.volume_until = lv_tick_get() / 1000.0 + 5;
    tick(4900);
    assert(!lv_obj_has_flag(objects.main_volume_osd, LV_OBJ_FLAG_HIDDEN));
    tick(200);
    assert(lv_obj_has_flag(objects.main_volume_osd, LV_OBJ_FLAG_HIDDEN));
    unlink(art_path);
    puts("EEZ Flow: tabs, fonts, clock, motion, seek/track identity, storage and password bindings OK");
}
