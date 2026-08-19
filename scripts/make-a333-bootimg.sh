#!/bin/sh
set -eu

if [ "$#" -ne 1 ]; then
    echo "usage: $0 <Buildroot binaries directory>" >&2
    exit 2
fi

BINARIES_DIR="$(CDPATH= cd -- "$1" && pwd)"
PROJECT_ROOT="$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)"
MKBOOTIMG="$PROJECT_ROOT/vendor/allwinner-a333/pack-sdk/tools/pack/pctools/linux/android/mkbootimg"

for input in Image board.dtb; do
    if [ ! -f "$BINARIES_DIR/$input" ]; then
        echo "make-a333-bootimg: missing $BINARIES_DIR/$input" >&2
        exit 1
    fi
done

if [ ! -x "$MKBOOTIMG" ]; then
    echo "make-a333-bootimg: mkbootimg is not executable: $MKBOOTIMG" >&2
    exit 1
fi

BOOT_BASE=0x40000000

# Keep the kernel on a 2 MiB aligned physical base.
# Current Linux 6.6 Image has text_offset=0.
KERNEL_BASE_OFFSET=0x00200000

kernel_size=$(stat -c '%s' "$BINARIES_DIR/Image")


image_info=$(python3 - "$BINARIES_DIR/Image" <<'PY'
import struct
import sys

with open(sys.argv[1], "rb") as f:
    h = f.read(64)

if len(h) < 64:
    raise SystemExit("ARM64 Image header is too short")

text_offset = struct.unpack_from("<Q", h, 0x08)[0]
image_size  = struct.unpack_from("<Q", h, 0x10)[0]
magic       = struct.unpack_from("<I", h, 0x38)[0]

if magic != 0x644d5241:
    raise SystemExit(
        f"invalid ARM64 Image magic: 0x{magic:08x}"
    )

print(text_offset, image_size)
PY
)

set -- $image_info
text_offset=$1
image_size=$2

# Image must start text_offset bytes after a 2 MiB aligned base.
kernel_offset=$((KERNEL_BASE_OFFSET + text_offset))

# image_size is the effective memory footprint from the ARM64 header.
# Old kernels may report zero, in which case use the actual file size.
effective_kernel_size=$kernel_size
if [ "$image_size" -gt "$effective_kernel_size" ]; then
    effective_kernel_size=$image_size
fi

# Put DTB after the complete effective kernel region.
# 1 MiB alignment is more than sufficient for the DTB itself.
dtb_offset=$(( \
    (kernel_offset + effective_kernel_size + 0xfffff) \
    & ~0xfffff \
))

# No ramdisk is currently supplied, but keep its header address outside
# the kernel/DTB area to avoid the misleading U-Boot overlap warning.
RAMDISK_OFFSET=0x04000000

"$MKBOOTIMG" \
    --kernel "$BINARIES_DIR/Image" \
    --dtb "$BINARIES_DIR/board.dtb" \
    --base "$BOOT_BASE" \
    --kernel_offset "$kernel_offset" \
    --ramdisk_offset "$RAMDISK_OFFSET" \
    --dtb_offset "$dtb_offset" \
    --header_version 2 \
    --pagesize 4096 \
    -o "$BINARIES_DIR/boot.img"

printf '%s\n' \
    "make-a333-bootimg: created $BINARIES_DIR/boot.img" \
    "  kernel file size      : $kernel_size" \
    "  kernel image_size     : $image_size" \
    "  kernel text_offset    : $text_offset" \
    "  kernel offset         : $kernel_offset" \
    "  kernel physical addr  : $((BOOT_BASE + kernel_offset))" \
    "  ramdisk physical addr : $((BOOT_BASE + RAMDISK_OFFSET))" \
    "  dtb offset            : $dtb_offset" \
    "  dtb physical addr     : $((BOOT_BASE + dtb_offset))"

