#include "media_transition.h"
#include "ui/eez-flow.h"
#include "ui/screens.h"
#include <initializer_list>

namespace {
using PageHook = void (*)(int16_t, uint32_t, uint32_t, uint32_t);
PageHook original_hook;
lv_obj_t *incoming, *outgoing;
int32_t travel;
int direction;
constexpr int32_t progress_max = 1024;
constexpr int32_t fade_threshold = 358; // Material fade-through: 35%.
struct Request {
    int16_t page;
    uint32_t animation, duration, delay;
};
Request pending;
bool has_pending;

void enter(void *object, int32_t progress)
{
    if (object != incoming) return;
    auto *screen = static_cast<lv_obj_t *>(object);
    lv_obj_set_x(screen, direction * travel * (progress_max - progress) / progress_max);
    int32_t opacity = progress <= fade_threshold ? 0 :
        (progress - fade_threshold) * LV_OPA_COVER / (progress_max - fade_threshold);
    lv_obj_set_style_opa(screen, opacity, 0);
}

void leave(void *object, int32_t progress)
{
    if (object != outgoing) return;
    auto *screen = static_cast<lv_obj_t *>(object);
    lv_obj_set_x(screen, -direction * travel * progress / progress_max);
    int32_t opacity = progress >= fade_threshold ? 0 :
        (fade_threshold - progress) * LV_OPA_COVER / fade_threshold;
    lv_obj_set_style_opa(screen, opacity, 0);
}

void finished(lv_event_t *event)
{
    if (lv_event_get_target(event) != incoming) return;
    for (auto *screen : {incoming, outgoing}) {
        if (!screen) continue;
        lv_obj_set_x(screen, 0);
        lv_obj_remove_local_style_prop(screen, LV_STYLE_OPA, 0);
    }
    // The outgoing animation may get its final tick after this event.
    // Ignore it; both reusable screens must remain in their base state.
    incoming = outgoing = nullptr;
}

void configure(lv_anim_t *animation, lv_anim_exec_xcb_t execute)
{
    lv_anim_set_exec_cb(animation, execute);
    lv_anim_set_values(animation, 0, progress_max);
    lv_anim_set_path_cb(animation, lv_anim_path_custom_bezier3);
    // Fast-out-slow-in cubic Bezier (0.4, 0, 0.2, 1), fixed-point /1024.
    lv_anim_set_bezier3_param(animation, 410, 0, 205, 1024);
}

void change_page(int16_t page, uint32_t animation, uint32_t duration, uint32_t delay)
{
    if (incoming) {
        // Keep the last navigation request. Do not interrupt LVGL's page-load
        // bookkeeping or leave an old screen partly translated/transparent.
        pending = {page, animation, duration, delay};
        has_pending = true;
        return;
    }
    auto *next = eez::flow::getLvglObjectFromIndexHook(page - 1);
    auto *old = lv_screen_active();
    bool horizontal = animation == LV_SCREEN_LOAD_ANIM_OVER_LEFT ||
                      animation == LV_SCREEN_LOAD_ANIM_OVER_RIGHT;
    if (!horizontal || !duration || !next || !old || next == old) {
        original_hook(page, animation, duration, delay);
        return;
    }
    // EEZ retains ownership of screen IDs, page-flow states and load events.
    // Reuse LVGL's two screen-load animations and their completion callbacks;
    // change only their visual path. Generated files remain untouched.
    original_hook(page, LV_SCREEN_LOAD_ANIM_FADE_IN, duration, delay);
    auto *enter_animation = lv_anim_get(next, nullptr);
    auto *leave_animation = lv_anim_get(old, nullptr);
    if (!enter_animation || !leave_animation) return;
    incoming = next;
    outgoing = old;
    direction = animation == LV_SCREEN_LOAD_ANIM_OVER_LEFT ? 1 : -1;
    travel = lv_display_dpx(lv_obj_get_display(next), 32);
    configure(enter_animation, enter);
    configure(leave_animation, leave);
    enter(next, 0);
    leave(old, 0);
}
} // namespace

void media_transition_init(void)
{
    if (original_hook) return;
    original_hook = eez::flow::replacePageHook;
    // Fade-through must reveal a dark surface, never the default layer color.
    lv_obj_set_style_bg_color(lv_layer_bottom(), lv_color_hex(0x101820), 0);
    lv_obj_set_style_bg_opa(lv_layer_bottom(), LV_OPA_COVER, 0);
    for (auto *screen : {objects.main, objects.player_screen, objects.wifi_screen})
        lv_obj_add_event_cb(screen, finished, LV_EVENT_SCREEN_LOADED, nullptr);
    eez::flow::replacePageHook = change_page;
}

void media_transition_tick(void)
{
    if (!incoming && has_pending) {
        auto request = pending;
        has_pending = false;
        change_page(request.page, request.animation, request.duration, request.delay);
    }
}
