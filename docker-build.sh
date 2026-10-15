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
    --env A333_DOCKER_BUILD=1
    --volume "${project_dir}:/workspace"
    --workdir /workspace/buildroot
)

# Commands such as menuconfig need both stdin and a terminal, not just the
# no-argument shell. Keep stdin usable for piped shell commands too, but do
# not allocate a TTY for redirected build logs or non-interactive CI jobs.
run_args+=(--interactive)
if [[ -t 0 && -t 1 ]]; then
    run_args+=(--tty --env "TERM=${TERM:-xterm-256color}")
fi

# Optional RAUC settings are passed through when supplied by the caller. Use
# paths visible inside the container (for example /workspace/keys/prod.key.pem).
for env_name in A333_RAUC_KEY A333_RAUC_CERT A333_RAUC_KEY_DIR \
    A333_RAUC_BUNDLE A333_RAUC_VERSION A333_RAUC_BUNDLE_NAME A333_CREDENTIALS_ROOT; do
    if [[ -v "$env_name" ]]; then
        run_args+=(--env "$env_name=${!env_name}")
    fi
done

if [[ $# -eq 0 ]]; then
    exec docker "${run_args[@]}" "$image_name" bash
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
