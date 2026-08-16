#!/usr/bin/env bash
set -euo pipefail

project_dir="$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)"
image_name="allwinner-a333-buildroot:2025.02.16"
build_uid="$(id -u)"
build_gid="$(id -g)"

run_args=(
    run --rm --init
    --env "BUILD_UID=${build_uid}"
    --env "BUILD_GID=${build_gid}"
    --env BR2_DL_DIR=/workspace/dl
    --volume "${project_dir}:/workspace"
    --workdir /workspace
)

if [[ $# -eq 0 ]]; then
    exec docker "${run_args[@]}" --interactive --tty "$image_name" bash
fi

if [[ "$1" == "build" ]]; then
    shift
    exec docker build \
        --build-arg UBUNTU_VERSION=24.04 \
        --tag "$image_name" \
        "$project_dir" "$@"
fi

exec docker "${run_args[@]}" "$image_name" "$@"
