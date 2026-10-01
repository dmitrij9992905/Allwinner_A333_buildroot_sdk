#!/usr/bin/env bash
set -euo pipefail
project_dir="$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)"
action=${1:-build}
profile=${2:-${A333_DEV_PROFILE:-media}}
target=${3:-${A333_TARGET:-root@192.168.0.144}}
usage() {
    echo "Usage: $0 {configure|build|deploy|run|cycle|logs} [media|media-kernel-debug-logs] [user@host]"
    echo "Build: A333_SDK_DIR=<exported SDK> for native CMake, otherwise docker-build.sh."
    echo "Build options: A333_DEV_BUILD_TYPE=RelWithDebInfo A333_DEV_LVGL_LOGGING=OFF A333_DEV_G2D=ON A333_DEV_JOBS=<n>"
    echo "FPS overlay: A333_DEV_PERF_MONITOR=ON|OFF (default ON)"
    echo "Run options: A333_DEV_RENDERER=auto|software|g2d A333_DEV_PANEL_PROFILE=0|1"
}
case "$action" in
    -h|--help) usage; exit 0 ;;
    configure|build|deploy|run|cycle|logs) ;;
    *) usage >&2; exit 2 ;;
esac
if [[ $# -gt 3 || ! $profile =~ ^[a-z0-9][a-z0-9_-]*$ ||
      ! -f $project_dir/profiles/$profile.conf ]]; then
    usage >&2; exit 2
fi
# Project-maintained profiles, not arbitrary user scripts.
source "$project_dir/profiles/$profile.conf"
if ! grep -q '^BR2_A333_PROFILE_MEDIA=y$' "$project_dir/configs/configs/$DEFCONFIG"; then
    echo "Profile $profile does not contain media-panel" >&2; exit 2
fi
if [[ ! $OUTPUT_DIR =~ ^output/[a-zA-Z0-9_/-]+$ || $OUTPUT_DIR == *..* ]]; then
    echo "Invalid profile output directory" >&2; exit 2
fi
dev_dir="$project_dir/output/dev/media-panel/$profile"
binary="$dev_dir/media-panel"
if [[ $action == cycle ]]; then
    bash "$0" build "$profile"
    bash "$0" deploy "$profile" "$target"
    exec bash "$0" run "$profile" "$target"
fi
case "$action" in
    configure|build)
        if [[ $project_dir != /workspace && -z ${A333_SDK_DIR:-} ]]; then
            "$project_dir/docker-build.sh" env \
                "A333_DEV_BUILD_TYPE=${A333_DEV_BUILD_TYPE:-RelWithDebInfo}" \
                "A333_DEV_LVGL_LOGGING=${A333_DEV_LVGL_LOGGING:-OFF}" \
                "A333_DEV_G2D=${A333_DEV_G2D:-ON}" \
                "A333_DEV_PERF_MONITOR=${A333_DEV_PERF_MONITOR:-ON}" \
                "A333_DEV_JOBS=${A333_DEV_JOBS:-$(nproc)}" \
                bash /workspace/scripts/media-panel-dev.sh "$action" "$profile"
            python3 "$project_dir/scripts/remap-compile-commands.py" \
                "$dev_dir/compile_commands.json" "$dev_dir/compile_commands.host.json" \
                /workspace "$project_dir"
            kernel_dev="$project_dir/output/dev/kernel/$profile"
            python3 "$project_dir/scripts/remap-compile-commands.py" \
                "$kernel_dev/compile_commands.json" "$kernel_dev/compile_commands.host.json" \
                /workspace "$project_dir"
            exit 0
        fi
        sdk_dir=${A333_SDK_DIR:-"$project_dir/$OUTPUT_DIR/host"}
        toolchain="$sdk_dir/share/buildroot/toolchainfile.cmake"
        if [[ ! -f $toolchain ]]; then
            echo "Missing SDK/toolchain: $toolchain. Build the $profile firmware first." >&2
            exit 1
        fi
        config="$project_dir/$OUTPUT_DIR/.config"
        [[ -f $config ]] || config="$project_dir/configs/configs/$DEFCONFIG"
        rotation=$(sed -n 's/^BR2_A333_DISPLAY_ROTATION=//p' "$config")
        build_type=${A333_DEV_BUILD_TYPE:-RelWithDebInfo}
        lvgl_logging=${A333_DEV_LVGL_LOGGING:-OFF}
        g2d=${A333_DEV_G2D:-ON}
        perf_monitor=${A333_DEV_PERF_MONITOR:-ON}
        jobs=${A333_DEV_JOBS:-$(nproc)}
        case "$build_type" in Debug|Release|RelWithDebInfo|MinSizeRel) ;; *) exit 2 ;; esac
        case "$lvgl_logging" in ON|OFF) ;; *) exit 2 ;; esac
        case "$g2d" in ON|OFF) ;; *) exit 2 ;; esac
        case "$perf_monitor" in ON|OFF) ;; *) exit 2 ;; esac
        [[ $jobs =~ ^[1-9][0-9]*$ ]] || exit 2
        # Use the firmware kernel's exported UAPI, not host /usr/include/linux.
        bash "$project_dir/scripts/kernel-vscode.sh" "$profile"
        cmake="$sdk_dir/bin/cmake"
        [[ -x $cmake ]] || cmake=cmake
        "$cmake" -S "$project_dir/oem/a333/src/media-panel" -B "$dev_dir" \
            -DCMAKE_TOOLCHAIN_FILE="$toolchain" -DCMAKE_BUILD_TYPE="$build_type" \
            -DMEDIA_PANEL_COMMON_DIR="$project_dir/oem/a333/src/panel-common" \
            -DPANEL_COMMON_KERNEL_UAPI_DIR="$project_dir/output/dev/kernel/$profile/headers/include" \
            -DMEDIA_PANEL_DISPLAY_ROTATION="$rotation" \
            -DMEDIA_PANEL_G2D="$g2d" \
            -DMEDIA_PANEL_PERF_MONITOR="$perf_monitor" \
            -DMEDIA_PANEL_LOGGING=ON -DMEDIA_PANEL_LVGL_LOGGING="$lvgl_logging"
        if [[ $action == build ]]; then "$cmake" --build "$dev_dir" --parallel "$jobs"; fi
        python3 "$project_dir/scripts/remap-compile-commands.py" \
            "$dev_dir/compile_commands.json" "$dev_dir/compile_commands.host.json" \
            /workspace "$project_dir"
        ;;
    deploy|run|logs)
        # SSH aliases are supported; put ports/options in ~/.ssh/config.
        [[ $target =~ ^[a-zA-Z0-9_][a-zA-Z0-9_.@-]*$ ]] || { echo "Invalid SSH target" >&2; exit 2; }
        case "$action" in
            deploy)
                [[ -f $binary ]] || { echo "Run '$0 build $profile' first" >&2; exit 1; }
                ssh "$target" 'mkdir -p /userdata/dev/media-panel'
                # Use legacy SCP protocol: the board need not run sftp-server.
                scp -O "$binary" "$target:/userdata/dev/media-panel/media-panel.new"
                ssh "$target" 'chmod 0755 /userdata/dev/media-panel/media-panel.new && mv -f /userdata/dev/media-panel/media-panel.new /userdata/dev/media-panel/media-panel'
                ;;
            run)
                renderer=${A333_DEV_RENDERER:-auto}
                profiling=${A333_DEV_PANEL_PROFILE:-0}
                case "$renderer" in auto|software|g2d) ;; *) echo "Invalid A333_DEV_RENDERER" >&2; exit 2 ;; esac
                case "$profiling" in 0|1) ;; *) echo "Invalid A333_DEV_PANEL_PROFILE" >&2; exit 2 ;; esac
                # Stop only the UI. Keep MPD/audio/Bluetooth/backend running.
                # Restore the stock UI on exit, Ctrl-C or SSH disconnect.
                ssh -tt "$target" "export A333_PANEL_RENDERER=$renderer A333_PANEL_PROFILE=$profiling;" 'set -eu
test -x /userdata/dev/media-panel/media-panel
was_active=0
if systemctl is-active --quiet a333-media.service; then was_active=1; fi
cleanup() {
    status=$?
    trap - EXIT HUP INT TERM
    if [ "$was_active" = 1 ]; then systemctl start a333-media.service || true; fi
    exit "$status"
}
trap cleanup EXIT
trap "exit 129" HUP
trap "exit 130" INT
trap "exit 143" TERM
systemctl stop a333-media.service
mkdir -p /run/a333-media
/userdata/dev/media-panel/media-panel'
                ;;
            logs)
                ssh -t "$target" 'journalctl -f -n 80 -u a333-media.service -u a333-media-backend.service'
                ;;
        esac
        ;;
esac
