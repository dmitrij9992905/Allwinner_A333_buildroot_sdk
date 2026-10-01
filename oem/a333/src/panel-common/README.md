# A333 panel-common

Shared static library for `dmx-panel` and `media-panel`. This directory owns
the framebuffer/G2D output, evdev input, LVGL port, canvas and font support,
as well as the copied LVGL 9.5.0 and unmodified Roboto font assets/licenses.
No application-specific controller, backend or UI code belongs here.

CMake consumers use `add_subdirectory()` and `target_link_libraries(...
panel_common)`. Public include paths and LVGL/Threads/math dependencies are
exported by the target; `panel_common_lvgl` contains the curated LVGL sources.
Options: `PANEL_COMMON_DISPLAY_ROTATION`, `PANEL_COMMON_G2D`, and
`PANEL_COMMON_LVGL_LOGGING`, and `PANEL_COMMON_PERF_MONITOR` (FPS/CPU overlay,
default ON). Media forwards its corresponding options to these.
`PANEL_COMMON_KERNEL_UAPI_DIR` optionally selects exported firmware-kernel
headers for development; the SDK dev runner sets it automatically.

The DMX Makefile includes `common.mk`, which builds `libpanel_common.a` and
`liblvgl.a` in the application's own build directory. No objects are stored in
this source directory, and different profiles do not share compiled archives.

The library has no dependency on DMX headers. `panel_lvgl_font_set_t` describes
font pointers; the DMX UI retains its own alias for that type. Orientation is
selected with the application-neutral `PANEL_DISPLAY_ROTATION` definition.
Runtime G2D/fallback/profiling behavior is unchanged.
Display pixels use XRGB8888 (`LV_COLOR_DEPTH=32`); transparent images can use
ARGB8888. DMX Make can disable the FPS overlay with `PERF_MONITOR=0`.
