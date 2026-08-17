#!/bin/sh
set -eu

TARGET_DIR="$1"
PROJECT_ROOT="$(CDPATH= cd -- "$(dirname -- "$0")/../../../.." && pwd)"
KEY_HELPER="$PROJECT_ROOT/scripts/ensure-a333-rauc-keys.sh"

if [ ! -x "$KEY_HELPER" ]; then
	echo "post-build: missing executable $KEY_HELPER" >&2
	exit 1
fi

set -- $(A333_PROJECT_ROOT="$PROJECT_ROOT" "$KEY_HELPER")
CERT_FILE="$2"

install -d -m 0755 "$TARGET_DIR/etc/rauc"
install -m 0644 "$PROJECT_ROOT/configs/boards/a333/helperboard-a333/rauc/system.conf" \
	"$TARGET_DIR/etc/rauc/system.conf"
install -m 0644 "$CERT_FILE" "$TARGET_DIR/etc/rauc/ca.cert.pem"
