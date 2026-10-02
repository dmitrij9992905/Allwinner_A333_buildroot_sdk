#ifndef A333_MEDIA_BACKEND_H
#define A333_MEDIA_BACKEND_H

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    /* Backend limits metadata to 240 Unicode characters, not bytes. */
    char source[1024], title[1024], artist[1024], album[1024], volume[1024];
    unsigned long volume_sequence;
    double volume_until;
    bool volume_visible;
} media_backend_state_t;

void media_backend_init(media_backend_state_t *state);
/* Reads the backend's atomic six-line snapshot; never edits media files. */
bool media_backend_poll(media_backend_state_t *state, const char *path, double now);
bool media_backend_send(const char *socket_path, const char *command);
bool media_backend_request(const char *socket_path, const char *request);

#ifdef __cplusplus
}
#endif
#endif
