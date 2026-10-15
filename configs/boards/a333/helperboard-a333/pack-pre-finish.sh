#!/bin/sh
set -eu

: "${A333_BINARIES_DIR:?A333_BINARIES_DIR is required}"

restore_factory_boot0()
{
	[ -n "${A333_FACTORY_IMAGE:-}${A333_FACTORY_BOOT0:-}${A333_FACTORY_SCP:-}" ] || return 0
	: "${A333_PACK_WORK:?A333_PACK_WORK is required for factory boot files}"

	# do_common has already packed SCP. Restore the original factory boot0
	# after the vendor sys_config updates, preserving the existing behavior.
	if [ ! -s "$A333_PACK_WORK/factory-boot0.fex" ] || [ ! -s scp.fex ]; then
		echo "A333 pack: factory boot0/SCP were not prepared before packaging" >&2
		exit 1
	fi
	cp -f "$A333_PACK_WORK/factory-boot0.fex" boot0_sdcard.fex
}

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

patch_boot0_chip_tag()
{
	: "${A333_PROJECT_ROOT:?A333_PROJECT_ROOT is required for BOOT0 patching}"

	boot0="boot0_sdcard.fex"
	patcher="$A333_PROJECT_ROOT/scripts/patch-a333-boot0-chip-tag.py"

	if [ ! -f "$boot0" ]; then
		echo "A333 pack: boot0 not found: $boot0" >&2
		exit 1
	fi

	if [ ! -f "$patcher" ]; then
		echo "A333 pack: BOOT0 chip-tag patcher not found: $patcher" >&2
		exit 1
	fi

	echo "A333 pack: patching BOOT0 chip tag"
	python3 "$patcher" "$boot0"
}

# build/pack has already changed the current directory to pack_out here.
restore_factory_boot0
patch_boot0_chip_tag

copy_payload boot.img bootA.fex
copy_payload boot.img bootB.fex
copy_payload rootfsA.ext4 rootfsA.fex
copy_payload rootfsB.ext4 rootfsB.fex
copy_payload oemA.ext4 oemA.fex
copy_payload oemB.ext4 oemB.fex
copy_payload credentials.ext4 credentials.fex
chmod 0600 credentials.fex

# Keep the userdata image small; first boot grows its partition and filesystem.
truncate -s 32M userdata.fex
mkfs.ext4 -F -L userdata userdata.fex >/dev/null

# fsbuild is a 32-bit vendor utility and is unavailable in some host
# sandboxes. Recreate its FAT16 result when it did not produce the file.
if [ ! -f boot-resource.fex ]; then
	truncate -s 32M boot-resource.fex
	mkfs.fat -F 16 -n BOOTRES boot-resource.fex >/dev/null
	if command -v mcopy >/dev/null 2>&1; then
		mcopy -i boot-resource.fex -s boot-resource/* ::
	else
		echo "A333 pack: mtools/mcopy is unavailable; boot-resource FAT is empty" >&2
	fi
fi

# The vendor packer generates env.fex from env.cfg in pack_out. Keep the
# redundant partition identical until U-Boot's exact redundant-env semantics
# are validated on the target board.
if [ ! -f env.fex ]; then
	echo "A333 pack: vendor packer did not generate env.fex" >&2
	exit 1
fi
cp -f env.fex env-redund.fex
