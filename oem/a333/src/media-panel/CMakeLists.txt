cmake_minimum_required(VERSION 3.16)
project(media_panel VERSION 1.0.0 LANGUAGES C CXX ASM)

# Buildroot's generated toolchain deliberately defaults Debug CFLAGS to empty.
if(CMAKE_C_FLAGS_DEBUG STREQUAL "")
    set(CMAKE_C_FLAGS_DEBUG "-Og -g3" CACHE STRING "Debug CFLAGS" FORCE)
endif()
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# Older development caches pointed at the DMX application instead of a
# standalone library. Migrate only that known old default, not custom paths.
if(MEDIA_PANEL_COMMON_DIR STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}/../dmx-panel")
    unset(MEDIA_PANEL_COMMON_DIR CACHE)
    unset(MEDIA_PANEL_COMMON_DIR)
endif()
set(MEDIA_PANEL_COMMON_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../panel-common" CACHE PATH
    "Shared A333 display/input/LVGL library")
set(MEDIA_PANEL_DISPLAY_ROTATION "90" CACHE STRING "Display rotation: 0, 90, 180, 270")
set_property(CACHE MEDIA_PANEL_DISPLAY_ROTATION PROPERTY STRINGS 0 90 180 270)
option(MEDIA_PANEL_LOGGING "Application lifecycle/control diagnostics on stderr" ON)
option(MEDIA_PANEL_LVGL_LOGGING "LVGL warning/error diagnostics" OFF)
option(MEDIA_PANEL_G2D "Allwinner DMA-BUF G2D output with software fallback" ON)
option(MEDIA_PANEL_PERF_MONITOR "Show LVGL FPS/CPU overlay" ON)
if(NOT MEDIA_PANEL_DISPLAY_ROTATION MATCHES "^(0|90|180|270)$")
    message(FATAL_ERROR "MEDIA_PANEL_DISPLAY_ROTATION must be 0, 90, 180 or 270")
endif()

set(PANEL_COMMON_DISPLAY_ROTATION "${MEDIA_PANEL_DISPLAY_ROTATION}")
set(PANEL_COMMON_G2D "${MEDIA_PANEL_G2D}")
set(PANEL_COMMON_LVGL_LOGGING "${MEDIA_PANEL_LVGL_LOGGING}")
set(PANEL_COMMON_PERF_MONITOR "${MEDIA_PANEL_PERF_MONITOR}")
set(PANEL_COMMON_EEZ_FONTS ON)
set(PANEL_COMMON_EEZ_WIDGETS ON)
set(PANEL_COMMON_ARTWORK ON)
set(PANEL_COMMON_FREETYPE ON)
add_subdirectory("${MEDIA_PANEL_COMMON_DIR}" panel-common)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
add_library(media_panel_eez STATIC ui/eez-flow.cpp)
target_link_libraries(media_panel_eez PUBLIC panel_common)
target_include_directories(media_panel_eez PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/ui")
target_compile_options(media_panel_eez PUBLIC -ULV_DISABLE_API_MAPPING)
file(GLOB ui_sources CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/ui/*.c")
# Studio keeps bitmap fonts for its preview. Firmware uses small compatibility
# descriptors backed by FreeType instead; no generated glyph tables are linked.
list(FILTER ui_sources EXCLUDE REGEX "/ui_font_[^/]+\\.c$")
include(cmake/eez-freetype.cmake)
# Only the native variable table requires C++; widgets/images remain C.
# Studio 0.29 stores function pointers in void* without an explicit cast.
set_source_files_properties(ui/ui.c PROPERTIES LANGUAGE CXX COMPILE_OPTIONS -fpermissive)
add_library(media_panel_ui STATIC media_backend.c media_ui.cpp media_transition.cpp
    media_fonts.c "${CMAKE_CURRENT_BINARY_DIR}/eez_freetype_fonts.c" ${ui_sources})
set(MEDIA_PANEL_FONT_PATH "/oem/usr/share/media-panel/fonts/SkodaPro-Medium.ttf"
    CACHE STRING "Installed runtime Skoda TTF path")
target_compile_definitions(media_panel_ui PRIVATE MEDIA_PANEL_FONT_PATH="${MEDIA_PANEL_FONT_PATH}"
    MEDIA_PANEL_FONT_SIZES=${MEDIA_PANEL_FONT_SIZES}
    MEDIA_PANEL_FONT_MAX_SIZE=${MEDIA_PANEL_FONT_MAX_SIZE})
target_compile_definitions(media_panel_ui PRIVATE
    MEDIA_PANEL_LOGGING=$<BOOL:${MEDIA_PANEL_LOGGING}>)
target_compile_options(media_panel_ui PRIVATE -Wall -Wextra)
target_link_libraries(media_panel_ui PUBLIC media_panel_eez)
target_include_directories(media_panel_ui PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
add_executable(media-panel media_panel.c)
target_compile_definitions(media-panel PRIVATE
    MEDIA_PANEL_LOGGING=$<BOOL:${MEDIA_PANEL_LOGGING}>)
target_compile_options(media-panel PRIVATE -Wall -Wextra)
target_link_libraries(media-panel PRIVATE media_panel_ui)
install(TARGETS media-panel RUNTIME DESTINATION bin)
install(FILES assets/fonts/SkodaPro-Medium.ttf DESTINATION share/media-panel/fonts)

option(MEDIA_PANEL_BUILD_TESTS "Headless EEZ Flow/UI smoke tests (host only)" OFF)
if(MEDIA_PANEL_BUILD_TESTS)
    if(CMAKE_CROSSCOMPILING)
        message(FATAL_ERROR "Run MEDIA_PANEL_BUILD_TESTS with a native host compiler")
    endif()
    enable_testing()
    add_executable(media-panel-ui-test tests/ui_smoke.cpp)
    target_link_libraries(media-panel-ui-test PRIVATE media_panel_ui)
    target_link_options(media-panel-ui-test PRIVATE -Wl,--wrap=media_backend_poll
        -Wl,--wrap=media_backend_request)
    target_compile_options(media-panel-ui-test PRIVATE -UNDEBUG)
    add_test(NAME media-panel-ui COMMAND media-panel-ui-test)
    set_tests_properties(media-panel-ui PROPERTIES ENVIRONMENT
        "A333_MEDIA_FONT=${CMAKE_CURRENT_SOURCE_DIR}/assets/fonts/SkodaPro-Medium.ttf")
endif()
