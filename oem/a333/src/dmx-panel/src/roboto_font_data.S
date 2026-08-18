/* Keep the original Roboto TTF byte-for-byte in the application ELF. */
    .section .rodata.dmx_roboto_regular,"a"
    .balign 4
    .global dmx_roboto_regular_ttf
#if defined(__arm__) || defined(__aarch64__)
    .type dmx_roboto_regular_ttf, %object
#else
    .type dmx_roboto_regular_ttf, @object
#endif
dmx_roboto_regular_ttf:
    .incbin DMX_ROBOTO_TTF_PATH
dmx_roboto_regular_ttf_end:
    .global dmx_roboto_regular_ttf_end
    .size dmx_roboto_regular_ttf, . - dmx_roboto_regular_ttf

#if defined(__arm__) || defined(__aarch64__)
    .section .note.GNU-stack,"",%progbits
#else
    .section .note.GNU-stack,"",@progbits
#endif
