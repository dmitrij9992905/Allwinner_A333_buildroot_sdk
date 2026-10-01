#ifndef PANEL_COMMON_ROBOTO_FONT_DATA_H
#define PANEL_COMMON_ROBOTO_FONT_DATA_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Linker symbols emitted by src/roboto_font_data.S. */
extern const uint8_t panel_roboto_regular_ttf[];
extern const uint8_t panel_roboto_regular_ttf_end[];

static inline size_t panel_roboto_regular_ttf_size(void)
{
    return (size_t)((uintptr_t)panel_roboto_regular_ttf_end -
                    (uintptr_t)panel_roboto_regular_ttf);
}

#ifdef __cplusplus
}
#endif

#endif
