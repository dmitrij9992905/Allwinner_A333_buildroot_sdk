#!/bin/sh
# Print only logging-related fw_setenv -s entries; never change A/B selection.
set -eu
PROJECT_ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
case "${1:-}" in
    quiet) file="$PROJECT_ROOT/configs/boards/a333/helperboard-a333/env-ab.cfg" ;;
    debug) file="$PROJECT_ROOT/configs/boards/a333/helperboard-a333/env-ab-kernel-debug-logs.cfg" ;;
    *) echo "Usage: $0 quiet|debug" >&2; exit 2 ;;
esac
awk -F= '$1 ~ /^(earlyprintk|initcall_debug|loglevel|setargs_mmc|setargs_nand|setargs_nand_ubi)$/ {
    key=$1; sub(/^[^=]*=/, ""); print key " " $0
}' "$file"
