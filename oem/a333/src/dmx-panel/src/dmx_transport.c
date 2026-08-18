#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#define _POSIX_C_SOURCE 200809L

#include "dmx_transport.h"

#include <errno.h>
#include <fcntl.h>
#include <linux/serial.h>
#include <poll.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

#ifndef BOTHER
#define BOTHER 0010000
#endif
#ifndef CBAUD
#define CBAUD 0010017
#endif

#define DMX_BAUD_RATE 250000u
#define DMX_BREAK_US 100u
#define DMX_MAB_US 16u
#define RDM_BREAK_US 200u
#define RDM_MAB_US 16u
#define RDM_FIRST_BYTE_TIMEOUT_US 3500u
#define RDM_DISCOVERY_WINDOW_US 5800u
#define RDM_RESPONSE_WINDOW_US 35000u
#define RDM_INTERSLOT_TIMEOUT_US 2500u

/* Linux termios2 is not exposed by libc termios.h. Keep its ABI local so the
 * normal POSIX termios API can still be used in this file. RV1106 uses the
 * asm-generic 32-bit layout (NCCS == 19).
 */
struct dmx_linux_termios2 {
    uint32_t c_iflag;
    uint32_t c_oflag;
    uint32_t c_cflag;
    uint32_t c_lflag;
    uint8_t c_line;
    uint8_t c_cc[19];
    uint32_t c_ispeed;
    uint32_t c_ospeed;
};

#define DMX_TCGETS2 _IOR('T', 0x2a, struct dmx_linux_termios2)
#define DMX_TCSETS2 _IOW('T', 0x2b, struct dmx_linux_termios2)

static void set_error(dmx_transport_t *transport, const char *format, ...)
{
    va_list arguments;

    if (transport == NULL)
        return;
    va_start(arguments, format);
    (void)vsnprintf(transport->error,
                    sizeof(transport->error),
                    format,
                    arguments);
    va_end(arguments);
}

static uint64_t monotonic_us(void)
{
    struct timespec now;

    if (clock_gettime(CLOCK_MONOTONIC, &now) != 0)
        return 0;
    return ((uint64_t)now.tv_sec * UINT64_C(1000000)) +
           ((uint64_t)now.tv_nsec / UINT64_C(1000));
}

/* The short E1.20 BREAK and MAB intervals cannot be represented reliably by
 * a scheduler tick. A monotonic spin keeps the usual case precise; preemption
 * is still detected by measuring the resulting BREAK below.
 */
static void spin_us(unsigned int duration_us)
{
    uint64_t deadline = monotonic_us() + duration_us;

    while (monotonic_us() < deadline) {
    }
}

static void sleep_us(unsigned int duration_us)
{
    struct timespec duration;

    duration.tv_sec = (time_t)(duration_us / 1000000u);
    duration.tv_nsec = (long)((duration_us % 1000000u) * 1000u);
    while (nanosleep(&duration, &duration) != 0 && errno == EINTR) {
    }
}

static int set_custom_baud(dmx_transport_t *transport, unsigned int baud)
{
    struct dmx_linux_termios2 settings;

    if (ioctl(transport->fd, DMX_TCGETS2, &settings) != 0) {
        int saved_errno = errno;
        set_error(transport, "TCGETS2 failed: %s", strerror(saved_errno));
        return -saved_errno;
    }
    settings.c_cflag &= (uint32_t)~CBAUD;
    settings.c_cflag |= BOTHER;
    settings.c_ispeed = baud;
    settings.c_ospeed = baud;
    if (ioctl(transport->fd, DMX_TCSETS2, &settings) != 0) {
        int saved_errno = errno;
        set_error(transport, "TCSETS2(%u) failed: %s", baud, strerror(saved_errno));
        return -saved_errno;
    }
    return 0;
}

static int drain_output(dmx_transport_t *transport)
{
    int result;

    do {
        result = tcdrain(transport->fd);
    } while (result != 0 && errno == EINTR);
    if (result != 0) {
        int saved_errno = errno;
        set_error(transport, "tcdrain failed: %s", strerror(saved_errno));
        return -saved_errno;
    }
    return 0;
}

static int write_all(dmx_transport_t *transport,
                     const uint8_t *data,
                     size_t length)
{
    size_t written = 0;

    while (written < length) {
        ssize_t result = write(transport->fd, data + written, length - written);

        if (result > 0) {
            written += (size_t)result;
            continue;
        }
        if (result < 0 && errno == EINTR)
            continue;
        if (result < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
            struct pollfd descriptor = {transport->fd, POLLOUT, 0};
            int poll_result;

            do {
                poll_result = poll(&descriptor, 1, 100);
            } while (poll_result < 0 && errno == EINTR);
            if (poll_result > 0)
                continue;
            if (poll_result == 0)
                errno = ETIMEDOUT;
        }
        {
            int saved_errno = errno == 0 ? EIO : errno;
            set_error(transport, "serial write failed: %s", strerror(saved_errno));
            return -saved_errno;
        }
    }
    return 0;
}

static int emit_ioctl_break(dmx_transport_t *transport,
                            unsigned int break_us,
                            unsigned int mab_us,
                            bool rdm)
{
    uint64_t asserted_at;
    uint64_t released_at;

    /* Take the timestamp before the ioctl. This is deliberately conservative:
     * if the thread is descheduled immediately after the driver asserts
     * BREAK, that delay must be included in the RDM upper-bound warning.
     */
    asserted_at = monotonic_us();
    if (ioctl(transport->fd, TIOCSBRK) != 0) {
        int saved_errno = errno;
        set_error(transport, "TIOCSBRK failed: %s", strerror(saved_errno));
        return -saved_errno;
    }
    spin_us(break_us);
    if (ioctl(transport->fd, TIOCCBRK) != 0) {
        int saved_errno = errno;
        set_error(transport, "TIOCCBRK failed: %s", strerror(saved_errno));
        return -saved_errno;
    }
    released_at = monotonic_us();
    if (rdm && released_at - asserted_at > 352u)
        transport->timing_warning = true;
    spin_us(mab_us);
    return 0;
}

/* A zero byte at 100 kbaud is a 90 us SPACE followed by a 20 us MARK. At
 * 50 kbaud it is a 180 us SPACE followed by a 40 us MARK. This is useful on
 * UART drivers that do not implement TIOCSBRK, although ioctl BREAK is the
 * preferred mode on RV1106 because RDM also places an upper bound on MAB.
 */
static int emit_baud_break(dmx_transport_t *transport,
                           unsigned int break_us,
                           unsigned int mab_us)
{
    uint8_t zero = 0;
    unsigned int break_baud = break_us >= 176u ? 50000u : 100000u;
    int result;

    result = set_custom_baud(transport, break_baud);
    if (result != 0)
        return result;
    result = write_all(transport, &zero, 1);
    if (result == 0)
        result = drain_output(transport);
    if (set_custom_baud(transport, DMX_BAUD_RATE) != 0 && result == 0)
        result = -EIO;
    if (result == 0)
        spin_us(mab_us);
    return result;
}

static int emit_break(dmx_transport_t *transport,
                      unsigned int break_us,
                      unsigned int mab_us,
                      bool rdm)
{
    if (transport->break_mode == DMX_BREAK_BAUD)
        return emit_baud_break(transport, break_us, mab_us);
    return emit_ioctl_break(transport, break_us, mab_us, rdm);
}

static int send_packet(dmx_transport_t *transport,
                       const uint8_t *packet,
                       size_t packet_length,
                       bool rdm)
{
    int result = emit_break(transport,
                            rdm ? RDM_BREAK_US : DMX_BREAK_US,
                            rdm ? RDM_MAB_US : DMX_MAB_US,
                            rdm);

    if (result != 0)
        return result;
    result = write_all(transport, packet, packet_length);
    if (result != 0)
        return result;
    return drain_output(transport);
}

static int wait_readable(int fd, uint64_t deadline_us)
{
    for (;;) {
        uint64_t now = monotonic_us();
        uint64_t remaining;
        int timeout_ms;
        struct pollfd descriptor = {fd, POLLIN | POLLPRI, 0};
        int result;

        if (now >= deadline_us)
            return 0;
        remaining = deadline_us - now;
        timeout_ms = (int)((remaining + 999u) / 1000u);
        do {
            result = poll(&descriptor, 1, timeout_ms);
        } while (result < 0 && errno == EINTR);
        if (result <= 0)
            return result;
        if ((descriptor.revents & (POLLERR | POLLHUP | POLLNVAL)) != 0) {
            errno = EIO;
            return -1;
        }
        if ((descriptor.revents & (POLLIN | POLLPRI)) != 0)
            return 1;
    }
}

static ssize_t collect_response(dmx_transport_t *transport,
                                bool discovery,
                                uint64_t request_eop,
                                uint8_t *response,
                                size_t response_capacity)
{
    uint64_t first_deadline = request_eop +
                              (discovery ? RDM_DISCOVERY_WINDOW_US
                                         : RDM_FIRST_BYTE_TIMEOUT_US);
    uint64_t response_deadline = request_eop +
                                 (discovery ? RDM_DISCOVERY_WINDOW_US
                                            : RDM_RESPONSE_WINDOW_US);
    uint64_t idle_deadline = 0;
    size_t received = 0;

    for (;;) {
        uint64_t deadline = !discovery && idle_deadline != 0 &&
                                    idle_deadline < response_deadline
                                ? idle_deadline
                                : response_deadline;
        int ready;

        if (received == 0) {
            deadline = first_deadline;
        } else if (!discovery && deadline < first_deadline) {
            /* Bytes received before the responder turnaround may be a local
             * echo of our own request. They must not shorten the original
             * first-byte window for the real responder.
             */
            deadline = first_deadline;
        }
        ready = wait_readable(transport->fd, deadline);
        if (ready < 0) {
            int saved_errno = errno;
            set_error(transport, "serial poll failed: %s", strerror(saved_errno));
            return -saved_errno;
        }
        if (ready == 0)
            break;

        for (;;) {
            ssize_t result;

            if (received == response_capacity)
                break;
            result = read(transport->fd,
                          response + received,
                          response_capacity - received);
            if (result > 0) {
                received += (size_t)result;
                idle_deadline = monotonic_us() +
                                (discovery ? 1000u : RDM_INTERSLOT_TIMEOUT_US);
                continue;
            }
            if (result < 0 && errno == EINTR)
                continue;
            if (result < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
                int saved_errno = errno;
                set_error(transport, "serial read failed: %s", strerror(saved_errno));
                return -saved_errno;
            }
            break;
        }
        if (received == response_capacity)
            break;
    }

    if (discovery) {
        uint64_t now = monotonic_us();
        uint64_t next_request_at = request_eop + RDM_DISCOVERY_WINDOW_US;

        if (now < next_request_at)
            sleep_us((unsigned int)(next_request_at - now));
    }
    return (ssize_t)received;
}

void dmx_transport_init(dmx_transport_t *transport)
{
    if (transport == NULL)
        return;
    memset(transport, 0, sizeof(*transport));
    transport->fd = -1;
}

int dmx_transport_open(dmx_transport_t *transport,
                       const char *serial_path,
                       dmx_rs485_mode_t rs485_mode,
                       dmx_break_mode_t break_mode)
{
    struct termios settings;
    int fd;
    int result;

    if (transport == NULL || serial_path == NULL || serial_path[0] == '\0')
        return -EINVAL;
    if (rs485_mode != DMX_RS485_AUTO_DIRECTION &&
        rs485_mode != DMX_RS485_KERNEL_RTS)
        return -EINVAL;
    if (break_mode != DMX_BREAK_IOCTL && break_mode != DMX_BREAK_BAUD)
        return -EINVAL;

    dmx_transport_close(transport);
    fd = open(serial_path, O_RDWR | O_NOCTTY | O_CLOEXEC | O_NONBLOCK);
    if (fd < 0) {
        int saved_errno = errno;
        set_error(transport, "open(%s) failed: %s", serial_path, strerror(saved_errno));
        return -saved_errno;
    }
    transport->fd = fd;
    transport->rs485_mode = rs485_mode;
    transport->break_mode = break_mode;
    /* The generated stop bits, TCSETS2 latency and the additional MARK all
     * contribute to MAB in baud-break mode. Userspace cannot prove the E1.20
     * 88 us maximum on this path, so expose that limitation unconditionally.
     */
    transport->timing_warning = break_mode == DMX_BREAK_BAUD;

    if (tcgetattr(fd, &settings) != 0) {
        int saved_errno = errno;
        set_error(transport, "tcgetattr failed: %s", strerror(saved_errno));
        dmx_transport_close(transport);
        return -saved_errno;
    }
    settings.c_iflag = 0;
    settings.c_oflag = 0;
    settings.c_lflag = 0;
    settings.c_cflag &= (tcflag_t)~(CSIZE | PARENB | PARODD | CSTOPB | CRTSCTS);
    settings.c_cflag |= CS8 | CSTOPB | CREAD | CLOCAL;
    settings.c_cc[VMIN] = 0;
    settings.c_cc[VTIME] = 0;
    (void)cfsetispeed(&settings, B38400);
    (void)cfsetospeed(&settings, B38400);
    if (tcsetattr(fd, TCSANOW, &settings) != 0) {
        int saved_errno = errno;
        set_error(transport, "tcsetattr failed: %s", strerror(saved_errno));
        dmx_transport_close(transport);
        return -saved_errno;
    }
    result = set_custom_baud(transport, DMX_BAUD_RATE);
    if (result != 0) {
        dmx_transport_close(transport);
        return result;
    }
    (void)tcflush(fd, TCIOFLUSH);

    if (rs485_mode == DMX_RS485_KERNEL_RTS) {
        struct serial_rs485 rs485;

        memset(&rs485, 0, sizeof(rs485));
        rs485.flags = SER_RS485_ENABLED | SER_RS485_RTS_ON_SEND;
        if (ioctl(fd, TIOCSRS485, &rs485) != 0) {
            int saved_errno = errno;
            set_error(transport,
                      "TIOCSRS485 failed (RV1106 UART4 has no RTS/DE pin): %s",
                      strerror(saved_errno));
            dmx_transport_close(transport);
            return -saved_errno;
        }
    }

    transport->error[0] = '\0';
    return 0;
}

void dmx_transport_close(dmx_transport_t *transport)
{
    if (transport == NULL)
        return;
    if (transport->fd >= 0)
        (void)close(transport->fd);
    transport->fd = -1;
}

int dmx_transport_send_dmx(dmx_transport_t *transport,
                           const uint8_t slots[DMX_UNIVERSE_SLOTS])
{
    uint8_t frame[DMX_UNIVERSE_SLOTS + 1u];
    int result;

    if (transport == NULL || transport->fd < 0 || slots == NULL)
        return -EINVAL;
    frame[0] = 0x00;
    memcpy(frame + 1, slots, DMX_UNIVERSE_SLOTS);
    result = send_packet(transport, frame, sizeof(frame), false);
    /* Ordinary DMX512 has no responder slot. Clearing a possible local echo
     * here prevents an auto-direction transceiver from filling the tty RX
     * queue between RDM requests.
     */
    if (result == 0)
        (void)tcflush(transport->fd, TCIFLUSH);
    return result;
}

ssize_t dmx_transport_rdm_exchange(dmx_transport_t *transport,
                                   const uint8_t *request,
                                   size_t request_length,
                                   bool discovery_response,
                                   uint8_t *response,
                                   size_t response_capacity)
{
    uint64_t request_eop;
    int result;

    if (transport == NULL || transport->fd < 0 || request == NULL ||
        request_length == 0 || response == NULL || response_capacity == 0)
        return -EINVAL;
    (void)tcflush(transport->fd, TCIFLUSH);
    result = send_packet(transport, request, request_length, true);
    if (result != 0)
        return result;
    request_eop = monotonic_us();

    /* Keep any local echo in the buffer and let the protocol layer skip the
     * request. A process can be preempted after tcdrain(); flushing here could
     * otherwise erase a legal response which starts only 176 us after EOP.
     */
    return collect_response(transport,
                            discovery_response,
                            request_eop,
                            response,
                            response_capacity);
}

int dmx_transport_rdm_send(dmx_transport_t *transport,
                           const uint8_t *request,
                           size_t request_length)
{
    int result;

    if (transport == NULL || transport->fd < 0 || request == NULL ||
        request_length == 0)
        return -EINVAL;
    (void)tcflush(transport->fd, TCIFLUSH);
    result = send_packet(transport, request, request_length, true);
    if (result == 0)
        sleep_us(RDM_FIRST_BYTE_TIMEOUT_US);
    (void)tcflush(transport->fd, TCIFLUSH);
    return result;
}

const char *dmx_transport_error(const dmx_transport_t *transport)
{
    if (transport == NULL)
        return "invalid transport";
    return transport->error;
}
