#!/usr/bin/env bash
set -euo pipefail

project_dir="$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)"
invocation=("$@")
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
KERNEL_LOGGING=quiet
# shellcheck source=/dev/null
source "$profile_file"
: "${DEFCONFIG:?profile must set DEFCONFIG}"
case "$KERNEL_LOGGING" in
    quiet)
        logging_fragment=../configs/boards/a333/linux-quiet-logs.fragment
        uboot_env=../configs/boards/a333/helperboard-a333/env-ab.cfg
        logging_config='# BR2_A333_KERNEL_DEBUG_LOGS is not set'
        ;;
    debug)
        logging_fragment=../configs/boards/a333/linux-kernel-debug-logs.fragment
        uboot_env=../configs/boards/a333/helperboard-a333/env-ab-kernel-debug-logs.cfg
        logging_config=BR2_A333_KERNEL_DEBUG_LOGS=y
        ;;
    *) echo "Invalid KERNEL_LOGGING in $profile: $KERNEL_LOGGING" >&2; exit 2 ;;
esac
kernel_fragments="../configs/boards/a333/linux-no-btf.fragment $logging_fragment"
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

interactive_config=0
for target in "$action" "$@"; do
    case "$target" in
        menuconfig|nconfig|*-menuconfig|*-nconfig) interactive_config=1 ;;
    esac
done
if [[ $interactive_config == 1 && ( ! -t 0 || ! -t 1 ) ]]; then
    echo "Interactive configuration requires a terminal for stdin and stdout; run without pipes/redirection (use ssh -t for remote sessions)." >&2
    exit 2
fi

build_context=host
if [[ ${A333_DOCKER_BUILD:-0} == 1 || ( -f /.dockerenv && $project_dir == /workspace ) ]]; then
    build_context=container
fi
if [[ $build_context == container ]]; then
    echo "build.sh: $profile / $action — using the current build container" >&2
else
    echo "build.sh: $profile / $action — running in Docker via docker-build.sh" >&2
    # Run the entire workflow in one container, not a new container per make
    # call. Configuration migration, cleanup and stamp updates use the same
    # environment as the requested build/configurator.
    exec "$project_dir/docker-build.sh" /workspace/build.sh "${invocation[@]}"
fi
if [[ $action != clean && -f $selection_file &&
      $(<"$selection_file") != "$overlay_selection" &&
      -d $project_dir/$OUTPUT_DIR/target ]]; then
    echo "Overlay stack changed for $OUTPUT_DIR; run '$0 $profile clean' or choose a fresh OUTPUT_DIR" >&2
    exit 2
fi

make_args=("O=../$OUTPUT_DIR" "BR2_EXTERNAL=../configs")
export A333_RAUC_BUNDLE_NAME
run_make() {
    (
        cd "$project_dir/buildroot"
        env \
            "A333_RAUC_BUNDLE_NAME=$A333_RAUC_BUNDLE_NAME" \
            "A333_OEM_OVERLAYS=$oem_overlay_list" \
            "${PROFILE_ENV[@]}" \
            make "${make_args[@]}" \
            "BR2_ROOTFS_OVERLAY=$rootfs_overlay_list" "$@"
    )
}
invalidate_package_config() {
    # Buildroot does not automatically reconfigure packages when fragment
    # filenames change. Remove only rebuild stamps, preserving sources/cache.
    local dir="$project_dir/$OUTPUT_DIR/build/$1-custom"
    [[ -d $dir ]] || return 0
    rm -f "$dir/.stamp_dotconfig" "$dir/.stamp_configured" "$dir/.stamp_built" \
        "$dir/.stamp_installed" "$dir/.stamp_target_installed" \
        "$dir/.stamp_staging_installed" "$dir/.stamp_images_installed" \
        "$dir/.stamp_host_installed"
}
sync_profile_config() {
    local config_file="$project_dir/$OUTPUT_DIR/.config"
    local expected="BR2_ROOTFS_OVERLAY=\"$rootfs_overlay_list\""
    local changed=0
    [[ -f $config_file ]] || return 0
    # Vendor U-Boot disables its bundled dtc build. Ask Buildroot to build
    # host-dtc and pass DTC explicitly, including for existing output trees.
    if ! grep -Fxq 'BR2_TARGET_UBOOT_NEEDS_DTC=y' "$config_file"; then
        sed -i '/^BR2_TARGET_UBOOT_NEEDS_DTC=/d; /^# BR2_TARGET_UBOOT_NEEDS_DTC is not set$/d' "$config_file"
        printf 'BR2_TARGET_UBOOT_NEEDS_DTC=y\n' >> "$config_file"
        invalidate_package_config uboot
        changed=1
    fi
    if ! grep -Fxq "$expected" "$config_file"; then
        if ! grep -q '^BR2_ROOTFS_OVERLAY=' "$config_file"; then
            echo "Missing BR2_ROOTFS_OVERLAY in $config_file" >&2
            exit 1
        fi
        sed -i "s|^BR2_ROOTFS_OVERLAY=.*|$expected|" "$config_file"
        changed=1
    fi
    if ! grep -Fxq "BR2_LINUX_KERNEL_CONFIG_FRAGMENT_FILES=\"$kernel_fragments\"" "$config_file"; then
        grep -q '^BR2_LINUX_KERNEL_CONFIG_FRAGMENT_FILES=' "$config_file" ||
            printf 'BR2_LINUX_KERNEL_CONFIG_FRAGMENT_FILES=""\n' >> "$config_file"
        sed -i "s|^BR2_LINUX_KERNEL_CONFIG_FRAGMENT_FILES=.*|BR2_LINUX_KERNEL_CONFIG_FRAGMENT_FILES=\"$kernel_fragments\"|" "$config_file"
        invalidate_package_config linux
        # The old ordinary profile used this same env filename with verbose
        # contents. Rebuild its compiled fallback too during that migration.
        invalidate_package_config uboot
        changed=1
    fi
    if ! grep -Fxq "BR2_TARGET_UBOOT_DEFAULT_ENV_FILE=\"$uboot_env\"" "$config_file"; then
        grep -q '^BR2_TARGET_UBOOT_DEFAULT_ENV_FILE=' "$config_file" ||
            printf 'BR2_TARGET_UBOOT_DEFAULT_ENV_FILE=""\n' >> "$config_file"
        sed -i "s|^BR2_TARGET_UBOOT_DEFAULT_ENV_FILE=.*|BR2_TARGET_UBOOT_DEFAULT_ENV_FILE=\"$uboot_env\"|" "$config_file"
        invalidate_package_config uboot
        changed=1
    fi
    if ! grep -Fxq "$logging_config" "$config_file"; then
        sed -i '/^BR2_A333_KERNEL_DEBUG_LOGS=/d; /^# BR2_A333_KERNEL_DEBUG_LOGS is not set$/d' "$config_file"
        printf '%s\n' "$logging_config" >> "$config_file"
        invalidate_package_config linux
        invalidate_package_config uboot
        changed=1
    fi
    if [[ $changed == 1 ]]; then run_make olddefconfig; fi
    # Also cover explicit 'configure': it replaces .config before the checks
    # above, while the old kernel/U-Boot may still have valid build stamps.
    if [[ $action == configure ]]; then
        invalidate_package_config linux
        invalidate_package_config uboot
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
if [[ $action == build && $interactive_config == 0 ]]; then
    uboot_toolchain_dir="$project_dir/toolchains/gcc-linaro-7.2.1-2017.11-x86_64_arm-linux-gnueabi"
    for tool in gcc as ld ar nm strip objcopy objdump; do
        if [[ ! -x $uboot_toolchain_dir/bin/arm-linux-gnueabi-$tool ]]; then
            echo "Missing or non-executable vendor U-Boot tool: $uboot_toolchain_dir/bin/arm-linux-gnueabi-$tool" >&2
            echo "Copy the complete Linaro 7.2.1 toolchain into $uboot_toolchain_dir (see README_RU.md / README.md)." >&2
            exit 1
        fi
    done
    for tool in gcc as; do
        if ! "$uboot_toolchain_dir/bin/arm-linux-gnueabi-$tool" --version >/dev/null; then
            echo "Cannot run vendor U-Boot tool: $uboot_toolchain_dir/bin/arm-linux-gnueabi-$tool; check the toolchain files and host runtime libraries." >&2
            exit 1
        fi
    done
    "$project_dir/scripts/prepare-vendor-sources.sh"
fi
if [[ $action == configure || ! -f $project_dir/$OUTPUT_DIR/.config ]]; then
    run_make "$DEFCONFIG"
fi
sync_profile_config
printf '%s\n' "$overlay_selection" > "$selection_file"
case "$action" in
    configure) ;;
    build) run_make "$@" ;;
    menuconfig|savedefconfig) run_make "$action" "$@" ;;
esac
