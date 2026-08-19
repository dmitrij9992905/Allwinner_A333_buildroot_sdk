#!/bin/sh
set -eu

: "${A333_BINARIES_DIR:?A333_BINARIES_DIR is required}"

install_factory_boot_files()
{
	[ -n "${A333_FACTORY_IMAGE:-}" ] || return 0
	: "${A333_PROJECT_ROOT:?A333_PROJECT_ROOT is required for factory boot files}"
	: "${A333_PACK_WORK:?A333_PACK_WORK is required for factory boot files}"

	factory_image="$A333_FACTORY_IMAGE"
	parser_img="$A333_PROJECT_ROOT/vendor/allwinner-a333/pack-sdk/tools/pack/pctools/linux/mod_update/parser_img"
	factory_boot0="${A333_FACTORY_BOOT0:-$A333_PACK_WORK/factory-boot0.fex}"
	factory_package="$A333_PACK_WORK/factory-boot_package.fex"
	factory_scp="${A333_FACTORY_SCP:-$A333_PACK_WORK/factory-scp.fex}"

	if [ ! -f "$factory_image" ]; then
		echo "A333 pack: factory image not found: $factory_image" >&2
		exit 1
	fi
	if [ ! -x "$parser_img" ]; then
		echo "A333 pack: parser_img not found: $parser_img" >&2
		exit 1
	fi

	extract_image_item()
	{
		input_rel=$(realpath -m --relative-to="$A333_PROJECT_ROOT" "$factory_image")
		output_rel=$(realpath -m --relative-to="$A333_PROJECT_ROOT" "$1")
		(
			cd "$A333_PROJECT_ROOT"
			"$parser_img" "$input_rel" "$output_rel" "$2" "$3"
		)
	}

	if [ ! -f "$factory_boot0" ]; then
		echo "A333 pack: extracting factory boot0"
		extract_image_item "$factory_boot0" 12345678 1234567890BOOT_0
	fi

	if [ ! -f "$factory_scp" ]; then
		if [ ! -f "$factory_package" ]; then
			echo "A333 pack: extracting factory boot package"
			extract_image_item "$factory_package" 12345678 BOOTPKG-00000000
		fi

		# dragonsecboot stores the item offset and size in the descriptor:
		#   name at +0x00, offset at +0x44, size at +0x48.
		scp_desc=$(grep -aob -m1 'IIE;scp' "$factory_package" | cut -d: -f1)
		if [ -z "$scp_desc" ]; then
			echo "A333 pack: factory boot package has no scp item" >&2
			exit 1
		fi
		scp_offset=$(od -An -tu4 -j $((scp_desc + 0x44)) -N4 "$factory_package" | tr -d '[:space:]')
		scp_size=$(od -An -tu4 -j $((scp_desc + 0x48)) -N4 "$factory_package" | tr -d '[:space:]')
		package_size=$(stat -c '%s' "$factory_package")
		if [ -z "$scp_offset" ] || [ -z "$scp_size" ] || [ "$scp_size" -eq 0 ] || \
			[ $((scp_offset + scp_size)) -gt "$package_size" ]; then
			echo "A333 pack: invalid factory scp descriptor" >&2
			exit 1
		fi
		echo "A333 pack: extracting factory scp (${scp_size} bytes)"
		dd if="$factory_package" of="$factory_scp" bs=1 skip="$scp_offset" count="$scp_size" status=none
	fi

	cp -f "$factory_boot0" boot0_sdcard.fex
	cp -f "$factory_scp" scp.fex

	# The default A333 package omits SCP. Re-enable it in the generated
	# pack_out config and rebuild boot_package.fex with the custom U-Boot and
	# monitor plus the factory SCP.
	sed -i '/standalone A333 BSP does not ship an SCP firmware binary/d; s/^[[:space:]]*;[[:space:]]*item=scp[[:space:]]*,/item=scp,/' boot_package.cfg
	if ! grep -q '^[[:space:]]*item=scp[[:space:]]*,' boot_package.cfg; then
		sed -i '$a item=scp,                scp.fex' boot_package.cfg
	fi
	dragonsecboot -pack boot_package.cfg
	if ! grep -aob -m1 'IIE;scp' boot_package.fex >/dev/null; then
		echo "A333 pack: rebuilt boot package still has no SCP" >&2
		exit 1
	fi
	echo "A333 pack: using factory boot0 and SCP"
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
install_factory_boot_files
patch_boot0_chip_tag

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
