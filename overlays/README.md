# Root filesystem overlays

Overlays contain target-system files that belong to the base OS: init scripts,
system configuration, users, permissions and update integration.

The current base system uses `systemd` as PID 1 and `glibc` as the C library.
The OEM mount is provided by `a333-oem-mount.service`; it reads the active
`oemdev=` kernel command-line parameter selected by U-Boot.

Business applications do not belong here. They are kept under `oem/` and are
installed into the separate `oemA`/`oemB` partition pair by the image/update
pipeline.
