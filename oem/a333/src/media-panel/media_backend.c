#define _POSIX_C_SOURCE 200809L
#include "media_backend.h"

#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

void media_backend_init(media_backend_state_t *state)
{
    memset(state, 0, sizeof(*state));
    snprintf(state->source, sizeof(state->source), "%s", "Music / radio");
    snprintf(state->title, sizeof(state->title), "%s", "Waiting for player...");
}

bool media_backend_poll(media_backend_state_t *state, const char *path, double now)
{
    char lines[13][1024] = {{0}};
    state->volume_visible = state->volume[0] && now < state->volume_until;
    FILE *file = fopen(path, "r");
    if (!file)
        return false;
    bool complete = true;
    for (size_t i = 0; i < 6; ++i) {
        if (!fgets(lines[i], sizeof(lines[i]), file) || !strchr(lines[i], '\n')) {
            complete = false;
            break;
        }
        lines[i][strcspn(lines[i], "\r\n")] = '\0';
    }
    // App-only deployments must keep working with the older backend.
    size_t count = 6;
    while (complete && count < 13 && fgets(lines[count], sizeof(lines[count]), file)) {
        if (!strchr(lines[count], '\n')) complete = false;
        lines[count][strcspn(lines[count], "\r\n")] = '\0';
        ++count;
    }
    if (count > 7 && count != 13) complete = false;
    if (ferror(file)) complete = false;
    fclose(file);
    if (!complete)
        return false;
    char *end;
    errno = 0;
    unsigned long sequence = strtoul(lines[5], &end, 10);
    if (errno || !lines[5][0] || *end || lines[5][0] == '-')
        return false;
    double position = 0, duration = 0;
    if (count == 13) {
        position = strtod(lines[7], &end);
        if (!lines[7][0] || *end || !isfinite(position) || position < 0 || position > 8640000)
            return false;
        duration = strtod(lines[8], &end);
        if (!lines[8][0] || *end || !isfinite(duration) || duration < 0 || duration > 8640000)
            return false;
        if (strcmp(lines[9], "0") && strcmp(lines[9], "1")) return false;
        if (strlen(lines[10]) >= sizeof(state->track_id)) return false;
        if (lines[10][strspn(lines[10], "0123456789")]) return false;
        if (lines[11][0] && strncmp(lines[11], "/run/a333-media/covers/", 23)) return false;
    }
    snprintf(state->source, sizeof(state->source), "%s", lines[0]);
    snprintf(state->title, sizeof(state->title), "%s", lines[1]);
    snprintf(state->artist, sizeof(state->artist), "%s", lines[2]);
    snprintf(state->album, sizeof(state->album), "%s", lines[3]);
    snprintf(state->filename, sizeof(state->filename), "%s", lines[6]);
    state->position = position;
    state->duration = duration;
    state->seekable = count == 13 && !strcmp(lines[9], "1") && duration > 0 && lines[10][0];
    state->usb_available = lines[12][0] == '1';
    state->sd_available = lines[12][0] && lines[12][1] == ' ' && lines[12][2] == '1';
    snprintf(state->track_id, sizeof(state->track_id), "%.63s", lines[10]);
    snprintf(state->cover_path, sizeof(state->cover_path), "%s", lines[11]);
    if (lines[4][0] && sequence != state->volume_sequence) {
        state->volume_until = now + 5.0;
        snprintf(state->volume, sizeof(state->volume), "%s", lines[4]);
    }
    state->volume_sequence = sequence;
    state->volume_visible = state->volume[0] && now < state->volume_until;
    return true;
}

bool media_backend_send(const char *socket_path, const char *command)
{
    const char *allowed[] = {"previous", "toggle", "next", "radio", "music", "pair", "usb", "sd"};
    bool known = false;
    for (size_t i = 0; i < sizeof(allowed) / sizeof(allowed[0]); ++i)
        known |= strcmp(command, allowed[i]) == 0;
    if (!known) {
        errno = EINVAL;
        return false;
    }
    return media_backend_request(socket_path, command);
}

bool media_backend_request(const char *socket_path, const char *command)
{
    struct sockaddr_un address = {.sun_family = AF_UNIX};
    if (strlen(socket_path) >= sizeof(address.sun_path) || strlen(command) >= 4096) {
        errno = EINVAL;
        return false;
    }
    int sock = socket(AF_UNIX, SOCK_DGRAM | SOCK_CLOEXEC, 0);
    if (sock < 0)
        return false;
    snprintf(address.sun_path, sizeof(address.sun_path), "%s", socket_path);
    ssize_t sent = sendto(sock, command, strlen(command), MSG_DONTWAIT,
                          (struct sockaddr *)&address, sizeof(address));
    int saved_errno = errno;
    close(sock);
    errno = saved_errno;
    return sent == (ssize_t)strlen(command);
}
