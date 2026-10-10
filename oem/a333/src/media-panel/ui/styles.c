#include "styles.h"
#include "images.h"
#include "fonts.h"

#include "ui.h"
#include "screens.h"

//
// Style: SkodaStyleScreen
//

void init_style_skoda_style_screen_MAIN_DEFAULT(lv_style_t *style) {
    lv_style_set_bg_image_src(style, &img_bg_1280_800);
};

lv_style_t *get_style_skoda_style_screen_MAIN_DEFAULT() {
    static lv_style_t *style;
    if (!style) {
        style = (lv_style_t *)lv_malloc(sizeof(lv_style_t));
        lv_style_init(style);
        init_style_skoda_style_screen_MAIN_DEFAULT(style);
    }
    return style;
};

void add_style_skoda_style_screen(lv_obj_t *obj) {
    (void)obj;
    lv_obj_add_style(obj, get_style_skoda_style_screen_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

void remove_style_skoda_style_screen(lv_obj_t *obj) {
    (void)obj;
    lv_obj_remove_style(obj, get_style_skoda_style_screen_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

//
// Style: SkodaStyleTopBar
//

void init_style_skoda_style_top_bar_MAIN_DEFAULT(lv_style_t *style) {
    lv_style_set_bg_opa(style, 120);
    lv_style_set_bg_color(style, lv_color_hex(0x000000));
    lv_style_set_bg_grad_color(style, lv_color_hex(0x898989));
    lv_style_set_bg_grad_dir(style, LV_GRAD_DIR_VER);
    lv_style_set_bg_grad_stop(style, 255);
};

lv_style_t *get_style_skoda_style_top_bar_MAIN_DEFAULT() {
    static lv_style_t *style;
    if (!style) {
        style = (lv_style_t *)lv_malloc(sizeof(lv_style_t));
        lv_style_init(style);
        init_style_skoda_style_top_bar_MAIN_DEFAULT(style);
    }
    return style;
};

void add_style_skoda_style_top_bar(lv_obj_t *obj) {
    (void)obj;
    lv_obj_add_style(obj, get_style_skoda_style_top_bar_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

void remove_style_skoda_style_top_bar(lv_obj_t *obj) {
    (void)obj;
    lv_obj_remove_style(obj, get_style_skoda_style_top_bar_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

//
// Style: SkodaStyleNotifyPanel
//

void init_style_skoda_style_notify_panel_MAIN_DEFAULT(lv_style_t *style) {
    lv_style_set_border_color(style, lv_color_hex(0xe8e8e8));
    lv_style_set_border_width(style, 3);
    lv_style_set_bg_color(style, lv_color_hex(0x000000));
    lv_style_set_bg_opa(style, 240);
};

lv_style_t *get_style_skoda_style_notify_panel_MAIN_DEFAULT() {
    static lv_style_t *style;
    if (!style) {
        style = (lv_style_t *)lv_malloc(sizeof(lv_style_t));
        lv_style_init(style);
        init_style_skoda_style_notify_panel_MAIN_DEFAULT(style);
    }
    return style;
};

void add_style_skoda_style_notify_panel(lv_obj_t *obj) {
    (void)obj;
    lv_obj_add_style(obj, get_style_skoda_style_notify_panel_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

void remove_style_skoda_style_notify_panel(lv_obj_t *obj) {
    (void)obj;
    lv_obj_remove_style(obj, get_style_skoda_style_notify_panel_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

//
// Style: SkodaStyleTabView
//

void init_style_skoda_style_tab_view_MAIN_DEFAULT(lv_style_t *style) {
    lv_style_set_bg_opa(style, 0);
};

lv_style_t *get_style_skoda_style_tab_view_MAIN_DEFAULT() {
    static lv_style_t *style;
    if (!style) {
        style = (lv_style_t *)lv_malloc(sizeof(lv_style_t));
        lv_style_init(style);
        init_style_skoda_style_tab_view_MAIN_DEFAULT(style);
    }
    return style;
};

void add_style_skoda_style_tab_view(lv_obj_t *obj) {
    (void)obj;
    lv_obj_add_style(obj, get_style_skoda_style_tab_view_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

void remove_style_skoda_style_tab_view(lv_obj_t *obj) {
    (void)obj;
    lv_obj_remove_style(obj, get_style_skoda_style_tab_view_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

//
// Style: SkodaStyleTabBar
//

void init_style_skoda_style_tab_bar_MAIN_DEFAULT(lv_style_t *style) {
    lv_style_set_bg_opa(style, 120);
    lv_style_set_bg_color(style, lv_color_hex(0x000000));
    lv_style_set_bg_grad_stop(style, 255);
    lv_style_set_bg_grad_color(style, lv_color_hex(0x898989));
    lv_style_set_bg_grad_dir(style, LV_GRAD_DIR_VER);
    lv_style_set_outline_color(style, lv_color_hex(0xfafafa));
    lv_style_set_outline_width(style, 3);
    lv_style_set_outline_opa(style, 180);
    lv_style_set_text_font(style, &ui_font_skoda_pro_medium_28);
};

lv_style_t *get_style_skoda_style_tab_bar_MAIN_DEFAULT() {
    static lv_style_t *style;
    if (!style) {
        style = (lv_style_t *)lv_malloc(sizeof(lv_style_t));
        lv_style_init(style);
        init_style_skoda_style_tab_bar_MAIN_DEFAULT(style);
    }
    return style;
};

void init_style_skoda_style_tab_bar_ITEMS_DEFAULT(lv_style_t *style) {
    lv_style_set_border_opa(style, 255);
    lv_style_set_border_width(style, 2);
    lv_style_set_border_color(style, lv_color_hex(0xff0000));
    lv_style_set_outline_color(style, lv_color_hex(0xff0000));
    lv_style_set_outline_width(style, 3);
};

lv_style_t *get_style_skoda_style_tab_bar_ITEMS_DEFAULT() {
    static lv_style_t *style;
    if (!style) {
        style = (lv_style_t *)lv_malloc(sizeof(lv_style_t));
        lv_style_init(style);
        init_style_skoda_style_tab_bar_ITEMS_DEFAULT(style);
    }
    return style;
};

void init_style_skoda_style_tab_bar_ITEMS_CHECKED(lv_style_t *style) {
    lv_style_set_border_color(style, lv_color_hex(0xff0000));
    lv_style_set_border_width(style, 3);
    lv_style_set_border_opa(style, 255);
    lv_style_set_outline_width(style, 3);
    lv_style_set_outline_color(style, lv_color_hex(0xff0000));
    lv_style_set_outline_opa(style, 255);
};

lv_style_t *get_style_skoda_style_tab_bar_ITEMS_CHECKED() {
    static lv_style_t *style;
    if (!style) {
        style = (lv_style_t *)lv_malloc(sizeof(lv_style_t));
        lv_style_init(style);
        init_style_skoda_style_tab_bar_ITEMS_CHECKED(style);
    }
    return style;
};

void add_style_skoda_style_tab_bar(lv_obj_t *obj) {
    (void)obj;
    lv_obj_add_style(obj, get_style_skoda_style_tab_bar_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_style(obj, get_style_skoda_style_tab_bar_ITEMS_DEFAULT(), LV_PART_ITEMS | LV_STATE_DEFAULT);
    lv_obj_add_style(obj, get_style_skoda_style_tab_bar_ITEMS_CHECKED(), LV_PART_ITEMS | LV_STATE_CHECKED);
};

void remove_style_skoda_style_tab_bar(lv_obj_t *obj) {
    (void)obj;
    lv_obj_remove_style(obj, get_style_skoda_style_tab_bar_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_remove_style(obj, get_style_skoda_style_tab_bar_ITEMS_DEFAULT(), LV_PART_ITEMS | LV_STATE_DEFAULT);
    lv_obj_remove_style(obj, get_style_skoda_style_tab_bar_ITEMS_CHECKED(), LV_PART_ITEMS | LV_STATE_CHECKED);
};

//
// Style: SkodaStyleImageButtonPlayback
//

void init_style_skoda_style_image_button_playback_MAIN_DEFAULT(lv_style_t *style) {
    lv_style_set_bg_opa(style, 0);
    lv_style_set_bg_color(style, lv_color_hex(0x000000));
};

lv_style_t *get_style_skoda_style_image_button_playback_MAIN_DEFAULT() {
    static lv_style_t *style;
    if (!style) {
        style = (lv_style_t *)lv_malloc(sizeof(lv_style_t));
        lv_style_init(style);
        init_style_skoda_style_image_button_playback_MAIN_DEFAULT(style);
    }
    return style;
};

void init_style_skoda_style_image_button_playback_MAIN_PRESSED(lv_style_t *style) {
    lv_style_set_image_recolor(style, lv_color_hex(0x000000));
    lv_style_set_image_recolor_opa(style, 180);
    lv_style_set_bg_image_opa(style, 70);
};

lv_style_t *get_style_skoda_style_image_button_playback_MAIN_PRESSED() {
    static lv_style_t *style;
    if (!style) {
        style = (lv_style_t *)lv_malloc(sizeof(lv_style_t));
        lv_style_init(style);
        init_style_skoda_style_image_button_playback_MAIN_PRESSED(style);
    }
    return style;
};

void add_style_skoda_style_image_button_playback(lv_obj_t *obj) {
    (void)obj;
    lv_obj_add_style(obj, get_style_skoda_style_image_button_playback_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_style(obj, get_style_skoda_style_image_button_playback_MAIN_PRESSED(), LV_PART_MAIN | LV_STATE_PRESSED);
};

void remove_style_skoda_style_image_button_playback(lv_obj_t *obj) {
    (void)obj;
    lv_obj_remove_style(obj, get_style_skoda_style_image_button_playback_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_remove_style(obj, get_style_skoda_style_image_button_playback_MAIN_PRESSED(), LV_PART_MAIN | LV_STATE_PRESSED);
};

//
// Style: SkodaStyleTabs
//

void add_style_skoda_style_tabs(lv_obj_t *obj) {
    (void)obj;
};

void remove_style_skoda_style_tabs(lv_obj_t *obj) {
    (void)obj;
};

//
// Style: SkodaStyleProgressBar
//

void init_style_skoda_style_progress_bar_MAIN_DEFAULT(lv_style_t *style) {
    lv_style_set_align(style, LV_ALIGN_CENTER);
    lv_style_set_bg_color(style, lv_color_hex(0xffffff));
    lv_style_set_bg_opa(style, 60);
};

lv_style_t *get_style_skoda_style_progress_bar_MAIN_DEFAULT() {
    static lv_style_t *style;
    if (!style) {
        style = (lv_style_t *)lv_malloc(sizeof(lv_style_t));
        lv_style_init(style);
        init_style_skoda_style_progress_bar_MAIN_DEFAULT(style);
    }
    return style;
};

void init_style_skoda_style_progress_bar_INDICATOR_DEFAULT(lv_style_t *style) {
    lv_style_set_bg_color(style, lv_color_darken(lv_color_hex(0x7ce735), 26));
    lv_style_set_bg_grad_dir(style, LV_GRAD_DIR_HOR);
    lv_style_set_bg_grad_color(style, lv_color_darken(lv_color_hex(0xceee70), 26));
    lv_style_set_bg_grad_stop(style, 120);
};

lv_style_t *get_style_skoda_style_progress_bar_INDICATOR_DEFAULT() {
    static lv_style_t *style;
    if (!style) {
        style = (lv_style_t *)lv_malloc(sizeof(lv_style_t));
        lv_style_init(style);
        init_style_skoda_style_progress_bar_INDICATOR_DEFAULT(style);
    }
    return style;
};

void add_style_skoda_style_progress_bar(lv_obj_t *obj) {
    (void)obj;
    lv_obj_add_style(obj, get_style_skoda_style_progress_bar_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_style(obj, get_style_skoda_style_progress_bar_INDICATOR_DEFAULT(), LV_PART_INDICATOR | LV_STATE_DEFAULT);
};

void remove_style_skoda_style_progress_bar(lv_obj_t *obj) {
    (void)obj;
    lv_obj_remove_style(obj, get_style_skoda_style_progress_bar_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_remove_style(obj, get_style_skoda_style_progress_bar_INDICATOR_DEFAULT(), LV_PART_INDICATOR | LV_STATE_DEFAULT);
};

//
// Style: SkodaStyleSlider
//

void init_style_skoda_style_slider_MAIN_DEFAULT(lv_style_t *style) {
    lv_style_set_align(style, LV_ALIGN_CENTER);
    lv_style_set_bg_color(style, lv_color_hex(0xffffff));
    lv_style_set_bg_opa(style, 60);
};

lv_style_t *get_style_skoda_style_slider_MAIN_DEFAULT() {
    static lv_style_t *style;
    if (!style) {
        style = (lv_style_t *)lv_malloc(sizeof(lv_style_t));
        lv_style_init(style);
        init_style_skoda_style_slider_MAIN_DEFAULT(style);
    }
    return style;
};

void init_style_skoda_style_slider_INDICATOR_DEFAULT(lv_style_t *style) {
    lv_style_set_bg_color(style, lv_color_darken(lv_color_hex(0x7ce735), 26));
    lv_style_set_bg_grad_dir(style, LV_GRAD_DIR_HOR);
    lv_style_set_bg_grad_color(style, lv_color_darken(lv_color_hex(0xceee70), 26));
};

lv_style_t *get_style_skoda_style_slider_INDICATOR_DEFAULT() {
    static lv_style_t *style;
    if (!style) {
        style = (lv_style_t *)lv_malloc(sizeof(lv_style_t));
        lv_style_init(style);
        init_style_skoda_style_slider_INDICATOR_DEFAULT(style);
    }
    return style;
};

void add_style_skoda_style_slider(lv_obj_t *obj) {
    (void)obj;
    lv_obj_add_style(obj, get_style_skoda_style_slider_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_style(obj, get_style_skoda_style_slider_INDICATOR_DEFAULT(), LV_PART_INDICATOR | LV_STATE_DEFAULT);
};

void remove_style_skoda_style_slider(lv_obj_t *obj) {
    (void)obj;
    lv_obj_remove_style(obj, get_style_skoda_style_slider_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_remove_style(obj, get_style_skoda_style_slider_INDICATOR_DEFAULT(), LV_PART_INDICATOR | LV_STATE_DEFAULT);
};

//
//
//

void add_style(lv_obj_t *obj, int32_t styleIndex) {
    typedef void (*AddStyleFunc)(lv_obj_t *obj);
    static const AddStyleFunc add_style_funcs[] = {
        add_style_skoda_style_screen,
        add_style_skoda_style_top_bar,
        add_style_skoda_style_notify_panel,
        add_style_skoda_style_tab_view,
        add_style_skoda_style_tab_bar,
        add_style_skoda_style_image_button_playback,
        add_style_skoda_style_tabs,
        add_style_skoda_style_progress_bar,
        add_style_skoda_style_slider,
    };
    add_style_funcs[styleIndex](obj);
}

void remove_style(lv_obj_t *obj, int32_t styleIndex) {
    typedef void (*RemoveStyleFunc)(lv_obj_t *obj);
    static const RemoveStyleFunc remove_style_funcs[] = {
        remove_style_skoda_style_screen,
        remove_style_skoda_style_top_bar,
        remove_style_skoda_style_notify_panel,
        remove_style_skoda_style_tab_view,
        remove_style_skoda_style_tab_bar,
        remove_style_skoda_style_image_button_playback,
        remove_style_skoda_style_tabs,
        remove_style_skoda_style_progress_bar,
        remove_style_skoda_style_slider,
    };
    remove_style_funcs[styleIndex](obj);
}