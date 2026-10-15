#!/bin/sh
set -eu

: "${A333_PROJECT_ROOT:?A333_PROJECT_ROOT is required}"
: "${A333_PACK_WORK:?A333_PACK_WORK is required}"

# Called in pack_out after vendor files are copied, before do_common creates
# boot_package.fex. SCP must be present at that first packaging step.
factory_image="${A333_FACTORY_IMAGE:-}"
factory_boot0="${A333_FACTORY_BOOT0:-}"
factory_scp="${A333_FACTORY_SCP:-}"

if [ -n "$factory_image$factory_boot0$factory_scp" ]; then
	if [ -n "$factory_image" ] && [ ! -s "$factory_image" ]; then
		echo "A333 pack: factory image not found or empty: $factory_image" >&2
		exit 1
	fi
	if [ -z "$factory_image" ] && { [ -z "$factory_boot0" ] || [ -z "$factory_scp" ]; }; then
		echo "A333 pack: provide both A333_FACTORY_BOOT0 and A333_FACTORY_SCP, or A333_FACTORY_IMAGE" >&2
		exit 1
	fi

	stage_dir="$(mktemp -d "$A333_PACK_WORK/.factory-boot.XXXXXX")"
	trap 'rm -rf -- "$stage_dir"' EXIT
	parser_img="$A333_PROJECT_ROOT/vendor/allwinner-a333/pack-sdk/tools/pack/pctools/linux/mod_update/parser_img"
	extract_image_item()
	{
		if [ ! -x "$parser_img" ]; then
			echo "A333 pack: parser_img not found: $parser_img" >&2
			exit 1
		fi
		input_rel=$(realpath -m --relative-to="$A333_PROJECT_ROOT" "$factory_image")
		output_rel=$(realpath -m --relative-to="$A333_PROJECT_ROOT" "$1")
		(cd "$A333_PROJECT_ROOT"; "$parser_img" "$input_rel" "$output_rel" "$2" "$3")
	}

	if [ -n "$factory_boot0" ]; then
		cp -f "$factory_boot0" "$stage_dir/boot0.fex"
	else
		echo "A333 pack: extracting factory boot0"
		extract_image_item "$stage_dir/boot0.fex" 12345678 1234567890BOOT_0
	fi
	if [ -n "$factory_scp" ]; then
		cp -f "$factory_scp" "$stage_dir/scp.fex"
	else
		echo "A333 pack: extracting factory boot package"
		extract_image_item "$stage_dir/boot_package.fex" 12345678 BOOTPKG-00000000
		python3 - "$stage_dir/boot_package.fex" "$stage_dir/scp.fex" <<'PY'
import struct
import sys
from pathlib import Path

data = Path(sys.argv[1]).read_bytes()
# The item name follows the preceding descriptor's IIE; end marker.
# Offset/length are at +0x44/+0x48 relative to that marker.
descriptor = data.find(b"IIE;scp\0")
if descriptor < 0 or descriptor + 0x4c > len(data):
    sys.exit("A333 pack: factory boot package has no valid scp descriptor")
offset, size = struct.unpack_from("<II", data, descriptor + 0x44)
if not size or offset < descriptor + 0x4c or offset + size > len(data):
    sys.exit("A333 pack: invalid factory scp offset/size")
Path(sys.argv[2]).write_bytes(data[offset:offset + size])
PY
	fi
	if [ ! -s "$stage_dir/boot0.fex" ] || [ ! -s "$stage_dir/scp.fex" ]; then
		echo "A333 pack: factory boot0/SCP extraction produced an empty or missing file" >&2
		exit 1
	fi
	# Refresh on every invocation; cached firmware may belong to another image.
	cp -f "$stage_dir/boot0.fex" "$A333_PACK_WORK/factory-boot0.fex"
	cp -f "$stage_dir/scp.fex" "$A333_PACK_WORK/factory-scp.fex"
	cp -f "$stage_dir/boot0.fex" boot0_sdcard.fex
	cp -f "$stage_dir/scp.fex" scp.fex
	echo "A333 pack: installed factory boot0 and SCP before boot package creation"
fi

if [ ! -s scp.fex ]; then
	echo "A333 pack: missing SCP firmware (scp.fex / vendor scp.bin)." >&2
	echo "Set A333_FACTORY_IMAGE to the HelperBoard A333 factory image, or provide a matching A333_FACTORY_BOOT0/A333_FACTORY_SCP pair (paths inside /workspace)." >&2
	exit 1
fi

# Ensure the generated package includes SCP, even with older SDK configs.
sed -i '/standalone A333 BSP does not ship an SCP firmware binary/d; s/^[[:space:]]*;[[:space:]]*item=scp[[:space:]]*,/item=scp,/' boot_package.cfg
if ! grep -q '^[[:space:]]*item=scp[[:space:]]*,' boot_package.cfg; then
	printf '\nitem=scp,                scp.fex\n' >> boot_package.cfg
fi
