# Build helper scripts

Scripts in this directory operate on the project layout and are called from
the Docker build environment. They do not modify the vendor source tree.

`prepare-vendor-sources.sh` creates local Buildroot download archives from the
copied kernel and U-Boot sources. The archives are generated in `dl/`, which
is ignored as a download cache. Buildroot's compiler cache is kept separately
in `ccache/`.

The ignored `toolchains/` directory contains the copied Allwinner SDK Linaro
toolchain used only for the 32-bit vendor U-Boot build. The main Buildroot
target toolchain remains AArch64/glibc.

`patch-a333-efex-uboot.py` prepares the vendor U-Boot header for running from
FEL in USB EFEX mode. It updates `work_mode`, the detected DRAM size and the
Allwinner additive checksum.

`flash-a333-fel.sh` uploads FES1, the corrected U-Boot, `sunxi.fex`,
`config.fex` and `board.fex` with the A537/A333-aware `xfel` binary. The DTB
is loaded at `0x4a200000`, as required by this vendor U-Boot. Without it U-Boot
prints FDT errors and resets before entering USB EFEX. `xfel` does not
implement the USB EFEX eMMC protocol, so the script does not write storage itself. An
external LiveSuit/PhoenixSuit-compatible EFEX client can be passed with
`--efex-command`; otherwise the script stops after the handoff.

After FES1 starts, the USB device keeps VID/PID `1f3a:efe8` but may no longer
answer the FEL version request. The local xfel tree therefore contains the
`--no-version` mode used for the second-stage memory uploads. Rebuild it after pulling
or changing the xfel sources:

```sh
(cd ../xfel && make)
```
