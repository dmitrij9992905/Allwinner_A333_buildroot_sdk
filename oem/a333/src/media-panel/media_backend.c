#define _POSIX_C_SOURCE 200809L
#include "media_backend.h"

#include <errno.h>
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
    char lines[6][1024];
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
    fclose(file);
    if (!complete)
        return false;
    char *end;
    errno = 0;
    unsigned long sequence = strtoul(lines[5], &end, 10);
    if (errno || !lines[5][0] || *end || lines[5][0] == '-')
        return false;
    snprintf(state->source, sizeof(state->source), "%s", lines[0]);
    snprintf(state->title, sizeof(state->title), "%s", lines[1]);
    snprintf(state->artist, sizeof(state->artist), "%s", lines[2]);
    snprintf(state->album, sizeof(state->album), "%s", lines[3]);
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
    const char *allowed[] = {"previous", "toggle", "next", "radio", "music", "pair"};
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
