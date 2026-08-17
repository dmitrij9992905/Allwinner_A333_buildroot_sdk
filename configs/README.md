# Project configuration layer

This is a `BR2_EXTERNAL` tree, separate from the Buildroot source in
`../buildroot/`.

- `configs/` — Buildroot defconfigs;
- `boards/a333/` — board configuration, U-Boot fragments and vendor patches;
- `../overlays/` — target root filesystem overlays;
- `../oem/` — business applications installed into a dedicated OEM partition.

The Buildroot source tree remains vendor-neutral. Project-specific files must
not be added to `buildroot/board` or `buildroot/configs`.
