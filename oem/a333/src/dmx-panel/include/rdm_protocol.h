#ifndef DMX_PANEL_RDM_PROTOCOL_H
#define DMX_PANEL_RDM_PROTOCOL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RDM_UID_SIZE 6u
#define RDM_HEADER_SIZE 24u
#define RDM_CHECKSUM_SIZE 2u
#define RDM_MAX_PARAMETER_DATA 231u
#define RDM_MAX_PACKET_SIZE 257u
#define RDM_DISCOVERY_RESPONSE_SIZE 24u

#define RDM_START_CODE 0xccu
#define RDM_SUB_START_CODE 0x01u

enum rdm_command_class {
    RDM_CC_DISCOVERY_COMMAND = 0x10,
    RDM_CC_DISCOVERY_COMMAND_RESPONSE = 0x11,
    RDM_CC_GET_COMMAND = 0x20,
    RDM_CC_GET_COMMAND_RESPONSE = 0x21,
    RDM_CC_SET_COMMAND = 0x30,
    RDM_CC_SET_COMMAND_RESPONSE = 0x31,
};

enum rdm_response_type {
    RDM_RESPONSE_ACK = 0x00,
    RDM_RESPONSE_ACK_TIMER = 0x01,
    RDM_RESPONSE_NACK_REASON = 0x02,
    RDM_RESPONSE_ACK_OVERFLOW = 0x03,
};

enum rdm_pid {
    RDM_PID_DISC_UNIQUE_BRANCH = 0x0001,
    RDM_PID_DISC_MUTE = 0x0002,
    RDM_PID_DISC_UN_MUTE = 0x0003,
    RDM_PID_SUPPORTED_PARAMETERS = 0x0050,
    RDM_PID_DEVICE_INFO = 0x0060,
    RDM_PID_DEVICE_MODEL_DESCRIPTION = 0x0080,
    RDM_PID_MANUFACTURER_LABEL = 0x0081,
    RDM_PID_DEVICE_LABEL = 0x0082,
    RDM_PID_SOFTWARE_VERSION_LABEL = 0x00c0,
    RDM_PID_DMX_START_ADDRESS = 0x00f0,
    RDM_PID_IDENTIFY_DEVICE = 0x1000,
};

typedef struct {
    uint8_t bytes[RDM_UID_SIZE];
} rdm_uid_t;

typedef struct {
    rdm_uid_t destination;
    rdm_uid_t source;
    uint8_t transaction_number;
    uint8_t port_id_or_response_type;
    uint8_t message_count;
    uint16_t sub_device;
    uint8_t command_class;
    uint16_t parameter_id;
    uint8_t parameter_data_length;
    uint8_t parameter_data[RDM_MAX_PARAMETER_DATA];
} rdm_message_t;

typedef enum {
    RDM_PARSE_OK = 0,
    RDM_PARSE_TOO_SHORT,
    RDM_PARSE_BAD_START_CODE,
    RDM_PARSE_BAD_SUB_START_CODE,
    RDM_PARSE_BAD_LENGTH,
    RDM_PARSE_BAD_CHECKSUM,
} rdm_parse_result_t;

extern const rdm_uid_t RDM_BROADCAST_UID;

bool rdm_uid_equal(const rdm_uid_t *left, const rdm_uid_t *right);
int rdm_uid_compare(const rdm_uid_t *left, const rdm_uid_t *right);
uint64_t rdm_uid_to_u64(const rdm_uid_t *uid);
rdm_uid_t rdm_uid_from_u64(uint64_t value);
bool rdm_uid_parse(const char *text, rdm_uid_t *uid);
void rdm_uid_format(const rdm_uid_t *uid, char output[14]);

uint16_t rdm_checksum(const uint8_t *data, size_t length);

size_t rdm_build_request(uint8_t *output,
                         size_t output_capacity,
                         const rdm_uid_t *destination,
                         const rdm_uid_t *source,
                         uint8_t transaction_number,
                         uint8_t port_id,
                         uint16_t sub_device,
                         uint8_t command_class,
                         uint16_t parameter_id,
                         const uint8_t *parameter_data,
                         uint8_t parameter_data_length);

rdm_parse_result_t rdm_parse_message(const uint8_t *packet,
                                     size_t packet_length,
                                     rdm_message_t *message);

/*
 * Searches an arbitrary UART receive buffer for a complete RDM message.
 * Returns the packet length and optionally its byte offset, or zero if none
 * is present. Invalid candidates are skipped.
 */
size_t rdm_find_message(const uint8_t *data,
                        size_t length,
                        size_t *packet_offset,
                        rdm_message_t *message);

/*
 * Decode a break-less DISC_UNIQUE_BRANCH response. The receive buffer may
 * contain UART noise and from zero through seven 0xfe preamble bytes.
 */
bool rdm_decode_discovery_response(const uint8_t *data,
                                   size_t length,
                                   rdm_uid_t *uid);

/* Test/helper counterpart to the decoder. */
size_t rdm_encode_discovery_response(uint8_t output[RDM_DISCOVERY_RESPONSE_SIZE],
                                     const rdm_uid_t *uid);

#ifdef __cplusplus
}
#endif

#endif
