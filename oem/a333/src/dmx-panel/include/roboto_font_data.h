#ifndef DMX_PANEL_ROBOTO_FONT_DATA_H
#define DMX_PANEL_ROBOTO_FONT_DATA_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Linker symbols emitted by src/roboto_font_data.S. */
extern const uint8_t dmx_roboto_regular_ttf[];
extern const uint8_t dmx_roboto_regular_ttf_end[];

static inline size_t dmx_roboto_regular_ttf_size(void)
{
    return (size_t)((uintptr_t)dmx_roboto_regular_ttf_end -
                    (uintptr_t)dmx_roboto_regular_ttf);
}

#ifdef __cplusplus
}
#endif

#endif
