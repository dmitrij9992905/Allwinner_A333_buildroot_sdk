#define _POSIX_C_SOURCE 200809L

#include "dmx_controller.h"
#include "dmx_transport.h"

#include <errno.h>
#include <pthread.h>
#include <sched.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define DMX_DEFAULT_SERIAL_PATH "/dev/ttyS4"
#define DMX_SERIAL_PATH_SIZE 256u
#define DMX_JOB_QUEUE_SIZE 16u
#define DMX_RDM_RECEIVE_SIZE 1024u
#define DMX_DISCOVERY_QUERY_BUDGET 4096u
#define DMX_SIMULATED_FRAME_US 23000u
#define RDM_RESPONSE_LENGTH_ANY (-1)
#define RDM_RESPONSE_LENGTH_MUTE (-2)

typedef enum {
    DMX_JOB_DISCOVER = 0,
    DMX_JOB_DEVICE_INFO,
    DMX_JOB_IDENTIFY,
    DMX_JOB_START_ADDRESS,
} dmx_job_type_t;

typedef struct {
    dmx_job_type_t type;
    rdm_uid_t uid;
    bool enabled;
    uint16_t start_address;
} dmx_job_t;

struct dmx_controller {
    pthread_mutex_t mutex;
    pthread_cond_t condition;
    pthread_t worker;
    bool thread_created;
    bool stop_requested;

    char serial_path[DMX_SERIAL_PATH_SIZE];
    rdm_uid_t controller_uid;
    dmx_rs485_mode_t rs485_mode;
    dmx_break_mode_t break_mode;
    bool simulate;

    dmx_transport_t transport;
    uint8_t transaction_number;
    unsigned int discovery_budget;

    dmx_job_t jobs[DMX_JOB_QUEUE_SIZE];
    size_t job_head;
    size_t job_tail;
    size_t job_count;

    dmx_controller_snapshot_t snapshot;
};

static bool transmit_dmx_frame(dmx_controller_t *controller);

static uint16_t read_be16(const uint8_t *data)
{
    return (uint16_t)(((uint16_t)data[0] << 8) | data[1]);
}

static uint32_t read_be32(const uint8_t *data)
{
    return ((uint32_t)data[0] << 24) | ((uint32_t)data[1] << 16) |
           ((uint32_t)data[2] << 8) | data[3];
}

static void write_be16(uint8_t *data, uint16_t value)
{
    data[0] = (uint8_t)(value >> 8);
    data[1] = (uint8_t)value;
}

static void status_locked(dmx_controller_t *controller, const char *format, ...)
{
    va_list arguments;

    va_start(arguments, format);
    (void)vsnprintf(controller->snapshot.status,
                    sizeof(controller->snapshot.status),
                    format,
                    arguments);
    va_end(arguments);
    ++controller->snapshot.generation;
}

static bool uid_is_zero(const rdm_uid_t *uid)
{
    static const rdm_uid_t zero_uid = {{0, 0, 0, 0, 0, 0}};

    return rdm_uid_equal(uid, &zero_uid);
}

static void sleep_us(unsigned int duration_us)
{
    struct timespec duration;

    duration.tv_sec = (time_t)(duration_us / 1000000u);
    duration.tv_nsec = (long)((duration_us % 1000000u) * 1000u);
    while (nanosleep(&duration, &duration) != 0 && errno == EINTR) {
    }
}

static bool stopping(dmx_controller_t *controller)
{
    bool value;

    (void)pthread_mutex_lock(&controller->mutex);
    value = controller->stop_requested;
    (void)pthread_mutex_unlock(&controller->mutex);
    return value;
}

static ssize_t find_device_locked(const dmx_controller_t *controller,
                                  const rdm_uid_t *uid)
{
    size_t i;

    for (i = 0; i < controller->snapshot.device_count; ++i) {
        if (rdm_uid_equal(&controller->snapshot.devices[i].uid, uid))
            return (ssize_t)i;
    }
    return -1;
}

static bool add_device(dmx_controller_t *controller, const rdm_uid_t *uid)
{
    bool added = false;

    (void)pthread_mutex_lock(&controller->mutex);
    if (find_device_locked(controller, uid) < 0 &&
        controller->snapshot.device_count < DMX_MAX_DISCOVERED_DEVICES) {
        dmx_rdm_device_t *device =
            &controller->snapshot.devices[controller->snapshot.device_count++];

        memset(device, 0, sizeof(*device));
        device->uid = *uid;
        added = true;
        ++controller->snapshot.generation;
    }
    (void)pthread_mutex_unlock(&controller->mutex);
    return added;
}

static void record_transaction(dmx_controller_t *controller, bool error)
{
    (void)pthread_mutex_lock(&controller->mutex);
    ++controller->snapshot.rdm_transactions;
    if (error)
        ++controller->snapshot.rdm_errors;
    ++controller->snapshot.generation;
    (void)pthread_mutex_unlock(&controller->mutex);
}

static void record_discovery_query(dmx_controller_t *controller)
{
    (void)pthread_mutex_lock(&controller->mutex);
    ++controller->snapshot.discovery_queries;
    ++controller->snapshot.generation;
    (void)pthread_mutex_unlock(&controller->mutex);
}

static void set_transport_error_status(dmx_controller_t *controller,
                                       const char *operation)
{
    (void)pthread_mutex_lock(&controller->mutex);
    status_locked(controller,
                  "%s: %s",
                  operation,
                  dmx_transport_error(&controller->transport));
    (void)pthread_mutex_unlock(&controller->mutex);
}

static bool response_matches(const dmx_controller_t *controller,
                             const rdm_message_t *response,
                             const rdm_uid_t *responder,
                             uint8_t transaction_number,
                             uint8_t response_command_class,
                             uint16_t parameter_id)
{
    return rdm_uid_equal(&response->destination, &controller->controller_uid) &&
           rdm_uid_equal(&response->source, responder) &&
           response->transaction_number == transaction_number &&
           response->sub_device == 0 &&
           response->command_class == response_command_class &&
           response->parameter_id == parameter_id;
}

static bool response_length_matches(int expected, uint8_t actual)
{
    if (expected == RDM_RESPONSE_LENGTH_ANY)
        return true;
    /* DISC_MUTE returns a 16-bit control field and, for a bound responder
     * port, may append the optional six-byte Binding UID.
     */
    if (expected == RDM_RESPONSE_LENGTH_MUTE)
        return actual == 2u || actual == 8u;
    return expected >= 0 && actual == (uint8_t)expected;
}

static bool exchange_message(dmx_controller_t *controller,
                             const rdm_uid_t *destination,
                             uint8_t command_class,
                             uint16_t parameter_id,
                             const uint8_t *parameter_data,
                             uint8_t parameter_data_length,
                             int expected_response_length,
                             rdm_message_t *response)
{
    uint8_t request[RDM_MAX_PACKET_SIZE];
    uint8_t receive_buffer[DMX_RDM_RECEIVE_SIZE];
    uint8_t transaction_number = controller->transaction_number++;
    uint8_t response_class = command_class == RDM_CC_GET_COMMAND
                                 ? RDM_CC_GET_COMMAND_RESPONSE
                                 : command_class == RDM_CC_SET_COMMAND
                                       ? RDM_CC_SET_COMMAND_RESPONSE
                                       : RDM_CC_DISCOVERY_COMMAND_RESPONSE;
    size_t request_length;
    ssize_t received;
    size_t cursor = 0;

    request_length = rdm_build_request(request,
                                       sizeof(request),
                                       destination,
                                       &controller->controller_uid,
                                       transaction_number,
                                       1,
                                       0,
                                       command_class,
                                       parameter_id,
                                       parameter_data,
                                       parameter_data_length);
    if (request_length == 0) {
        record_transaction(controller, true);
        return false;
    }

    received = dmx_transport_rdm_exchange(&controller->transport,
                                          request,
                                          request_length,
                                          false,
                                          receive_buffer,
                                          sizeof(receive_buffer));
    if (received >= 0)
        (void)transmit_dmx_frame(controller);
    if (received <= 0) {
        record_transaction(controller, true);
        if (received < 0)
            set_transport_error_status(controller, "RDM transport");
        return false;
    }

    while (cursor < (size_t)received) {
        rdm_message_t candidate;
        size_t offset = 0;
        size_t packet_length = rdm_find_message(receive_buffer + cursor,
                                                (size_t)received - cursor,
                                                &offset,
                                                &candidate);

        if (packet_length == 0)
            break;
        cursor += offset + packet_length;
        if (!response_matches(controller,
                              &candidate,
                              destination,
                              transaction_number,
                              response_class,
                              parameter_id))
            continue;
        if (candidate.port_id_or_response_type != RDM_RESPONSE_ACK) {
            record_transaction(controller, true);
            return false;
        }
        if (!response_length_matches(expected_response_length,
                                     candidate.parameter_data_length)) {
            record_transaction(controller, true);
            return false;
        }
        if (response != NULL)
            *response = candidate;
        record_transaction(controller, false);
        return true;
    }

    record_transaction(controller, true);
    return false;
}

static bool send_broadcast(dmx_controller_t *controller,
                           uint16_t parameter_id,
                           const uint8_t *parameter_data,
                           uint8_t parameter_data_length)
{
    uint8_t request[RDM_MAX_PACKET_SIZE];
    size_t request_length = rdm_build_request(request,
                                              sizeof(request),
                                              &RDM_BROADCAST_UID,
                                              &controller->controller_uid,
                                              controller->transaction_number++,
                                              1,
                                              0,
                                              RDM_CC_DISCOVERY_COMMAND,
                                              parameter_id,
                                              parameter_data,
                                              parameter_data_length);
    int result;

    if (request_length == 0) {
        record_transaction(controller, true);
        return false;
    }
    result = dmx_transport_rdm_send(&controller->transport,
                                    request,
                                    request_length);
    if (result == 0)
        (void)transmit_dmx_frame(controller);
    record_transaction(controller, result != 0);
    if (result != 0)
        set_transport_error_status(controller, "RDM broadcast");
    return result == 0;
}

static bool query_unique_branch(dmx_controller_t *controller,
                                uint64_t lower,
                                uint64_t upper,
                                bool *activity,
                                bool *valid_uid,
                                rdm_uid_t *uid)
{
    uint8_t bounds[12];
    uint8_t request[RDM_MAX_PACKET_SIZE];
    uint8_t response[DMX_RDM_RECEIVE_SIZE];
    rdm_uid_t lower_uid = rdm_uid_from_u64(lower);
    rdm_uid_t upper_uid = rdm_uid_from_u64(upper);
    size_t request_length;
    size_t response_offset = 0;
    ssize_t received;

    memcpy(bounds, lower_uid.bytes, RDM_UID_SIZE);
    memcpy(bounds + RDM_UID_SIZE, upper_uid.bytes, RDM_UID_SIZE);
    request_length = rdm_build_request(request,
                                       sizeof(request),
                                       &RDM_BROADCAST_UID,
                                       &controller->controller_uid,
                                       controller->transaction_number++,
                                       1,
                                       0,
                                       RDM_CC_DISCOVERY_COMMAND,
                                       RDM_PID_DISC_UNIQUE_BRANCH,
                                       bounds,
                                       sizeof(bounds));
    if (request_length == 0)
        return false;

    record_discovery_query(controller);
    received = dmx_transport_rdm_exchange(&controller->transport,
                                          request,
                                          request_length,
                                          true,
                                          response,
                                          sizeof(response));
    if (received >= 0)
        (void)transmit_dmx_frame(controller);
    if (received < 0) {
        record_transaction(controller, true);
        set_transport_error_status(controller, "RDM discovery");
        return false;
    }
    record_transaction(controller, false);

    /* An auto-direction transceiver can leave its receiver enabled while the
     * controller transmits. Preserve that echo in the transport (flushing it
     * after tcdrain could also discard a legal 176 us response), but do not
     * mistake it for a discovery collision. A responder cannot put bytes on
     * the bus until after this request, so everything following an exact echo
     * remains candidate discovery data.
     */
    if ((size_t)received >= request_length) {
        size_t offset;

        for (offset = 0; offset + request_length <= (size_t)received; ++offset) {
            if (memcmp(response + offset, request, request_length) == 0) {
                response_offset = offset + request_length;
                break;
            }
        }
    }
    *activity = (size_t)received > response_offset;
    *valid_uid = *activity &&
                 rdm_decode_discovery_response(response + response_offset,
                                               (size_t)received - response_offset,
                                               uid);
    return true;
}

static bool mute_device(dmx_controller_t *controller, const rdm_uid_t *uid)
{
    return exchange_message(controller,
                            uid,
                            RDM_CC_DISCOVERY_COMMAND,
                            RDM_PID_DISC_MUTE,
                            NULL,
                            0,
                            RDM_RESPONSE_LENGTH_MUTE,
                            NULL);
}

static bool device_list_full(dmx_controller_t *controller)
{
    bool full;

    (void)pthread_mutex_lock(&controller->mutex);
    full = controller->snapshot.device_count >= DMX_MAX_DISCOVERED_DEVICES;
    (void)pthread_mutex_unlock(&controller->mutex);
    return full;
}

static void discover_range(dmx_controller_t *controller,
                           uint64_t lower,
                           uint64_t upper)
{
    while (!stopping(controller) && !device_list_full(controller) &&
           controller->discovery_budget != 0) {
        bool activity = false;
        bool valid_uid = false;
        rdm_uid_t uid;

        --controller->discovery_budget;
        if (!query_unique_branch(controller,
                                 lower,
                                 upper,
                                 &activity,
                                 &valid_uid,
                                 &uid))
            return;
        if (!activity)
            return;

        if (valid_uid) {
            uint64_t value = rdm_uid_to_u64(&uid);

            if (value >= lower && value <= upper && mute_device(controller, &uid)) {
                (void)add_device(controller, &uid);
                /* E1.20 requires testing the same branch after muting. */
                continue;
            }
        }

        if (lower == upper) {
            rdm_uid_t exact_uid = rdm_uid_from_u64(lower);

            if (mute_device(controller, &exact_uid))
                (void)add_device(controller, &exact_uid);
            return;
        }

        {
            uint64_t midpoint = lower + ((upper - lower) / 2u);

            discover_range(controller, lower, midpoint);
            if (!stopping(controller) && !device_list_full(controller))
                discover_range(controller, midpoint + 1u, upper);
        }
        return;
    }
}

static void run_discovery(dmx_controller_t *controller)
{
    size_t count;

    (void)pthread_mutex_lock(&controller->mutex);
    controller->snapshot.device_count = 0;
    memset(controller->snapshot.devices, 0, sizeof(controller->snapshot.devices));
    status_locked(controller, "RDM discovery in progress");
    (void)pthread_mutex_unlock(&controller->mutex);

    if (!send_broadcast(controller, RDM_PID_DISC_UN_MUTE, NULL, 0))
        return;
    controller->discovery_budget = DMX_DISCOVERY_QUERY_BUDGET;
    discover_range(controller, 0, UINT64_C(0x0000ffffffffffff));

    (void)pthread_mutex_lock(&controller->mutex);
    count = controller->snapshot.device_count;
    if (controller->discovery_budget == 0)
        status_locked(controller,
                      "RDM discovery stopped at query limit (%zu devices)",
                      count);
    else
        status_locked(controller, "RDM discovery complete: %zu device(s)", count);
    (void)pthread_mutex_unlock(&controller->mutex);
}

static void copy_rdm_string(char output[DMX_RDM_LABEL_SIZE],
                            const uint8_t *input,
                            uint8_t input_length)
{
    size_t length = input_length;
    size_t i;

    if (length >= DMX_RDM_LABEL_SIZE)
        length = DMX_RDM_LABEL_SIZE - 1u;
    for (i = 0; i < length; ++i) {
        uint8_t character = input[i];

        output[i] = character >= 0x20u && character != 0x7fu
                        ? (char)character
                        : '?';
    }
    output[length] = '\0';
}

static bool get_parameter(dmx_controller_t *controller,
                          const rdm_uid_t *uid,
                          uint16_t parameter_id,
                          rdm_message_t *response)
{
    return exchange_message(controller,
                            uid,
                            RDM_CC_GET_COMMAND,
                            parameter_id,
                            NULL,
                            0,
                            RDM_RESPONSE_LENGTH_ANY,
                            response);
}

static void get_optional_label(dmx_controller_t *controller,
                               const rdm_uid_t *uid,
                               uint16_t parameter_id,
                               char output[DMX_RDM_LABEL_SIZE])
{
    rdm_message_t response;

    if (get_parameter(controller, uid, parameter_id, &response))
        copy_rdm_string(output,
                        response.parameter_data,
                        response.parameter_data_length);
}

static void request_device_information(dmx_controller_t *controller,
                                       const rdm_uid_t *uid)
{
    dmx_rdm_device_t updated;
    rdm_message_t response;
    ssize_t index;

    (void)pthread_mutex_lock(&controller->mutex);
    index = find_device_locked(controller, uid);
    if (index >= 0)
        updated = controller->snapshot.devices[index];
    (void)pthread_mutex_unlock(&controller->mutex);
    if (index < 0) {
        (void)pthread_mutex_lock(&controller->mutex);
        ++controller->snapshot.rdm_errors;
        status_locked(controller, "RDM device is not in the discovery snapshot");
        (void)pthread_mutex_unlock(&controller->mutex);
        return;
    }

    if (!get_parameter(controller, uid, RDM_PID_DEVICE_INFO, &response) ||
        response.parameter_data_length != 19u) {
        (void)pthread_mutex_lock(&controller->mutex);
        status_locked(controller, "DEVICE_INFO request failed");
        (void)pthread_mutex_unlock(&controller->mutex);
        return;
    }

    updated.protocol_version = read_be16(response.parameter_data);
    updated.device_model_id = read_be16(response.parameter_data + 2);
    updated.product_category = read_be16(response.parameter_data + 4);
    updated.software_version_id = read_be32(response.parameter_data + 6);
    updated.dmx_footprint = read_be16(response.parameter_data + 10);
    updated.current_personality = response.parameter_data[12];
    updated.personality_count = response.parameter_data[13];
    updated.dmx_start_address = read_be16(response.parameter_data + 14);
    updated.sub_device_count = read_be16(response.parameter_data + 16);
    updated.sensor_count = response.parameter_data[18];
    updated.information_valid = true;

    get_optional_label(controller,
                       uid,
                       RDM_PID_MANUFACTURER_LABEL,
                       updated.manufacturer_label);
    get_optional_label(controller,
                       uid,
                       RDM_PID_DEVICE_MODEL_DESCRIPTION,
                       updated.model_description);
    get_optional_label(controller,
                       uid,
                       RDM_PID_DEVICE_LABEL,
                       updated.device_label);
    get_optional_label(controller,
                       uid,
                       RDM_PID_SOFTWARE_VERSION_LABEL,
                       updated.software_version_label);

    if (get_parameter(controller, uid, RDM_PID_IDENTIFY_DEVICE, &response) &&
        response.parameter_data_length == 1u) {
        updated.identify_known = true;
        updated.identify_on = response.parameter_data[0] != 0;
    }

    (void)pthread_mutex_lock(&controller->mutex);
    index = find_device_locked(controller, uid);
    if (index >= 0)
        controller->snapshot.devices[index] = updated;
    status_locked(controller, "RDM device information updated");
    (void)pthread_mutex_unlock(&controller->mutex);
}

static void set_identify(dmx_controller_t *controller,
                         const rdm_uid_t *uid,
                         bool enabled)
{
    uint8_t value = enabled ? 1u : 0u;

    if (exchange_message(controller,
                         uid,
                         RDM_CC_SET_COMMAND,
                         RDM_PID_IDENTIFY_DEVICE,
                         &value,
                         1,
                         0,
                         NULL)) {
        (void)pthread_mutex_lock(&controller->mutex);
        {
            ssize_t index = find_device_locked(controller, uid);

            if (index >= 0) {
                controller->snapshot.devices[index].identify_known = true;
                controller->snapshot.devices[index].identify_on = enabled;
            }
        }
        status_locked(controller,
                      "RDM Identify %s",
                      enabled ? "enabled" : "disabled");
        (void)pthread_mutex_unlock(&controller->mutex);
    } else {
        (void)pthread_mutex_lock(&controller->mutex);
        status_locked(controller, "RDM Identify command failed");
        (void)pthread_mutex_unlock(&controller->mutex);
    }
}

static void set_start_address(dmx_controller_t *controller,
                              const rdm_uid_t *uid,
                              uint16_t start_address)
{
    uint8_t parameter_data[2];

    write_be16(parameter_data, start_address);
    if (exchange_message(controller,
                         uid,
                         RDM_CC_SET_COMMAND,
                         RDM_PID_DMX_START_ADDRESS,
                         parameter_data,
                         sizeof(parameter_data),
                         0,
                         NULL)) {
        (void)pthread_mutex_lock(&controller->mutex);
        {
            ssize_t index = find_device_locked(controller, uid);

            if (index >= 0)
                controller->snapshot.devices[index].dmx_start_address = start_address;
        }
        status_locked(controller, "RDM DMX start address set to %u", start_address);
        (void)pthread_mutex_unlock(&controller->mutex);
    } else {
        (void)pthread_mutex_lock(&controller->mutex);
        status_locked(controller, "RDM DMX_START_ADDRESS command failed");
        (void)pthread_mutex_unlock(&controller->mutex);
    }
}

static void populate_simulated_device(dmx_rdm_device_t *device,
                                      rdm_uid_t uid,
                                      uint16_t model_id,
                                      uint16_t address,
                                      const char *manufacturer,
                                      const char *model,
                                      const char *label,
                                      const char *version)
{
    memset(device, 0, sizeof(*device));
    device->uid = uid;
    device->information_valid = true;
    device->identify_known = true;
    device->protocol_version = 0x0100;
    device->device_model_id = model_id;
    device->product_category = 0x0101;
    device->software_version_id = 0x00010000;
    device->dmx_footprint = 3;
    device->current_personality = 1;
    device->personality_count = 1;
    device->dmx_start_address = address;
    (void)snprintf(device->manufacturer_label,
                   sizeof(device->manufacturer_label),
                   "%s",
                   manufacturer);
    (void)snprintf(device->model_description,
                   sizeof(device->model_description),
                   "%s",
                   model);
    (void)snprintf(device->device_label,
                   sizeof(device->device_label),
                   "%s",
                   label);
    (void)snprintf(device->software_version_label,
                   sizeof(device->software_version_label),
                   "%s",
                   version);
}

static void simulate_discovery(dmx_controller_t *controller)
{
    (void)pthread_mutex_lock(&controller->mutex);
    controller->snapshot.device_count = 2;
    populate_simulated_device(&controller->snapshot.devices[0],
                              rdm_uid_from_u64(UINT64_C(0x7ff000000001)),
                              0x1001,
                              1,
                              "Luckfox Demo",
                              "RGB Wash",
                              "Wash Left",
                              "1.0.0");
    populate_simulated_device(&controller->snapshot.devices[1],
                              rdm_uid_from_u64(UINT64_C(0x657400000002)),
                              0x1002,
                              10,
                              "Open Lighting",
                              "RGB PAR",
                              "PAR Right",
                              "2.1.0");
    controller->snapshot.discovery_queries += 3;
    controller->snapshot.rdm_transactions += 6;
    status_locked(controller, "Simulated RDM discovery complete: 2 devices");
    (void)pthread_mutex_unlock(&controller->mutex);
}

static void simulate_job(dmx_controller_t *controller, const dmx_job_t *job)
{
    ssize_t index;

    if (job->type == DMX_JOB_DISCOVER) {
        simulate_discovery(controller);
        return;
    }

    (void)pthread_mutex_lock(&controller->mutex);
    index = find_device_locked(controller, &job->uid);
    if (index < 0) {
        ++controller->snapshot.rdm_errors;
        status_locked(controller, "Simulated RDM device not found");
        (void)pthread_mutex_unlock(&controller->mutex);
        return;
    }

    ++controller->snapshot.rdm_transactions;
    switch (job->type) {
    case DMX_JOB_DEVICE_INFO:
        controller->snapshot.devices[index].information_valid = true;
        status_locked(controller, "Simulated device information updated");
        break;
    case DMX_JOB_IDENTIFY:
        controller->snapshot.devices[index].identify_known = true;
        controller->snapshot.devices[index].identify_on = job->enabled;
        status_locked(controller,
                      "Simulated Identify %s",
                      job->enabled ? "enabled" : "disabled");
        break;
    case DMX_JOB_START_ADDRESS:
        controller->snapshot.devices[index].dmx_start_address = job->start_address;
        status_locked(controller,
                      "Simulated DMX start address set to %u",
                      job->start_address);
        break;
    case DMX_JOB_DISCOVER:
        break;
    }
    (void)pthread_mutex_unlock(&controller->mutex);
}

static void process_job(dmx_controller_t *controller, const dmx_job_t *job)
{
    if (controller->simulate) {
        simulate_job(controller, job);
        return;
    }

    switch (job->type) {
    case DMX_JOB_DISCOVER:
        run_discovery(controller);
        break;
    case DMX_JOB_DEVICE_INFO:
        request_device_information(controller, &job->uid);
        break;
    case DMX_JOB_IDENTIFY:
        set_identify(controller, &job->uid, job->enabled);
        break;
    case DMX_JOB_START_ADDRESS:
        set_start_address(controller, &job->uid, job->start_address);
        break;
    }
}

static void make_universe_locked(const dmx_controller_t *controller,
                                 uint8_t slots[DMX_UNIVERSE_SLOTS])
{
    uint16_t address = controller->snapshot.scene_address;

    memset(slots, 0, DMX_UNIVERSE_SLOTS);
    if (controller->snapshot.blackout || address < 1u || address > 510u)
        return;
    slots[address - 1u] = (uint8_t)(((uint16_t)controller->snapshot.red *
                                     controller->snapshot.brightness) /
                                    255u);
    slots[address] = (uint8_t)(((uint16_t)controller->snapshot.green *
                                controller->snapshot.brightness) /
                               255u);
    slots[address + 1u] = (uint8_t)(((uint16_t)controller->snapshot.blue *
                                     controller->snapshot.brightness) /
                                    255u);
}

/* RDM packets replace, rather than run alongside, DMX data on the wire. Send
 * a complete current universe between RDM transactions so a long binary-tree
 * discovery does not leave fixtures without ordinary DMX for several seconds.
 */
static bool transmit_dmx_frame(dmx_controller_t *controller)
{
    uint8_t slots[DMX_UNIVERSE_SLOTS];

    (void)pthread_mutex_lock(&controller->mutex);
    make_universe_locked(controller, slots);
    (void)pthread_mutex_unlock(&controller->mutex);

    if (dmx_transport_send_dmx(&controller->transport, slots) != 0) {
        (void)pthread_mutex_lock(&controller->mutex);
        ++controller->snapshot.rdm_errors;
        status_locked(controller,
                      "DMX output failed: %s",
                      dmx_transport_error(&controller->transport));
        (void)pthread_mutex_unlock(&controller->mutex);
        return false;
    }

    (void)pthread_mutex_lock(&controller->mutex);
    ++controller->snapshot.dmx_frames_sent;
    controller->snapshot.timing_warning =
        controller->snapshot.timing_warning ||
        controller->transport.timing_warning;
    ++controller->snapshot.generation;
    (void)pthread_mutex_unlock(&controller->mutex);
    return true;
}

static bool pop_job_locked(dmx_controller_t *controller, dmx_job_t *job)
{
    if (controller->job_count == 0)
        return false;
    *job = controller->jobs[controller->job_head];
    controller->job_head = (controller->job_head + 1u) % DMX_JOB_QUEUE_SIZE;
    --controller->job_count;
    return true;
}

static void *worker_main(void *context)
{
    dmx_controller_t *controller = context;

    if (!controller->simulate) {
        struct sched_param parameters;
        int scheduler_result;

        memset(&parameters, 0, sizeof(parameters));
        parameters.sched_priority = 20;
        scheduler_result = pthread_setschedparam(pthread_self(),
                                                SCHED_FIFO,
                                                &parameters);
        if (scheduler_result != 0) {
            (void)pthread_mutex_lock(&controller->mutex);
            controller->snapshot.timing_warning = true;
            status_locked(controller,
                          "UART open; SCHED_FIFO unavailable (%s)",
                          strerror(scheduler_result));
            (void)pthread_mutex_unlock(&controller->mutex);
        }
    }

    for (;;) {
        dmx_job_t job;
        bool have_job;
        bool should_stop;

        (void)pthread_mutex_lock(&controller->mutex);
        should_stop = controller->stop_requested;
        have_job = !should_stop && pop_job_locked(controller, &job);
        if (have_job)
            controller->snapshot.rdm_busy = true;
        (void)pthread_mutex_unlock(&controller->mutex);

        if (should_stop)
            break;
        if (have_job) {
            process_job(controller, &job);
            (void)pthread_mutex_lock(&controller->mutex);
            controller->snapshot.rdm_busy = controller->job_count != 0;
            ++controller->snapshot.generation;
            (void)pthread_mutex_unlock(&controller->mutex);
            continue;
        }

        if (controller->simulate) {
            sleep_us(DMX_SIMULATED_FRAME_US);
        } else if (!transmit_dmx_frame(controller)) {
            sleep_us(DMX_SIMULATED_FRAME_US);
            continue;
        }

        if (controller->simulate) {
            (void)pthread_mutex_lock(&controller->mutex);
            ++controller->snapshot.dmx_frames_sent;
            ++controller->snapshot.generation;
            (void)pthread_mutex_unlock(&controller->mutex);
        }
    }
    return NULL;
}

static bool enqueue_job(dmx_controller_t *controller, const dmx_job_t *job)
{
    bool accepted = false;

    if (controller == NULL || job == NULL)
        return false;
    (void)pthread_mutex_lock(&controller->mutex);
    if (controller->thread_created && !controller->stop_requested &&
        controller->job_count < DMX_JOB_QUEUE_SIZE) {
        controller->jobs[controller->job_tail] = *job;
        controller->job_tail = (controller->job_tail + 1u) % DMX_JOB_QUEUE_SIZE;
        ++controller->job_count;
        controller->snapshot.rdm_busy = true;
        status_locked(controller, "RDM operation queued");
        (void)pthread_cond_signal(&controller->condition);
        accepted = true;
    }
    (void)pthread_mutex_unlock(&controller->mutex);
    return accepted;
}

dmx_controller_t *dmx_controller_create(const dmx_controller_config_t *config)
{
    dmx_controller_t *controller = calloc(1, sizeof(*controller));
    const char *serial_path = DMX_DEFAULT_SERIAL_PATH;
    int result;

    if (controller == NULL)
        return NULL;
    result = pthread_mutex_init(&controller->mutex, NULL);
    if (result != 0) {
        free(controller);
        return NULL;
    }
    result = pthread_cond_init(&controller->condition, NULL);
    if (result != 0) {
        (void)pthread_mutex_destroy(&controller->mutex);
        free(controller);
        return NULL;
    }

    controller->rs485_mode = DMX_RS485_AUTO_DIRECTION;
    controller->break_mode = DMX_BREAK_IOCTL;
    controller->controller_uid =
        rdm_uid_from_u64(UINT64_C(0x7ff000000001));
    if (config != NULL) {
        if (config->serial_path != NULL && config->serial_path[0] != '\0')
            serial_path = config->serial_path;
        controller->controller_uid = config->controller_uid;
        controller->rs485_mode = config->rs485_mode;
        controller->break_mode = config->break_mode;
        controller->simulate = config->simulate;
    }
    if (uid_is_zero(&controller->controller_uid))
        controller->controller_uid =
            rdm_uid_from_u64(UINT64_C(0x7ff000000001));
    if (controller->rs485_mode != DMX_RS485_AUTO_DIRECTION &&
        controller->rs485_mode != DMX_RS485_KERNEL_RTS)
        controller->rs485_mode = DMX_RS485_AUTO_DIRECTION;
    if (controller->break_mode != DMX_BREAK_IOCTL &&
        controller->break_mode != DMX_BREAK_BAUD)
        controller->break_mode = DMX_BREAK_IOCTL;

    (void)snprintf(controller->serial_path,
                   sizeof(controller->serial_path),
                   "%s",
                   serial_path);
    dmx_transport_init(&controller->transport);
    controller->snapshot.scene_address = 1;
    controller->snapshot.brightness = 255;
    (void)snprintf(controller->snapshot.status,
                   sizeof(controller->snapshot.status),
                   "Controller created; not started");
    return controller;
}

int dmx_controller_start(dmx_controller_t *controller)
{
    int result;

    if (controller == NULL)
        return -EINVAL;
    (void)pthread_mutex_lock(&controller->mutex);
    if (controller->thread_created) {
        (void)pthread_mutex_unlock(&controller->mutex);
        return 0;
    }
    controller->stop_requested = false;
    controller->job_head = 0;
    controller->job_tail = 0;
    controller->job_count = 0;
    controller->snapshot.rdm_busy = false;
    (void)pthread_mutex_unlock(&controller->mutex);

    if (!controller->simulate) {
        result = dmx_transport_open(&controller->transport,
                                    controller->serial_path,
                                    controller->rs485_mode,
                                    controller->break_mode);
        if (result != 0) {
            (void)pthread_mutex_lock(&controller->mutex);
            controller->snapshot.serial_open = false;
            status_locked(controller,
                          "Serial open failed: %s",
                          dmx_transport_error(&controller->transport));
            (void)pthread_mutex_unlock(&controller->mutex);
            return result;
        }
    }

    (void)pthread_mutex_lock(&controller->mutex);
    controller->snapshot.serial_open = true;
    controller->snapshot.worker_running = true;
    controller->thread_created = true;
    status_locked(controller,
                  controller->simulate ? "Simulation started"
                                       : "DMX output started on %s",
                  controller->serial_path);
    (void)pthread_mutex_unlock(&controller->mutex);

    result = pthread_create(&controller->worker, NULL, worker_main, controller);
    if (result != 0) {
        (void)pthread_mutex_lock(&controller->mutex);
        controller->thread_created = false;
        controller->snapshot.worker_running = false;
        controller->snapshot.serial_open = false;
        status_locked(controller, "pthread_create failed: %s", strerror(result));
        (void)pthread_mutex_unlock(&controller->mutex);
        dmx_transport_close(&controller->transport);
        return -result;
    }
    return 0;
}

void dmx_controller_stop(dmx_controller_t *controller)
{
    bool join_worker;

    if (controller == NULL)
        return;
    (void)pthread_mutex_lock(&controller->mutex);
    join_worker = controller->thread_created;
    if (join_worker) {
        controller->stop_requested = true;
        (void)pthread_cond_broadcast(&controller->condition);
    }
    (void)pthread_mutex_unlock(&controller->mutex);
    if (!join_worker)
        return;

    (void)pthread_join(controller->worker, NULL);
    dmx_transport_close(&controller->transport);
    (void)pthread_mutex_lock(&controller->mutex);
    controller->thread_created = false;
    controller->snapshot.worker_running = false;
    controller->snapshot.serial_open = false;
    controller->snapshot.rdm_busy = false;
    controller->job_count = 0;
    status_locked(controller, "Controller stopped");
    (void)pthread_mutex_unlock(&controller->mutex);
}

void dmx_controller_destroy(dmx_controller_t *controller)
{
    if (controller == NULL)
        return;
    dmx_controller_stop(controller);
    (void)pthread_cond_destroy(&controller->condition);
    (void)pthread_mutex_destroy(&controller->mutex);
    free(controller);
}

void dmx_controller_set_scene(dmx_controller_t *controller,
                              uint16_t start_address,
                              uint8_t red,
                              uint8_t green,
                              uint8_t blue,
                              uint8_t brightness)
{
    if (controller == NULL)
        return;
    if (start_address < 1u)
        start_address = 1u;
    if (start_address > 510u)
        start_address = 510u;
    (void)pthread_mutex_lock(&controller->mutex);
    controller->snapshot.scene_address = start_address;
    controller->snapshot.red = red;
    controller->snapshot.green = green;
    controller->snapshot.blue = blue;
    controller->snapshot.brightness = brightness;
    ++controller->snapshot.generation;
    (void)pthread_mutex_unlock(&controller->mutex);
}

void dmx_controller_set_blackout(dmx_controller_t *controller, bool blackout)
{
    if (controller == NULL)
        return;
    (void)pthread_mutex_lock(&controller->mutex);
    controller->snapshot.blackout = blackout;
    ++controller->snapshot.generation;
    (void)pthread_mutex_unlock(&controller->mutex);
}

bool dmx_controller_discover(dmx_controller_t *controller)
{
    const dmx_job_t job = {DMX_JOB_DISCOVER, {{0}}, false, 0};

    return enqueue_job(controller, &job);
}

bool dmx_controller_request_device_info(dmx_controller_t *controller,
                                        const rdm_uid_t *uid)
{
    dmx_job_t job;

    if (uid == NULL)
        return false;
    memset(&job, 0, sizeof(job));
    job.type = DMX_JOB_DEVICE_INFO;
    job.uid = *uid;
    return enqueue_job(controller, &job);
}

bool dmx_controller_set_identify(dmx_controller_t *controller,
                                 const rdm_uid_t *uid,
                                 bool enabled)
{
    dmx_job_t job;

    if (uid == NULL)
        return false;
    memset(&job, 0, sizeof(job));
    job.type = DMX_JOB_IDENTIFY;
    job.uid = *uid;
    job.enabled = enabled;
    return enqueue_job(controller, &job);
}

bool dmx_controller_set_start_address(dmx_controller_t *controller,
                                      const rdm_uid_t *uid,
                                      uint16_t start_address)
{
    dmx_job_t job;

    if (uid == NULL || start_address < 1u || start_address > 512u)
        return false;
    memset(&job, 0, sizeof(job));
    job.type = DMX_JOB_START_ADDRESS;
    job.uid = *uid;
    job.start_address = start_address;
    return enqueue_job(controller, &job);
}

void dmx_controller_get_snapshot(dmx_controller_t *controller,
                                 dmx_controller_snapshot_t *snapshot)
{
    if (snapshot == NULL)
        return;
    if (controller == NULL) {
        memset(snapshot, 0, sizeof(*snapshot));
        return;
    }
    (void)pthread_mutex_lock(&controller->mutex);
    *snapshot = controller->snapshot;
    (void)pthread_mutex_unlock(&controller->mutex);
}
