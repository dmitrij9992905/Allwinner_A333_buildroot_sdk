# Buildroot-based Allwinner A333 Build System

[Russian version](README_RU.md)

This project provides an independent Linux build system for an Allwinner A333
module installed on a HelperBoard with a 1280×800 MIPI display.

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
overlays/                   base rootfs files
oem/                        applications and OEM partition contents
vendor/allwinner-a333/      Allwinner kernel, BSP, DTS, U-Boot, and pack SDK
scripts/                    boot.img, full image, and RAUC bundle generation
Dockerfile                  build environment container
docker-build.sh             Docker build wrapper
output/                     build output, ignored by Git
dl/, ccache/, toolchains/   local caches and toolchains, ignored by Git
```

## Building

Docker Engine is required. The Docker Compose plugin is not required by the
main wrapper. All host dependencies are installed inside the Docker image.

First, build the Docker image:

```sh
./docker-build.sh build
```

Prepare local vendor source archives and apply the board configuration:

```sh
./docker-build.sh prepare-sources
./docker-build.sh make BR2_EXTERNAL=../configs O=../output a333_helperboard_defconfig
```

Start the build:

```sh
./docker-build.sh make BR2_EXTERNAL=../configs O=../output
```

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
./docker-build.sh make BR2_EXTERNAL=../configs O=../output menuconfig
```

Downloaded sources and the compiler cache are kept in the project's `dl/` and
`ccache/` directories. This speeds up subsequent builds and avoids depending
on the Ubuntu version installed on the host.

## Build artifacts

After a successful build, the files are available in:

```text
output/images/
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
| `a333-helperboard.raucb` | signed RAUC update bundle |

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

Service status can be inspected with:

```sh
systemctl status sshd adbd NetworkManager bluetooth
journalctl -u sshd -u adbd -u NetworkManager -u bluetooth
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

The bundle contains two images:

- `rootfs` — updates the inactive rootfs slot;
- `oem` — updates the corresponding OEM slot.

Start the update on the target system with:

```sh
rauc install /path/to/a333-helperboard.raucb
reboot
```

The custom RAUC backend maps RAUC operations to the Allwinner U-Boot
`systemAB_next`, `systemAB_damage`, and `bootcount` variables. After a
successful boot, run an application health check and mark the new slot as
good. If the boot fails, U-Boot should return the system to the previous slot.

The current build signs the bundle with an automatically generated development
certificate stored in the ignored `keys/` directory. For production, provide
controlled credentials that are accessible inside the container:

```sh
A333_RAUC_KEY=/workspace/keys/prod.key.pem \
A333_RAUC_CERT=/workspace/keys/prod.cert.pem \
./docker-build.sh make BR2_EXTERNAL=../configs O=../output
```

The RAUC bundle updates rootfs and OEM. It is not intended to modify the
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
