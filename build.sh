#!/usr/bin/env bash
set -euo pipefail

project_dir="$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)"
usage() {
    printf 'Usage: %s list | <profile> [configure|build|clean|menuconfig|savedefconfig] [make arguments...]\n' "$0"
}

if [[ ${1:-} == list ]]; then
    for config in "$project_dir"/profiles/*.conf; do
        [[ -f $config ]] || continue
        profile=${config##*/}
        printf '%s\n' "${profile%.conf}"
    done
    exit 0
fi

if [[ $# -lt 1 ]]; then usage; exit 2; fi
profile=$1
shift
if [[ ! $profile =~ ^[a-z0-9][a-z0-9_-]*$ ]]; then
    echo "Invalid profile name: $profile" >&2
    exit 2
fi
profile_file="$project_dir/profiles/$profile.conf"
if [[ ! -f $profile_file ]]; then
    echo "Unknown profile: $profile (try: $0 list)" >&2
    exit 2
fi

# Profiles are project-maintained shell configuration, never user input.
DEFCONFIG=
OUTPUT_DIR=
A333_RAUC_BUNDLE_NAME=
PROFILE_ENV=()
ROOTFS_OVERLAYS=()
OEM_OVERLAYS=()
# shellcheck source=/dev/null
source "$profile_file"
: "${DEFCONFIG:?profile must set DEFCONFIG}"
OUTPUT_DIR=${OUTPUT_DIR:-"output/profiles/$profile"}
if [[ ! $DEFCONFIG =~ ^[a-zA-Z0-9_]+_defconfig$ ||
      ! -f $project_dir/configs/configs/$DEFCONFIG ||
      ! $OUTPUT_DIR =~ ^output/[a-zA-Z0-9_/-]+$ ||
      $OUTPUT_DIR == *..* ]]; then
    echo "Invalid profile defconfig or output directory" >&2
    exit 2
fi
if ((${#ROOTFS_OVERLAYS[@]} == 0)); then
    echo "Profile $profile must specify at least one ROOTFS_OVERLAYS directory" >&2
    exit 2
fi
validate_overlays() {
    local kind=$1 entry
    shift
    for entry in "$@"; do
        if [[ ! $entry =~ ^[a-zA-Z0-9_./-]+$ || $entry == /* ||
              $entry == *..* || ! -d $project_dir/$entry ]]; then
            echo "Invalid $kind overlay in $profile: $entry" >&2
            exit 2
        fi
    done
}
validate_overlays ROOTFS "${ROOTFS_OVERLAYS[@]}"
validate_overlays OEM "${OEM_OVERLAYS[@]}"
rootfs_paths=()
for overlay in "${ROOTFS_OVERLAYS[@]}"; do rootfs_paths+=("../$overlay"); done
rootfs_overlay_list="${rootfs_paths[*]}"
oem_overlay_list="$(IFS=:; echo "${OEM_OVERLAYS[*]}")"
selection_file="$project_dir/$OUTPUT_DIR/.a333-overlay-selection"
printf -v overlay_selection 'rootfs=%s\noem=%s' \
    "$rootfs_overlay_list" "$oem_overlay_list"
action=${1:-build}
if [[ $# -gt 0 ]]; then shift; fi
case "$action" in
    configure|build|clean|menuconfig|savedefconfig) ;;
    *) usage; exit 2 ;;
esac
if [[ $action != clean && -f $selection_file &&
      $(<"$selection_file") != "$overlay_selection" &&
      -d $project_dir/$OUTPUT_DIR/target ]]; then
    echo "Overlay stack changed for $OUTPUT_DIR; run '$0 $profile clean' or choose a fresh OUTPUT_DIR" >&2
    exit 2
fi

make_args=("O=../$OUTPUT_DIR" "BR2_EXTERNAL=../configs")
export A333_RAUC_BUNDLE_NAME
run_make() {
    if [[ -f /.dockerenv && $project_dir == /workspace ]]; then
        (
            cd "$project_dir/buildroot"
            env "A333_RAUC_BUNDLE_NAME=$A333_RAUC_BUNDLE_NAME" \
                "A333_OEM_OVERLAYS=$oem_overlay_list" \
                "${PROFILE_ENV[@]}" make "${make_args[@]}" \
                "BR2_ROOTFS_OVERLAY=$rootfs_overlay_list" "$@"
        )
    else
        "$project_dir/docker-build.sh" env \
            "A333_RAUC_BUNDLE_NAME=$A333_RAUC_BUNDLE_NAME" \
            "A333_OEM_OVERLAYS=$oem_overlay_list" \
            "${PROFILE_ENV[@]}" \
            make "${make_args[@]}" \
            "BR2_ROOTFS_OVERLAY=$rootfs_overlay_list" "$@"
    fi
}
sync_overlay_config() {
    local config_file="$project_dir/$OUTPUT_DIR/.config"
    local expected="BR2_ROOTFS_OVERLAY=\"$rootfs_overlay_list\""
    [[ -f $config_file ]] || return 0
    if ! grep -Fxq "$expected" "$config_file"; then
        if ! grep -q '^BR2_ROOTFS_OVERLAY=' "$config_file"; then
            echo "Missing BR2_ROOTFS_OVERLAY in $config_file" >&2
            exit 1
        fi
        sed -i "s|^BR2_ROOTFS_OVERLAY=.*|$expected|" "$config_file"
        run_make olddefconfig
    fi
}
if [[ $action == clean ]]; then
    if [[ -f $project_dir/$OUTPUT_DIR/.config ]]; then
        run_make clean
    else
        echo "Nothing to clean for $profile: no configured output in $OUTPUT_DIR"
    fi
    exit 0
fi
if [[ $action == configure || ( $action == build && ! -f $project_dir/$OUTPUT_DIR/.config ) ]]; then
    run_make "$DEFCONFIG"
fi
sync_overlay_config
printf '%s\n' "$overlay_selection" > "$selection_file"
case "$action" in
    configure) ;;
    build) run_make "$@" ;;
    menuconfig|savedefconfig) run_make "$action" "$@" ;;
esac
