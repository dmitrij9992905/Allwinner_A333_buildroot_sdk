# Allwinner A333 HelperBoard

This directory contains the board-specific layer for the Buildroot build.
The vendor kernel, BSP, device-tree sources and U-Boot are kept under
`vendor/allwinner-a333/`; this layer only contains project-owned configuration
and integration files.

## What is reused from the Luckfox SDK

The Luckfox SDK is a useful reference for the build flow:

- one board defconfig;
- board overlays and post-image scripts;
- two copies of boot/rootfs artifacts;
- a signed RAUC bundle generated on the build host;
- a bootloader-specific A/B selection mechanism.

Paths written as `/home/...` in the Luckfox notes are container paths. When
the SDK root is mounted as `/home`, they correspond to `sdk/...` on the host:
`/home/sysdrv` → `sdk/sysdrv`, `/home/project` → `sdk/project`, and
`/home/build.sh` → `sdk/build.sh`.

Its Rockchip-specific changes in `fit.c`, `spl_boot_image.c` and
`boot_part` are not copied. The Allwinner U-Boot source already has the
corresponding path: `SUNXI_SWITCH_SYSTEM` selects `boot_partition` and
`root_partition`, while the normal boot command reads the selected partition.

## A/B design for A333

The initial Allwinner layout uses the following logical names:

| Slot | boot | root filesystem |
|---|---|---|
| A | `bootA` | `rootfsA` |
| B | `bootB` | `rootfsB` |

The capital `A`/`B` suffix is intentional. It avoids enabling the unrelated
Android `boot_a`/`boot_b` detection path when the target is Buildroot/Linux.

U-Boot state is stored redundantly in `env` and `env-redund`. Boot selection
uses `systemAB_next`, `systemAB_now`, `systemAB_damage`, `systemA`, `systemB`,
`rootfsA`, and `rootfsB`. `bootcount`/`bootlimit` provide the failed-boot
fallback.

The partition sizes in `sys_partition-ab.fex` are a starting point for a 16 GiB
device, not a final production layout. They must be checked against the actual
eMMC capacity and the board vendor's flashing tool before use.

## RAUC status

RAUC is intentionally not enabled by this directory yet. The standard RAUC
U-Boot backend expects its own `BOOT_ORDER`/`BOOT_<slot>_LEFT` environment
protocol, while the A333 vendor U-Boot uses `systemAB_*`. The remaining glue
must atomically do the following after a successful inactive-slot write:

1. set `systemAB_next` to the newly written slot;
2. reset its boot counter and reboot;
3. mark the slot healthy from userspace after the health check;
4. keep the previous slot bootable as rollback.

This is the A333 equivalent of Luckfox's U-Boot `boot_part` changes and is kept
separate from the generic Buildroot image generation.

## Display

The vendor SDK has MIPI examples for 480x800 and 1200x1920, but no verified
1280x800 panel description. The final DTS must be selected only after the
panel controller, lane count, pixel format, timings and reset/backlight GPIOs
are confirmed for the HelperBoard.
