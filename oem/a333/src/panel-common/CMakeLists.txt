cmake_minimum_required(VERSION 3.16)
project(panel_common LANGUAGES C ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(PANEL_COMMON_DISPLAY_ROTATION "90" CACHE STRING "Display rotation: 0, 90, 180, 270")
option(PANEL_COMMON_G2D "Allwinner DMA-BUF G2D output with software fallback" ON)
option(PANEL_COMMON_LVGL_LOGGING "LVGL warning/error diagnostics" OFF)
option(PANEL_COMMON_PERF_MONITOR "Show LVGL FPS/CPU overlay" ON)
set(PANEL_COMMON_KERNEL_UAPI_DIR "" CACHE PATH "Optional exported firmware kernel UAPI for development")
if(NOT PANEL_COMMON_DISPLAY_ROTATION MATCHES "^(0|90|180|270)$")
    message(FATAL_ERROR "PANEL_COMMON_DISPLAY_ROTATION must be 0, 90, 180 or 270")
endif()

set(LVGL_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/vendor/lvgl")
if(NOT EXISTS "${LVGL_ROOT}/lv_version.h")
    message(FATAL_ERROR "Copied LVGL sources not found: ${LVGL_ROOT}")
endif()
file(READ "${LVGL_ROOT}/lv_version.h" lvgl_version)
foreach(pair "MAJOR;9" "MINOR;5" "PATCH;0")
    list(GET pair 0 component)
    list(GET pair 1 expected)
    if(NOT lvgl_version MATCHES "#define LVGL_VERSION_${component}[ \t]+${expected}[\r\n]")
        message(FATAL_ERROR "panel-common requires the copied LVGL 9.5.0")
    endif()
endforeach()
set(font "${CMAKE_CURRENT_SOURCE_DIR}/assets/fonts/Roboto-Regular.ttf")
file(SHA256 "${font}" font_sha256)
if(NOT font_sha256 STREQUAL "797e35f7f5d6020a5c6ea13b42ecd668bcfb3bbc4baa0e74773527e5b6cb3174")
    message(FATAL_ERROR "Embedded Roboto font checksum mismatch")
endif()
foreach(notice LICENSE.txt NOTICE.txt)
    if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/assets/fonts/${notice}")
        message(FATAL_ERROR "Missing Roboto ${notice}")
    endif()
endforeach()

set(PANEL_COMMON_LVGL_LOGGING_VALUE 0)
if(PANEL_COMMON_LVGL_LOGGING)
    set(PANEL_COMMON_LVGL_LOGGING_VALUE 1)
endif()
configure_file(cmake/lv_conf.h.in generated/lv_conf.h @ONLY)
configure_file(include/lv_conf.h generated/panel_common_base_lv_conf.h COPYONLY)
add_library(panel_common_config INTERFACE)
target_include_directories(panel_common_config INTERFACE
    "${CMAKE_CURRENT_BINARY_DIR}/generated" "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${LVGL_ROOT}" "${CMAKE_CURRENT_SOURCE_DIR}/vendor")
target_compile_definitions(panel_common_config INTERFACE _GNU_SOURCE
    LV_CONF_INCLUDE_SIMPLE=1 LV_DISABLE_API_MAPPING
    PANEL_LVGL_PERF_MONITOR=$<BOOL:${PANEL_COMMON_PERF_MONITOR}>
    PANEL_DISPLAY_ROTATION=${PANEL_COMMON_DISPLAY_ROTATION})
if(PANEL_COMMON_KERNEL_UAPI_DIR)
    if(NOT EXISTS "${PANEL_COMMON_KERNEL_UAPI_DIR}/linux/types.h")
        message(FATAL_ERROR "Export kernel UAPI with scripts/kernel-vscode.sh first")
    endif()
    target_include_directories(panel_common_config SYSTEM BEFORE INTERFACE
        "${PANEL_COMMON_KERNEL_UAPI_DIR}")
endif()

# Curated shared software renderer, TinyTTF and widget sources.
set(lvgl_sources)
foreach(directory core display draw/convert draw/sw font indev misc osal stdlib
        tick layouts/flex libs/bin_decoder libs/tiny_ttf themes/default
        debugging/sysmon
        widgets/bar widgets/button widgets/buttonmatrix widgets/canvas
        widgets/image widgets/label widgets/list widgets/slider widgets/spinbox
        widgets/switch widgets/tabview widgets/textarea)
    file(GLOB_RECURSE sources CONFIGURE_DEPENDS "${LVGL_ROOT}/src/${directory}/*.c")
    list(APPEND lvgl_sources ${sources})
endforeach()
foreach(directory "" draw layouts themes)
    file(GLOB sources CONFIGURE_DEPENDS "${LVGL_ROOT}/src/${directory}/*.c")
    list(APPEND lvgl_sources ${sources})
endforeach()
list(REMOVE_DUPLICATES lvgl_sources)
add_library(panel_common_lvgl STATIC ${lvgl_sources})
target_link_libraries(panel_common_lvgl PUBLIC panel_common_config)
target_compile_options(panel_common_lvgl PRIVATE -Wno-unused-function -Wno-unused-parameter)

find_package(Threads REQUIRED)
add_library(panel_common STATIC
    src/lvgl_port.c src/panel_fbdev.c src/panel_g2d.c src/panel_input.c
    src/panel_canvas.c src/lvgl_fonts.c src/roboto_font_data.S)
target_compile_definitions(panel_common PRIVATE
    PANEL_ENABLE_G2D=$<BOOL:${PANEL_COMMON_G2D}>
    PANEL_ROBOTO_TTF_PATH="${font}")
set_property(SOURCE src/roboto_font_data.S APPEND PROPERTY OBJECT_DEPENDS "${font}")
target_compile_options(panel_common PRIVATE $<$<COMPILE_LANGUAGE:C>:-Wall;-Wextra>)
target_link_libraries(panel_common PUBLIC panel_common_lvgl Threads::Threads m)
