/* Keep the original Roboto TTF byte-for-byte in the application ELF. */
    .section .rodata.panel_roboto_regular,"a"
    .balign 4
    .global panel_roboto_regular_ttf
#if defined(__arm__) || defined(__aarch64__)
    .type panel_roboto_regular_ttf, %object
#else
    .type panel_roboto_regular_ttf, @object
#endif
panel_roboto_regular_ttf:
    .incbin PANEL_ROBOTO_TTF_PATH
panel_roboto_regular_ttf_end:
    .global panel_roboto_regular_ttf_end
    .size panel_roboto_regular_ttf, . - panel_roboto_regular_ttf

#if defined(__arm__) || defined(__aarch64__)
    .section .note.GNU-stack,"",%progbits
#else
    .section .note.GNU-stack,"",@progbits
#endif
