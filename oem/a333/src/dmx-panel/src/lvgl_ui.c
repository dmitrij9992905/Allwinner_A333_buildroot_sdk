#include "lvgl_ui.h"

#include <inttypes.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef DMX_PANEL_VERSION
#define DMX_PANEL_VERSION "development"
#endif

#if LVGL_VERSION_MAJOR != 9 || LVGL_VERSION_MINOR != 5
#error "dmx_panel LVGL UI requires LVGL 9.5.x"
#endif

#define ACTION_QUEUE_CAPACITY 24u
#define COLOR_WHEEL_SIZE 340
#define COLOR_WHEEL_RADIUS (COLOR_WHEEL_SIZE / 2)

#define COLOR_BACKGROUND lv_color_hex(0x0c121d)
#define COLOR_SURFACE lv_color_hex(0x182232)
#define COLOR_SURFACE_ALT lv_color_hex(0x212e42)
#define COLOR_PRIMARY lv_color_hex(0x31bfd8)
#define COLOR_PRIMARY_DARK lv_color_hex(0x196e84)
#define COLOR_TEXT lv_color_hex(0xeef4fa)
#define COLOR_MUTED lv_color_hex(0x8e9eb1)
#define COLOR_GOOD lv_color_hex(0x36d399)
#define COLOR_WARNING lv_color_hex(0xfbbf24)
#define COLOR_DANGER lv_color_hex(0xef4444)

/* The controls were designed on a compact 720x720 touch surface. Keep that
 * geometry intact and center it on the A333's 1280x800 display. */
#define DMX_PANEL_UI_WIDTH 720
#define DMX_PANEL_UI_HEIGHT 720

struct dmx_lvgl_ui {
    dmx_lvgl_ui_fonts_t fonts;
    dmx_controller_snapshot_t snapshot;
    lv_obj_t *root;
    lv_obj_t *tabview;

    lv_obj_t *scene_wheel;
    lv_obj_t *scene_wheel_marker;
    uint16_t *scene_wheel_pixels;
    lv_color_t scene_color;
    uint16_t scene_hue;
    uint8_t scene_saturation;
    lv_obj_t *scene_swatch;
    lv_obj_t *scene_rgb_label;
    lv_obj_t *scene_brightness;
    lv_obj_t *scene_brightness_label;
    lv_obj_t *scene_address;
    lv_obj_t *scene_blackout;
    lv_obj_t *scene_status;
    lv_obj_t *software_version_label;

    lv_obj_t *rdm_discover;
    lv_obj_t *rdm_bus_status;
    lv_obj_t *rdm_list;
    lv_obj_t *rdm_detail;
    lv_obj_t *rdm_info;
    lv_obj_t *rdm_identify;
    lv_obj_t *rdm_address;
    lv_obj_t *rdm_address_minus;
    lv_obj_t *rdm_address_plus;
    lv_obj_t *rdm_address_apply;
    lv_obj_t *rdm_status;

    rdm_uid_t selected_uid;
    bool selected_valid;
    bool syncing;
    bool rdm_address_editing;
    bool rdm_address_pending;
    uint16_t rdm_address_pending_value;
    uint64_t device_signature;

    panel_ui_action_t action_queue[ACTION_QUEUE_CAPACITY];
    size_t action_head;
    size_t action_tail;
    size_t action_count;
};

static void set_obj_enabled(lv_obj_t *object, bool enabled)
{
    if (enabled)
        lv_obj_remove_state(object, LV_STATE_DISABLED);
    else
        lv_obj_add_state(object, LV_STATE_DISABLED);
}

static lv_obj_t *create_label(lv_obj_t *parent,
                              const char *text,
                              const lv_font_t *font,
                              lv_color_t color)
{
    lv_obj_t *label = lv_label_create(parent);

    lv_label_set_text(label, text);
    lv_obj_set_style_text_font(label, font, LV_PART_MAIN);
    lv_obj_set_style_text_color(label, color, LV_PART_MAIN);
    return label;
}

static lv_obj_t *create_button(lv_obj_t *parent,
                               const char *text,
                               const lv_font_t *font)
{
    lv_obj_t *button = lv_button_create(parent);
    lv_obj_t *label = create_label(button, text, font, COLOR_TEXT);

    lv_obj_set_style_bg_color(button, COLOR_PRIMARY_DARK, LV_PART_MAIN);
    lv_obj_set_style_bg_color(button, COLOR_PRIMARY,
                              LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_radius(button, 12, LV_PART_MAIN);
    lv_obj_center(label);
    return button;
}

static bool uid_in_snapshot(const dmx_controller_snapshot_t *snapshot,
                            const rdm_uid_t *uid,
                            size_t *index)
{
    size_t device_index;

    for (device_index = 0; device_index < snapshot->device_count;
         ++device_index) {
        if (rdm_uid_equal(uid, &snapshot->devices[device_index].uid)) {
            if (index != NULL)
                *index = device_index;
            return true;
        }
    }
    return false;
}

static void queue_action(dmx_lvgl_ui_t *ui,
                         const panel_ui_action_t *action)
{
    size_t last_index;

    if (ui == NULL || action == NULL || action->type == PANEL_UI_ACTION_NONE)
        return;

    /* Pointer drags can produce many scene updates. Keep only the newest one. */
    if (action->type == PANEL_UI_ACTION_SET_SCENE && ui->action_count != 0u) {
        last_index = (ui->action_tail + ACTION_QUEUE_CAPACITY - 1u) %
                     ACTION_QUEUE_CAPACITY;
        if (ui->action_queue[last_index].type == PANEL_UI_ACTION_SET_SCENE) {
            ui->action_queue[last_index] = *action;
            return;
        }
    }

    /* Never execute serial work here. If full, discard the oldest UI action. */
    if (ui->action_count == ACTION_QUEUE_CAPACITY) {
        ui->action_head = (ui->action_head + 1u) % ACTION_QUEUE_CAPACITY;
        --ui->action_count;
    }
    ui->action_queue[ui->action_tail] = *action;
    ui->action_tail = (ui->action_tail + 1u) % ACTION_QUEUE_CAPACITY;
    ++ui->action_count;
}

static void current_scene_color(dmx_lvgl_ui_t *ui,
                                uint8_t *red,
                                uint8_t *green,
                                uint8_t *blue)
{
    *red = ui->scene_color.red;
    *green = ui->scene_color.green;
    *blue = ui->scene_color.blue;
}

static void update_scene_swatch(dmx_lvgl_ui_t *ui)
{
    uint8_t red;
    uint8_t green;
    uint8_t blue;
    int32_t brightness;
    char text[48];

    current_scene_color(ui, &red, &green, &blue);
    brightness = lv_slider_get_value(ui->scene_brightness);
    lv_obj_set_style_bg_color(ui->scene_swatch, lv_color_make(red, green, blue),
                              LV_PART_MAIN);
    (void)snprintf(text, sizeof(text), "RGB  %u  %u  %u",
                   (unsigned int)red, (unsigned int)green,
                   (unsigned int)blue);
    lv_label_set_text(ui->scene_rgb_label, text);
    (void)snprintf(text, sizeof(text), "Яркость: %ld%%",
                   (long)((brightness * 100 + 127) / 255));
    lv_label_set_text(ui->scene_brightness_label, text);
}

static void emit_scene_action(dmx_lvgl_ui_t *ui);

static void position_wheel_marker(dmx_lvgl_ui_t *ui)
{
    double radians = (double)ui->scene_hue * 3.14159265358979323846 / 180.0;
    double distance = (double)ui->scene_saturation *
                      (double)(COLOR_WHEEL_RADIUS - 12) / 100.0;
    int32_t center_x = COLOR_WHEEL_RADIUS + (int32_t)lround(cos(radians) *
                                                            distance);
    int32_t center_y = COLOR_WHEEL_RADIUS - (int32_t)lround(sin(radians) *
                                                            distance);

    lv_obj_set_pos(ui->scene_wheel_marker, center_x - 10, center_y - 10);
}

static bool create_color_wheel_pixels(dmx_lvgl_ui_t *ui)
{
    const uint16_t background = lv_color_to_u16(COLOR_SURFACE);
    int32_t y;

    ui->scene_wheel_pixels =
        malloc((size_t)COLOR_WHEEL_SIZE * (size_t)COLOR_WHEEL_SIZE *
               sizeof(*ui->scene_wheel_pixels));
    if (ui->scene_wheel_pixels == NULL)
        return false;

    for (y = 0; y < COLOR_WHEEL_SIZE; ++y) {
        int32_t x;

        for (x = 0; x < COLOR_WHEEL_SIZE; ++x) {
            const int32_t dx = x - COLOR_WHEEL_RADIUS;
            const int32_t dy = y - COLOR_WHEEL_RADIUS;
            const int32_t radius_squared = dx * dx + dy * dy;
            uint16_t pixel = background;

            if (radius_squared <= COLOR_WHEEL_RADIUS * COLOR_WHEEL_RADIUS) {
                double angle = atan2((double)-dy, (double)dx) * 180.0 /
                               3.14159265358979323846;
                uint8_t saturation;
                lv_color_t color;

                if (angle < 0.0)
                    angle += 360.0;
                saturation = (uint8_t)lround(sqrt((double)radius_squared) *
                                             100.0 /
                                             (double)COLOR_WHEEL_RADIUS);
                color = lv_color_hsv_to_rgb((uint16_t)angle, saturation, 100u);
                pixel = lv_color_to_u16(color);
            }
            ui->scene_wheel_pixels[(size_t)y * COLOR_WHEEL_SIZE + (size_t)x] =
                pixel;
        }
    }
    return true;
}

static void scene_wheel_event(lv_event_t *event)
{
    dmx_lvgl_ui_t *ui = lv_event_get_user_data(event);
    const lv_event_code_t code = lv_event_get_code(event);
    lv_indev_t *input;
    lv_point_t point;
    lv_area_t area;
    int32_t dx;
    int32_t dy;
    double distance;
    double angle;

    if (code != LV_EVENT_PRESSED && code != LV_EVENT_PRESSING)
        return;
    input = lv_indev_active();
    if (input == NULL)
        return;
    lv_indev_get_point(input, &point);
    lv_obj_get_coords(ui->scene_wheel, &area);
    dx = point.x - area.x1 - COLOR_WHEEL_RADIUS;
    dy = point.y - area.y1 - COLOR_WHEEL_RADIUS;
    distance = sqrt((double)dx * dx + (double)dy * dy);
    if (distance > (double)COLOR_WHEEL_RADIUS)
        return;
    angle = atan2((double)-dy, (double)dx) * 180.0 /
            3.14159265358979323846;
    if (angle < 0.0)
        angle += 360.0;
    ui->scene_hue = (uint16_t)angle;
    ui->scene_saturation =
        (uint8_t)lround(distance * 100.0 / (double)COLOR_WHEEL_RADIUS);
    ui->scene_color = lv_color_hsv_to_rgb(ui->scene_hue,
                                          ui->scene_saturation, 100u);
    position_wheel_marker(ui);
    update_scene_swatch(ui);
    emit_scene_action(ui);
}

static void emit_scene_action(dmx_lvgl_ui_t *ui)
{
    panel_ui_action_t action;

    if (ui->syncing)
        return;
    memset(&action, 0, sizeof(action));
    action.type = PANEL_UI_ACTION_SET_SCENE;
    action.data.scene.start_address =
        (uint16_t)lv_spinbox_get_value(ui->scene_address);
    current_scene_color(ui, &action.data.scene.red, &action.data.scene.green,
                        &action.data.scene.blue);
    action.data.scene.brightness =
        (uint8_t)lv_slider_get_value(ui->scene_brightness);
    queue_action(ui, &action);
}

static void scene_value_event(lv_event_t *event)
{
    dmx_lvgl_ui_t *ui = lv_event_get_user_data(event);

    if (lv_event_get_code(event) != LV_EVENT_VALUE_CHANGED)
        return;
    update_scene_swatch(ui);
    emit_scene_action(ui);
}

static void scene_address_minus_event(lv_event_t *event)
{
    dmx_lvgl_ui_t *ui = lv_event_get_user_data(event);

    if (lv_event_get_code(event) != LV_EVENT_CLICKED)
        return;
    lv_spinbox_decrement(ui->scene_address);
    emit_scene_action(ui);
}

static void scene_address_plus_event(lv_event_t *event)
{
    dmx_lvgl_ui_t *ui = lv_event_get_user_data(event);

    if (lv_event_get_code(event) != LV_EVENT_CLICKED)
        return;
    lv_spinbox_increment(ui->scene_address);
    emit_scene_action(ui);
}

static void scene_blackout_event(lv_event_t *event)
{
    dmx_lvgl_ui_t *ui = lv_event_get_user_data(event);
    panel_ui_action_t action;

    if (lv_event_get_code(event) != LV_EVENT_VALUE_CHANGED || ui->syncing)
        return;
    memset(&action, 0, sizeof(action));
    action.type = PANEL_UI_ACTION_SET_BLACKOUT;
    action.data.blackout.enabled =
        lv_obj_has_state(ui->scene_blackout, LV_STATE_CHECKED);
    queue_action(ui, &action);
}

static void rdm_discover_event(lv_event_t *event)
{
    dmx_lvgl_ui_t *ui = lv_event_get_user_data(event);
    panel_ui_action_t action;

    if (lv_event_get_code(event) != LV_EVENT_CLICKED || ui->syncing)
        return;
    memset(&action, 0, sizeof(action));
    action.type = PANEL_UI_ACTION_RDM_DISCOVER;
    queue_action(ui, &action);
}

static void rdm_info_event(lv_event_t *event)
{
    dmx_lvgl_ui_t *ui = lv_event_get_user_data(event);
    panel_ui_action_t action;

    if (lv_event_get_code(event) != LV_EVENT_CLICKED || ui->syncing ||
        !ui->selected_valid)
        return;
    memset(&action, 0, sizeof(action));
    action.type = PANEL_UI_ACTION_RDM_REQUEST_INFO;
    action.data.rdm_info.uid = ui->selected_uid;
    queue_action(ui, &action);
}

static void rdm_identify_event(lv_event_t *event)
{
    dmx_lvgl_ui_t *ui = lv_event_get_user_data(event);
    panel_ui_action_t action;

    if (lv_event_get_code(event) != LV_EVENT_VALUE_CHANGED || ui->syncing ||
        !ui->selected_valid)
        return;
    memset(&action, 0, sizeof(action));
    action.type = PANEL_UI_ACTION_RDM_SET_IDENTIFY;
    action.data.rdm_identify.uid = ui->selected_uid;
    action.data.rdm_identify.enabled =
        lv_obj_has_state(ui->rdm_identify, LV_STATE_CHECKED);
    queue_action(ui, &action);
}

static void rdm_address_minus_event(lv_event_t *event)
{
    dmx_lvgl_ui_t *ui = lv_event_get_user_data(event);

    if (lv_event_get_code(event) == LV_EVENT_CLICKED)
        lv_spinbox_decrement(ui->rdm_address);
}

static void rdm_address_value_event(lv_event_t *event)
{
    dmx_lvgl_ui_t *ui = lv_event_get_user_data(event);
    lv_event_code_t code = lv_event_get_code(event);

    if (ui == NULL || ui->syncing)
        return;
    if (code == LV_EVENT_FOCUSED || code == LV_EVENT_VALUE_CHANGED)
        ui->rdm_address_editing = true;
}

static void rdm_address_plus_event(lv_event_t *event)
{
    dmx_lvgl_ui_t *ui = lv_event_get_user_data(event);

    if (lv_event_get_code(event) == LV_EVENT_CLICKED)
        lv_spinbox_increment(ui->rdm_address);
}

static void rdm_address_apply_event(lv_event_t *event)
{
    dmx_lvgl_ui_t *ui = lv_event_get_user_data(event);
    panel_ui_action_t action;

    if (lv_event_get_code(event) != LV_EVENT_CLICKED || ui->syncing ||
        !ui->selected_valid)
        return;
    memset(&action, 0, sizeof(action));
    action.type = PANEL_UI_ACTION_RDM_SET_ADDRESS;
    action.data.rdm_address.uid = ui->selected_uid;
    action.data.rdm_address.start_address =
        (uint16_t)lv_spinbox_get_value(ui->rdm_address);
    ui->rdm_address_pending = true;
    ui->rdm_address_pending_value = action.data.rdm_address.start_address;
    ui->rdm_address_editing = true;
    queue_action(ui, &action);
}

static void refresh_rdm_detail(dmx_lvgl_ui_t *ui);

static void rdm_list_item_event(lv_event_t *event)
{
    dmx_lvgl_ui_t *ui = lv_event_get_user_data(event);
    lv_obj_t *button = lv_event_get_target(event);
    uintptr_t encoded_index;
    size_t device_index;
    uint32_t child_count;
    uint32_t child_index;

    if (lv_event_get_code(event) != LV_EVENT_CLICKED || ui->syncing)
        return;
    encoded_index = (uintptr_t)lv_obj_get_user_data(button);
    if (encoded_index == 0u)
        return;
    device_index = (size_t)(encoded_index - 1u);
    if (device_index >= ui->snapshot.device_count)
        return;
    ui->selected_uid = ui->snapshot.devices[device_index].uid;
    ui->selected_valid = true;
    ui->rdm_address_editing = false;
    ui->rdm_address_pending = false;
    child_count = lv_obj_get_child_count(ui->rdm_list);
    for (child_index = 0u; child_index < child_count; ++child_index) {
        lv_obj_t *child = lv_obj_get_child(ui->rdm_list,
                                           (int32_t)child_index);
        uintptr_t child_encoded = (uintptr_t)lv_obj_get_user_data(child);

        if (child_encoded != 0u)
            lv_obj_set_style_bg_color(
                child,
                child_encoded - 1u == device_index ? COLOR_PRIMARY_DARK
                                                    : COLOR_SURFACE_ALT,
                LV_PART_MAIN);
    }
    refresh_rdm_detail(ui);
}

static lv_obj_t *create_spinbox_with_buttons(lv_obj_t *parent,
                                              int32_t x,
                                              int32_t y,
                                              int32_t maximum,
                                              lv_obj_t **minus,
                                              lv_obj_t **plus,
                                              lv_event_cb_t minus_callback,
                                              lv_event_cb_t plus_callback,
                                              dmx_lvgl_ui_t *ui)
{
    lv_obj_t *spinbox = lv_spinbox_create(parent);

    lv_obj_set_pos(spinbox, x + 66, y);
    lv_obj_set_size(spinbox, 130, 56);
    lv_spinbox_set_range(spinbox, 1, maximum);
    lv_spinbox_set_digit_format(spinbox, 3u, 0u);
    lv_spinbox_set_step(spinbox, 1u);
    lv_obj_set_style_text_font(spinbox, ui->fonts.large, LV_PART_MAIN);
    lv_obj_set_style_text_color(spinbox, COLOR_TEXT, LV_PART_MAIN);
    lv_obj_set_style_bg_color(spinbox, COLOR_SURFACE_ALT, LV_PART_MAIN);
    lv_obj_set_style_border_color(spinbox, COLOR_PRIMARY_DARK, LV_PART_MAIN);
    lv_obj_set_style_border_width(spinbox, 2, LV_PART_MAIN);
    lv_obj_set_style_radius(spinbox, 10, LV_PART_MAIN);

    *minus = create_button(parent, "−", ui->fonts.large);
    lv_obj_set_pos(*minus, x, y);
    lv_obj_set_size(*minus, 56, 56);
    lv_obj_add_event_cb(*minus, minus_callback, LV_EVENT_CLICKED, ui);

    *plus = create_button(parent, "+", ui->fonts.large);
    lv_obj_set_pos(*plus, x + 206, y);
    lv_obj_set_size(*plus, 56, 56);
    lv_obj_add_event_cb(*plus, plus_callback, LV_EVENT_CLICKED, ui);
    return spinbox;
}

static bool create_scene_tab(dmx_lvgl_ui_t *ui, lv_obj_t *tab)
{
    lv_obj_t *label;
    lv_obj_t *minus;
    lv_obj_t *plus;

    lv_obj_remove_flag(tab, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(tab, COLOR_BACKGROUND, LV_PART_MAIN);
    lv_obj_set_style_pad_all(tab, 0, LV_PART_MAIN);

    ui->scene_wheel = lv_canvas_create(tab);
    if (!create_color_wheel_pixels(ui))
        return false;
    lv_canvas_set_buffer(ui->scene_wheel, ui->scene_wheel_pixels,
                         COLOR_WHEEL_SIZE, COLOR_WHEEL_SIZE,
                         LV_COLOR_FORMAT_RGB565);
    lv_obj_set_pos(ui->scene_wheel, 22, 87);
    lv_obj_add_flag(ui->scene_wheel, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_remove_flag(ui->scene_wheel, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_event_cb(ui->scene_wheel, scene_wheel_event, LV_EVENT_ALL, ui);

    ui->scene_wheel_marker = lv_obj_create(ui->scene_wheel);
    lv_obj_set_size(ui->scene_wheel_marker, 20, 20);
    lv_obj_set_style_radius(ui->scene_wheel_marker, LV_RADIUS_CIRCLE,
                            LV_PART_MAIN);
    lv_obj_set_style_bg_opa(ui->scene_wheel_marker, LV_OPA_TRANSP,
                            LV_PART_MAIN);
    lv_obj_set_style_border_color(ui->scene_wheel_marker, lv_color_white(),
                                  LV_PART_MAIN);
    lv_obj_set_style_border_width(ui->scene_wheel_marker, 3, LV_PART_MAIN);
    lv_obj_remove_flag(ui->scene_wheel_marker, LV_OBJ_FLAG_CLICKABLE |
                       LV_OBJ_FLAG_SCROLLABLE);

    label = create_label(tab, "Цвет сцены", ui->fonts.large, COLOR_TEXT);
    lv_obj_set_pos(label, 414, 22);

    ui->scene_swatch = lv_obj_create(tab);
    lv_obj_set_pos(ui->scene_swatch, 414, 70);
    lv_obj_set_size(ui->scene_swatch, 272, 72);
    lv_obj_set_style_radius(ui->scene_swatch, 14, LV_PART_MAIN);
    lv_obj_set_style_border_width(ui->scene_swatch, 2, LV_PART_MAIN);
    lv_obj_set_style_border_color(ui->scene_swatch, COLOR_TEXT, LV_PART_MAIN);
    lv_obj_remove_flag(ui->scene_swatch, LV_OBJ_FLAG_SCROLLABLE);

    ui->scene_rgb_label =
        create_label(tab, "RGB  255  0  0", ui->fonts.normal, COLOR_TEXT);
    lv_obj_set_pos(ui->scene_rgb_label, 414, 151);

    ui->scene_brightness_label =
        create_label(tab, "Яркость: 100%", ui->fonts.normal, COLOR_TEXT);
    lv_obj_set_pos(ui->scene_brightness_label, 414, 202);
    ui->scene_brightness = lv_slider_create(tab);
    lv_obj_set_pos(ui->scene_brightness, 414, 240);
    lv_obj_set_size(ui->scene_brightness, 272, 28);
    lv_slider_set_range(ui->scene_brightness, 0, 255);
    lv_slider_set_value(ui->scene_brightness, 255, LV_ANIM_OFF);
    lv_obj_set_style_bg_color(ui->scene_brightness, COLOR_SURFACE_ALT,
                              LV_PART_MAIN);
    lv_obj_set_style_bg_color(ui->scene_brightness, COLOR_PRIMARY,
                              LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(ui->scene_brightness, COLOR_TEXT,
                              LV_PART_KNOB);
    lv_obj_add_event_cb(ui->scene_brightness, scene_value_event,
                        LV_EVENT_VALUE_CHANGED, ui);

    label = create_label(tab, "Стартовый адрес RGB", ui->fonts.normal,
                         COLOR_TEXT);
    lv_obj_set_pos(label, 414, 301);
    ui->scene_address = create_spinbox_with_buttons(
        tab, 414, 340, 510, &minus, &plus, scene_address_minus_event,
        scene_address_plus_event, ui);

    label = create_label(tab, "Blackout", ui->fonts.large, COLOR_TEXT);
    lv_obj_set_pos(label, 414, 433);
    ui->scene_blackout = lv_switch_create(tab);
    lv_obj_set_pos(ui->scene_blackout, 598, 425);
    lv_obj_set_size(ui->scene_blackout, 88, 48);
    lv_obj_set_style_bg_color(ui->scene_blackout, COLOR_DANGER,
                              LV_PART_INDICATOR | LV_STATE_CHECKED);
    lv_obj_add_event_cb(ui->scene_blackout, scene_blackout_event,
                        LV_EVENT_VALUE_CHANGED, ui);

    ui->scene_status = create_label(tab, "Контроллер не запущен",
                                    ui->fonts.small, COLOR_MUTED);
    lv_obj_set_pos(ui->scene_status, 22, 550);
    lv_obj_set_width(ui->scene_status, 664);
    lv_label_set_long_mode(ui->scene_status, LV_LABEL_LONG_MODE_WRAP);

    ui->software_version_label = create_label(
        tab, "ПО dmx-panel " DMX_PANEL_VERSION, ui->fonts.small, COLOR_MUTED);
    lv_obj_set_pos(ui->software_version_label, 414, 615);

    ui->scene_color = lv_color_make(255u, 0u, 0u);
    ui->scene_hue = 0u;
    ui->scene_saturation = 100u;
    position_wheel_marker(ui);
    update_scene_swatch(ui);
    return true;
}

static bool create_rdm_tab(dmx_lvgl_ui_t *ui, lv_obj_t *tab)
{
    lv_obj_t *label;

    lv_obj_remove_flag(tab, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(tab, COLOR_BACKGROUND, LV_PART_MAIN);
    lv_obj_set_style_pad_all(tab, 0, LV_PART_MAIN);

    ui->rdm_discover = create_button(tab, "Discovery", ui->fonts.normal);
    lv_obj_set_pos(ui->rdm_discover, 18, 16);
    lv_obj_set_size(ui->rdm_discover, 210, 56);
    lv_obj_add_event_cb(ui->rdm_discover, rdm_discover_event,
                        LV_EVENT_CLICKED, ui);

    ui->rdm_bus_status = create_label(tab, "RDM: ожидание",
                                      ui->fonts.small, COLOR_MUTED);
    lv_obj_set_pos(ui->rdm_bus_status, 250, 31);
    lv_obj_set_width(ui->rdm_bus_status, 440);
    lv_label_set_long_mode(ui->rdm_bus_status, LV_LABEL_LONG_MODE_DOTS);

    ui->rdm_list = lv_list_create(tab);
    lv_obj_set_pos(ui->rdm_list, 18, 88);
    lv_obj_set_size(ui->rdm_list, 306, 462);
    lv_obj_set_style_bg_color(ui->rdm_list, COLOR_SURFACE, LV_PART_MAIN);
    lv_obj_set_style_border_width(ui->rdm_list, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(ui->rdm_list, 14, LV_PART_MAIN);
    lv_obj_set_style_text_font(ui->rdm_list, ui->fonts.small, LV_PART_MAIN);

    ui->rdm_detail = create_label(tab, "Выберите RDM-устройство",
                                  ui->fonts.small, COLOR_TEXT);
    lv_obj_set_pos(ui->rdm_detail, 346, 96);
    lv_obj_set_size(ui->rdm_detail, 350, 246);
    lv_label_set_long_mode(ui->rdm_detail, LV_LABEL_LONG_MODE_WRAP);

    ui->rdm_info = create_button(tab, "Обновить info", ui->fonts.small);
    lv_obj_set_pos(ui->rdm_info, 346, 350);
    lv_obj_set_size(ui->rdm_info, 168, 50);
    lv_obj_add_event_cb(ui->rdm_info, rdm_info_event, LV_EVENT_CLICKED, ui);

    label = create_label(tab, "Identify", ui->fonts.normal, COLOR_TEXT);
    lv_obj_set_pos(label, 536, 360);
    ui->rdm_identify = lv_switch_create(tab);
    lv_obj_set_pos(ui->rdm_identify, 626, 351);
    lv_obj_set_size(ui->rdm_identify, 70, 46);
    lv_obj_set_style_bg_color(ui->rdm_identify, COLOR_WARNING,
                              LV_PART_INDICATOR | LV_STATE_CHECKED);
    lv_obj_add_event_cb(ui->rdm_identify, rdm_identify_event,
                        LV_EVENT_VALUE_CHANGED, ui);

    label = create_label(tab, "Новый DMX-адрес", ui->fonts.small,
                         COLOR_TEXT);
    lv_obj_set_pos(label, 346, 417);
    ui->rdm_address = lv_spinbox_create(tab);
    lv_obj_set_pos(ui->rdm_address, 408, 449);
    lv_obj_set_size(ui->rdm_address, 122, 52);
    lv_spinbox_set_range(ui->rdm_address, 1, 512);
    lv_spinbox_set_digit_format(ui->rdm_address, 3u, 0u);
    lv_spinbox_set_step(ui->rdm_address, 1u);
    lv_obj_set_style_text_font(ui->rdm_address, ui->fonts.normal,
                               LV_PART_MAIN);
    lv_obj_set_style_bg_color(ui->rdm_address, COLOR_SURFACE_ALT,
                              LV_PART_MAIN);
    lv_obj_set_style_text_color(ui->rdm_address, COLOR_TEXT, LV_PART_MAIN);
    lv_obj_add_event_cb(ui->rdm_address, rdm_address_value_event,
                        LV_EVENT_FOCUSED, ui);
    lv_obj_add_event_cb(ui->rdm_address, rdm_address_value_event,
                        LV_EVENT_VALUE_CHANGED, ui);

    ui->rdm_address_minus = create_button(tab, "−", ui->fonts.large);
    lv_obj_set_pos(ui->rdm_address_minus, 346, 449);
    lv_obj_set_size(ui->rdm_address_minus, 52, 52);
    lv_obj_add_event_cb(ui->rdm_address_minus, rdm_address_minus_event,
                        LV_EVENT_CLICKED, ui);
    ui->rdm_address_plus = create_button(tab, "+", ui->fonts.large);
    lv_obj_set_pos(ui->rdm_address_plus, 540, 449);
    lv_obj_set_size(ui->rdm_address_plus, 52, 52);
    lv_obj_add_event_cb(ui->rdm_address_plus, rdm_address_plus_event,
                        LV_EVENT_CLICKED, ui);
    ui->rdm_address_apply = create_button(tab, "Задать", ui->fonts.small);
    lv_obj_set_pos(ui->rdm_address_apply, 602, 449);
    lv_obj_set_size(ui->rdm_address_apply, 94, 52);
    lv_obj_add_event_cb(ui->rdm_address_apply, rdm_address_apply_event,
                        LV_EVENT_CLICKED, ui);

    ui->rdm_status = create_label(tab, "Устройств: 0", ui->fonts.small,
                                  COLOR_MUTED);
    lv_obj_set_pos(ui->rdm_status, 18, 564);
    lv_obj_set_width(ui->rdm_status, 678);
    lv_label_set_long_mode(ui->rdm_status, LV_LABEL_LONG_MODE_DOTS);
    return true;
}

static uint64_t hash_bytes(uint64_t hash, const void *data, size_t length)
{
    const uint8_t *bytes = data;
    size_t index;

    for (index = 0; index < length; ++index) {
        hash ^= bytes[index];
        hash *= UINT64_C(1099511628211);
    }
    return hash;
}

static uint64_t rdm_device_signature(const dmx_controller_snapshot_t *snapshot)
{
    uint64_t hash = UINT64_C(1469598103934665603);
    size_t index;

    hash = hash_bytes(hash, &snapshot->device_count,
                      sizeof(snapshot->device_count));
    for (index = 0; index < snapshot->device_count; ++index) {
        const dmx_rdm_device_t *device = &snapshot->devices[index];

        hash = hash_bytes(hash, device->uid.bytes, sizeof(device->uid.bytes));
        hash = hash_bytes(hash, &device->information_valid,
                          sizeof(device->information_valid));
        hash = hash_bytes(hash, &device->identify_known,
                          sizeof(device->identify_known));
        hash = hash_bytes(hash, &device->identify_on,
                          sizeof(device->identify_on));
        hash = hash_bytes(hash, &device->dmx_start_address,
                          sizeof(device->dmx_start_address));
        hash = hash_bytes(hash, device->manufacturer_label,
                          sizeof(device->manufacturer_label));
        hash = hash_bytes(hash, device->model_description,
                          sizeof(device->model_description));
        hash = hash_bytes(hash, device->device_label,
                          sizeof(device->device_label));
    }
    return hash;
}

static void refresh_rdm_detail(dmx_lvgl_ui_t *ui)
{
    size_t index;
    const dmx_rdm_device_t *device;
    char uid_text[14];
    char text[1024];
    bool previous_syncing;
    uint16_t address;

    previous_syncing = ui->syncing;
    ui->syncing = true;
    if (!ui->selected_valid ||
        !uid_in_snapshot(&ui->snapshot, &ui->selected_uid, &index)) {
        ui->selected_valid = false;
        lv_label_set_text(ui->rdm_detail, "Выберите RDM-устройство");
        set_obj_enabled(ui->rdm_info, false);
        set_obj_enabled(ui->rdm_identify, false);
        set_obj_enabled(ui->rdm_address, false);
        set_obj_enabled(ui->rdm_address_minus, false);
        set_obj_enabled(ui->rdm_address_plus, false);
        set_obj_enabled(ui->rdm_address_apply, false);
        ui->syncing = previous_syncing;
        return;
    }

    device = &ui->snapshot.devices[index];
    rdm_uid_format(&device->uid, uid_text);
    if (device->information_valid) {
        (void)snprintf(
            text, sizeof(text),
            "UID: %s\n"
            "Производитель: %s\n"
            "Модель: %s\n"
            "Имя: %s\n"
            "ПО: %s\n"
            "Протокол: %u.%u\n"
            "DMX footprint: %u\n"
            "DMX-адрес: %u\n"
            "Personality: %u/%u\n"
            "Sub-devices: %u   Sensors: %u",
            uid_text,
            device->manufacturer_label[0] != '\0' ? device->manufacturer_label
                                                    : "—",
            device->model_description[0] != '\0' ? device->model_description
                                                   : "—",
            device->device_label[0] != '\0' ? device->device_label : "—",
            device->software_version_label[0] != '\0'
                ? device->software_version_label
                : "—",
            (unsigned int)(device->protocol_version >> 8),
            (unsigned int)(device->protocol_version & 0xffu),
            (unsigned int)device->dmx_footprint,
            (unsigned int)device->dmx_start_address,
            (unsigned int)device->current_personality,
            (unsigned int)device->personality_count,
            (unsigned int)device->sub_device_count,
            (unsigned int)device->sensor_count);
    }
    else {
        (void)snprintf(text, sizeof(text),
                       "UID: %s\n\nИнформация ещё не получена.\n"
                       "Нажмите «Обновить info».",
                       uid_text);
    }
    lv_label_set_text(ui->rdm_detail, text);
    set_obj_enabled(ui->rdm_info, !ui->snapshot.rdm_busy);
    set_obj_enabled(ui->rdm_identify, !ui->snapshot.rdm_busy);
    set_obj_enabled(ui->rdm_address, !ui->snapshot.rdm_busy);
    set_obj_enabled(ui->rdm_address_minus, !ui->snapshot.rdm_busy);
    set_obj_enabled(ui->rdm_address_plus, !ui->snapshot.rdm_busy);
    set_obj_enabled(ui->rdm_address_apply, !ui->snapshot.rdm_busy);

    if (device->identify_known && device->identify_on)
        lv_obj_add_state(ui->rdm_identify, LV_STATE_CHECKED);
    else
        lv_obj_remove_state(ui->rdm_identify, LV_STATE_CHECKED);
    if (ui->rdm_address_pending &&
        device->dmx_start_address == ui->rdm_address_pending_value) {
        ui->rdm_address_pending = false;
        ui->rdm_address_editing = false;
    }
    address = device->dmx_start_address;
    if (address < 1u || address > 512u)
        address = 1u;
    if (!ui->rdm_address_editing && !ui->rdm_address_pending)
        lv_spinbox_set_value(ui->rdm_address, address);
    ui->syncing = previous_syncing;
}

static void refresh_rdm_list(dmx_lvgl_ui_t *ui)
{
    const uint64_t signature = rdm_device_signature(&ui->snapshot);
    size_t index;

    if (signature == ui->device_signature)
        return;
    ui->device_signature = signature;
    if (ui->selected_valid &&
        !uid_in_snapshot(&ui->snapshot, &ui->selected_uid, NULL))
        ui->selected_valid = false;
    if (!ui->selected_valid && ui->snapshot.device_count != 0u) {
        ui->selected_uid = ui->snapshot.devices[0].uid;
        ui->selected_valid = true;
    }

    lv_obj_clean(ui->rdm_list);
    if (ui->snapshot.device_count == 0u) {
        lv_obj_t *empty = lv_list_add_text(ui->rdm_list,
                                           "RDM-устройства не найдены");
        lv_obj_set_style_text_font(empty, ui->fonts.small, LV_PART_MAIN);
        lv_obj_set_style_text_color(empty, COLOR_MUTED, LV_PART_MAIN);
    }
    for (index = 0; index < ui->snapshot.device_count; ++index) {
        const dmx_rdm_device_t *device = &ui->snapshot.devices[index];
        char uid_text[14];
        char row_text[96];
        lv_obj_t *button;

        rdm_uid_format(&device->uid, uid_text);
        if (device->device_label[0] != '\0')
            (void)snprintf(row_text, sizeof(row_text), "%s\n%s",
                           device->device_label, uid_text);
        else
            (void)snprintf(row_text, sizeof(row_text), "%s", uid_text);
        button = lv_list_add_button(ui->rdm_list, NULL, row_text);
        lv_obj_set_user_data(button, (void *)(uintptr_t)(index + 1u));
        lv_obj_set_style_text_font(button, ui->fonts.small, LV_PART_MAIN);
        lv_obj_set_style_text_color(button, COLOR_TEXT, LV_PART_MAIN);
        lv_obj_set_style_bg_color(
            button,
            ui->selected_valid && rdm_uid_equal(&device->uid,
                                                &ui->selected_uid)
                ? COLOR_PRIMARY_DARK
                : COLOR_SURFACE_ALT,
            LV_PART_MAIN);
        lv_obj_add_event_cb(button, rdm_list_item_event, LV_EVENT_CLICKED, ui);
    }
    refresh_rdm_detail(ui);
}

bool dmx_lvgl_ui_apply_roboto_theme(lv_display_t *display,
                                    const dmx_lvgl_ui_fonts_t *fonts)
{
    lv_theme_t *theme;

    if (fonts == NULL || fonts->small == NULL || fonts->normal == NULL ||
        fonts->large == NULL)
        return false;
    if (display == NULL)
        display = lv_display_get_default();
    if (display == NULL)
        return false;

    /* LVGL 9.5's default theme accepts one base font. Size roles are applied
     * explicitly to this UI's widgets using the other two Roboto fonts. */
    theme = lv_theme_default_init(display, COLOR_PRIMARY, COLOR_DANGER, true,
                                  fonts->normal);
    if (theme == NULL)
        return false;
    lv_display_set_theme(display, theme);
    return true;
}

dmx_lvgl_ui_t *dmx_lvgl_ui_create(const dmx_lvgl_ui_config_t *config)
{
    dmx_lvgl_ui_t *ui;
    lv_obj_t *parent;
    lv_obj_t *scene_tab;
    lv_obj_t *rdm_tab;
    lv_obj_t *tab_bar;
    dmx_controller_snapshot_t initial_snapshot;

    if (config == NULL || config->fonts.small == NULL ||
        config->fonts.normal == NULL || config->fonts.large == NULL)
        return NULL;
    if (config->install_theme &&
        !dmx_lvgl_ui_apply_roboto_theme(config->display, &config->fonts))
        return NULL;
    parent = config->parent != NULL ? config->parent : lv_screen_active();
    if (parent == NULL)
        return NULL;

    ui = calloc(1u, sizeof(*ui));
    if (ui == NULL)
        return NULL;
    ui->fonts = config->fonts;

    ui->root = lv_obj_create(parent);
    lv_obj_set_pos(ui->root, 0, 0);
    lv_obj_set_size(ui->root, lv_pct(100), lv_pct(100));
    lv_obj_set_style_pad_all(ui->root, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(ui->root, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(ui->root, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_color(ui->root, COLOR_BACKGROUND, LV_PART_MAIN);
    lv_obj_set_style_text_font(ui->root, ui->fonts.normal, LV_PART_MAIN);
    lv_obj_remove_flag(ui->root, LV_OBJ_FLAG_SCROLLABLE);

    ui->tabview = lv_tabview_create(ui->root);
    lv_obj_set_size(ui->tabview, DMX_PANEL_UI_WIDTH, DMX_PANEL_UI_HEIGHT);
    lv_obj_align(ui->tabview, LV_ALIGN_CENTER, 0, 0);
    lv_tabview_set_tab_bar_position(ui->tabview, LV_DIR_TOP);
    lv_tabview_set_tab_bar_size(ui->tabview, 68);
    tab_bar = lv_tabview_get_tab_bar(ui->tabview);
    lv_obj_set_style_bg_color(tab_bar, COLOR_SURFACE, LV_PART_MAIN);
    lv_obj_set_style_text_font(tab_bar, ui->fonts.large, LV_PART_MAIN);
    lv_obj_set_style_text_color(tab_bar, COLOR_TEXT, LV_PART_MAIN);

    scene_tab = lv_tabview_add_tab(ui->tabview, "Сцена");
    rdm_tab = lv_tabview_add_tab(ui->tabview, "RDM");
    /* The color wheel and brightness slider are vertical/horizontal controls,
     * not tab gestures. Keep tab changes on the buttons only. */
    lv_obj_remove_flag(lv_tabview_get_content(ui->tabview),
                       LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ONE);
    lv_obj_set_scroll_dir(lv_tabview_get_content(ui->tabview), LV_DIR_NONE);
    lv_obj_set_style_text_font(lv_tabview_get_tab_button(ui->tabview, 0),
                               ui->fonts.large, LV_PART_MAIN);
    lv_obj_set_style_text_font(lv_tabview_get_tab_button(ui->tabview, 1),
                               ui->fonts.large, LV_PART_MAIN);
    if (!create_scene_tab(ui, scene_tab) || !create_rdm_tab(ui, rdm_tab)) {
        dmx_lvgl_ui_destroy(ui);
        return NULL;
    }

    memset(&initial_snapshot, 0, sizeof(initial_snapshot));
    initial_snapshot.brightness = 255u;
    initial_snapshot.red = 255u;
    initial_snapshot.scene_address = 1u;
    (void)snprintf(initial_snapshot.status, sizeof(initial_snapshot.status),
                   "Контроллер не запущен");
    dmx_lvgl_ui_set_snapshot(ui, &initial_snapshot);
    return ui;
}

void dmx_lvgl_ui_destroy(dmx_lvgl_ui_t *ui)
{
    if (ui == NULL)
        return;
    if (ui->root != NULL && lv_obj_is_valid(ui->root))
        lv_obj_delete(ui->root);
    free(ui->scene_wheel_pixels);
    free(ui);
}

void dmx_lvgl_ui_set_snapshot(dmx_lvgl_ui_t *ui,
                              const dmx_controller_snapshot_t *snapshot)
{
    char text[256];
    lv_color_hsv_t hsv;

    if (ui == NULL || snapshot == NULL)
        return;
    ui->snapshot = *snapshot;
    if (ui->snapshot.device_count > DMX_MAX_DISCOVERED_DEVICES)
        ui->snapshot.device_count = DMX_MAX_DISCOVERED_DEVICES;
    snapshot = &ui->snapshot;
    ui->syncing = true;

    ui->scene_color = lv_color_make(snapshot->red, snapshot->green,
                                    snapshot->blue);
    hsv = lv_color_rgb_to_hsv(snapshot->red, snapshot->green,
                              snapshot->blue);
    ui->scene_hue = hsv.h;
    ui->scene_saturation = hsv.s;
    position_wheel_marker(ui);
    lv_slider_set_value(ui->scene_brightness, snapshot->brightness,
                        LV_ANIM_OFF);
    lv_spinbox_set_value(ui->scene_address,
                         snapshot->scene_address < 1u ? 1u
                                                      : snapshot->scene_address);
    if (snapshot->blackout)
        lv_obj_add_state(ui->scene_blackout, LV_STATE_CHECKED);
    else
        lv_obj_remove_state(ui->scene_blackout, LV_STATE_CHECKED);
    update_scene_swatch(ui);

    (void)snprintf(text, sizeof(text),
                   "%s%s  •  DMX: %" PRIu32 "  •  RDM: %" PRIu32
                   " / ошибок %" PRIu32,
                   snapshot->status,
                   snapshot->timing_warning ? "  •  TIMING!" : "",
                   snapshot->dmx_frames_sent, snapshot->rdm_transactions,
                   snapshot->rdm_errors);
    lv_label_set_text(ui->scene_status, text);
    lv_obj_set_style_text_color(ui->scene_status,
                                snapshot->timing_warning ? COLOR_WARNING
                                                         : COLOR_MUTED,
                                LV_PART_MAIN);

    (void)snprintf(text, sizeof(text), "RDM: %s%s",
                   snapshot->rdm_busy ? "операция выполняется" : "готов",
                   snapshot->serial_open ? "" : "  •  UART закрыт");
    lv_label_set_text(ui->rdm_bus_status, text);
    set_obj_enabled(ui->rdm_discover, !snapshot->rdm_busy);
    (void)snprintf(text, sizeof(text),
                   "Устройств: %zu  •  Discovery-запросов: %" PRIu32
                   "  •  %s",
                   snapshot->device_count, snapshot->discovery_queries,
                   snapshot->status);
    lv_label_set_text(ui->rdm_status, text);

    refresh_rdm_list(ui);
    refresh_rdm_detail(ui);
    ui->syncing = false;
}

bool dmx_lvgl_ui_pop_action(dmx_lvgl_ui_t *ui, panel_ui_action_t *action)
{
    if (ui == NULL || action == NULL || ui->action_count == 0u)
        return false;
    *action = ui->action_queue[ui->action_head];
    ui->action_head = (ui->action_head + 1u) % ACTION_QUEUE_CAPACITY;
    --ui->action_count;
    return true;
}

size_t dmx_lvgl_ui_pending_actions(const dmx_lvgl_ui_t *ui)
{
    return ui != NULL ? ui->action_count : 0u;
}

lv_obj_t *dmx_lvgl_ui_root(dmx_lvgl_ui_t *ui)
{
    return ui != NULL ? ui->root : NULL;
}
