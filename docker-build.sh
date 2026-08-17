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
    --env BR2_CCACHE_DIR=/workspace/ccache
    --volume "${project_dir}:/workspace"
    --workdir /workspace/buildroot
)

# Optional RAUC settings are passed through when supplied by the caller. Use
# paths visible inside the container (for example /workspace/keys/prod.key.pem).
for env_name in A333_RAUC_KEY A333_RAUC_CERT A333_RAUC_KEY_DIR \
    A333_RAUC_BUNDLE A333_RAUC_VERSION A333_RAUC_BUNDLE_NAME; do
    if [[ -v "$env_name" ]]; then
        run_args+=(--env "$env_name=${!env_name}")
    fi
done

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

if [[ "$1" == "prepare-sources" ]]; then
    shift
    exec docker "${run_args[@]}" "$image_name" \
        /workspace/scripts/prepare-vendor-sources.sh "$@"
fi

exec docker "${run_args[@]}" "$image_name" "$@"
