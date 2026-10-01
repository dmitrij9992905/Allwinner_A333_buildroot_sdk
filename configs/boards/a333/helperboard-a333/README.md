# Allwinner A333 HelperBoard

This directory contains the board-specific layer for the Buildroot build.
The vendor kernel, BSP, device-tree sources and U-Boot are kept under
`vendor/allwinner-a333/`; this layer only contains project-owned configuration
and integration files.

## U-Boot toolchain

The Allwinner vendor U-Boot is a 32-bit ARMv7 build, although it boots the
AArch64 A333 Linux kernel. The project therefore uses the original Allwinner
SDK's `gcc-linaro-7.2.1-2017.11-x86_64_arm-linux-gnueabi` toolchain from the
ignored project-local `toolchains/` directory for U-Boot only. The Buildroot
target toolchain remains AArch64/glibc.

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

| Slot | boot | root filesystem | OEM filesystem |
|---|---|---|---|
| A | `bootA` | `rootfsA` | `oemA` |
| B | `bootB` | `rootfsB` | `oemB` |

The capital `A`/`B` suffix is intentional. It avoids enabling the unrelated
Android `boot_a`/`boot_b` detection path when the target is Buildroot/Linux.

U-Boot uses a single unflagged 128 KiB environment in `env` (`p2`);
`env-redund` (`p3`) is an initial backup, not live redundancy. Boot selection
uses `systemAB_next`, `systemAB_now`, `systemAB_damage`, `systemA`, `systemB`,
`rootfsA`, `rootfsB`, `oemdevA`, and `oemdevB`. `bootcount`/`bootlimit` provide
the basis for failed-boot fallback, which still requires a separate failure
test before relying on unattended rollback.

The base OS is built by Buildroot into `rootfs.ext4`. The post-image script
duplicates it into `rootfsA.ext4`/`rootfsB.ext4` and creates a separate OEM
filesystem from `oem/a333/rootfs`, producing `oemA.ext4`/`oemB.ext4`.
The base OS uses `systemd` and `glibc`; OEM is mounted by the systemd unit from
the root filesystem.

The partition sizes in `sys_partition-ab.fex` are a starting point for a 16 GiB
device, not a final production layout. They must be checked against the actual
eMMC capacity and the board vendor's flashing tool before use.

## RAUC status

RAUC is enabled as a Buildroot package and uses a custom bootloader backend in
`/usr/lib/rauc/a333-bootloader.sh`. The backend maps RAUC's slot operations to
the Allwinner U-Boot variables `systemAB_next`, `systemAB_damage` and
`bootcount`. `/etc/fw_env.config` contains only `/dev/mmcblk0p2 0x0 0x20000`,
matching the non-redundant U-Boot build. Adding p3 would change the assumed
header/CRC and is incorrect for this build.

The post-image step first creates `boot.img`, then a signed development bundle
containing `boot`, `rootfs` and `oem` slot classes. Boot and OEM are children of
the rootfs slot, so kernel/DTB, OS and application switch together. Shared RAUC
status lives in `/userdata/var/lib/rauc`. The development key is generated under the
ignored project-local `keys/` directory. For production, set
`A333_RAUC_KEY` and `A333_RAUC_CERT` to the controlled signing credentials.

The env partition names and the custom backend must be checked on the actual
board before enabling unattended updates. The initial boot and rollback test
should verify: install to the inactive slot, reboot, health check, mark good,
and force a failed boot to confirm U-Boot returns to the previous slot.

## Display

The HelperBoard panel has a native 800x1280 MIPI raster. A single Buildroot
choice under `Allwinner A333 project -> Display orientation` controls the
boot logo, the dmx-panel framebuffer presentation and touchscreen coordinate
mapping. The default is `Landscape, 90 degrees clockwise`, which exposes a
logical 1280x800 dmx-panel canvas without stretching it onto the portrait
framebuffer.

The selected values are also installed as `/etc/a333/display.conf`. Changing
the orientation requires rebuilding the complete image because the vendor
U-Boot does not rotate its boot logo at runtime; the packing step pre-rotates
the BMP using the same setting.
