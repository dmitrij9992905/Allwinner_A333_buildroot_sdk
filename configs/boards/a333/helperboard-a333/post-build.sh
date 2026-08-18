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

# Keep business logic in the separately generated OEM filesystem.  The
# Buildroot package installs into TARGET_DIR first so the normal package
# dependency/build machinery can be used; move the resulting payload before
# the rootfs and OEM images are generated.
OEM_ROOT="$PROJECT_ROOT/oem/a333/rootfs"
if [ -x "$TARGET_DIR/usr/bin/dmx-panel" ] &&
   [ -f "$TARGET_DIR/usr/share/dmx-panel/VERSION" ]; then
	install -d -m 0755 "$OEM_ROOT/usr/bin" "$OEM_ROOT/usr/share/dmx-panel"
	install -m 0755 "$TARGET_DIR/usr/bin/dmx-panel" "$OEM_ROOT/usr/bin/dmx-panel"
	install -m 0644 "$TARGET_DIR/usr/share/dmx-panel/VERSION" \
		"$OEM_ROOT/usr/share/dmx-panel/VERSION"
	rm -f "$TARGET_DIR/usr/bin/dmx-panel" \
		"$TARGET_DIR/usr/share/dmx-panel/VERSION"
fi

if [ -x "$OEM_ROOT/usr/bin/dmx-panel" ] &&
   [ -f "$OEM_ROOT/usr/share/dmx-panel/VERSION" ]; then
	install -d -m 0755 "$TARGET_DIR/etc/systemd/system/multi-user.target.wants"
	ln -sfn ../dmx-panel.service \
		"$TARGET_DIR/etc/systemd/system/multi-user.target.wants/dmx-panel.service"
else
	rm -f "$TARGET_DIR/etc/systemd/system/multi-user.target.wants/dmx-panel.service"
fi

# Enable the display demo and TCP ADB access in the systemd image.  Both
# services are intentionally part of the demo image; the dmx-panel unit uses
# --simulate and therefore never opens the RS485 device.
install -d -m 0755 "$TARGET_DIR/etc/systemd/system/multi-user.target.wants"
if [ -x "$TARGET_DIR/usr/bin/adbd" ]; then
	ln -sfn ../adbd.service \
		"$TARGET_DIR/etc/systemd/system/multi-user.target.wants/adbd.service"
else
	rm -f "$TARGET_DIR/etc/systemd/system/multi-user.target.wants/adbd.service"
fi
