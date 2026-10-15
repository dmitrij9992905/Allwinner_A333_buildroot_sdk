#!/usr/bin/env bash
set -Eeuo pipefail

project_root="$(CDPATH= cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
dl_dir="${DL_DIR:-$project_root/dl}"
vendor_root="$project_root/vendor/allwinner-a333"
echo "using project vendor copy: $vendor_root"

mkdir -p "$dl_dir"

stage_dir=
archive_tmp=
cleanup()
{
	[[ -z "$stage_dir" ]] || rm -rf -- "$stage_dir"
	[[ -z "$archive_tmp" ]] || rm -f -- "$archive_tmp"
	return 0
}
trap cleanup EXIT

make_archive()
{
	local name="$1"
	local source_parent="$2"
	local target="$dl_dir/$3"
	local extra_tree="${4:-}"
	local -a tar_args

	if [[ -f "$target" ]]; then
		if [[ -z "$extra_tree" ]] || \
			tar --zstd --list --file "$target" \
				"$name/bsp/Kconfig" "$name/bsp/include/sunxi-autogen.h" >/dev/null 2>&1; then
			echo "exists: $target"
			return 0
		fi

		echo "stale or invalid archive lacks required BSP files: $target"
		mv -- "$target" "$target.stale-missing-bsp"
	fi

	if [[ ! -f "$source_parent/$name/Makefile" ]]; then
		echo "Missing vendor source tree: $source_parent/$name" >&2
		return 1
	fi
	if [[ -n "$extra_tree" && ! -f "$extra_tree/Kconfig" ]]; then
		echo "Missing vendor BSP: $extra_tree" >&2
		return 1
	fi

	echo "creating: $target"
	archive_tmp="$(mktemp "$target.tmp.XXXXXX")"
	tar_args=(--zstd --create --file "$archive_tmp" --exclude-vcs)
	if [[ -n "$extra_tree" ]]; then
		stage_dir="$(mktemp -d "$project_root/.allwinner-a333-archive.XXXXXX")"
		cp -al -- "$source_parent/$name" "$stage_dir/"
		# Git clones omit the original SDK's ignored BSP symlink.
		if [[ -L "$stage_dir/$name/bsp" ]]; then
			rm -- "$stage_dir/$name/bsp"
		elif [[ -e "$stage_dir/$name/bsp" ]]; then
			rm -rf -- "$stage_dir/$name/bsp"
		fi
		cp -al -- "$extra_tree" "$stage_dir/$name/bsp"
		# mkkernel.sh normally generates this ignored version header. The
		# independent Buildroot build does not run that SDK script.
		if [[ ! -f "$stage_dir/$name/bsp/include/sunxi-autogen.h" ]]; then
			mkdir -p "$stage_dir/$name/bsp/include"
			printf '#define AW_BSP_VERSION "Allwinner A333 Buildroot vendor snapshot"\n' \
				> "$stage_dir/$name/bsp/include/sunxi-autogen.h"
		fi
		tar "${tar_args[@]}" -C "$stage_dir" "$name"
		rm -rf -- "$stage_dir"
		stage_dir=
	else
		tar "${tar_args[@]}" -C "$source_parent" "$name"
	fi
	mv -- "$archive_tmp" "$target"
	archive_tmp=
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
