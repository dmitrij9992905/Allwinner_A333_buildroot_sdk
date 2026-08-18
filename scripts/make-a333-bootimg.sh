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

kernel_size=$(stat -c '%s' "$BINARIES_DIR/Image")
dtb_offset=$(( (0x80000 + kernel_size + 0xfffff) & ~0xfffff ))

"$MKBOOTIMG" \
	--kernel "$BINARIES_DIR/Image" \
	--dtb "$BINARIES_DIR/board.dtb" \
	--base 0x40000000 \
	--kernel_offset 0x80000 \
	--dtb_offset "$dtb_offset" \
	--header_version 2 \
	--pagesize 4096 \
	-o "$BINARIES_DIR/boot.img"

echo "make-a333-bootimg: created $BINARIES_DIR/boot.img (dtb offset $dtb_offset)"
