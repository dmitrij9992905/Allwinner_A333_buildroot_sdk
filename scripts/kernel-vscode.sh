#!/usr/bin/env bash
# Export this profile's kernel UAPI and real Kbuild commands for host VSCode.
set -euo pipefail
project_dir="$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)"
profile=${1:-media}
if [[ $# -gt 1 || ! $profile =~ ^[a-z0-9][a-z0-9_-]*$ ||
      ! -f $project_dir/profiles/$profile.conf ]]; then
    echo "Usage: bash $0 [profile]" >&2; exit 2
fi
source "$project_dir/profiles/$profile.conf"
if [[ ! $OUTPUT_DIR =~ ^output/[a-zA-Z0-9_/-]+$ || $OUTPUT_DIR == *..* ]]; then
    echo "Invalid profile output directory" >&2; exit 2
fi
dev_dir="$project_dir/output/dev/kernel/$profile"
if [[ $project_dir != /workspace && -z ${A333_SDK_DIR:-} ]]; then
    "$project_dir/docker-build.sh" bash /workspace/scripts/kernel-vscode.sh "$profile"
else
    kernel="$project_dir/$OUTPUT_DIR/build/linux-custom"
    sdk=${A333_SDK_DIR:-"$project_dir/$OUTPUT_DIR/host"}
    if [[ ! -f $kernel/include/generated/autoconf.h ||
          ! -x $sdk/bin/aarch64-buildroot-linux-gnu-gcc ]]; then
        echo "Build the $profile kernel/toolchain first: ./build.sh $profile build linux" >&2
        exit 1
    fi
    mkdir -p "$dev_dir"
    # headers_install removes kernel-private definitions and generates ARM64 asm.
    PATH="$sdk/bin:$PATH" make -s -C "$kernel" ARCH=arm64 \
        CROSS_COMPILE="$sdk/bin/aarch64-buildroot-linux-gnu-" \
        INSTALL_HDR_PATH="$dev_dir/headers" headers_install
    python3 "$kernel/scripts/clang-tools/gen_compile_commands.py" \
        -d "$kernel" -o "$dev_dir/compile_commands.json"
fi
python3 "$project_dir/scripts/remap-compile-commands.py" \
    "$dev_dir/compile_commands.json" "$dev_dir/compile_commands.host.json" \
    /workspace "$project_dir"
echo "VSCode kernel database and ARM64 UAPI: $dev_dir"
