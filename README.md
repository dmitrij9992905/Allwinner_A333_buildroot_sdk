# Buildroot-based Allwinner A333 Build System

[Russian version](README_RU.md)

This project provides an independent Linux build system for an Allwinner A333
module installed on a HelperBoard. Three product profiles are available:
DMX tablet, Bluetooth/media tablet, and a headless controller.

It is based on Buildroot 2025.02.16 and builds a system using `systemd`,
`glibc`, and an AArch64 target toolchain. The vendor Linux 6.6 kernel, BSP,
DTS, and U-Boot are taken from the original Allwinner SDK. The original 32-bit
Linaro toolchain is used only to build the vendor U-Boot.

The project separates the base operating system from an `OEM` partition that
contains application-specific logic. It supports an A/B partition layout with
separate rootfs and OEM slots and can produce a signed RAUC bundle for updating
an installed system.

## Project layout

```text
buildroot/                  Buildroot sources
configs/                    BR2_EXTERNAL, defconfig, and board configuration
overlays/a333/rootfs/       common rootfs layer
overlays/components/        optional display, DMX, media, headless layers
oem/                        applications and OEM partition contents
vendor/allwinner-a333/      Allwinner kernel, BSP, DTS, U-Boot, and pack SDK
scripts/                    boot.img, full image, and RAUC bundle generation
Dockerfile                  build environment container
docker-build.sh             Docker build wrapper
build.sh                    profile-aware build entry point
profiles/*.conf             defconfig, ordered overlay lists, output paths
output/                     build output, ignored by Git
dl/, ccache/, toolchains/   local caches and toolchains, ignored by Git
```

## Building

Docker Engine is required; Docker Compose is not. Run the commands below from
the repository root. The 32-bit vendor U-Boot toolchain is **not tracked by
Git**: before the first build, provide
`toolchains/gcc-linaro-7.2.1-2017.11-x86_64_arm-linux-gnueabi/` so that its
`bin/arm-linux-gnueabi-gcc` is executable. The vendor sources and packer under
`vendor/allwinner-a333/` must also be present.

### First build after cloning

```sh
./docker-build.sh build
./docker-build.sh prepare-sources
./build.sh list
./build.sh dmx configure
./build.sh dmx build
```

Replace `dmx` with any name from `./build.sh list` to build another profile. Each has a
separate `output/profiles/<profile>/` directory. `build` automatically runs
`configure` when that profile has no `.config`, but the explicit step above
makes the first-build sequence easier to inspect.

### Clean rebuild of one profile

To discard compiled packages, target rootfs and images for `dmx`, while
keeping its current `.config` (including menuconfig changes), run:

```sh
./build.sh dmx clean
./build.sh dmx build
```

To start again from the profile's defconfig instead:

```sh
./build.sh dmx clean
./build.sh dmx configure
./build.sh dmx build
```

`clean` affects only the selected profile's build output. It preserves
`dl/` (downloaded sources and prepared vendor archives), `ccache/`, the
untracked U-Boot toolchain in `toolchains/`, and signing keys in `keys/`.
Neither rebuilding the Docker image nor rerunning `prepare-sources` is needed
for an ordinary clean rebuild. Do not use Buildroot `distclean` for this
workflow: it also removes `.config` and other configuration state. If the
overlay list changes, use `clean` before building in the same output
directory, or choose a new `OUTPUT_DIR` in the profile.

Other useful commands:

```sh
./build.sh media build
./build.sh headless build
./build.sh media menuconfig
```

`profiles/*.conf` selects the defconfig, output directory, RAUC bundle name,
and ordered `ROOTFS_OVERLAYS` and `OEM_OVERLAYS` arrays. Each path is relative
to the project root. Buildroot copies rootfs layers into the base image; the
post-build script copies OEM layers into the separate A/B OEM image. Later
layers override earlier files with the same path. For example:

```sh
ROOTFS_OVERLAYS=(overlays/a333/rootfs overlays/components/display/rootfs overlays/components/media/rootfs)
OEM_OVERLAYS=(oem/a333/common-rootfs oem/a333/media-rootfs)
```

Use a separate `OUTPUT_DIR` for each distinct stack; Buildroot's existing
`target/` directory is incremental and does not delete files removed from a
rootfs overlay. `./build.sh <profile> configure` synchronizes the selected
rootfs list into `.config`. A profile can also set
`PROFILE_ENV=("NAME=value")` for additional container build variables.
Each profile uses `output/profiles/<profile>/` by default.
The legacy `output/` build can still be used directly with `docker-build.sh`.
Run `build.sh` on the host; it uses `docker-build.sh` automatically. It can
also run from `/workspace` inside an interactive `docker-build.sh` session.

### Kernel logging variants

| Normal profile | Verbose diagnostic profile |
|---|---|
| `dmx` | `dmx-kernel-debug-logs` |
| `media` | `media-kernel-debug-logs` |
| `headless` | `headless-kernel-debug-logs` |

Normal profiles disable the vendor BSP debug-log options and initcall tracing,
remove `earlyprintk`, `ignore_loglevel` and `keep_bootcon` from kernel boot
arguments, and use `loglevel=5` (warnings/errors on UART). `printk`/`dmesg`, the
serial login console and diagnostic interfaces remain available. Debug variants
preserve the previous verbose bring-up configuration and boot arguments.
This reduces serial logging overhead, but boot-time improvement must be measured
on the board; it does not eliminate unrelated probe delays.

```sh
./build.sh media build                         # normal kernel logging
./build.sh media-kernel-debug-logs build       # verbose kernel logging
```

Each variant has its own defconfig, output directory and `.raucb` name. Debug
profiles inherit their product's rootfs/OEM overlay stacks. `KERNEL_LOGGING=quiet`
or `debug` in the profile selects the matching kernel fragments and U-Boot env.
`build.sh` also synchronizes existing `.config` files and invalidates only
kernel/U-Boot configuration/build stamps when changing the logging selection,
so an old output directory does not silently retain the previous kernel config.

The quiet/debug pair retains the same product RAUC compatible ID and A/B layout.
A signed bundle hook updates the logging-related U-Boot variables after all
three slot images have been written. It leaves slot selection and partition
mapping alone, allowing OTA transitions without a full flash. The logging
policy is global U-Boot state, not per-slot state; switching slots manually does
not restore an earlier logging policy. The complete factory image also packs
the selected environment. Do not use the new bundles on an unrepaired old
environment; see the one-time RAUC migration below.

The final post-image step automatically creates `boot.img`, A/B payload
files, a RAUC bundle, and a complete vendor image using the `dragon` packer
copied from the original SDK. To repack the image without rebuilding Buildroot,
run the following command inside the container:

```sh
./docker-build.sh /workspace/scripts/pack-a333-image.sh /workspace/output/images
```

Temporary packer data is stored in `output/images/.a333-pack/` and ignored by
Git.

To change Buildroot settings:

```sh
./build.sh dmx menuconfig
```

Downloaded sources and the compiler cache are kept in the project's `dl/` and
`ccache/` directories. This speeds up subsequent builds and avoids depending
on the Ubuntu version installed on the host.

## Developing media-panel in VSCode

`oem/a333/src/media-panel/CMakeLists.txt` is the application's CMake project.
Buildroot uses the same project through `cmake-package`. Both applications
link the static `panel-common` library in `oem/a333/src/panel-common/`: fbdev/G2D,
evdev, the LVGL port and font support. The copied LVGL 9.5.0 and Roboto assets
also live there, with their license notices. DMX keeps its Makefile frontend
and consumes `panel-common/common.mk`; media uses its CMake target. Neither
application takes common sources from the other or an external SDK.

```text
oem/a333/src/
├── panel-common/  # include/, src/, vendor/lvgl/, assets/fonts/
├── dmx-panel/     # DMX/RDM controller and DMX-specific UI
└── media-panel/   # media UI and its CMake project
```

1. Build the matching `media` or `media-kernel-debug-logs` firmware once to
   obtain its AArch64/glibc toolchain and sysroot. If these already exist, do
   not rebuild the firmware merely to develop the UI.
2. Open the **repository root** in VSCode (`code .`) and install the recommended
   C/C++ extension. CMake Tools is optional; the provided tasks use Docker and
   do not require host CMake. Host Python 3 and OpenSSH `ssh`/`scp` are needed.
3. Press **Ctrl+Shift+B** to build only the application. Choose the matching
   profile. Its output is `output/dev/media-panel/<profile>/media-panel`.
4. Use **Terminal → Run Task → media-panel: run (build + deploy + logs)**.
   Select the profile and SSH target (default `root@192.168.0.144`). The
   terminal asks for the root password (`allwinner`) unless SSH keys are set up.

The run task builds incrementally, uploads to `/userdata/dev/media-panel/`,
stops only `a333-media.service` and runs the test UI in the SSH terminal with
stdout/stderr visible. MPD, Bluetooth and `a333-media-backend.service` stay up.
**Ctrl+C** exits the test and restores the stock UI if it was active before.
The script also restores it on a handled SSH disconnect; after an abrupt
failure you can always run `systemctl start a333-media.service` on the board.
It does not replace `/oem/usr/bin/media-panel`, modify A/B slots or enable a
development version on subsequent boots. Test binaries persist in userdata.

The same commands work outside VSCode:

```sh
./scripts/media-panel-dev.sh build media
./scripts/media-panel-dev.sh deploy media root@192.168.0.144
./scripts/media-panel-dev.sh run media root@192.168.0.144
# Or all three steps, with one profile/target selection:
./scripts/media-panel-dev.sh cycle media root@192.168.0.144
# Logs of the installed UI and media backend:
./scripts/media-panel-dev.sh logs media root@192.168.0.144
```

SSH aliases/ports/identity files belong in the host's `~/.ssh/config`. Use
`ssh-copy-id root@192.168.0.144` to avoid repeated password prompts. No password
is stored in VSCode settings. SCP uses `-O` so a target SFTP server is not needed.

Development builds default to `RelWithDebInfo`, application lifecycle and
control-command logging, and no GDB requirement. Set `A333_DEV_BUILD_TYPE=Debug`
or `A333_DEV_LVGL_LOGGING=ON` before opening VSCode or invoking the script if
needed. CMake options are `MEDIA_PANEL_DISPLAY_ROTATION` (0/90/180/270),
`MEDIA_PANEL_LOGGING`, `MEDIA_PANEL_LVGL_LOGGING` and `MEDIA_PANEL_G2D` (ON by
default); the script takes rotation
from the profile's `.config` (or defconfig). Application diagnostics are
independent of the kernel-debug-logs firmware variants.

For IntelliSense, select `media` or `media-kernel-debug-logs` through
**C/C++: Select a Configuration**. CMake generates `compile_commands.json`;
the script generates a host-path/space-safe `compile_commands.host.json` for
the checked-in C/C++ configuration, including the shared library sources.
Run `./scripts/media-panel-dev.sh configure media` to generate this database
without compiling. Explicit include paths/defines also cover headers and
DMX files absent from the database; `dmx` and `dmx-kernel-debug-logs` editor
configurations use their respective profile compilers. The fallback assumes
90-degree rotation; media compilation-database entries use the actual build
settings. After changing a profile's rotation, reconfigure and, for DMX,
adjust the fallback `PANEL_DISPLAY_ROTATION` define to match.
If stale red underlines remain after the move, run **C/C++: Reset IntelliSense
Database**, then **Developer: Reload Window**. Install the recommended
`ms-vscode.cpptools` extension; header files are associated with C, and semantic
highlighting is enabled. Do not edit application sources in
`output/profiles/.../build/`: these are disposable Buildroot copies.

Linux headers do not come from the host. The development runner exports ARM64
UAPI with `headers_install` from the **built kernel of the selected profile**
to `output/dev/kernel/<profile>/headers/include`. Both development compilation
and IntelliSense use these headers; libc comes from the target sysroot.
Kernel-private `include/linux` headers are not mixed into application builds.

To edit the kernel itself, after building it run:

```bash
bash scripts/kernel-vscode.sh media
```

Select **C/C++: Select a Configuration → kernel-media** in VSCode.
Equivalent `kernel-<profile>` configurations exist for all six profiles.
They use actual Kbuild commands, `.config`, generated ARM64 headers and
`bsp/include` from `output/profiles/<profile>/build/linux-custom`, with host
system headers disabled by `-nostdinc`. Refresh after rebuilding/changing the
kernel, using that command or **SDK kernel: refresh headers and IntelliSense**.
Disabled drivers may need enabling/building to obtain their exact Kbuild command.

### EEZ-Studio: color and FPS

The applications use **LVGL 9.5.0**, `LV_COLOR_DEPTH=32`, and an
`LV_COLOR_FORMAT_XRGB8888` display buffer (4 bytes per pixel, 8 bits per RGB
channel; X is unused). Choose an **LVGL 9.x** project in EEZ-Studio and **32-bit**
color depth where offered. Use **XRGB8888** for opaque images or **ARGB8888**
for images with transparency. RGB565 is only used by the existing DMX color
wheel canvas, not the overall UI. Logical UI resolution is 1280×800 at the
default 90°/270° rotation, or 800×1280 at 0°/180°. The SDK port rotates the
physical framebuffer. These are rendering settings, not MIPI
lane/pixel-format settings.

Both applications now enable LVGL's built-in FPS/CPU monitor at bottom right.
It counts LVGL refreshes, not physical panel scanout; FPS may be low on a static
screen. CPU is LVGL handler activity, not whole-system Linux CPU utilization.
Disable for development with
`A333_DEV_PERF_MONITOR=OFF ./scripts/media-panel-dev.sh build media`,
or CMake's `-DMEDIA_PANEL_PERF_MONITOR=OFF`; DMX Make uses `PERF_MONITOR=0`.

For a separately exported SDK, run `./build.sh media build sdk`, extract the
resulting `*_sdk-buildroot.tar.gz` from `output/profiles/media/images/` elsewhere
and run its `relocate-sdk.sh`. Then
`A333_SDK_DIR=/absolute/path/to/extracted-sdk ./scripts/media-panel-dev.sh build media`
uses native CMake from that SDK instead of Docker. Use a **fresh development
build directory** when changing between Docker and native SDK paths (for
example, rename `output/dev/media-panel/media` first); CMake caches absolute
paths. Set the C/C++ compiler path to the exported SDK's compiler if the
original profile toolchain is no longer present. Standalone CMake also works:

```sh
cmake -S oem/a333/src/media-panel -B output/dev/media-panel-native \
  -DCMAKE_TOOLCHAIN_FILE=/absolute/path/to/sdk/share/buildroot/toolchainfile.cmake \
  -DCMAKE_BUILD_TYPE=RelWithDebInfo -DMEDIA_PANEL_DISPLAY_ROTATION=90
cmake --build output/dev/media-panel-native --parallel
```

To ship changes in the normal OEM/RAUC image, run:

```sh
./build.sh media build media-panel-rebuild
./build.sh media build
```

The old Makefile package is automatically resynchronized/reconfigured on the
first build after the CMake migration. Development build/deploy does **not**
generate or install a firmware update.

### Allwinner G2D display output (DMX and media)

Both panels share a DMA-BUF-based Allwinner G2D output adapter. LVGL still
draws widgets in software; the G2D RCQ **rotator** replaces the expensive CPU
rotation/copy into `/dev/fb0`. It is not the NXP `LV_USE_G2D` renderer, and it
does not use Mali, Xorg, proprietary user-space libraries or `/dev/mem`.

The common Linux fragment enables `CONFIG_AW_G2D=y`, RCQ and rotation, with
DMA heaps. The adapter exports the framebuffer once via `FBIOGET_DMABUF`,
allocates a reusable DMA source, brackets CPU writes with `DMA_BUF_IOCTL_SYNC`,
and waits for the G2D completion IRQ before reporting the frame as flushed.
A sequential source copy remains; this is not a zero-copy LVGL renderer.

Runtime controls for either panel:

- `A333_PANEL_RENDERER=auto` (default): use G2D when supported, otherwise log
  the reason and keep the software display path. A failed hardware path is
  disabled for that process, avoiding repeated timeouts/log spam.
- `A333_PANEL_RENDERER=software`: force the previous output for comparison.
- `A333_PANEL_RENDERER=g2d`: strict diagnostic mode; hardware failure is
  reported instead of silently falling back. Use `auto` for normal boot.
- `A333_PANEL_PROFILE=1`: print frame presentation count, G2D frame count,
  mean/max presentation time every five seconds **when frames are submitted**.
  This includes the source copy, not LVGL drawing or end-to-end input latency;
  first-frame initialization can affect the first interval's maximum.

For the VSCode/development runner, corresponding settings are
`A333_DEV_RENDERER`, `A333_DEV_PANEL_PROFILE`, and build-time `A333_DEV_G2D=ON|OFF`:

```sh
./scripts/media-panel-dev.sh build media
./scripts/media-panel-dev.sh deploy media root@192.168.0.144
A333_DEV_RENDERER=software A333_DEV_PANEL_PROFILE=1 \
  ./scripts/media-panel-dev.sh run media root@192.168.0.144
# Exit with Ctrl-C, then compare the same animation using strict hardware output:
A333_DEV_RENDERER=g2d A333_DEV_PANEL_PROFILE=1 \
  ./scripts/media-panel-dev.sh run media root@192.168.0.144
```

The target must run the new kernel (`/dev/g2d` and `/dev/dma_heap/*`). A local
BSP patch also initializes the optional G2D power-domain list safely. For an
existing build, clean the kernel package so the new patch is applied:

```sh
./build.sh media build linux-dirclean
./build.sh media build media-panel-rebuild
./build.sh media build
```

For DMX replace `media` with `dmx` and `media-panel-rebuild` with
`dmx-panel-rebuild`. Kernel changes require the new boot image in the RAUC
bundle, not just deployment of the application binary. Headless has no panel
application and does not open G2D.

RCQ rotation preserves format and does not scale. The adapter currently
supports opaque ARGB/ABGR 32-bit framebuffers, 8-byte-aligned destination
stride, and 1:1 geometry after rotation (0/90/180/270). Other layouts use the
CPU fallback. It writes the existing visible framebuffer; page flipping and
VSync are **not** added in this first stage, so G2D alone does not guarantee
tear-free output or 60 FPS. Physical V+/V- sensing and ALSA volume handling
remain unchanged; only displaying their UI feedback may become quicker.

Supplier Ubuntu inspection (reference only, not a build dependency): in
`allwinner-a333/source/src/longan/test/dragonboard/baijie_extra`, the XFCE
overlay's `extra-xfce/etc/X11/xorg.conf` selects DRM `modesetting` with `glamor`.
The Qt overlay selects `eglfs_mali` and contains a Valhall r32p0 `libmali`.
This is distinct from G2D, and does not prove runtime GPU acceleration in
XFCE. These libraries should not be mixed blindly with another Mali kernel
driver version. The G2D UAPI and rotator in the supplier's Longan BSP match
our vendored BSP; the small ABI subset is copied into the application, with
its original Linux-syscall license notice and regression checks.

## Build artifacts

After a successful build, the files are available in:

```text
output/profiles/<profile>/images/
```

Main artifacts:

| File | Purpose |
|---|---|
| `u-boot.bin` | vendor U-Boot for initial installation |
| `board.dtb` | board device tree |
| `Image.A`, `Image.B` | kernel Image copies for A/B slots |
| `rootfsA.ext4`, `rootfsB.ext4` | base operating system for rootfs A/B slots |
| `oemA.ext4`, `oemB.ext4` | OEM A/B partitions |
| `boot.img` | kernel and DTB for the vendor packer |
| `a333-helperboard-full.img` | complete vendor image for the initial eMMC flash |
| `a333-helperboard-sys_partition.fex` | A/B partition map used by the packer |
| `a333-<profile>.raucb` | signed RAUC update bundle |

RAUC compatible IDs are distinct for the DMX, media and headless products, so
an OTA payload for one profile cannot be installed onto another by mistake.
All three retain rootfs A/B and OEM A/B slots. The OEM staging directory is
per-output (`output/profiles/<profile>/build/a333-oem-root`), preventing one
profile's application from leaking into another profile's image.

The media profile provides an A2DP sink, a BlueALSA HFP hands-free endpoint,
MPD for internet radio/local/NFS music, and a basic touch controller showing
BlueZ AVRCP track metadata. Put radio URLs in `/userdata/media/radio.m3u` and
music files under `/userdata/media/music`. Its media UI is a first integration
pass: phone cover-art transfer, HFP microphone routing, USB automount and
network-share browsing are **not yet implemented or hardware-verified**.
The **Music** button rescans `/userdata/media/music`, fills the MPD queue and
starts playback; **Pair BT** opens a two-minute pairing window when `hci0` is
available. If the adapter is absent, check the AIC Wi-Fi/SDIO power-on errors
in `dmesg` before debugging BlueZ or the media UI.
The tablet's V+/V- keys adjust the codec's left and right DAC gain for both
local playback and Bluetooth audio (when the adapter is working).
Linux reads the GPADC resistor ladder and reports `KEY_VOLUMEUP` (115) and
`KEY_VOLUMEDOWN` (114) through evdev. The media backend sets `DACL Volume`
and `DACR Volume` together in 3 dB steps over -60..0 dB, plus mute,
capped at 0 dB (register 159); the top volume
banner disappears five seconds after the last keypress. GPADC key channels
stay powered so buttons also work after the board has been idle.

`a333-audio-init.service` initializes the analog route once per boot at
moderate volume, including LINEOUT gain (values 0/1 mute the output).
To recover after manual mixer changes, run `a333-audio-init --force`.
The media profile shares a 48 kHz ALSA `plug`/`dmix` output between MPD and
Bluetooth, resampling other source rates. The codec patch removes advertised
rates such as 88.2 kHz that its hardware setup table cannot configure.
Only the profile's `bluealsad`/`bluealsa-playback` services run; upstream
duplicate BlueALSA services and MPD socket activation are masked.

AIC8800 uses the kernel's Allwinner power/rescan backend, with firmware in
`/vendor/etc/firmware`. `a333-bluetooth-uart.service` unblocks the two Bluetooth
rfkill controls and attaches `/dev/ttyS1` as H4 at 1500000 baud with RTS/CTS
after the SDIO firmware upload. NetworkManager uses the D-Bus-enabled
`wpa_supplicant.service` with EAP enabled (otherwise its D-Bus `EapMethods`
getter fails and NetworkManager cannot initialize Wi-Fi).
Check `nmcli device status`, `nmcli device wifi list ifname wlan0`, `bluetoothctl show`,
and `systemctl status a333-bluetooth-uart` after boot.

The headless profile disables the Linux DSI/LVDS/display/touch nodes in its
DTB and installs GPIO command-line tools. The vendor bootloader still uses its
own display configuration during early boot; connector pin ownership and
electrical suitability must be verified against the board schematic before
driving any MIPI/LVDS pin as GPIO.

All profiles include BLE Wi-Fi provisioning GATT. An administrator must open
the pairing window on the target with `a333-ble-pairing on`, pair a phone,
write UTF-8 JSON `{"ssid":"...","password":"..."}` to characteristic
`e591cbb7-4d2a-4a04-aa78-209eb9b4ca33`, then close with
`a333-ble-pairing off`. The link is encrypted, but headless Just Works pairing
does not provide MITM protection; use a trusted physical setup environment.
The pairing window closes automatically after five minutes.

The OEM partition currently contains the `dmx-panel` demonstration
application, ported from the Luckfox Pico Panel86 and adapted for a 1280×800
framebuffer. Its sources are located in
[`oem/a333/src/dmx-panel`](oem/a333/src/dmx-panel). After the post-build step,
the binary is installed as `/oem/usr/bin/dmx-panel`.

During boot, systemd automatically starts the demo with
`--simulate --allow-missing-input`. This mode is intended for display and UI
testing: the application uses virtual RDM devices and does not open the real
RS485/UART interface.

## Device access

Default credentials:

```text
Username: root
Password: allwinner
```

### SSH, SCP, and SFTP

The image includes an OpenSSH server with password login enabled for root. Once
the board has obtained an IP address:

```sh
ssh root@<BOARD_IP>
scp local-file root@<BOARD_IP>:/userdata/
sftp root@<BOARD_IP>
```

SSH host keys are generated in persistent `/var/lib/ssh` during the first
boot. They survive A/B updates but are regenerated after a factory reset.

### ADB shell

`adbd` is available through USB FunctionFS and TCP port 5555 at the same
time. For USB access:

```sh
output/host/bin/adb devices
output/host/bin/adb shell
```

To connect over Ethernet or Wi-Fi:

```sh
output/host/bin/adb connect <BOARD_IP>:5555
output/host/bin/adb shell
```

This version of `adbd` provides a root shell without ADB authentication.
Port 5555 must therefore only be used on a trusted network.

USB ADB uses configfs FunctionFS (`CONFIG_USB_CONFIGFS_F_FS`), not the legacy
`CONFIG_USB_FUNCTIONFS`/`g_ffs` gadget, which conflicts with `ffs.adb`. The bind
helper waits for FunctionFS ep1/ep2 and retries UDC binding; this kernel has no
configfs FunctionFS `ready` attribute.

Service status can be inspected with:

```sh
systemctl status sshd adbd NetworkManager bluetooth
journalctl -u sshd -u adbd -u NetworkManager -u bluetooth
ls /sys/class/udc
cat /sys/kernel/config/usb_gadget/a333/functions/ffs.adb/ready
```

## Network management

Ethernet and Wi-Fi are managed by NetworkManager. The legacy parallel
`systemd-networkd` and `wpa_supplicant@wlan0` services are not started.
Useful commands:

```sh
nmcli general status
nmcli device status
nmcli connection show
nmcli device wifi list
nmcli device wifi connect '<SSID>' password '<PASSWORD>' ifname wlan0
nmcli connection up '<CONNECTION_NAME>'
```

NetworkManager profiles are stored in persistent `/var/lib/NetworkManager`
and survive A/B updates. A factory reset removes them together with the rest of
the `userdata` contents.

## Bluetooth management

The image includes `bluetoothd`, `bluetoothctl`, the `btmon` diagnostic
tool, additional BlueZ tools, and `rfkill`. Example discovery and connection
sequence:

```sh
rfkill unblock bluetooth
systemctl enable --now bluetooth
bluetoothctl
power on
agent on
default-agent
scan on
pair <MAC>
trust <MAC>
connect <MAC>
```

## Storage layout and initial board flashing

The RAUC bundle is not intended for the initial installation on an empty
board. At that point there is no running Linux system, RAUC installation,
U-Boot A/B logic, or configured partition layout.

The project contains partition maps copied from the original SDK:

- [`sys_partition-vendor-dragonboard-8G.fex`](configs/boards/a333/helperboard-a333/sys_partition-vendor-dragonboard-8G.fex) — original single-slot map for 8 GB storage;
- [`sys_partition-vendor-dragonboard-16G.fex`](configs/boards/a333/helperboard-a333/sys_partition-vendor-dragonboard-16G.fex) — original single-slot map for 16 GB storage;
- [`sys_partition-ab.fex`](configs/boards/a333/helperboard-a333/sys_partition-ab.fex) — this project's A/B partition map.

In the A/B map, the partition numbers are: `p1` boot-resource, `p2` env,
`p3` env-redund, `p4/p5` bootA/bootB, `p6/p7` rootfsA/rootfsB, and
`p8/p9` oemA/oemB. Sizes in `sys_partition-ab.fex` are specified in
512-byte sectors, except for `mbr.size`, which is specified in kilobytes.

The following image must be used for the initial flash:

```text
output/images/a333-helperboard-full.img
```

This is a complete vendor image built with the original Allwinner SDK tools. It
contains the boot package, A/B partition map, env, bootA/bootB,
rootfsA/rootfsB, and oemA/oemB. Flash it with the standard Allwinner tool
recommended by the board manufacturer, such as PhoenixSuit, LiveSuit, or a
compatible alternative.

The image overwrites the beginning of the eMMC and is built for the current A/B
layout. Verify the actual eMMC capacity and back up important data before
flashing.

Individual `rootfs*.ext4`, `oem*.ext4`, `boot.img`, and `.fex` files
are useful for diagnostics and manual flashing, but they do not replace the
complete image during initial installation. Do not flash
`a333-helperboard.raucb` to an empty board.

## FEL and sunxi-fel verification

The A333 is detected in FEL mode, for example:

```sh
sudo sunxi-fel --list --verbose
sudo sunxi-fel -v ver
```

For this board, the expected SoC ID is `0x1919`, with an identification
string such as `AWUSBFEX soc=00001919`. The warning
`no 'soc_sram_info' data for your SoC` means that the installed upstream
`sunxi-tools` version does not yet know the A333 SRAM map.

Use the built A333-aware `xfel` implementation to load FES instead of the
regular upstream `sunxi-fel`:

```sh
sudo ../xfel/xfel version
```

FEL stage script:

```sh
scripts/flash-a333-fel.sh --dry-run
sudo scripts/flash-a333-fel.sh --xfel ../xfel/xfel
```

It performs the following sequence:

1. Loads `fes1.fex` at address `0x0004c000` and starts it.
2. Waits for the A333 to reappear in FEL after DRAM initialization.
3. Loads `u-boot.fex` at `0x4a000000`, `sunxi.fex` at `0x4a200000`,
   `config.fex` at `0x4a300000`, and `board.fex` at `0x4a380000`.
4. Starts U-Boot with `work_mode=0x10`, which selects USB EFEX/product mode.

`sunxi.fex` is the mandatory DTB/config blob expected by vendor U-Boot at
`CONFIG_SYS_TEXT_BASE + 2 MiB` (`0x4a200000`). If it is not loaded, the
UART log contains `FDT ERROR`, and U-Boot normally reboots before printing
the `workmode` and `run usb efex` messages.

When preparing U-Boot, the script also writes the confirmed DRAM size of
`2048 MiB` and recalculates the header checksum. This is important during a
manual procedure: the checksum field is located at offset `0x0c`. Without
recalculation, FES2 may reject the image and the board may return to booting
from its regular flash storage.

`xfel` only implements FEL memory operations and code execution. It does not
implement the USB EFEX protocol and cannot write to eMMC. LiveSuit,
PhoenixSuit, or another EFEX client is therefore required after the final
`exec`. An external command can be passed to the script.

After FES1 starts, the device may still appear as `1f3a:efe8`, but it stops
responding to the FEL `version` request. The local `xfel` build therefore
includes a `--no-version` mode. The script uses it only to load U-Boot,
`sunxi.fex`, `config.fex`, and `board.fex` after FES1. If the tool has
been rebuilt or replaced, run `(cd ../xfel && make)`.

```sh
scripts/flash-a333-fel.sh \
  --efex-command 'sudo ./LiveSuit'
```

Once USB EFEX appears in the client, select:

```text
output/images/a333-helperboard-full.img
```

If `--efex-command` is not provided, the script stops after the handoff and
does not write anything to eMMC. In particular, the complete `*.img` file
must not be passed to `xfel write`: that command writes to RAM, not to
persistent storage. The `sys_partition-ab.fex` and
`sys_partition-vendor-*.fex` files are input data for the vendor flasher,
not FEL address maps.

The current flashing workflow is:

1. Initial installation: `a333-helperboard-full.img` through USB EFEX and a
   vendor flasher.
2. Subsequent updates: `a333-helperboard.raucb` from the running system.
3. `sunxi-fel`/`xfel`: diagnostics and loading intermediate FEL
   components.

Do not install an old LiveSuit release directly on Ubuntu 26.04. Its
`awusb.ko` and `libpng12` were built for older kernel and userspace
versions. Allwinner documentation permits using a separate Ubuntu 20.04 or
22.04 environment and rebuilding `awusb.ko`, but this does not remove the
EFEX protocol limitation in `xfel`.

## Updating with RAUC

Use the following bundle to update an already running system:

```text
output/images/a333-helperboard.raucb
```

The bundle contains three images, installed into one inactive A/B group:

- `boot` — kernel and DTB (`boot.img`), in bootA/bootB (`p4/p5`);
- `rootfs` — updates the inactive rootfs slot;
- `oem` — updates the corresponding OEM slot.

RAUC status is kept in `/userdata/var/lib/rauc`, outside the updated slots.
Boot0, BL31, SCP, U-Boot and userdata are not replaced by an OTA bundle.

Start the update on the target system with:

```sh
rauc install /path/to/a333-helperboard.raucb
reboot
```

The custom RAUC backend maps RAUC operations to the Allwinner U-Boot
`systemAB_next`, `systemAB_damage`, and `bootcount` variables. After a
successful boot, run an application health check and mark the new slot as
good (`rauc status mark-good booted`). The custom backend uses the vendor's
single health marker, not independent per-slot health flags. Automatic retry/
rollback requires separately verified U-Boot bootcount integration; a successful
OTA reboot alone does not prove unattended recovery from a failed kernel.

Older images need a one-time migration before accepting the new three-image
bundle: add the boot child slots to `/etc/rauc/system.conf` and correct
`/etc/fw_env.config` to the single `/dev/mmcblk0p2 0x0 0x20000` entry. This U-Boot
does not use a redundant environment header. Some older vendor-generated env
images also contain a leading NUL, hiding all variables from `fw_printenv`.
Back up the first 128 KiB of both env partitions before repairing them;
`scripts/normalize-a333-env.py INPUT_BACKUP OUTPUT_FILE` validates the CRC,
preserves all variables and produces a standard environment file without
writing to a device. Verify it with `fw_printenv` before installing it. Do not
run `fw_setenv` against an invalid/empty environment: it can discard boot settings.
Fresh full images generate a matching standard environment automatically.

Verified on the media board on 2026-10-01: signed verity bundle
`2026.10.01-adb-ota1` installed bootB/rootfsB/oemB through RAUC and rebooted into
slot B with kernel build #9. The bootA checksum and userdata UUID were unchanged;
ADB service, USB UDC configuration, TCP `adb shell`, and media/Wi-Fi/Bluetooth
services were checked, and slot B was marked good. This test did not inject a
failed boot or verify automatic rollback.

The current build signs the bundle with an automatically generated development
certificate stored in the ignored `keys/` directory. For production, provide
controlled credentials that are accessible inside the container:

```sh
A333_RAUC_KEY=/workspace/keys/prod.key.pem \
A333_RAUC_CERT=/workspace/keys/prod.cert.pem \
./docker-build.sh make BR2_EXTERNAL=../configs O=../output
```

The RAUC bundle updates boot (kernel/DTB), rootfs and OEM. It does not modify the
partition table, boot-resource, or U-Boot itself. Those components require a
separate initial/vendor flashing procedure.

## Current status

The Buildroot build, vendor kernel/BSP, U-Boot, systemd/glibc rootfs, RAUC
bundle, and complete `a333-helperboard-full.img` have been verified in
Docker. Before production use on real hardware, verify the 1280×800 MIPI
panel, actual eMMC capacity, initial flashing procedure, A/B slot switching,
and rollback after a failed boot.

Additional board-specific details are available in
[`configs/boards/a333/helperboard-a333/README.md`](configs/boards/a333/helperboard-a333/README.md).
The Docker environment is described in [`DOCKER.md`](DOCKER.md).
