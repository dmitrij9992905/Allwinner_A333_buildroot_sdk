#ifndef DMX_PANEL_DMX_TRANSPORT_H
#define DMX_PANEL_DMX_TRANSPORT_H

#include "dmx_controller.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DMX_TRANSPORT_ERROR_SIZE 128u

typedef struct {
    int fd;
    dmx_rs485_mode_t rs485_mode;
    dmx_break_mode_t break_mode;
    bool timing_warning;
    char error[DMX_TRANSPORT_ERROR_SIZE];
} dmx_transport_t;

void dmx_transport_init(dmx_transport_t *transport);
int dmx_transport_open(dmx_transport_t *transport,
                       const char *serial_path,
                       dmx_rs485_mode_t rs485_mode,
                       dmx_break_mode_t break_mode);
void dmx_transport_close(dmx_transport_t *transport);

int dmx_transport_send_dmx(dmx_transport_t *transport,
                           const uint8_t slots[DMX_UNIVERSE_SLOTS]);

/*
 * Sends one E1.20 controller request and collects the responder data. A
 * positive return value is the number of received bytes, zero means that no
 * line activity was detected in the response window, and a negative value is
 * -errno. Discovery responses are break-less and use the longer discovery
 * turn-around window.
 */
ssize_t dmx_transport_rdm_exchange(dmx_transport_t *transport,
                                   const uint8_t *request,
                                   size_t request_length,
                                   bool discovery_response,
                                   uint8_t *response,
                                   size_t response_capacity);

/* Used for broadcast requests, for which E1.20 forbids a response. */
int dmx_transport_rdm_send(dmx_transport_t *transport,
                           const uint8_t *request,
                           size_t request_length);

const char *dmx_transport_error(const dmx_transport_t *transport);

#ifdef __cplusplus
}
#endif

#endif
