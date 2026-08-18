#!/bin/sh
set -eu

: "${A333_BINARIES_DIR:?A333_BINARIES_DIR is required}"

copy_payload()
{
	src="$1"
	dst="$2"

	if [ ! -f "$A333_BINARIES_DIR/$src" ]; then
		echo "A333 pack: missing payload $A333_BINARIES_DIR/$src" >&2
		exit 1
	fi

	cp -f "$A333_BINARIES_DIR/$src" "$dst"
}

# build/pack has already changed the current directory to pack_out here.
copy_payload boot.img bootA.fex
copy_payload boot.img bootB.fex
copy_payload rootfsA.ext4 rootfsA.fex
copy_payload rootfsB.ext4 rootfsB.fex
copy_payload oemA.ext4 oemA.fex
copy_payload oemB.ext4 oemB.fex

# fsbuild is a 32-bit vendor utility and is unavailable in some host
# sandboxes. Recreate its FAT16 result when it did not produce the file.
if [ ! -f boot-resource.fex ]; then
	truncate -s 32M boot-resource.fex
	mkfs.fat -F 16 -n BOOTRES boot-resource.fex >/dev/null
	mcopy -i boot-resource.fex -s boot-resource/* ::
fi

# The vendor packer generates env.fex from env.cfg in pack_out. Keep the
# redundant partition identical until U-Boot's exact redundant-env semantics
# are validated on the target board.
if [ ! -f env.fex ]; then
	echo "A333 pack: vendor packer did not generate env.fex" >&2
	exit 1
fi
cp -f env.fex env-redund.fex
