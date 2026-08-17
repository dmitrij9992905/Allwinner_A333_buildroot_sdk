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
