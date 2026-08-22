#!/bin/sh
set -eu

if [ "$#" -ne 1 ]; then
	echo "usage: $0 <Buildroot binaries directory>" >&2
	exit 2
fi

BINARIES_DIR="$(CDPATH= cd -- "$1" && pwd)"
PROJECT_ROOT="$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)"
PACK_ROOT="$PROJECT_ROOT/vendor/allwinner-a333/pack-sdk"
PACK_WORK="$BINARIES_DIR/.a333-pack"
PACK_PLAT_OUT="$PACK_WORK/plat"
PACK_OUT_DIR="$PACK_WORK/pack_out"
PACK_PLATFORM_OUT="$PACK_WORK/a333/pro/dragonboard"
PACK_CONFIG="$PACK_ROOT/out/a333/pro/dragonboard/.buildconfig"
HOOK="$PROJECT_ROOT/configs/boards/a333/helperboard-a333/pack-pre-finish.sh"
PARTITION_CONFIG="$PROJECT_ROOT/configs/boards/a333/helperboard-a333/sys_partition-ab.fex"
PACK_PARTITION_CONFIG="$PACK_ROOT/device/config/chips/a333/configs/pro/dragonboard/sys_partition.fex"
FACTORY_IMAGE_DEFAULT="$PROJECT_ROOT/../allwinner-a333/helpera333_ubuntu22.04_xfce_mipi8.0_800x1280_20251114.img"
CONFIG_FILE="${BR2_CONFIG:-$PROJECT_ROOT/output/.config}"
DISPLAY_ROTATION="${A333_DISPLAY_ROTATION:-}"

CUSTOM_ENV="$PROJECT_ROOT/configs/boards/a333/helperboard-a333/env-ab.cfg"
PACK_ENV="$PACK_ROOT/device/config/chips/a333/configs/pro/dragonboard/env.cfg"

for input in boot.img u-boot.bin board.dtb rootfsA.ext4 rootfsB.ext4 oemA.ext4 oemB.ext4; do
	if [ ! -f "$BINARIES_DIR/$input" ]; then
		echo "pack-a333-image: missing $BINARIES_DIR/$input" >&2
		exit 1
	fi
done

mkdir -p "$PACK_PLAT_OUT" "$PACK_OUT_DIR" "$PACK_PLATFORM_OUT"
touch "$PACK_ROOT/.disclaimer_accpet"
cp -f "$BINARIES_DIR/boot.img" "$PACK_PLAT_OUT/boot.img"
cp -f "$BINARIES_DIR/rootfs.ext4" "$PACK_PLAT_OUT/rootfs.ext4"
cp -f "$BINARIES_DIR/board.dtb" "$PACK_PLAT_OUT/sunxi.dtb"
cp -f "$BINARIES_DIR/u-boot.bin" "$PACK_PLAT_OUT/u-boot-sun65iw1p1.bin"
cp -f /usr/bin/dtc "$PACK_PLAT_OUT/dtc"

if [ ! -f "$CUSTOM_ENV" ]; then
	echo "pack-a333-image: missing custom environment: $CUSTOM_ENV" >&2
	exit 1
fi

echo "pack-a333-image: installing A/B U-Boot environment"
cp -f "$CUSTOM_ENV" "$PACK_ENV"
echo "pack-a333-image: installing project partition layout"
cp -f "$PARTITION_CONFIG" "$PACK_PARTITION_CONFIG"

# build/pack locates .buildconfig relative to its own SDK root. Generate the
# small, project-local configuration in the ignored pack output directory.
mkdir -p "$(dirname "$PACK_CONFIG")"
{
	printf '%s\n' "export LICHEE_PLATFORM=linux"
	printf '%s\n' "export LICHEE_LINUX_DEV=dragonboard"
	printf '%s\n' "export LICHEE_IC=a333"
	printf '%s\n' "export LICHEE_BOARD=pro"
	printf '%s\n' "export LICHEE_FLASH=default"
	printf '%s\n' "export LICHEE_STORAGE=16G"
	printf '%s\n' "export LICHEE_KERNEL_ARCH=arm64"
	printf '%s\n' "export LICHEE_ARCH=arm64"
	printf '%s\n' "export LICHEE_KERN_VER=linux-6.6"
	printf '%s\n' "export LICHEE_BRANDY_DEFCONF=sun65iw1p1_a333_defconfig"
	printf '%s\n' "export LICHEE_BRANDY_UBOOT_VER=2018"
	printf '%s\n' "export LICHEE_CHIP=sun65iw1p1"
	printf '%s\n' "export LICHEE_BOOT0_BIN_NAME="
	printf '%s\n' "export LICHEE_EFEX_BIN_NAME="
	printf '%s\n' "export LICHEE_BUSSINESS="
	printf '%s\n' "export LICHEE_KERN_DIR=$PROJECT_ROOT/output/build/linux-custom"
	printf '%s\n' "export LICHEE_TOP_DIR=$PACK_ROOT"
	printf '%s\n' "export LICHEE_BUILD_DIR=$PACK_ROOT/build"
	printf '%s\n' "export LICHEE_DEVICE_DIR=$PACK_ROOT/device"
	printf '%s\n' "export LICHEE_TOOLS_DIR=$PACK_ROOT/tools"
	printf '%s\n' "export LICHEE_COMMON_CONFIG_DIR=$PACK_ROOT/device/config/common"
	printf '%s\n' "export LICHEE_CHIP_CONFIG_DIR=$PACK_ROOT/device/config/chips/a333"
	printf '%s\n' "export LICHEE_BOARD_CONFIG_DIR=$PACK_ROOT/device/config/chips/a333/configs/pro"
	printf '%s\n' "export LICHEE_OUT_DIR=$PACK_WORK"
	printf '%s\n' "export LICHEE_BRANDY_OUT_DIR=$PACK_ROOT/device/config/chips/a333/bin"
	printf '%s\n' "export LICHEE_PACK_OUT_DIR=$PACK_OUT_DIR"
	printf '%s\n' "export LICHEE_PLAT_OUT=$PACK_PLAT_OUT"
	printf '%s\n' 'export LICHEE_POSSIBLE_BIN_PATH="bin configs/pro/bin configs/pro/dragonboard/bin"'
	printf '%s\n' "export LICHEE_PACK_SECURE_TYPE=none"
	printf '%s\n' "export LICHEE_REDUNDANT_ENV_SIZE="
	printf '%s\n' "export LICHEE_ONE_ENV_SIZE="
	printf '%s\n' "export BUILD_SATA=false"
} > "$PACK_CONFIG"

BOOTLOGO_SRC="$PROJECT_ROOT/configs/boards/a333/helperboard-a333/bootlogo.bmp"
BOOTLOGO_ROTATOR="$PROJECT_ROOT/scripts/rotate-a333-bootlogo.py"
PACK_BOARD_DIR="$PACK_ROOT/device/config/chips/a333/configs/pro/dragonboard"
if [ -z "$DISPLAY_ROTATION" ] && [ -f "$CONFIG_FILE" ]; then
	DISPLAY_ROTATION="$(sed -n 's/^BR2_A333_DISPLAY_ROTATION=//p' "$CONFIG_FILE")"
fi
DISPLAY_ROTATION="${DISPLAY_ROTATION:-90}"
case "$DISPLAY_ROTATION" in
	0|90|180|270) ;;
	*)
		echo "pack-a333-image: invalid display rotation: $DISPLAY_ROTATION" >&2
		exit 1
		;;
esac
if [ ! -f "$BOOTLOGO_SRC" ]; then
	echo "pack-a333-image: missing boot logo: $BOOTLOGO_SRC" >&2
	exit 1
fi
if [ ! -f "$BOOTLOGO_ROTATOR" ]; then
	echo "pack-a333-image: missing boot logo rotator: $BOOTLOGO_ROTATOR" >&2
	exit 1
fi
mkdir -p "$PACK_BOARD_DIR"
python3 "$BOOTLOGO_ROTATOR" --rotation "$DISPLAY_ROTATION" \
	"$BOOTLOGO_SRC" "$PACK_BOARD_DIR/bootlogo.bmp"
echo "pack-a333-image: boot logo rotation is $DISPLAY_ROTATION degrees"

export A333_BINARIES_DIR="$BINARIES_DIR"
export A333_PROJECT_ROOT="$PROJECT_ROOT"
export A333_PACK_WORK="$PACK_WORK"
# If the vendor Ubuntu image is present next to this repository, use it as
# the source of the known-good boot0/SCP pair. The variable can be overridden
# for another factory image, or left empty to keep the SDK binaries.
if [ -z "${A333_FACTORY_IMAGE:-}" ] && [ -f "$FACTORY_IMAGE_DEFAULT" ]; then
	export A333_FACTORY_IMAGE="$FACTORY_IMAGE_DEFAULT"
fi
export A333_PACK_PRE_FINISH_HOOK="$HOOK"
export A333_PACK_SKIP_FLASHMAP=1

"$PACK_ROOT/build/pack" \
	-c sun65iw1p1 \
	-i a333 \
	-p dragonboard \
	-b pro \
	-k linux-6.6 \
	-d uart0 \
	-v none \
	-m normal \
	-n none

image="$PACK_PLATFORM_OUT/a333_dragonboard_pro_uart0.img"
if [ ! -f "$image" ]; then
	echo "pack-a333-image: packer did not create $image" >&2
	exit 1
fi

cp -f "$image" "$BINARIES_DIR/a333-helperboard-full.img"
cp -f "$PACK_OUT_DIR/sys_partition.fex" "$BINARIES_DIR/a333-helperboard-sys_partition.fex"
echo "pack-a333-image: created $BINARIES_DIR/a333-helperboard-full.img"
