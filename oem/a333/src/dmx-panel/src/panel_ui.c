#include "panel_ui.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ARRAY_SIZE(array) (sizeof(array) / sizeof((array)[0]))
#define WHEEL_CENTER_X 230
#define WHEEL_CENTER_Y 365
#define WHEEL_RADIUS 202
#define WHEEL_SIDE (WHEEL_RADIUS * 2 + 1)
#define SCENE_BRIGHTNESS_X 480
#define SCENE_BRIGHTNESS_Y 270
#define SCENE_BRIGHTNESS_WIDTH 200
#define SCENE_BRIGHTNESS_HEIGHT 42
#define RDM_LIST_X 20
#define RDM_LIST_Y 171
#define RDM_LIST_WIDTH 315
#define RDM_ROW_HEIGHT 61
#define RDM_VISIBLE_ROWS 6

typedef struct {
    int x;
    int y;
    int width;
    int height;
} rectangle_t;

typedef enum {
    DRAG_NONE = 0,
    DRAG_COLOR_WHEEL,
    DRAG_BRIGHTNESS,
} drag_target_t;

struct panel_ui {
    dmx_controller_snapshot_t snapshot;
    panel_ui_view_t view;
    drag_target_t drag;
    panel_color_t *wheel_pixels;
    rdm_uid_t selected_uid;
    bool selected_valid;
    uint16_t rdm_edit_address;
    bool rdm_address_dirty;
    size_t rdm_scroll;
};

static const panel_color_t COLOR_BACKGROUND = PANEL_RGB(12, 18, 29);
static const panel_color_t COLOR_SURFACE = PANEL_RGB(24, 34, 50);
static const panel_color_t COLOR_SURFACE_ALT = PANEL_RGB(33, 46, 66);
static const panel_color_t COLOR_PRIMARY = PANEL_RGB(49, 191, 216);
static const panel_color_t COLOR_PRIMARY_DARK = PANEL_RGB(25, 110, 132);
static const panel_color_t COLOR_TEXT = PANEL_RGB(238, 244, 250);
static const panel_color_t COLOR_MUTED = PANEL_RGB(142, 158, 177);
static const panel_color_t COLOR_GOOD = PANEL_RGB(54, 211, 153);
static const panel_color_t COLOR_WARNING = PANEL_RGB(251, 191, 36);
static const panel_color_t COLOR_DANGER = PANEL_RGB(239, 68, 68);
static const panel_color_t COLOR_BLACK = PANEL_RGB(0, 0, 0);
static const panel_color_t COLOR_WHITE = PANEL_RGB(255, 255, 255);

static int minimum_int(int left, int right)
{
    return left < right ? left : right;
}

static int maximum_int(int left, int right)
{
    return left > right ? left : right;
}

static int clamp_int(int value, int minimum, int maximum)
{
    return minimum_int(maximum_int(value, minimum), maximum);
}

static bool point_in_rectangle(int x, int y, rectangle_t rectangle)
{
    return x >= rectangle.x && y >= rectangle.y &&
           x < rectangle.x + rectangle.width &&
           y < rectangle.y + rectangle.height;
}

static void fill_rectangle(panel_canvas_t *canvas,
                           rectangle_t rectangle,
                           panel_color_t color)
{
    int left;
    int top;
    int right;
    int bottom;
    int y;

    if (canvas == NULL || canvas->pixels == NULL)
        return;
    left = maximum_int(0, rectangle.x);
    top = maximum_int(0, rectangle.y);
    right = minimum_int(canvas->width, rectangle.x + rectangle.width);
    bottom = minimum_int(canvas->height, rectangle.y + rectangle.height);
    for (y = top; y < bottom; ++y) {
        panel_color_t *row =
            canvas->pixels + (size_t)y * canvas->stride + (size_t)left;
        int x;

        for (x = left; x < right; ++x)
            *row++ = color;
    }
}

static void stroke_rectangle(panel_canvas_t *canvas,
                             rectangle_t rectangle,
                             int thickness,
                             panel_color_t color)
{
    fill_rectangle(canvas,
                   (rectangle_t){rectangle.x,
                                 rectangle.y,
                                 rectangle.width,
                                 thickness},
                   color);
    fill_rectangle(canvas,
                   (rectangle_t){rectangle.x,
                                 rectangle.y + rectangle.height - thickness,
                                 rectangle.width,
                                 thickness},
                   color);
    fill_rectangle(canvas,
                   (rectangle_t){rectangle.x,
                                 rectangle.y,
                                 thickness,
                                 rectangle.height},
                   color);
    fill_rectangle(canvas,
                   (rectangle_t){rectangle.x + rectangle.width - thickness,
                                 rectangle.y,
                                 thickness,
                                 rectangle.height},
                   color);
}

static void fill_circle(panel_canvas_t *canvas,
                        int center_x,
                        int center_y,
                        int radius,
                        panel_color_t color)
{
    int y;

    for (y = -radius; y <= radius; ++y) {
        int x;
        int half_width = 0;

        while ((half_width + 1) * (half_width + 1) + y * y <=
               radius * radius)
            ++half_width;
        for (x = -half_width; x <= half_width; ++x)
            panel_canvas_put_pixel(canvas, center_x + x, center_y + y, color);
    }
}

static const uint8_t DIGIT_FONT[10][7] = {
    {14, 17, 19, 21, 25, 17, 14},
    {4, 12, 4, 4, 4, 4, 14},
    {14, 17, 1, 2, 4, 8, 31},
    {30, 1, 1, 14, 1, 1, 30},
    {2, 6, 10, 18, 31, 2, 2},
    {31, 16, 16, 30, 1, 1, 30},
    {6, 8, 16, 30, 17, 17, 14},
    {31, 1, 2, 4, 8, 8, 8},
    {14, 17, 17, 14, 17, 17, 14},
    {14, 17, 17, 15, 1, 2, 12},
};

static const uint8_t LETTER_FONT[26][7] = {
    {14, 17, 17, 31, 17, 17, 17},
    {30, 17, 17, 30, 17, 17, 30},
    {14, 17, 16, 16, 16, 17, 14},
    {28, 18, 17, 17, 17, 18, 28},
    {31, 16, 16, 30, 16, 16, 31},
    {31, 16, 16, 30, 16, 16, 16},
    {14, 17, 16, 23, 17, 17, 15},
    {17, 17, 17, 31, 17, 17, 17},
    {14, 4, 4, 4, 4, 4, 14},
    {7, 2, 2, 2, 2, 18, 12},
    {17, 18, 20, 24, 20, 18, 17},
    {16, 16, 16, 16, 16, 16, 31},
    {17, 27, 21, 21, 17, 17, 17},
    {17, 25, 21, 19, 17, 17, 17},
    {14, 17, 17, 17, 17, 17, 14},
    {30, 17, 17, 30, 16, 16, 16},
    {14, 17, 17, 17, 21, 18, 13},
    {30, 17, 17, 30, 20, 18, 17},
    {15, 16, 16, 14, 1, 1, 30},
    {31, 4, 4, 4, 4, 4, 4},
    {17, 17, 17, 17, 17, 17, 14},
    {17, 17, 17, 17, 17, 10, 4},
    {17, 17, 17, 21, 21, 21, 10},
    {17, 17, 10, 4, 10, 17, 17},
    {17, 17, 10, 4, 4, 4, 4},
    {31, 1, 2, 4, 8, 16, 31},
};

static void glyph_rows(char character, uint8_t rows[7])
{
    int i;

    memset(rows, 0, 7);
    if (character >= 'a' && character <= 'z')
        character = (char)(character - 'a' + 'A');
    if (character >= 'A' && character <= 'Z') {
        memcpy(rows, LETTER_FONT[character - 'A'], 7);
        return;
    }
    if (character >= '0' && character <= '9') {
        memcpy(rows, DIGIT_FONT[character - '0'], 7);
        return;
    }
    switch (character) {
    case '-':
        rows[3] = 14;
        break;
    case '_':
        rows[6] = 31;
        break;
    case ':':
        rows[2] = 4;
        rows[5] = 4;
        break;
    case '.':
        rows[6] = 4;
        break;
    case '/':
        rows[0] = 1;
        rows[1] = 2;
        rows[2] = 2;
        rows[3] = 4;
        rows[4] = 8;
        rows[5] = 8;
        rows[6] = 16;
        break;
    case '+':
        rows[2] = 4;
        rows[3] = 14;
        rows[4] = 4;
        break;
    case '%':
        rows[0] = 17;
        rows[1] = 2;
        rows[2] = 4;
        rows[3] = 4;
        rows[4] = 8;
        rows[5] = 16;
        rows[6] = 17;
        break;
    case '(':
        rows[1] = 2;
        rows[2] = 4;
        rows[3] = 4;
        rows[4] = 4;
        rows[5] = 2;
        break;
    case ')':
        rows[1] = 8;
        rows[2] = 4;
        rows[3] = 4;
        rows[4] = 4;
        rows[5] = 8;
        break;
    case '!':
        rows[0] = 4;
        rows[1] = 4;
        rows[2] = 4;
        rows[3] = 4;
        rows[6] = 4;
        break;
    case ' ':
        break;
    default:
        for (i = 0; i < 7; ++i)
            rows[i] = (i == 0 || i == 6) ? 31 : 17;
        break;
    }
}

static void draw_text(panel_canvas_t *canvas,
                      int x,
                      int y,
                      const char *text,
                      int scale,
                      panel_color_t color,
                      int maximum_width)
{
    int cursor = x;

    if (text == NULL || scale <= 0)
        return;
    while (*text != '\0') {
        uint8_t rows[7];
        int row;

        if (maximum_width > 0 && cursor + 5 * scale > x + maximum_width)
            break;
        glyph_rows(*text++, rows);
        for (row = 0; row < 7; ++row) {
            int column;

            for (column = 0; column < 5; ++column) {
                if ((rows[row] & (1u << (4 - column))) != 0)
                    fill_rectangle(canvas,
                                   (rectangle_t){cursor + column * scale,
                                                 y + row * scale,
                                                 scale,
                                                 scale},
                                   color);
            }
        }
        cursor += 6 * scale;
    }
}

static void draw_button(panel_canvas_t *canvas,
                        rectangle_t rectangle,
                        const char *label,
                        panel_color_t background,
                        panel_color_t foreground,
                        int scale)
{
    int text_width = (int)strlen(label) * 6 * scale - scale;
    int text_height = 7 * scale;

    fill_rectangle(canvas, rectangle, background);
    stroke_rectangle(canvas, rectangle, 2, COLOR_SURFACE_ALT);
    draw_text(canvas,
              rectangle.x + (rectangle.width - text_width) / 2,
              rectangle.y + (rectangle.height - text_height) / 2,
              label,
              scale,
              foreground,
              rectangle.width - 8);
}

static uint32_t integer_square_root(uint32_t value)
{
    uint32_t result = 0;
    uint32_t bit = UINT32_C(1) << 30;

    while (bit > value)
        bit >>= 2;
    while (bit != 0) {
        if (value >= result + bit) {
            value -= result + bit;
            result = (result >> 1) + bit;
        } else {
            result >>= 1;
        }
        bit >>= 2;
    }
    return result;
}

/* Approximate polar angle, in HSV units 0..1535, without libm. */
static uint16_t point_hue(int x, int y)
{
    int absolute_x = x < 0 ? -x : x;
    int absolute_y = y < 0 ? -y : y;
    int hue;

    if (x >= 0 && y >= 0) {
        hue = absolute_x >= absolute_y
                  ? (absolute_y * 192) / maximum_int(absolute_x, 1)
                  : 384 - (absolute_x * 192) / maximum_int(absolute_y, 1);
    } else if (x < 0 && y >= 0) {
        hue = absolute_y >= absolute_x
                  ? 384 + (absolute_x * 192) / maximum_int(absolute_y, 1)
                  : 768 - (absolute_y * 192) / maximum_int(absolute_x, 1);
    } else if (x < 0) {
        hue = absolute_x >= absolute_y
                  ? 768 + (absolute_y * 192) / maximum_int(absolute_x, 1)
                  : 1152 - (absolute_x * 192) / maximum_int(absolute_y, 1);
    } else {
        hue = absolute_y >= absolute_x
                  ? 1152 + (absolute_x * 192) / maximum_int(absolute_y, 1)
                  : 1536 - (absolute_y * 192) / maximum_int(absolute_x, 1);
    }
    return (uint16_t)(hue % 1536);
}

static panel_color_t hsv_color(uint16_t hue, uint8_t saturation, uint8_t value)
{
    unsigned int sector = (hue % 1536u) / 256u;
    unsigned int fraction = hue % 256u;
    unsigned int p = (unsigned int)value * (255u - saturation) / 255u;
    unsigned int q =
        (unsigned int)value *
        (255u - (unsigned int)saturation * fraction / 255u) / 255u;
    unsigned int t =
        (unsigned int)value *
        (255u - (unsigned int)saturation * (255u - fraction) / 255u) /
        255u;
    unsigned int red;
    unsigned int green;
    unsigned int blue;

    switch (sector) {
    case 0:
        red = value;
        green = t;
        blue = p;
        break;
    case 1:
        red = q;
        green = value;
        blue = p;
        break;
    case 2:
        red = p;
        green = value;
        blue = t;
        break;
    case 3:
        red = p;
        green = q;
        blue = value;
        break;
    case 4:
        red = t;
        green = p;
        blue = value;
        break;
    default:
        red = value;
        green = p;
        blue = q;
        break;
    }
    return PANEL_RGB(red, green, blue);
}

static void rgb_to_hsv(uint8_t red,
                       uint8_t green,
                       uint8_t blue,
                       uint16_t *hue,
                       uint8_t *saturation)
{
    int maximum = maximum_int(red, maximum_int(green, blue));
    int minimum = minimum_int(red, minimum_int(green, blue));
    int delta = maximum - minimum;
    int computed_hue;

    *saturation = maximum == 0 ? 0 : (uint8_t)(delta * 255 / maximum);
    if (delta == 0) {
        *hue = 0;
        return;
    }
    if (maximum == red)
        computed_hue = 256 * ((int)green - blue) / delta;
    else if (maximum == green)
        computed_hue = 512 + 256 * ((int)blue - red) / delta;
    else
        computed_hue = 1024 + 256 * ((int)red - green) / delta;
    while (computed_hue < 0)
        computed_hue += 1536;
    *hue = (uint16_t)(computed_hue % 1536);
}

static void hue_to_point(uint16_t hue, int radius, int *x, int *y)
{
    int quadrant = (hue % 1536u) / 384u;
    int local = (hue % 1536u) % 384;
    int first;
    int second;
    uint32_t denominator;

    if (local <= 192) {
        denominator = integer_square_root((uint32_t)(192 * 192 + local * local));
        first = radius * 192 / maximum_int((int)denominator, 1);
        second = radius * local / maximum_int((int)denominator, 1);
    } else {
        int reverse = 384 - local;

        denominator =
            integer_square_root((uint32_t)(192 * 192 + reverse * reverse));
        first = radius * reverse / maximum_int((int)denominator, 1);
        second = radius * 192 / maximum_int((int)denominator, 1);
    }

    switch (quadrant) {
    case 0:
        *x = first;
        *y = second;
        break;
    case 1:
        *x = -second;
        *y = first;
        break;
    case 2:
        *x = -first;
        *y = -second;
        break;
    default:
        *x = second;
        *y = -first;
        break;
    }
}

static int build_wheel(panel_ui_t *ui)
{
    int y;

    ui->wheel_pixels = calloc((size_t)WHEEL_SIDE * WHEEL_SIDE,
                              sizeof(*ui->wheel_pixels));
    if (ui->wheel_pixels == NULL)
        return -1;
    for (y = -WHEEL_RADIUS; y <= WHEEL_RADIUS; ++y) {
        int x;

        for (x = -WHEEL_RADIUS; x <= WHEEL_RADIUS; ++x) {
            uint32_t squared = (uint32_t)(x * x + y * y);
            uint32_t distance;
            uint8_t saturation;

            if (squared > (uint32_t)(WHEEL_RADIUS * WHEEL_RADIUS))
                continue;
            distance = integer_square_root(squared);
            saturation =
                (uint8_t)minimum_int(255, (int)(distance * 255u / WHEEL_RADIUS));
            ui->wheel_pixels[(size_t)(y + WHEEL_RADIUS) * WHEEL_SIDE +
                             (size_t)(x + WHEEL_RADIUS)] =
                hsv_color(point_hue(x, y), saturation, 255);
        }
    }
    return 0;
}

panel_ui_t *panel_ui_create(void)
{
    panel_ui_t *ui = calloc(1, sizeof(*ui));

    if (ui == NULL)
        return NULL;
    ui->snapshot.red = 255;
    ui->snapshot.brightness = 255;
    ui->snapshot.scene_address = 1;
    (void)snprintf(ui->snapshot.status,
                   sizeof(ui->snapshot.status),
                   "CONTROLLER NOT CONNECTED");
    if (build_wheel(ui) < 0) {
        panel_ui_destroy(ui);
        errno = ENOMEM;
        return NULL;
    }
    return ui;
}

void panel_ui_destroy(panel_ui_t *ui)
{
    if (ui == NULL)
        return;
    free(ui->wheel_pixels);
    free(ui);
}

static int find_selected_index(const panel_ui_t *ui)
{
    size_t i;

    if (!ui->selected_valid)
        return -1;
    for (i = 0; i < ui->snapshot.device_count; ++i) {
        if (rdm_uid_equal(&ui->snapshot.devices[i].uid, &ui->selected_uid))
            return (int)i;
    }
    return -1;
}

static void select_device(panel_ui_t *ui, size_t index)
{
    if (index >= ui->snapshot.device_count) {
        ui->selected_valid = false;
        ui->rdm_edit_address = 1;
        ui->rdm_address_dirty = false;
        return;
    }
    ui->selected_uid = ui->snapshot.devices[index].uid;
    ui->selected_valid = true;
    ui->rdm_edit_address = ui->snapshot.devices[index].dmx_start_address;
    if (ui->rdm_edit_address < 1 || ui->rdm_edit_address > DMX_UNIVERSE_SLOTS)
        ui->rdm_edit_address = 1;
    ui->rdm_address_dirty = false;
}

void panel_ui_set_snapshot(panel_ui_t *ui,
                           const dmx_controller_snapshot_t *snapshot)
{
    int selected_index;
    uint16_t previous_device_address = 0;
    size_t i;

    if (ui == NULL || snapshot == NULL)
        return;
    selected_index = find_selected_index(ui);
    if (selected_index >= 0)
        previous_device_address =
            ui->snapshot.devices[selected_index].dmx_start_address;
    memcpy(&ui->snapshot, snapshot, sizeof(ui->snapshot));
    if (ui->snapshot.device_count > DMX_MAX_DISCOVERED_DEVICES)
        ui->snapshot.device_count = DMX_MAX_DISCOVERED_DEVICES;
    ui->snapshot.status[DMX_STATUS_TEXT_SIZE - 1u] = '\0';
    for (i = 0; i < ui->snapshot.device_count; ++i) {
        ui->snapshot.devices[i].manufacturer_label[DMX_RDM_LABEL_SIZE - 1u] =
            '\0';
        ui->snapshot.devices[i].model_description[DMX_RDM_LABEL_SIZE - 1u] =
            '\0';
        ui->snapshot.devices[i].device_label[DMX_RDM_LABEL_SIZE - 1u] = '\0';
        ui->snapshot.devices[i]
            .software_version_label[DMX_RDM_LABEL_SIZE - 1u] = '\0';
    }
    if (ui->snapshot.scene_address < 1 || ui->snapshot.scene_address > 510)
        ui->snapshot.scene_address = 1;

    selected_index = find_selected_index(ui);
    if (selected_index < 0 && ui->snapshot.device_count != 0)
        select_device(ui, 0);
    else if (selected_index < 0)
        select_device(ui, ui->snapshot.device_count);
    else if (!ui->rdm_address_dirty ||
             previous_device_address !=
                 ui->snapshot.devices[selected_index].dmx_start_address) {
        ui->rdm_edit_address =
            ui->snapshot.devices[selected_index].dmx_start_address;
        ui->rdm_address_dirty = false;
    }
    if (ui->rdm_scroll >= ui->snapshot.device_count)
        ui->rdm_scroll = ui->snapshot.device_count == 0
                             ? 0
                             : ui->snapshot.device_count - 1u;
}

void panel_ui_set_view(panel_ui_t *ui, panel_ui_view_t view)
{
    if (ui != NULL && (view == PANEL_UI_VIEW_SCENE || view == PANEL_UI_VIEW_RDM))
        ui->view = view;
}

panel_ui_view_t panel_ui_get_view(const panel_ui_t *ui)
{
    return ui != NULL ? ui->view : PANEL_UI_VIEW_SCENE;
}

static panel_color_t scaled_scene_color(const dmx_controller_snapshot_t *snapshot)
{
    unsigned int brightness = snapshot->blackout ? 0u : snapshot->brightness;

    return PANEL_RGB((unsigned int)snapshot->red * brightness / 255u,
                     (unsigned int)snapshot->green * brightness / 255u,
                     (unsigned int)snapshot->blue * brightness / 255u);
}

static void render_header(panel_ui_t *ui, panel_canvas_t *canvas)
{
    panel_color_t connection = ui->snapshot.serial_open ? COLOR_GOOD : COLOR_DANGER;
    char status[48];

    fill_rectangle(canvas, (rectangle_t){0, 0, 720, 91}, COLOR_SURFACE);
    draw_text(canvas, 20, 14, "DMX MASTER", 3, COLOR_TEXT, 280);
    fill_circle(canvas, 467, 26, 7, connection);
    draw_text(canvas,
              482,
              18,
              ui->snapshot.serial_open ? "DMX ONLINE" : "DMX OFFLINE",
              2,
              COLOR_MUTED,
              210);
    (void)snprintf(status,
                   sizeof(status),
                   "FRAMES %lu",
                   (unsigned long)ui->snapshot.dmx_frames_sent);
    draw_text(canvas, 482, 43, status, 1, COLOR_MUTED, 210);

    draw_button(canvas,
                (rectangle_t){20, 57, 145, 34},
                "SCENE",
                ui->view == PANEL_UI_VIEW_SCENE ? COLOR_PRIMARY_DARK
                                                : COLOR_SURFACE_ALT,
                COLOR_TEXT,
                2);
    draw_button(canvas,
                (rectangle_t){176, 57, 145, 34},
                "RDM",
                ui->view == PANEL_UI_VIEW_RDM ? COLOR_PRIMARY_DARK
                                              : COLOR_SURFACE_ALT,
                COLOR_TEXT,
                2);
}

static void render_footer(panel_ui_t *ui, panel_canvas_t *canvas)
{
    panel_color_t status_color = ui->snapshot.timing_warning ? COLOR_WARNING
                                                              : COLOR_MUTED;

    fill_rectangle(canvas, (rectangle_t){0, 672, 720, 48}, COLOR_SURFACE);
    draw_text(canvas, 18, 682, ui->snapshot.status, 1, status_color, 684);
    if (ui->snapshot.rdm_busy)
        draw_text(canvas, 610, 701, "RDM BUSY", 1, COLOR_WARNING, 92);
}

static void render_wheel(panel_ui_t *ui, panel_canvas_t *canvas)
{
    int y;
    uint16_t hue;
    uint8_t saturation;
    int marker_x;
    int marker_y;

    for (y = 0; y < WHEEL_SIDE; ++y) {
        int x;

        for (x = 0; x < WHEEL_SIDE; ++x) {
            panel_color_t pixel =
                ui->wheel_pixels[(size_t)y * WHEEL_SIDE + (size_t)x];

            if (pixel != 0)
                panel_canvas_put_pixel(canvas,
                                       WHEEL_CENTER_X - WHEEL_RADIUS + x,
                                       WHEEL_CENTER_Y - WHEEL_RADIUS + y,
                                       pixel);
        }
    }
    rgb_to_hsv(ui->snapshot.red,
               ui->snapshot.green,
               ui->snapshot.blue,
               &hue,
               &saturation);
    hue_to_point(hue,
                 (int)saturation * WHEEL_RADIUS / 255,
                 &marker_x,
                 &marker_y);
    marker_x += WHEEL_CENTER_X;
    marker_y += WHEEL_CENTER_Y;
    fill_circle(canvas, marker_x, marker_y, 11, COLOR_BLACK);
    fill_circle(canvas, marker_x, marker_y, 7, COLOR_WHITE);
    fill_circle(canvas,
                marker_x,
                marker_y,
                4,
                PANEL_RGB(ui->snapshot.red,
                          ui->snapshot.green,
                          ui->snapshot.blue));
}

static void render_scene(panel_ui_t *ui, panel_canvas_t *canvas)
{
    char text[80];
    int filled =
        (int)ui->snapshot.brightness * SCENE_BRIGHTNESS_WIDTH / 255;

    draw_text(canvas, 58, 118, "COLOR", 2, COLOR_MUTED, 160);
    render_wheel(ui, canvas);

    fill_rectangle(canvas, (rectangle_t){458, 112, 242, 531}, COLOR_SURFACE);
    draw_text(canvas, 480, 132, "OUTPUT", 2, COLOR_MUTED, 190);
    fill_rectangle(canvas,
                   (rectangle_t){480, 160, 200, 64},
                   scaled_scene_color(&ui->snapshot));
    stroke_rectangle(canvas,
                     (rectangle_t){480, 160, 200, 64},
                     2,
                     COLOR_SURFACE_ALT);

    (void)snprintf(text,
                   sizeof(text),
                   "BRIGHTNESS %u%%",
                   (unsigned int)ui->snapshot.brightness * 100u / 255u);
    draw_text(canvas, 480, 242, text, 2, COLOR_TEXT, 205);
    fill_rectangle(canvas,
                   (rectangle_t){SCENE_BRIGHTNESS_X,
                                 SCENE_BRIGHTNESS_Y,
                                 SCENE_BRIGHTNESS_WIDTH,
                                 SCENE_BRIGHTNESS_HEIGHT},
                   COLOR_SURFACE_ALT);
    fill_rectangle(canvas,
                   (rectangle_t){SCENE_BRIGHTNESS_X,
                                 SCENE_BRIGHTNESS_Y,
                                 filled,
                                 SCENE_BRIGHTNESS_HEIGHT},
                   COLOR_PRIMARY);
    stroke_rectangle(canvas,
                     (rectangle_t){SCENE_BRIGHTNESS_X,
                                   SCENE_BRIGHTNESS_Y,
                                   SCENE_BRIGHTNESS_WIDTH,
                                   SCENE_BRIGHTNESS_HEIGHT},
                     2,
                     COLOR_MUTED);

    draw_text(canvas, 480, 341, "START ADDRESS", 2, COLOR_MUTED, 205);
    draw_button(canvas,
                (rectangle_t){480, 374, 54, 58},
                "-",
                COLOR_SURFACE_ALT,
                COLOR_TEXT,
                3);
    (void)snprintf(text, sizeof(text), "%03u", ui->snapshot.scene_address);
    draw_text(canvas, 550, 392, text, 3, COLOR_TEXT, 70);
    draw_button(canvas,
                (rectangle_t){626, 374, 54, 58},
                "+",
                COLOR_SURFACE_ALT,
                COLOR_TEXT,
                3);

    draw_button(canvas,
                (rectangle_t){480, 470, 200, 72},
                ui->snapshot.blackout ? "BLACKOUT ON" : "BLACKOUT",
                ui->snapshot.blackout ? COLOR_DANGER : COLOR_SURFACE_ALT,
                COLOR_TEXT,
                2);
    (void)snprintf(text,
                   sizeof(text),
                   "RGB %03u %03u %03u",
                   ui->snapshot.red,
                   ui->snapshot.green,
                   ui->snapshot.blue);
    draw_text(canvas, 480, 577, text, 1, COLOR_MUTED, 205);
    draw_text(canvas, 480, 603, "DMX: R G B", 1, COLOR_MUTED, 205);
}

static void device_title(const dmx_rdm_device_t *device,
                         char *output,
                         size_t output_size)
{
    if (device->device_label[0] != '\0')
        (void)snprintf(output, output_size, "%s", device->device_label);
    else if (device->model_description[0] != '\0')
        (void)snprintf(output, output_size, "%s", device->model_description);
    else
        (void)snprintf(output, output_size, "UNNAMED DEVICE");
}

static void render_device_list(panel_ui_t *ui, panel_canvas_t *canvas)
{
    size_t row;

    fill_rectangle(canvas,
                   (rectangle_t){RDM_LIST_X,
                                 RDM_LIST_Y - 10,
                                 RDM_LIST_WIDTH,
                                 450},
                   COLOR_SURFACE);
    for (row = 0; row < RDM_VISIBLE_ROWS; ++row) {
        size_t index = ui->rdm_scroll + row;
        int y = RDM_LIST_Y + (int)row * RDM_ROW_HEIGHT;
        rectangle_t item = {RDM_LIST_X + 8,
                            y,
                            RDM_LIST_WIDTH - 16,
                            RDM_ROW_HEIGHT - 5};

        if (index < ui->snapshot.device_count) {
            const dmx_rdm_device_t *device = &ui->snapshot.devices[index];
            char title[DMX_RDM_LABEL_SIZE];
            char uid[14];
            char detail[40];
            bool selected = ui->selected_valid &&
                            rdm_uid_equal(&device->uid, &ui->selected_uid);

            fill_rectangle(canvas,
                           item,
                           selected ? COLOR_PRIMARY_DARK : COLOR_SURFACE_ALT);
            device_title(device, title, sizeof(title));
            draw_text(canvas,
                      item.x + 10,
                      item.y + 8,
                      title,
                      1,
                      COLOR_TEXT,
                      184);
            rdm_uid_format(&device->uid, uid);
            (void)snprintf(detail,
                           sizeof(detail),
                           "%s A%03u",
                           uid,
                           device->dmx_start_address);
            draw_text(canvas,
                      item.x + 10,
                      item.y + 31,
                      detail,
                      1,
                      COLOR_MUTED,
                      270);
        } else if (row == 0 && ui->snapshot.device_count == 0) {
            draw_text(canvas,
                      item.x + 10,
                      item.y + 18,
                      "NO RDM DEVICES",
                      2,
                      COLOR_MUTED,
                      item.width - 20);
        }
    }
    draw_button(canvas,
                (rectangle_t){RDM_LIST_X + 8, 548, 140, 48},
                "UP",
                COLOR_SURFACE_ALT,
                COLOR_TEXT,
                2);
    draw_button(canvas,
                (rectangle_t){RDM_LIST_X + 159, 548, 140, 48},
                "DOWN",
                COLOR_SURFACE_ALT,
                COLOR_TEXT,
                2);
}

static void draw_detail_line(panel_canvas_t *canvas,
                             int y,
                             const char *label,
                             const char *value)
{
    draw_text(canvas, 373, y, label, 1, COLOR_MUTED, 118);
    draw_text(canvas, 493, y, value, 1, COLOR_TEXT, 188);
}

static void render_device_detail(panel_ui_t *ui, panel_canvas_t *canvas)
{
    int selected_index = find_selected_index(ui);
    char value[96];

    fill_rectangle(canvas, (rectangle_t){355, 161, 345, 481}, COLOR_SURFACE);
    if (selected_index < 0) {
        draw_text(canvas, 384, 282, "SELECT A DEVICE", 2, COLOR_MUTED, 290);
        return;
    }
    {
        const dmx_rdm_device_t *device = &ui->snapshot.devices[selected_index];
        char title[DMX_RDM_LABEL_SIZE];

        device_title(device, title, sizeof(title));
        draw_text(canvas, 373, 181, title, 2, COLOR_TEXT, 307);
        rdm_uid_format(&device->uid, value);
        draw_detail_line(canvas, 222, "UID", value);
        draw_detail_line(canvas,
                         247,
                         "MAKER",
                         device->manufacturer_label[0] != '\0'
                             ? device->manufacturer_label
                             : "-");
        draw_detail_line(canvas,
                         272,
                         "MODEL",
                         device->model_description[0] != '\0'
                             ? device->model_description
                             : "-");
        draw_detail_line(canvas,
                         297,
                         "SOFTWARE",
                         device->software_version_label[0] != '\0'
                             ? device->software_version_label
                             : "-");
        (void)snprintf(value,
                       sizeof(value),
                       "%u / P%u OF %u",
                       device->dmx_footprint,
                       device->current_personality,
                       device->personality_count);
        draw_detail_line(canvas, 322, "FOOTPRINT", value);
        (void)snprintf(value,
                       sizeof(value),
                       "V%u.%u  SUB %u",
                       device->protocol_version >> 8,
                       device->protocol_version & 0xffu,
                       device->sub_device_count);
        draw_detail_line(canvas, 347, "RDM", value);

        draw_button(canvas,
                    (rectangle_t){373, 382, 145, 55},
                    "REFRESH",
                    COLOR_SURFACE_ALT,
                    COLOR_TEXT,
                    2);
        draw_button(canvas,
                    (rectangle_t){535, 382, 145, 55},
                    device->identify_on ? "IDENTIFY ON" : "IDENTIFY",
                    device->identify_on ? COLOR_WARNING : COLOR_SURFACE_ALT,
                    device->identify_on ? COLOR_BLACK : COLOR_TEXT,
                    1);

        draw_text(canvas, 373, 470, "DMX START ADDRESS", 1, COLOR_MUTED, 280);
        draw_button(canvas,
                    (rectangle_t){373, 497, 55, 54},
                    "-",
                    COLOR_SURFACE_ALT,
                    COLOR_TEXT,
                    3);
        (void)snprintf(value, sizeof(value), "%03u", ui->rdm_edit_address);
        draw_text(canvas, 448, 514, value, 3, COLOR_TEXT, 72);
        draw_button(canvas,
                    (rectangle_t){535, 497, 55, 54},
                    "+",
                    COLOR_SURFACE_ALT,
                    COLOR_TEXT,
                    3);
        draw_button(canvas,
                    (rectangle_t){603, 497, 77, 54},
                    "SET",
                    ui->rdm_address_dirty ? COLOR_PRIMARY_DARK
                                          : COLOR_SURFACE_ALT,
                    COLOR_TEXT,
                    2);

        (void)snprintf(value,
                       sizeof(value),
                       "MODEL ID %04X  CATEGORY %04X",
                       device->device_model_id,
                       device->product_category);
        draw_text(canvas, 373, 584, value, 1, COLOR_MUTED, 307);
        draw_text(canvas,
                  373,
                  609,
                  device->information_valid ? "DEVICE INFO READY"
                                            : "DEVICE INFO NOT LOADED",
                  1,
                  device->information_valid ? COLOR_GOOD : COLOR_WARNING,
                  307);
    }
}

static void render_rdm(panel_ui_t *ui, panel_canvas_t *canvas)
{
    char count[48];

    draw_text(canvas, 20, 112, "RDM DEVICES", 2, COLOR_MUTED, 260);
    (void)snprintf(count,
                   sizeof(count),
                   "%u FOUND",
                   (unsigned int)ui->snapshot.device_count);
    draw_text(canvas, 210, 116, count, 1, COLOR_MUTED, 130);
    draw_button(canvas,
                (rectangle_t){500, 103, 200, 48},
                ui->snapshot.rdm_busy ? "DISCOVERING" : "DISCOVER",
                ui->snapshot.rdm_busy ? COLOR_WARNING : COLOR_PRIMARY_DARK,
                ui->snapshot.rdm_busy ? COLOR_BLACK : COLOR_TEXT,
                2);
    render_device_list(ui, canvas);
    render_device_detail(ui, canvas);
}

void panel_ui_render(panel_ui_t *ui, panel_canvas_t *canvas)
{
    if (ui == NULL || canvas == NULL || canvas->pixels == NULL)
        return;
    panel_canvas_clear(canvas, COLOR_BACKGROUND);
    render_header(ui, canvas);
    if (ui->view == PANEL_UI_VIEW_SCENE)
        render_scene(ui, canvas);
    else
        render_rdm(ui, canvas);
    render_footer(ui, canvas);
}

static void set_scene_action(panel_ui_t *ui, panel_ui_action_t *action)
{
    action->type = PANEL_UI_ACTION_SET_SCENE;
    action->data.scene.start_address = ui->snapshot.scene_address;
    action->data.scene.red = ui->snapshot.red;
    action->data.scene.green = ui->snapshot.green;
    action->data.scene.blue = ui->snapshot.blue;
    action->data.scene.brightness = ui->snapshot.brightness;
}

static bool update_wheel(panel_ui_t *ui,
                         int x,
                         int y,
                         panel_ui_action_t *action)
{
    int relative_x = x - WHEEL_CENTER_X;
    int relative_y = y - WHEEL_CENTER_Y;
    uint32_t distance = integer_square_root(
        (uint32_t)(relative_x * relative_x + relative_y * relative_y));
    uint8_t saturation;
    panel_color_t color;

    if (distance > WHEEL_RADIUS) {
        relative_x = relative_x * WHEEL_RADIUS / (int)distance;
        relative_y = relative_y * WHEEL_RADIUS / (int)distance;
        distance = WHEEL_RADIUS;
    }
    saturation = (uint8_t)(distance * 255u / WHEEL_RADIUS);
    color = hsv_color(point_hue(relative_x, relative_y), saturation, 255);
    ui->snapshot.red = (uint8_t)(color >> 16);
    ui->snapshot.green = (uint8_t)(color >> 8);
    ui->snapshot.blue = (uint8_t)color;
    set_scene_action(ui, action);
    return true;
}

static bool update_brightness(panel_ui_t *ui,
                              int x,
                              panel_ui_action_t *action)
{
    int relative = clamp_int(x - SCENE_BRIGHTNESS_X,
                             0,
                             SCENE_BRIGHTNESS_WIDTH);

    ui->snapshot.brightness =
        (uint8_t)(relative * 255 / SCENE_BRIGHTNESS_WIDTH);
    set_scene_action(ui, action);
    return true;
}

static bool handle_scene_down(panel_ui_t *ui,
                              int x,
                              int y,
                              panel_ui_action_t *action)
{
    int wheel_x = x - WHEEL_CENTER_X;
    int wheel_y = y - WHEEL_CENTER_Y;

    if (wheel_x * wheel_x + wheel_y * wheel_y <=
        WHEEL_RADIUS * WHEEL_RADIUS) {
        ui->drag = DRAG_COLOR_WHEEL;
        return update_wheel(ui, x, y, action);
    }
    if (point_in_rectangle(x,
                           y,
                           (rectangle_t){SCENE_BRIGHTNESS_X - 8,
                                         SCENE_BRIGHTNESS_Y - 12,
                                         SCENE_BRIGHTNESS_WIDTH + 16,
                                         SCENE_BRIGHTNESS_HEIGHT + 24})) {
        ui->drag = DRAG_BRIGHTNESS;
        return update_brightness(ui, x, action);
    }
    if (point_in_rectangle(x, y, (rectangle_t){480, 374, 54, 58})) {
        if (ui->snapshot.scene_address > 1)
            --ui->snapshot.scene_address;
        set_scene_action(ui, action);
        return true;
    }
    if (point_in_rectangle(x, y, (rectangle_t){626, 374, 54, 58})) {
        if (ui->snapshot.scene_address < 510)
            ++ui->snapshot.scene_address;
        set_scene_action(ui, action);
        return true;
    }
    if (point_in_rectangle(x, y, (rectangle_t){480, 470, 200, 72})) {
        ui->snapshot.blackout = !ui->snapshot.blackout;
        action->type = PANEL_UI_ACTION_SET_BLACKOUT;
        action->data.blackout.enabled = ui->snapshot.blackout;
        return true;
    }
    return false;
}

static bool emit_selected_info(panel_ui_t *ui, panel_ui_action_t *action)
{
    if (!ui->selected_valid)
        return false;
    action->type = PANEL_UI_ACTION_RDM_REQUEST_INFO;
    action->data.rdm_info.uid = ui->selected_uid;
    return true;
}

static bool handle_rdm_down(panel_ui_t *ui,
                            int x,
                            int y,
                            panel_ui_action_t *action)
{
    int selected_index = find_selected_index(ui);

    if (point_in_rectangle(x, y, (rectangle_t){500, 103, 200, 48})) {
        if (!ui->snapshot.rdm_busy)
            action->type = PANEL_UI_ACTION_RDM_DISCOVER;
        return true;
    }
    if (point_in_rectangle(x,
                           y,
                           (rectangle_t){RDM_LIST_X + 8,
                                         RDM_LIST_Y,
                                         RDM_LIST_WIDTH - 16,
                                         RDM_ROW_HEIGHT * RDM_VISIBLE_ROWS})) {
        size_t row = (size_t)(y - RDM_LIST_Y) / RDM_ROW_HEIGHT;
        size_t index = ui->rdm_scroll + row;

        if (index < ui->snapshot.device_count) {
            select_device(ui, index);
            return emit_selected_info(ui, action);
        }
        return false;
    }
    if (point_in_rectangle(x, y, (rectangle_t){RDM_LIST_X + 8, 548, 140, 48})) {
        if (ui->rdm_scroll != 0)
            --ui->rdm_scroll;
        return true;
    }
    if (point_in_rectangle(x,
                           y,
                           (rectangle_t){RDM_LIST_X + 159, 548, 140, 48})) {
        if (ui->rdm_scroll + RDM_VISIBLE_ROWS < ui->snapshot.device_count)
            ++ui->rdm_scroll;
        return true;
    }
    if (selected_index < 0)
        return false;
    if (point_in_rectangle(x, y, (rectangle_t){373, 382, 145, 55}))
        return emit_selected_info(ui, action);
    if (point_in_rectangle(x, y, (rectangle_t){535, 382, 145, 55})) {
        dmx_rdm_device_t *device = &ui->snapshot.devices[selected_index];

        device->identify_on = !device->identify_on;
        device->identify_known = true;
        action->type = PANEL_UI_ACTION_RDM_SET_IDENTIFY;
        action->data.rdm_identify.uid = device->uid;
        action->data.rdm_identify.enabled = device->identify_on;
        return true;
    }
    if (point_in_rectangle(x, y, (rectangle_t){373, 497, 55, 54})) {
        if (ui->rdm_edit_address > 1)
            --ui->rdm_edit_address;
        ui->rdm_address_dirty = true;
        return true;
    }
    if (point_in_rectangle(x, y, (rectangle_t){535, 497, 55, 54})) {
        if (ui->rdm_edit_address < DMX_UNIVERSE_SLOTS)
            ++ui->rdm_edit_address;
        ui->rdm_address_dirty = true;
        return true;
    }
    if (point_in_rectangle(x, y, (rectangle_t){603, 497, 77, 54})) {
        action->type = PANEL_UI_ACTION_RDM_SET_ADDRESS;
        action->data.rdm_address.uid = ui->selected_uid;
        action->data.rdm_address.start_address = ui->rdm_edit_address;
        ui->rdm_address_dirty = false;
        return true;
    }
    return false;
}

bool panel_ui_handle_pointer(panel_ui_t *ui,
                             const panel_pointer_event_t *event,
                             panel_ui_action_t *action)
{
    panel_ui_action_t ignored_action;

    if (ui == NULL || event == NULL)
        return false;
    if (action == NULL)
        action = &ignored_action;
    memset(action, 0, sizeof(*action));

    if (event->type == PANEL_POINTER_UP) {
        bool changed = ui->drag != DRAG_NONE;

        ui->drag = DRAG_NONE;
        return changed;
    }
    if (event->type == PANEL_POINTER_MOVE) {
        if (ui->drag == DRAG_COLOR_WHEEL)
            return update_wheel(ui, event->x, event->y, action);
        if (ui->drag == DRAG_BRIGHTNESS)
            return update_brightness(ui, event->x, action);
        return false;
    }
    if (event->type != PANEL_POINTER_DOWN)
        return false;

    if (point_in_rectangle(event->x, event->y, (rectangle_t){20, 57, 145, 34})) {
        ui->view = PANEL_UI_VIEW_SCENE;
        ui->drag = DRAG_NONE;
        return true;
    }
    if (point_in_rectangle(event->x,
                           event->y,
                           (rectangle_t){176, 57, 145, 34})) {
        ui->view = PANEL_UI_VIEW_RDM;
        ui->drag = DRAG_NONE;
        return true;
    }
    if (ui->view == PANEL_UI_VIEW_SCENE)
        return handle_scene_down(ui, event->x, event->y, action);
    return handle_rdm_down(ui, event->x, event->y, action);
}
