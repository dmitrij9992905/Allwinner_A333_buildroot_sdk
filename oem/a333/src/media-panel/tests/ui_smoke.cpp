#include "media_ui.h"
#include "ui/screens.h"
#include "ui/vars.h"
#include <cassert>
#include <cstdio>
#include <cstring>
#include <vector>

static std::vector<uint32_t> frame(1280 * 800);
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
    assert(!lv_obj_has_flag(objects.main_wifi_connected_off, LV_OBJ_FLAG_HIDDEN));
    assert(lv_obj_has_flag(objects.main_wifi_connected_on, LV_OBJ_FLAG_HIDDEN));
    assert(lv_obj_has_flag(objects.main_volume_banner, LV_OBJ_FLAG_HIDDEN));
    assert(lv_color_eq(lv_obj_get_style_image_recolor(objects.main_wifi_connected_on, LV_PART_MAIN),
                       lv_color_hex(0xf5f8ff)));
    snapshot(argc > 1 ? argv[1] : nullptr, "main");
    lv_obj_send_event(objects.sources, LV_EVENT_CLICKED, nullptr);
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
    lv_obj_send_event(objects.network, LV_EVENT_CLICKED, nullptr);
    tick();
    assert(lv_screen_active() == objects.wifi_screen);
    assert(lv_keyboard_get_textarea(objects.wifi_keyboard) == objects.wifi_password_input);
    lv_area_t bounds;
    lv_obj_get_coords(objects.wifi_keyboard, &bounds);
    assert(bounds.y1 == 405 && bounds.y2 < 800 && bounds.x1 == 50 && bounds.x2 < 1280);
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
    lv_obj_send_event(objects.network_back, LV_EVENT_CLICKED, nullptr);
    tick();
    assert(lv_screen_active() == objects.main);
    // Rapid navigation while an animation is in progress must be queued;
    // all reused roots must end at x=0 and full opacity.
    lv_obj_send_event(objects.sources, LV_EVENT_CLICKED, nullptr);
    tick(40);
    eez_flow_set_screen(SCREEN_ID_WIFI_SCREEN, LV_SCREEN_LOAD_ANIM_OVER_LEFT, 300, 0);
    eez_flow_set_screen(SCREEN_ID_MAIN, LV_SCREEN_LOAD_ANIM_OVER_RIGHT, 300, 0);
    tick(900);
    assert(lv_screen_active() == objects.main);
    for (auto *screen : {objects.main, objects.player_screen, objects.wifi_screen}) {
        assert(lv_obj_get_x(screen) == 0);
        assert(lv_obj_get_style_opa(screen, LV_PART_MAIN) == LV_OPA_COVER);
    }
    puts("EEZ Flow: Android-style motion, rapid navigation, bindings, light icons and masked password input OK");
}
