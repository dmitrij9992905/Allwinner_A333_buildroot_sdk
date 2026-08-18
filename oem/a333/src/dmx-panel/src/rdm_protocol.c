#include "rdm_protocol.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

const rdm_uid_t RDM_BROADCAST_UID = {{0xff, 0xff, 0xff, 0xff, 0xff, 0xff}};

static uint16_t read_be16(const uint8_t *data)
{
    return (uint16_t)(((uint16_t)data[0] << 8) | data[1]);
}

static void write_be16(uint8_t *data, uint16_t value)
{
    data[0] = (uint8_t)(value >> 8);
    data[1] = (uint8_t)value;
}

bool rdm_uid_equal(const rdm_uid_t *left, const rdm_uid_t *right)
{
    return left != NULL && right != NULL &&
           memcmp(left->bytes, right->bytes, RDM_UID_SIZE) == 0;
}

int rdm_uid_compare(const rdm_uid_t *left, const rdm_uid_t *right)
{
    if (left == NULL || right == NULL)
        return left == right ? 0 : (left == NULL ? -1 : 1);
    return memcmp(left->bytes, right->bytes, RDM_UID_SIZE);
}

uint64_t rdm_uid_to_u64(const rdm_uid_t *uid)
{
    uint64_t value = 0;
    size_t i;

    if (uid == NULL)
        return 0;
    for (i = 0; i < RDM_UID_SIZE; ++i)
        value = (value << 8) | uid->bytes[i];
    return value;
}

rdm_uid_t rdm_uid_from_u64(uint64_t value)
{
    rdm_uid_t uid;
    int i;

    value &= UINT64_C(0x0000ffffffffffff);
    for (i = (int)RDM_UID_SIZE - 1; i >= 0; --i) {
        uid.bytes[i] = (uint8_t)value;
        value >>= 8;
    }
    return uid;
}

bool rdm_uid_parse(const char *text, rdm_uid_t *uid)
{
    unsigned int manufacturer;
    unsigned long device;
    char tail;

    if (text == NULL || uid == NULL)
        return false;

    if (sscanf(text, "%x:%lx%c", &manufacturer, &device, &tail) != 2 ||
        manufacturer > 0xffffu || device > 0xfffffffful)
        return false;

    uid->bytes[0] = (uint8_t)(manufacturer >> 8);
    uid->bytes[1] = (uint8_t)manufacturer;
    uid->bytes[2] = (uint8_t)(device >> 24);
    uid->bytes[3] = (uint8_t)(device >> 16);
    uid->bytes[4] = (uint8_t)(device >> 8);
    uid->bytes[5] = (uint8_t)device;
    return true;
}

void rdm_uid_format(const rdm_uid_t *uid, char output[14])
{
    if (output == NULL)
        return;
    if (uid == NULL) {
        memcpy(output, "----:--------", 14);
        return;
    }
    (void)snprintf(output,
                   14,
                   "%02X%02X:%02X%02X%02X%02X",
                   uid->bytes[0],
                   uid->bytes[1],
                   uid->bytes[2],
                   uid->bytes[3],
                   uid->bytes[4],
                   uid->bytes[5]);
}

uint16_t rdm_checksum(const uint8_t *data, size_t length)
{
    uint16_t checksum = 0;
    size_t i;

    if (data == NULL)
        return 0;
    for (i = 0; i < length; ++i)
        checksum = (uint16_t)(checksum + data[i]);
    return checksum;
}

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
                         uint8_t parameter_data_length)
{
    size_t message_length = RDM_HEADER_SIZE + parameter_data_length;
    size_t packet_length = message_length + RDM_CHECKSUM_SIZE;
    uint16_t checksum;

    if (output == NULL || destination == NULL || source == NULL ||
        output_capacity < packet_length ||
        parameter_data_length > RDM_MAX_PARAMETER_DATA ||
        (parameter_data_length != 0 && parameter_data == NULL))
        return 0;

    output[0] = RDM_START_CODE;
    output[1] = RDM_SUB_START_CODE;
    output[2] = (uint8_t)message_length;
    memcpy(output + 3, destination->bytes, RDM_UID_SIZE);
    memcpy(output + 9, source->bytes, RDM_UID_SIZE);
    output[15] = transaction_number;
    output[16] = port_id == 0 ? 1 : port_id;
    output[17] = 0;
    write_be16(output + 18, sub_device);
    output[20] = command_class;
    write_be16(output + 21, parameter_id);
    output[23] = parameter_data_length;
    if (parameter_data_length != 0)
        memcpy(output + RDM_HEADER_SIZE, parameter_data, parameter_data_length);

    checksum = rdm_checksum(output, message_length);
    write_be16(output + message_length, checksum);
    return packet_length;
}

rdm_parse_result_t rdm_parse_message(const uint8_t *packet,
                                     size_t packet_length,
                                     rdm_message_t *message)
{
    size_t message_length;
    uint8_t parameter_data_length;

    if (packet == NULL || packet_length < RDM_HEADER_SIZE + RDM_CHECKSUM_SIZE)
        return RDM_PARSE_TOO_SHORT;
    if (packet[0] != RDM_START_CODE)
        return RDM_PARSE_BAD_START_CODE;
    if (packet[1] != RDM_SUB_START_CODE)
        return RDM_PARSE_BAD_SUB_START_CODE;

    message_length = packet[2];
    parameter_data_length = packet[23];
    if (message_length < RDM_HEADER_SIZE ||
        message_length != RDM_HEADER_SIZE + parameter_data_length ||
        packet_length != message_length + RDM_CHECKSUM_SIZE)
        return RDM_PARSE_BAD_LENGTH;
    if (rdm_checksum(packet, message_length) != read_be16(packet + message_length))
        return RDM_PARSE_BAD_CHECKSUM;

    if (message != NULL) {
        memset(message, 0, sizeof(*message));
        memcpy(message->destination.bytes, packet + 3, RDM_UID_SIZE);
        memcpy(message->source.bytes, packet + 9, RDM_UID_SIZE);
        message->transaction_number = packet[15];
        message->port_id_or_response_type = packet[16];
        message->message_count = packet[17];
        message->sub_device = read_be16(packet + 18);
        message->command_class = packet[20];
        message->parameter_id = read_be16(packet + 21);
        message->parameter_data_length = parameter_data_length;
        if (parameter_data_length != 0)
            memcpy(message->parameter_data,
                   packet + RDM_HEADER_SIZE,
                   parameter_data_length);
    }
    return RDM_PARSE_OK;
}

size_t rdm_find_message(const uint8_t *data,
                        size_t length,
                        size_t *packet_offset,
                        rdm_message_t *message)
{
    size_t i;

    if (data == NULL)
        return 0;
    for (i = 0; i + RDM_HEADER_SIZE + RDM_CHECKSUM_SIZE <= length; ++i) {
        size_t packet_length;

        if (data[i] != RDM_START_CODE || data[i + 1] != RDM_SUB_START_CODE)
            continue;
        packet_length = (size_t)data[i + 2] + RDM_CHECKSUM_SIZE;
        if (packet_length < RDM_HEADER_SIZE + RDM_CHECKSUM_SIZE ||
            i + packet_length > length)
            continue;
        if (rdm_parse_message(data + i, packet_length, message) == RDM_PARSE_OK) {
            if (packet_offset != NULL)
                *packet_offset = i;
            return packet_length;
        }
    }
    return 0;
}

bool rdm_decode_discovery_response(const uint8_t *data,
                                   size_t length,
                                   rdm_uid_t *uid)
{
    size_t separator;

    if (data == NULL || uid == NULL || length < 17)
        return false;

    /* Up to all seven preamble bytes may be removed by in-line devices. */
    for (separator = 0; separator + 17 <= length; ++separator) {
        uint8_t decoded_uid[RDM_UID_SIZE];
        uint16_t encoded_sum;
        uint16_t decoded_sum;
        size_t i;
        bool valid_pairs = true;

        if (data[separator] != 0xaa)
            continue;
        encoded_sum = 0;
        for (i = 0; i < 12; ++i)
            encoded_sum = (uint16_t)(encoded_sum + data[separator + 1 + i]);
        for (i = 0; i < RDM_UID_SIZE; ++i) {
            uint8_t first = data[separator + 1 + (i * 2)];
            uint8_t second = data[separator + 2 + (i * 2)];
            if ((uint8_t)(first | second) != 0xffu) {
                valid_pairs = false;
                break;
            }
            decoded_uid[i] = (uint8_t)(first & second);
        }
        if (!valid_pairs)
            continue;
        if ((uint8_t)(data[separator + 13] | data[separator + 14]) != 0xffu ||
            (uint8_t)(data[separator + 15] | data[separator + 16]) != 0xffu)
            continue;
        decoded_sum = (uint16_t)(((uint16_t)(data[separator + 13] &
                                                     data[separator + 14])
                                  << 8) |
                                 (data[separator + 15] & data[separator + 16]));
        if (encoded_sum != decoded_sum)
            continue;
        memcpy(uid->bytes, decoded_uid, RDM_UID_SIZE);
        return true;
    }
    return false;
}

size_t rdm_encode_discovery_response(uint8_t output[RDM_DISCOVERY_RESPONSE_SIZE],
                                     const rdm_uid_t *uid)
{
    uint16_t checksum;
    size_t i;

    if (output == NULL || uid == NULL)
        return 0;
    memset(output, 0xfe, 7);
    output[7] = 0xaa;
    for (i = 0; i < RDM_UID_SIZE; ++i) {
        output[8 + (i * 2)] = (uint8_t)(uid->bytes[i] | 0xaau);
        output[9 + (i * 2)] = (uint8_t)(uid->bytes[i] | 0x55u);
    }
    checksum = rdm_checksum(output + 8, 12);
    output[20] = (uint8_t)((checksum >> 8) | 0xaau);
    output[21] = (uint8_t)((checksum >> 8) | 0x55u);
    output[22] = (uint8_t)((uint8_t)checksum | 0xaau);
    output[23] = (uint8_t)((uint8_t)checksum | 0x55u);
    return RDM_DISCOVERY_RESPONSE_SIZE;
}
