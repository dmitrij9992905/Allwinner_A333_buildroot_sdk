#define _POSIX_C_SOURCE 200809L

#include "panel_input.h"

#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <linux/input.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define PANEL_INPUT_MAX_SLOTS 16
#define BITS_PER_WORD (sizeof(unsigned long) * 8u)
#define BIT_WORD_COUNT(maximum) (((maximum) + BITS_PER_WORD) / BITS_PER_WORD)
#define TEST_BIT(bits, bit)                                                     \
    (((bits)[(unsigned int)(bit) / BITS_PER_WORD] >>                           \
      ((unsigned int)(bit) % BITS_PER_WORD)) &                                 \
     1ul)

typedef struct {
    bool active;
    int x;
    int y;
} touch_slot_t;

struct panel_input {
    int fd;
    char path[PATH_MAX];
    char name[128];
    int canvas_width;
    int canvas_height;
    bool swap_xy;
    bool invert_x;
    bool invert_y;
    bool multitouch;
    bool has_tracking_id;
    bool button_down;
    bool last_touching;
    bool coordinates_changed;
    bool sync_dropped;
    int current_slot;
    int slot_minimum;
    int slot_count;
    int single_x;
    int single_y;
    int last_x;
    int last_y;
    int pressure;
    struct input_absinfo x_axis;
    struct input_absinfo y_axis;
    touch_slot_t slots[PANEL_INPUT_MAX_SLOTS];
};

static void set_error(char *buffer, size_t size, const char *format, ...)
{
    va_list args;

    if (buffer == NULL || size == 0)
        return;
    va_start(args, format);
    (void)vsnprintf(buffer, size, format, args);
    va_end(args);
}

static bool contains_case_insensitive(const char *text, const char *needle)
{
    size_t needle_length;
    const char *position;

    if (text == NULL || needle == NULL)
        return false;
    needle_length = strlen(needle);
    if (needle_length == 0)
        return true;
    for (position = text; *position != '\0'; ++position) {
        size_t i;

        for (i = 0; i < needle_length; ++i) {
            if (position[i] == '\0' ||
                tolower((unsigned char)position[i]) !=
                    tolower((unsigned char)needle[i]))
                break;
        }
        if (i == needle_length)
            return true;
    }
    return false;
}

static int read_device_name(const char *path, char *name, size_t name_size)
{
    FILE *file;
    size_t length;

    file = fopen(path, "r");
    if (file == NULL)
        return -1;
    if (fgets(name, (int)name_size, file) == NULL) {
        int saved_errno = errno;
        (void)fclose(file);
        errno = saved_errno != 0 ? saved_errno : EIO;
        return -1;
    }
    (void)fclose(file);
    length = strlen(name);
    while (length != 0 && (name[length - 1] == '\n' ||
                           name[length - 1] == '\r')) {
        name[--length] = '\0';
    }
    return 0;
}

static int discover_through_sysfs(char *path, size_t path_size)
{
    DIR *directory;
    struct dirent *entry;
    char fallback[PATH_MAX] = "";

    directory = opendir("/sys/class/input");
    if (directory == NULL)
        return -1;
    while ((entry = readdir(directory)) != NULL) {
        char name_path[PATH_MAX];
        char name[128];
        int written;

        if (strncmp(entry->d_name, "event", 5) != 0)
            continue;
        written = snprintf(name_path,
                           sizeof(name_path),
                           "/sys/class/input/%s/device/name",
                           entry->d_name);
        if (written < 0 || (size_t)written >= sizeof(name_path) ||
            read_device_name(name_path, name, sizeof(name)) < 0 ||
            !contains_case_insensitive(name, "goodix"))
            continue;

        written = snprintf(name_path,
                           sizeof(name_path),
                           "/dev/input/%s",
                           entry->d_name);
        if (written < 0 || (size_t)written >= sizeof(name_path))
            continue;
        if (strcasecmp(name, "Goodix Capacitive TouchScreen") == 0) {
            if ((size_t)written >= path_size) {
                (void)closedir(directory);
                errno = ENAMETOOLONG;
                return -1;
            }
            memcpy(path, name_path, (size_t)written + 1u);
            (void)closedir(directory);
            return 0;
        }
        if (fallback[0] == '\0')
            (void)snprintf(fallback, sizeof(fallback), "%s", name_path);
    }
    (void)closedir(directory);

    if (fallback[0] != '\0') {
        size_t length = strlen(fallback);

        if (length >= path_size) {
            errno = ENAMETOOLONG;
            return -1;
        }
        memcpy(path, fallback, length + 1u);
        return 0;
    }
    errno = ENODEV;
    return -1;
}

static int discover_through_dev(char *path, size_t path_size)
{
    DIR *directory;
    struct dirent *entry;

    directory = opendir("/dev/input");
    if (directory == NULL)
        return -1;
    while ((entry = readdir(directory)) != NULL) {
        char candidate[PATH_MAX];
        char name[128] = "";
        int fd;
        int written;

        if (strncmp(entry->d_name, "event", 5) != 0)
            continue;
        written = snprintf(candidate,
                           sizeof(candidate),
                           "/dev/input/%s",
                           entry->d_name);
        if (written < 0 || (size_t)written >= sizeof(candidate))
            continue;
        fd = open(candidate, O_RDONLY | O_NONBLOCK | O_CLOEXEC);
        if (fd < 0)
            continue;
        if (ioctl(fd, EVIOCGNAME(sizeof(name)), name) < 0)
            name[0] = '\0';
        (void)close(fd);
        if (!contains_case_insensitive(name, "goodix"))
            continue;
        if ((size_t)written >= path_size) {
            (void)closedir(directory);
            errno = ENAMETOOLONG;
            return -1;
        }
        memcpy(path, candidate, (size_t)written + 1u);
        (void)closedir(directory);
        return 0;
    }
    (void)closedir(directory);
    errno = ENODEV;
    return -1;
}

int panel_input_find_goodix(char *path, size_t path_size)
{
    if (path == NULL || path_size == 0) {
        errno = EINVAL;
        return -1;
    }
    if (discover_through_sysfs(path, path_size) == 0)
        return 0;
    return discover_through_dev(path, path_size);
}

static bool query_axis(int fd, unsigned int code, struct input_absinfo *axis)
{
    return ioctl(fd, EVIOCGABS(code), axis) == 0 &&
           axis->maximum > axis->minimum;
}

panel_input_t *panel_input_open(const panel_input_config_t *config,
                               char *error_text,
                               size_t error_text_size)
{
    panel_input_config_t defaults = {NULL, 720, 720, false, false, false};
    const panel_input_config_t *effective = config != NULL ? config : &defaults;
    unsigned long abs_bits[BIT_WORD_COUNT(ABS_MAX)] = {0};
    panel_input_t *input;
    struct input_absinfo slot_axis;
    const char *device_path;
    char discovered_path[PATH_MAX];

    if (effective->canvas_width <= 0 || effective->canvas_height <= 0) {
        errno = EINVAL;
        set_error(error_text, error_text_size, "invalid input canvas size");
        return NULL;
    }
    if (effective->device_path == NULL) {
        if (panel_input_find_goodix(discovered_path,
                                    sizeof(discovered_path)) < 0) {
            set_error(error_text,
                      error_text_size,
                      "Goodix evdev device not found: %s",
                      strerror(errno));
            return NULL;
        }
        device_path = discovered_path;
    } else {
        device_path = effective->device_path;
    }

    input = calloc(1, sizeof(*input));
    if (input == NULL) {
        set_error(error_text, error_text_size, "out of memory");
        return NULL;
    }
    input->fd = -1;
    input->canvas_width = effective->canvas_width;
    input->canvas_height = effective->canvas_height;
    input->swap_xy = effective->swap_xy;
    input->invert_x = effective->invert_x;
    input->invert_y = effective->invert_y;
    input->current_slot = 0;
    input->slot_count = 1;
    (void)snprintf(input->path, sizeof(input->path), "%s", device_path);

    input->fd = open(device_path, O_RDONLY | O_NONBLOCK | O_CLOEXEC);
    if (input->fd < 0) {
        set_error(error_text,
                  error_text_size,
                  "open %s: %s",
                  device_path,
                  strerror(errno));
        panel_input_close(input);
        return NULL;
    }
    if (ioctl(input->fd, EVIOCGNAME(sizeof(input->name)), input->name) < 0)
        (void)snprintf(input->name, sizeof(input->name), "unknown input");
    if (ioctl(input->fd, EVIOCGBIT(EV_ABS, sizeof(abs_bits)), abs_bits) < 0) {
        set_error(error_text,
                  error_text_size,
                  "query %s capabilities: %s",
                  device_path,
                  strerror(errno));
        panel_input_close(input);
        return NULL;
    }

    input->multitouch = TEST_BIT(abs_bits, ABS_MT_POSITION_X) &&
                        TEST_BIT(abs_bits, ABS_MT_POSITION_Y) &&
                        query_axis(input->fd, ABS_MT_POSITION_X, &input->x_axis) &&
                        query_axis(input->fd, ABS_MT_POSITION_Y, &input->y_axis);
    input->has_tracking_id = TEST_BIT(abs_bits, ABS_MT_TRACKING_ID);
    if (!input->multitouch &&
        (!query_axis(input->fd, ABS_X, &input->x_axis) ||
         !query_axis(input->fd, ABS_Y, &input->y_axis))) {
        errno = ENOTSUP;
        set_error(error_text,
                  error_text_size,
                  "%s has no usable absolute X/Y axes",
                  device_path);
        panel_input_close(input);
        return NULL;
    }
    if (input->multitouch && TEST_BIT(abs_bits, ABS_MT_SLOT) &&
        query_axis(input->fd, ABS_MT_SLOT, &slot_axis)) {
        int reported_slots = slot_axis.maximum - slot_axis.minimum + 1;

        if (reported_slots > PANEL_INPUT_MAX_SLOTS)
            reported_slots = PANEL_INPUT_MAX_SLOTS;
        if (reported_slots > 0)
            input->slot_count = reported_slots;
        input->slot_minimum = slot_axis.minimum;
    }
    input->single_x = input->x_axis.minimum;
    input->single_y = input->y_axis.minimum;
    if (error_text != NULL && error_text_size != 0)
        error_text[0] = '\0';
    return input;
}

void panel_input_close(panel_input_t *input)
{
    if (input == NULL)
        return;
    if (input->fd >= 0)
        (void)close(input->fd);
    free(input);
}

int panel_input_fd(const panel_input_t *input)
{
    return input != NULL ? input->fd : -1;
}

const char *panel_input_path(const panel_input_t *input)
{
    return input != NULL ? input->path : "";
}

const char *panel_input_name(const panel_input_t *input)
{
    return input != NULL ? input->name : "";
}

static uint32_t normalize_axis(int value, const struct input_absinfo *axis)
{
    int64_t numerator;
    int64_t denominator;

    if (value <= axis->minimum)
        return 0;
    if (value >= axis->maximum)
        return 65535u;
    numerator = (int64_t)(value - axis->minimum) * 65535;
    denominator = axis->maximum - axis->minimum;
    return (uint32_t)((numerator + denominator / 2) / denominator);
}

static void transform_coordinates(const panel_input_t *input,
                                  int raw_x,
                                  int raw_y,
                                  int *x,
                                  int *y)
{
    uint32_t normalized_x = normalize_axis(raw_x, &input->x_axis);
    uint32_t normalized_y = normalize_axis(raw_y, &input->y_axis);
    uint32_t transformed_x;
    uint32_t transformed_y;

    if (input->invert_x)
        normalized_x = 65535u - normalized_x;
    if (input->invert_y)
        normalized_y = 65535u - normalized_y;
    transformed_x = input->swap_xy ? normalized_y : normalized_x;
    transformed_y = input->swap_xy ? normalized_x : normalized_y;
    *x = (int)((uint64_t)transformed_x *
               (unsigned int)(input->canvas_width - 1) / 65535u);
    *y = (int)((uint64_t)transformed_y *
               (unsigned int)(input->canvas_height - 1) / 65535u);
}

static bool current_contact(panel_input_t *input, int *raw_x, int *raw_y)
{
    int i;

    if (input->multitouch && input->has_tracking_id) {
        for (i = 0; i < input->slot_count; ++i) {
            if (input->slots[i].active) {
                *raw_x = input->slots[i].x;
                *raw_y = input->slots[i].y;
                return true;
            }
        }
        return false;
    }
    if (!input->button_down)
        return false;
    if (input->multitouch) {
        *raw_x = input->slots[0].x;
        *raw_y = input->slots[0].y;
    } else {
        *raw_x = input->single_x;
        *raw_y = input->single_y;
    }
    return true;
}

static bool query_mt_slot_values(panel_input_t *input,
                                 unsigned int code,
                                 int32_t values[PANEL_INPUT_MAX_SLOTS])
{
    int32_t request[PANEL_INPUT_MAX_SLOTS + 1];
    size_t request_size =
        (size_t)(input->slot_count + 1) * sizeof(request[0]);

    memset(request, 0, sizeof(request));
    request[0] = (int32_t)code;
    if (ioctl(input->fd, EVIOCGMTSLOTS(request_size), request) < 0)
        return false;
    memcpy(values,
           request + 1,
           (size_t)input->slot_count * sizeof(values[0]));
    return true;
}

/* Rebuild the current contact state after evdev reports SYN_DROPPED. Linux
 * requires clients to ignore the partial stream and query the device again at
 * the next SYN_REPORT.
 */
static void resynchronize_state(panel_input_t *input)
{
    int i;

    if (input->multitouch && input->has_tracking_id) {
        int32_t tracking[PANEL_INPUT_MAX_SLOTS];
        int32_t positions_x[PANEL_INPUT_MAX_SLOTS];
        int32_t positions_y[PANEL_INPUT_MAX_SLOTS];
        bool valid = query_mt_slot_values(input,
                                          ABS_MT_TRACKING_ID,
                                          tracking) &&
                     query_mt_slot_values(input,
                                          ABS_MT_POSITION_X,
                                          positions_x) &&
                     query_mt_slot_values(input,
                                          ABS_MT_POSITION_Y,
                                          positions_y);

        for (i = 0; i < input->slot_count; ++i) {
            input->slots[i].active = valid && tracking[i] >= 0;
            if (valid) {
                input->slots[i].x = positions_x[i];
                input->slots[i].y = positions_y[i];
            }
        }
    } else {
        unsigned long key_bits[BIT_WORD_COUNT(KEY_MAX)] = {0};
        struct input_absinfo current_x;
        struct input_absinfo current_y;
        unsigned int x_code = input->multitouch ? ABS_MT_POSITION_X : ABS_X;
        unsigned int y_code = input->multitouch ? ABS_MT_POSITION_Y : ABS_Y;
        bool valid_axes = ioctl(input->fd, EVIOCGABS(x_code), &current_x) == 0 &&
                          ioctl(input->fd, EVIOCGABS(y_code), &current_y) == 0;

        input->button_down =
            ioctl(input->fd, EVIOCGKEY(sizeof(key_bits)), key_bits) == 0 &&
            TEST_BIT(key_bits, BTN_TOUCH);
        if (valid_axes) {
            if (input->multitouch) {
                input->slots[0].x = current_x.value;
                input->slots[0].y = current_y.value;
            } else {
                input->single_x = current_x.value;
                input->single_y = current_y.value;
            }
        } else {
            input->button_down = false;
        }
    }
    input->coordinates_changed = true;
}

static void process_absolute(panel_input_t *input,
                             unsigned int code,
                             int value)
{
    switch (code) {
    case ABS_MT_SLOT:
        if (value >= input->slot_minimum &&
            value < input->slot_minimum + input->slot_count)
            input->current_slot = value - input->slot_minimum;
        break;
    case ABS_MT_TRACKING_ID:
        input->slots[input->current_slot].active = value >= 0;
        input->coordinates_changed = true;
        break;
    case ABS_MT_POSITION_X:
        input->slots[input->current_slot].x = value;
        input->coordinates_changed = true;
        break;
    case ABS_MT_POSITION_Y:
        input->slots[input->current_slot].y = value;
        input->coordinates_changed = true;
        break;
    case ABS_X:
        input->single_x = value;
        input->coordinates_changed = true;
        break;
    case ABS_Y:
        input->single_y = value;
        input->coordinates_changed = true;
        break;
    case ABS_PRESSURE:
    case ABS_MT_PRESSURE:
        input->pressure = value;
        break;
    default:
        break;
    }
}

static bool finish_report(panel_input_t *input,
                          const struct input_event *source,
                          panel_pointer_event_t *output)
{
    int raw_x = 0;
    int raw_y = 0;
    int x = input->last_x;
    int y = input->last_y;
    bool touching;

    touching = current_contact(input, &raw_x, &raw_y);
    if (touching)
        transform_coordinates(input, raw_x, raw_y, &x, &y);

    if (!input->last_touching && touching)
        output->type = PANEL_POINTER_DOWN;
    else if (input->last_touching && !touching)
        output->type = PANEL_POINTER_UP;
    else if (touching && input->coordinates_changed &&
             (x != input->last_x || y != input->last_y))
        output->type = PANEL_POINTER_MOVE;
    else {
        input->coordinates_changed = false;
        return false;
    }

    output->x = x;
    output->y = y;
    output->pressure = input->pressure;
    output->timestamp_ms = (uint64_t)source->time.tv_sec * 1000u +
                           (uint64_t)source->time.tv_usec / 1000u;
    input->last_touching = touching;
    input->last_x = x;
    input->last_y = y;
    input->coordinates_changed = false;
    return true;
}

int panel_input_read(panel_input_t *input,
                     panel_pointer_event_t *events,
                     size_t capacity)
{
    size_t count = 0;

    if (input == NULL || events == NULL || capacity == 0)
        return -EINVAL;

    while (count < capacity) {
        struct input_event event;
        ssize_t received = read(input->fd, &event, sizeof(event));

        if (received < 0) {
            if (errno == EINTR)
                continue;
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                break;
            return -errno;
        }
        if (received == 0)
            return count != 0 ? (int)count : -ENODEV;
        if ((size_t)received != sizeof(event))
            return count != 0 ? (int)count : -EIO;

        if (event.type == EV_SYN && event.code == SYN_DROPPED) {
            input->sync_dropped = true;
            continue;
        }
        if (input->sync_dropped) {
            if (event.type == EV_SYN && event.code == SYN_REPORT) {
                resynchronize_state(input);
                input->sync_dropped = false;
                if (finish_report(input, &event, &events[count]))
                    ++count;
            }
            continue;
        }
        if (event.type == EV_ABS)
            process_absolute(input, event.code, event.value);
        else if (event.type == EV_KEY && event.code == BTN_TOUCH) {
            input->button_down = event.value != 0;
            input->coordinates_changed = true;
        } else if (event.type == EV_SYN && event.code == SYN_REPORT &&
                   finish_report(input, &event, &events[count])) {
            ++count;
        }
    }
    return (int)count;
}
