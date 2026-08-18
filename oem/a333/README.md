# A333 OEM application layer

Business-logic applications are kept here and packed into the separate A/B OEM
filesystem. The rootfs contains only the systemd integration and the runtime
libraries; the demo executable is installed under `/oem/usr/bin`.

The current demo is `src/dmx-panel`, ported from the Luckfox Pico Panel86
application. Its Buildroot package builds the LVGL application for 1280x800,
and `post-build.sh` moves the executable and version file into this OEM tree.
The default service starts it with `--simulate`, so no RS485 device is opened.
