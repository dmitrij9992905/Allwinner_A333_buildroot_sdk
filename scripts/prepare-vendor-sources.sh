#!/usr/bin/env bash
set -Eeuo pipefail

project_root="$(CDPATH= cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
dl_dir="${DL_DIR:-$project_root/dl}"
vendor_root="$project_root/vendor/allwinner-a333"
echo "using project vendor copy: $vendor_root"

mkdir -p "$dl_dir"

make_archive()
{
	name="$1"
	source_parent="$2"
	target="$dl_dir/$3"
	extra_tree="${4:-}"

	if [[ -f "$target" ]]; then
		if [[ -z "$extra_tree" ]] || \
			(tar --zstd --list --file "$target" | grep -Fqx "$name/bsp/Kconfig" && \
			 tar --zstd --list --file "$target" | grep -Fqx "$name/bsp/include/sunxi-autogen.h"); then
			echo "exists: $target"
			return 0
		fi

		echo "stale archive lacks $name/bsp/Kconfig: $target"
		mv -- "$target" "$target.stale-missing-bsp"
	fi

	echo "creating: $target"
	tar_args=(--zstd --create --file "$target" --exclude-vcs)
	if [[ -n "$extra_tree" ]]; then
		stage_dir="$(mktemp -d "$project_root/.allwinner-a333-archive.XXXXXX")"
		trap 'rm -rf -- "$stage_dir"' RETURN
		cp -al -- "$source_parent/$name" "$stage_dir/"
		rm -- "$stage_dir/$name/bsp"
		cp -al -- "$extra_tree" "$stage_dir/$name/bsp"
		tar "${tar_args[@]}" -C "$stage_dir" "$name"
		trap - RETURN
		rm -rf -- "$stage_dir"
		return 0
	fi
	tar "${tar_args[@]}" -C "$source_parent" "$name"
}

make_archive \
	linux-6.6 \
	"$vendor_root/kernel" \
	linux-a333.tar.zst \
	"$vendor_root/bsp"

make_archive \
	u-boot-2018 \
	"$vendor_root/boot" \
	u-boot-a333.tar.zst

echo "vendor source archives are ready in $dl_dir"
