#!/bin/sh
set -eu

BINARIES_DIR="$1"
PROJECT_ROOT="$(CDPATH= cd -- "$(dirname -- "$0")/../../../.." && pwd)"
OEM_ROOT="${A333_OEM_ROOT:-$PROJECT_ROOT/oem/a333/rootfs}"
OEM_SIZE="${A333_OEM_SIZE:-512M}"

copy_slot_image()
{
	image="$1"
	slot_a="$2"
	slot_b="$3"

	if [ ! -f "$BINARIES_DIR/$image" ]; then
		echo "post-image: missing $BINARIES_DIR/$image" >&2
		exit 1
	fi

	cp -f "$BINARIES_DIR/$image" "$BINARIES_DIR/$slot_a"
	cp -f "$BINARIES_DIR/$image" "$BINARIES_DIR/$slot_b"
}

copy_slot_image rootfs.ext4 rootfsA.ext4 rootfsB.ext4

if [ ! -d "$OEM_ROOT" ]; then
	echo "post-image: creating empty OEM root directory: $OEM_ROOT" >&2
	mkdir -p "$OEM_ROOT"
fi

rm -f "$BINARIES_DIR/oem.ext4" "$BINARIES_DIR/oemA.ext4" "$BINARIES_DIR/oemB.ext4"
mke2fs -q -t ext4 -L oem -d "$OEM_ROOT" "$BINARIES_DIR/oem.ext4" "$OEM_SIZE"
copy_slot_image oem.ext4 oemA.ext4 oemB.ext4

if [ -f "$BINARIES_DIR/Image" ]; then
	cp -f "$BINARIES_DIR/Image" "$BINARIES_DIR/Image.A"
	cp -f "$BINARIES_DIR/Image" "$BINARIES_DIR/Image.B"
fi

"$PROJECT_ROOT/scripts/make-a333-rauc-bundle.sh" "$BINARIES_DIR"
"$PROJECT_ROOT/scripts/make-a333-bootimg.sh" "$BINARIES_DIR"
"$PROJECT_ROOT/scripts/pack-a333-image.sh" "$BINARIES_DIR"
