#!/bin/sh
set -eu

# Keep files created in mounted output/dl directories owned by the caller.
# Compose supplies these values from the host wrapper or defaults to 1000:1000.
build_uid="${BUILD_UID:-1000}"
build_gid="${BUILD_GID:-1000}"

if [ "$(id -u)" -eq 0 ]; then
    if ! getent group "${build_gid}" >/dev/null 2>&1; then
        groupadd --gid "${build_gid}" buildroot
    fi

    if ! getent passwd "${build_uid}" >/dev/null 2>&1; then
        useradd \
            --uid "${build_uid}" \
            --gid "${build_gid}" \
            --create-home \
            --shell /bin/bash \
            buildroot
    fi

    build_user="$(getent passwd "${build_uid}" | cut -d: -f1)"
    exec gosu "${build_user}" "$@"
fi

exec "$@"
