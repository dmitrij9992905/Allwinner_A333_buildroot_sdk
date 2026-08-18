#define _POSIX_C_SOURCE 200809L

#include "dmx_controller.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

static void sleep_ms(unsigned int milliseconds)
{
    struct timespec duration;

    duration.tv_sec = (time_t)(milliseconds / 1000u);
    duration.tv_nsec = (long)((milliseconds % 1000u) * 1000000u);
    (void)nanosleep(&duration, NULL);
}

static dmx_controller_snapshot_t wait_for_idle(dmx_controller_t *controller,
                                                size_t expected_devices,
                                                uint32_t minimum_transactions)
{
    dmx_controller_snapshot_t snapshot;
    unsigned int attempt;

    memset(&snapshot, 0, sizeof(snapshot));
    for (attempt = 0; attempt < 2000u; ++attempt) {
        dmx_controller_get_snapshot(controller, &snapshot);
        if (!snapshot.rdm_busy && snapshot.device_count == expected_devices &&
            snapshot.rdm_transactions >= minimum_transactions)
            return snapshot;
        sleep_ms(1);
    }
    assert(!"controller operation timed out");
    return snapshot;
}

static dmx_controller_snapshot_t wait_for_frames(dmx_controller_t *controller,
                                                  uint32_t minimum_frames)
{
    dmx_controller_snapshot_t snapshot;
    unsigned int attempt;

    memset(&snapshot, 0, sizeof(snapshot));
    for (attempt = 0; attempt < 1000u; ++attempt) {
        dmx_controller_get_snapshot(controller, &snapshot);
        if (snapshot.dmx_frames_sent >= minimum_frames)
            return snapshot;
        sleep_ms(1);
    }
    assert(!"DMX frame generation timed out");
    return snapshot;
}

int main(void)
{
    dmx_controller_config_t config;
    dmx_controller_snapshot_t snapshot;
    dmx_controller_t *controller;
    rdm_uid_t first_uid;
    uint32_t transactions;

    memset(&config, 0, sizeof(config));
    config.serial_path = "/dev/ttyS4";
    config.controller_uid = rdm_uid_from_u64(UINT64_C(0x7ff012345678));
    config.rs485_mode = DMX_RS485_AUTO_DIRECTION;
    config.break_mode = DMX_BREAK_IOCTL;
    config.simulate = true;

    controller = dmx_controller_create(&config);
    assert(controller != NULL);
    assert(!dmx_controller_discover(controller));
    assert(dmx_controller_start(controller) == 0);

    dmx_controller_get_snapshot(controller, &snapshot);
    assert(snapshot.worker_running);
    assert(snapshot.serial_open);
    assert(snapshot.scene_address == 1);
    assert(snapshot.brightness == 255);

    dmx_controller_set_scene(controller, 100, 128, 64, 32, 127);
    dmx_controller_set_blackout(controller, true);
    dmx_controller_get_snapshot(controller, &snapshot);
    assert(snapshot.scene_address == 100);
    assert(snapshot.red == 128);
    assert(snapshot.green == 64);
    assert(snapshot.blue == 32);
    assert(snapshot.brightness == 127);
    assert(snapshot.blackout);
    dmx_controller_set_blackout(controller, false);

    snapshot = wait_for_frames(controller, 2);
    assert(!snapshot.blackout);

    assert(dmx_controller_discover(controller));
    snapshot = wait_for_idle(controller, 2, 6);
    assert(snapshot.device_count == 2);
    assert(snapshot.devices[0].information_valid);
    assert(snapshot.devices[0].dmx_footprint == 3);
    assert(strcmp(snapshot.devices[0].device_label, "Wash Left") == 0);
    first_uid = snapshot.devices[0].uid;

    transactions = snapshot.rdm_transactions;
    assert(dmx_controller_request_device_info(controller, &first_uid));
    snapshot = wait_for_idle(controller, 2, transactions + 1u);
    assert(snapshot.devices[0].information_valid);

    transactions = snapshot.rdm_transactions;
    assert(dmx_controller_set_identify(controller, &first_uid, true));
    snapshot = wait_for_idle(controller, 2, transactions + 1u);
    assert(snapshot.devices[0].identify_known);
    assert(snapshot.devices[0].identify_on);

    transactions = snapshot.rdm_transactions;
    assert(dmx_controller_set_start_address(controller, &first_uid, 201));
    snapshot = wait_for_idle(controller, 2, transactions + 1u);
    assert(snapshot.devices[0].dmx_start_address == 201);
    assert(!dmx_controller_set_start_address(controller, &first_uid, 0));
    assert(!dmx_controller_set_start_address(controller, &first_uid, 513));

    dmx_controller_stop(controller);
    dmx_controller_get_snapshot(controller, &snapshot);
    assert(!snapshot.worker_running);
    assert(!snapshot.serial_open);
    assert(!snapshot.rdm_busy);
    assert(!dmx_controller_discover(controller));

    /* Starting the same instance again is supported for console testing. */
    assert(dmx_controller_start(controller) == 0);
    snapshot = wait_for_frames(controller, snapshot.dmx_frames_sent + 1u);
    assert(snapshot.worker_running);
    dmx_controller_destroy(controller);

    puts("dmx_controller: all tests passed");
    return 0;
}
