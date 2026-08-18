# Vendored LVGL

This directory contains the source portion of upstream LVGL **v9.5.0** used
only by `project/app/dmx_panel`.  The SDK-wide LVGL 8.x component is left
unchanged so other Luckfox applications keep their existing ABI and config.

- Upstream tag: <https://github.com/lvgl/lvgl/releases/tag/v9.5.0>
- Source archive: <https://github.com/lvgl/lvgl/archive/refs/tags/v9.5.0.tar.gz>
- Downloaded archive SHA-256:
  `34a955cdf3a2d005507b704e87357af669a114523b6d3f77b5344fdc68717bc6`
- Release commit shown upstream: `85aa60d`

Only the library headers, `src/`, version/config templates, and upstream
licensing notices are vendored.  Documentation, examples, demos, CI files,
and repository metadata from the source archive are intentionally omitted.
See `LICENCE.txt` and `COPYRIGHTS.md` in this directory.
