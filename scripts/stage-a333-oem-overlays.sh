#!/bin/sh
set -eu

OEM_ROOT=${1:?expected generated OEM staging directory}
OVERLAYS=${2-}
PROJECT_ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)

# This script may replace only the known generated OEM staging directory.
case "$OEM_ROOT" in
    */build/a333-oem-root) ;;
    *) echo "OEM staging target must end in /build/a333-oem-root" >&2; exit 2 ;;
esac

# Check the entire list before replacing any previous output.
if [ -n "$OVERLAYS" ]; then
    old_ifs=$IFS
    IFS=:
    set -f
    for overlay in $OVERLAYS; do
        case "$overlay" in
            ''|/*|*..*|*[!a-zA-Z0-9_./-]*)
                echo "Invalid OEM overlay: $overlay" >&2; exit 2 ;;
        esac
        [ -d "$PROJECT_ROOT/$overlay" ] || {
            echo "Missing OEM overlay: $overlay" >&2; exit 2;
        }
    done
    IFS=$old_ifs
fi

rm -rf -- "$OEM_ROOT"
install -d -m 0755 "$OEM_ROOT"
if [ -n "$OVERLAYS" ]; then
    IFS=:
    for overlay in $OVERLAYS; do
        cp -a "$PROJECT_ROOT/$overlay/." "$OEM_ROOT/"
    done
    IFS=$old_ifs
fi
