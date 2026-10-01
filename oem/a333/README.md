# A333 OEM application layer

Business-logic applications are kept here and packed into the separate A/B OEM
filesystem. `profiles/*.conf` selects an ordered `OEM_OVERLAYS` list; the common
layer is followed by a profile-specific layer. Files from later layers take
precedence. The rootfs contains the matching systemd integration and runtime
libraries; application executables are installed under `/oem/usr/bin`.

The DMX application is `src/dmx-panel`, ported from the Luckfox Pico Panel86
application. Its Buildroot package builds the LVGL application for 1280x800,
and `post-build.sh` moves the executable and version file into the OEM image.
The media profile similarly stages its panel executable and metadata bridge.
