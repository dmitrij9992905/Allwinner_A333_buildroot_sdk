#!/bin/sh
set -eu
# Bundle-supplied hook works even on older rootfs versions. Apply after OEM
# (the last image), so a failed boot/rootfs/OEM copy leaves logging unchanged.
[ "${1:-}" = slot-post-install ] || exit 1
[ "${RAUC_SLOT_CLASS:-}" = oem ] || exit 1
bundle_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
if [ "$(fw_printenv -n systemA)" != bootA ] ||
   [ "$(fw_printenv -n systemB)" != bootB ]; then
    echo "kernel-logging hook: invalid A333 boot environment" >&2
    exit 1
fi
fw_setenv -s "$bundle_dir/kernel-logging.env"
echo "kernel-logging hook: installed next-boot logging policy"
