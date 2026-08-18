#include "rdm_protocol.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_uid(void)
{
    rdm_uid_t uid;
    char text[14];

    assert(rdm_uid_parse("7ff0:1234abcd", &uid));
    assert(rdm_uid_to_u64(&uid) == UINT64_C(0x7ff01234abcd));
    rdm_uid_format(&uid, text);
    assert(strcmp(text, "7FF0:1234ABCD") == 0);
    assert(rdm_uid_equal(&uid, &(rdm_uid_t){{0x7f, 0xf0, 0x12, 0x34, 0xab, 0xcd}}));
    assert(!rdm_uid_parse("not-a-uid", &uid));
}

static void test_request_and_parser(void)
{
    const rdm_uid_t destination = {{0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc}};
    const rdm_uid_t source = {{0x7f, 0xf0, 0x00, 0x00, 0x00, 0x01}};
    uint8_t packet[RDM_MAX_PACKET_SIZE];
    rdm_message_t parsed;
    size_t length;

    length = rdm_build_request(packet,
                               sizeof(packet),
                               &destination,
                               &source,
                               42,
                               1,
                               0,
                               RDM_CC_GET_COMMAND,
                               RDM_PID_DEVICE_INFO,
                               NULL,
                               0);
    assert(length == 26);
    assert(packet[0] == RDM_START_CODE);
    assert(packet[2] == 24);
    assert(packet[15] == 42);
    assert(packet[20] == RDM_CC_GET_COMMAND);
    assert(packet[21] == 0x00 && packet[22] == 0x60);
    assert(rdm_parse_message(packet, length, &parsed) == RDM_PARSE_OK);
    assert(rdm_uid_equal(&parsed.destination, &destination));
    assert(rdm_uid_equal(&parsed.source, &source));
    assert(parsed.parameter_id == RDM_PID_DEVICE_INFO);

    packet[length - 1] ^= 1;
    assert(rdm_parse_message(packet, length, NULL) == RDM_PARSE_BAD_CHECKSUM);
}

static void test_stream_finder(void)
{
    const rdm_uid_t destination = {{0x7f, 0xf0, 0, 0, 0, 1}};
    const rdm_uid_t source = {{0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc}};
    uint8_t stream[64] = {0x00, 0xcc, 0x99, 0x11, 0x22};
    size_t packet_length;
    size_t offset;
    rdm_message_t parsed;

    packet_length = rdm_build_request(stream + 5,
                                      sizeof(stream) - 5,
                                      &destination,
                                      &source,
                                      7,
                                      1,
                                      0,
                                      RDM_CC_GET_COMMAND_RESPONSE,
                                      RDM_PID_DEVICE_LABEL,
                                      (const uint8_t *)"Wash 1",
                                      6);
    assert(packet_length != 0);
    assert(rdm_find_message(stream, 5 + packet_length, &offset, &parsed) == packet_length);
    assert(offset == 5);
    assert(parsed.parameter_data_length == 6);
    assert(memcmp(parsed.parameter_data, "Wash 1", 6) == 0);
}

static void test_discovery_response(void)
{
    const rdm_uid_t expected = {{0x01, 0x23, 0x45, 0x67, 0x89, 0xab}};
    rdm_uid_t decoded;
    uint8_t response[RDM_DISCOVERY_RESPONSE_SIZE];
    uint8_t noisy[40] = {0x00, 0x01, 0x02};

    assert(rdm_encode_discovery_response(response, &expected) == sizeof(response));
    assert(rdm_decode_discovery_response(response, sizeof(response), &decoded));
    assert(rdm_uid_equal(&decoded, &expected));

    /* A controller must accept a response with all preamble bytes stripped. */
    assert(rdm_decode_discovery_response(response + 7, sizeof(response) - 7, &decoded));
    assert(rdm_uid_equal(&decoded, &expected));

    memcpy(noisy + 3, response, sizeof(response));
    assert(rdm_decode_discovery_response(noisy, 3 + sizeof(response), &decoded));
    noisy[3 + 10] ^= 0x01;
    assert(!rdm_decode_discovery_response(noisy, 3 + sizeof(response), &decoded));
}

int main(void)
{
    test_uid();
    test_request_and_parser();
    test_stream_finder();
    test_discovery_response();
    puts("rdm_protocol: all tests passed");
    return 0;
}
