#!/bin/sh
set -eu

BINARIES_DIR="$1"
PROJECT_ROOT="${A333_PROJECT_ROOT:-$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)}"
OUTPUT_DIR="$(CDPATH= cd -- "$(dirname -- "$BINARIES_DIR")" && pwd)"
HOST_DIR="${HOST_DIR:-$OUTPUT_DIR/host}"
RAUC="$HOST_DIR/bin/rauc"
KEY_HELPER="$PROJECT_ROOT/scripts/ensure-a333-rauc-keys.sh"
BUNDLE_DIR="$BINARIES_DIR/a333-rauc-content"
VERSION="${A333_RAUC_VERSION:-$(date -u +%Y.%m.%d-%H%M)}"
BUNDLE_NAME="${A333_RAUC_BUNDLE_NAME:-a333-helperboard.raucb}"
CONFIG_FILE="${BR2_CONFIG:-$OUTPUT_DIR/.config}"
RAUC_COMPATIBLE='Allwinner A333 HelperBoard'
if grep -q '^BR2_A333_PROFILE_MEDIA=y$' "$CONFIG_FILE"; then
	RAUC_COMPATIBLE='Allwinner A333 Media'
fi
if grep -q '^BR2_A333_PROFILE_HEADLESS=y$' "$CONFIG_FILE"; then
	RAUC_COMPATIBLE='Allwinner A333 Headless'
fi

if [ "${A333_RAUC_BUNDLE:-1}" = "0" ]; then
	echo "post-image: RAUC bundle generation disabled (A333_RAUC_BUNDLE=0)" >&2
	exit 0
fi

for image in boot.img rootfs.ext4 oem.ext4; do
	if [ ! -f "$BINARIES_DIR/$image" ]; then
		echo "post-image: missing $BINARIES_DIR/$image for RAUC bundle" >&2
		exit 1
	fi
done

if [ ! -x "$RAUC" ]; then
	echo "post-image: missing host RAUC tool: $RAUC" >&2
	exit 1
fi

set -- $(A333_PROJECT_ROOT="$PROJECT_ROOT" "$KEY_HELPER")
KEY_FILE="$1"
CERT_FILE="$2"

rm -rf "$BUNDLE_DIR"
mkdir -p "$BUNDLE_DIR"
cp -f "$BINARIES_DIR/boot.img" "$BUNDLE_DIR/boot.img"
cp -f "$BINARIES_DIR/rootfs.ext4" "$BUNDLE_DIR/rootfs.ext4"
cp -f "$BINARIES_DIR/oem.ext4" "$BUNDLE_DIR/oem.ext4"

cat > "$BUNDLE_DIR/manifest.raucm" <<EOF
[update]
compatible=$RAUC_COMPATIBLE
description=Allwinner A333 Buildroot A/B update
version=$VERSION

[bundle]
format=verity

[image.boot]
filename=boot.img

[image.rootfs]
filename=rootfs.ext4

[image.oem]
filename=oem.ext4
EOF

rm -f "$BINARIES_DIR/$BUNDLE_NAME"
"$RAUC" bundle \
	--cert="$CERT_FILE" \
	--key="$KEY_FILE" \
	"$BUNDLE_DIR" "$BINARIES_DIR/$BUNDLE_NAME"
rm -rf "$BUNDLE_DIR"
echo "post-image: created $BINARIES_DIR/$BUNDLE_NAME" >&2
