#ifndef DMX_PANEL_DMX_CONTROLLER_H
#define DMX_PANEL_DMX_CONTROLLER_H

#include "rdm_protocol.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DMX_UNIVERSE_SLOTS 512u
#define DMX_MAX_DISCOVERED_DEVICES 64u
#define DMX_RDM_LABEL_SIZE 33u
#define DMX_STATUS_TEXT_SIZE 128u

typedef enum {
    DMX_RS485_AUTO_DIRECTION = 0,
    DMX_RS485_KERNEL_RTS,
} dmx_rs485_mode_t;

typedef enum {
    DMX_BREAK_IOCTL = 0,
    DMX_BREAK_BAUD,
} dmx_break_mode_t;

typedef struct {
    const char *serial_path;
    rdm_uid_t controller_uid;
    dmx_rs485_mode_t rs485_mode;
    dmx_break_mode_t break_mode;
    bool simulate;
} dmx_controller_config_t;

typedef struct {
    rdm_uid_t uid;
    bool information_valid;
    bool identify_known;
    bool identify_on;
    uint16_t protocol_version;
    uint16_t device_model_id;
    uint16_t product_category;
    uint32_t software_version_id;
    uint16_t dmx_footprint;
    uint8_t current_personality;
    uint8_t personality_count;
    uint16_t dmx_start_address;
    uint16_t sub_device_count;
    uint8_t sensor_count;
    char manufacturer_label[DMX_RDM_LABEL_SIZE];
    char model_description[DMX_RDM_LABEL_SIZE];
    char device_label[DMX_RDM_LABEL_SIZE];
    char software_version_label[DMX_RDM_LABEL_SIZE];
} dmx_rdm_device_t;

typedef struct {
    uint64_t generation;
    bool worker_running;
    bool serial_open;
    bool rdm_busy;
    bool blackout;
    bool timing_warning;
    uint8_t red;
    uint8_t green;
    uint8_t blue;
    uint8_t brightness;
    uint16_t scene_address;
    uint32_t dmx_frames_sent;
    uint32_t rdm_transactions;
    uint32_t rdm_errors;
    uint32_t discovery_queries;
    size_t device_count;
    dmx_rdm_device_t devices[DMX_MAX_DISCOVERED_DEVICES];
    char status[DMX_STATUS_TEXT_SIZE];
} dmx_controller_snapshot_t;

typedef struct dmx_controller dmx_controller_t;

dmx_controller_t *dmx_controller_create(const dmx_controller_config_t *config);
int dmx_controller_start(dmx_controller_t *controller);
void dmx_controller_stop(dmx_controller_t *controller);
void dmx_controller_destroy(dmx_controller_t *controller);

void dmx_controller_set_scene(dmx_controller_t *controller,
                              uint16_t start_address,
                              uint8_t red,
                              uint8_t green,
                              uint8_t blue,
                              uint8_t brightness);
void dmx_controller_set_blackout(dmx_controller_t *controller, bool blackout);

bool dmx_controller_discover(dmx_controller_t *controller);
bool dmx_controller_request_device_info(dmx_controller_t *controller,
                                        const rdm_uid_t *uid);
bool dmx_controller_set_identify(dmx_controller_t *controller,
                                 const rdm_uid_t *uid,
                                 bool enabled);
bool dmx_controller_set_start_address(dmx_controller_t *controller,
                                      const rdm_uid_t *uid,
                                      uint16_t start_address);

void dmx_controller_get_snapshot(dmx_controller_t *controller,
                                 dmx_controller_snapshot_t *snapshot);

#ifdef __cplusplus
}
#endif

#endif
