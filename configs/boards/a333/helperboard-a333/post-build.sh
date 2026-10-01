#!/bin/sh
set -eu

TARGET_DIR="$1"
PROJECT_ROOT="$(CDPATH= cd -- "$(dirname -- "$0")/../../../.." && pwd)"
KEY_HELPER="$PROJECT_ROOT/scripts/ensure-a333-rauc-keys.sh"
SDK_FIRMWARE="$PROJECT_ROOT/vendor/allwinner-a333/firmware/aic8800d80"
OUTPUT_DIR="$(CDPATH= cd -- "$(dirname -- "$TARGET_DIR")" && pwd)"
CONFIG_FILE="${BR2_CONFIG:-$OUTPUT_DIR/.config}"
DISPLAY_ROTATION="${A333_DISPLAY_ROTATION:-}"
OEM_ROOT="$OUTPUT_DIR/build/a333-oem-root"
PROFILE=dmx
if grep -q '^BR2_A333_PROFILE_MEDIA=y$' "$CONFIG_FILE"; then PROFILE=media; fi
if grep -q '^BR2_A333_PROFILE_HEADLESS=y$' "$CONFIG_FILE"; then PROFILE=headless; fi
if [ "${A333_OEM_OVERLAYS+x}" != x ]; then
	# Legacy direct Buildroot invocations still get the profile's OEM payload.
	case "$PROFILE" in
		dmx) A333_OEM_OVERLAYS=oem/a333/common-rootfs:oem/a333/rootfs ;;
		media) A333_OEM_OVERLAYS=oem/a333/common-rootfs:oem/a333/media-rootfs ;;
		headless) A333_OEM_OVERLAYS=oem/a333/common-rootfs ;;
	esac
fi
"$PROJECT_ROOT/scripts/stage-a333-oem-overlays.sh" \
	"$OEM_ROOT" "$A333_OEM_OVERLAYS"

if [ ! -x "$KEY_HELPER" ]; then
	echo "post-build: missing executable $KEY_HELPER" >&2
	exit 1
fi

if [ -z "$DISPLAY_ROTATION" ] && [ -f "$CONFIG_FILE" ]; then
	DISPLAY_ROTATION="$(sed -n 's/^BR2_A333_DISPLAY_ROTATION=//p' "$CONFIG_FILE")"
fi
DISPLAY_ROTATION="${DISPLAY_ROTATION:-90}"
case "$DISPLAY_ROTATION" in
	0)
		DISPLAY_ORIENTATION="portrait"
		LOGICAL_WIDTH=800
		LOGICAL_HEIGHT=1280
		;;
	90)
		DISPLAY_ORIENTATION="landscape-clockwise"
		LOGICAL_WIDTH=1280
		LOGICAL_HEIGHT=800
		;;
	180)
		DISPLAY_ORIENTATION="portrait-inverted"
		LOGICAL_WIDTH=800
		LOGICAL_HEIGHT=1280
		;;
	270)
		DISPLAY_ORIENTATION="landscape-counter-clockwise"
		LOGICAL_WIDTH=1280
		LOGICAL_HEIGHT=800
		;;
	*)
		echo "post-build: invalid display rotation: $DISPLAY_ROTATION" >&2
		exit 1
		;;
esac

install -d -m 0755 "$TARGET_DIR/etc/a333"
{
	printf 'orientation=%s\n' "$DISPLAY_ORIENTATION"
	printf 'rotation=%s\n' "$DISPLAY_ROTATION"
	printf 'native_width=800\n'
	printf 'native_height=1280\n'
	printf 'logical_width=%s\n' "$LOGICAL_WIDTH"
	printf 'logical_height=%s\n' "$LOGICAL_HEIGHT"
} > "$TARGET_DIR/etc/a333/display.conf"

# AIC8800D80 firmware is not part of the kernel module package.  Keep it in
# the vendor path expected by the SDK driver rather than relying on a host
# filesystem or a development image.
if [ ! -d "$SDK_FIRMWARE" ]; then
	echo "post-build: missing AIC firmware directory: $SDK_FIRMWARE" >&2
	exit 1
fi
install -d -m 0755 "$TARGET_DIR/vendor/etc/firmware"
find "$SDK_FIRMWARE" -maxdepth 1 -type f -exec install -m 0644 {} \
	"$TARGET_DIR/vendor/etc/firmware/" \;

set -- $(A333_PROJECT_ROOT="$PROJECT_ROOT" "$KEY_HELPER")
CERT_FILE="$2"

install -d -m 0755 "$TARGET_DIR/etc/rauc"
case "$PROFILE" in
	dmx) RAUC_COMPATIBLE='Allwinner A333 HelperBoard' ;;
	media) RAUC_COMPATIBLE='Allwinner A333 Media' ;;
	headless) RAUC_COMPATIBLE='Allwinner A333 Headless' ;;
esac
sed "s/^compatible=.*/compatible=$RAUC_COMPATIBLE/" \
	"$PROJECT_ROOT/configs/boards/a333/helperboard-a333/rauc/system.conf" \
	> "$TARGET_DIR/etc/rauc/system.conf"
install -m 0644 "$CERT_FILE" "$TARGET_DIR/etc/rauc/ca.cert.pem"

# Keep business logic in the separately generated OEM filesystem.  The
# Buildroot package installs into TARGET_DIR first so the normal package
# dependency/build machinery can be used; move the resulting payload before
# the rootfs and OEM images are generated.
if [ -x "$TARGET_DIR/usr/bin/dmx-panel" ] &&
   [ -f "$TARGET_DIR/usr/share/dmx-panel/VERSION" ]; then
	install -d -m 0755 "$OEM_ROOT/usr/bin" "$OEM_ROOT/usr/share/dmx-panel"
	install -m 0755 "$TARGET_DIR/usr/bin/dmx-panel" "$OEM_ROOT/usr/bin/dmx-panel"
	install -m 0644 "$TARGET_DIR/usr/share/dmx-panel/VERSION" \
		"$OEM_ROOT/usr/share/dmx-panel/VERSION"
	rm -f "$TARGET_DIR/usr/bin/dmx-panel" \
		"$TARGET_DIR/usr/share/dmx-panel/VERSION"
fi

if [ "$PROFILE" = media ]; then
	MEDIA_PANEL_BINARY="$TARGET_DIR/usr/bin/media-panel"
	# Finalization moves the executable out of rootfs. On incremental builds
	# Buildroot may skip package installation; reuse this profile's build output.
	if [ ! -x "$MEDIA_PANEL_BINARY" ]; then
		MEDIA_PANEL_BINARY="$OUTPUT_DIR/build/media-panel-1.0.0/media-panel/buildroot-build/media-panel"
	fi
	if [ ! -x "$MEDIA_PANEL_BINARY" ]; then
		echo "post-build: media profile requires media-panel binary" >&2
		exit 1
	fi
	install -d -m 0755 "$OEM_ROOT/usr/bin"
	install -m 0755 "$MEDIA_PANEL_BINARY" "$OEM_ROOT/usr/bin/media-panel"
	rm -f "$TARGET_DIR/usr/bin/media-panel"
	install -d -m 0755 "$TARGET_DIR/etc/systemd/system/multi-user.target.wants"
	# Upstream defaults start another org.bluealsa owner and another ALSA
	# player. The media profile supplies its own HF/A2DP receiver and player.
	for upstream_service in bluealsa.service bluealsa-aplay.service mpd.socket; do
		rm -f "$TARGET_DIR/etc/systemd/system/multi-user.target.wants/$upstream_service" \
			"$TARGET_DIR/etc/systemd/system/sockets.target.wants/$upstream_service"
		ln -sfn /dev/null "$TARGET_DIR/etc/systemd/system/$upstream_service"
	done
	for service in a333-audio-init.service bluealsad.service bluealsa-playback.service mpd.service \
		a333-media.service a333-media-backend.service; do
		if [ ! -f "$TARGET_DIR/etc/systemd/system/$service" ]; then
			echo "post-build: media rootfs overlay missing $service" >&2
			exit 1
		fi
		ln -sfn "../$service" \
			"$TARGET_DIR/etc/systemd/system/multi-user.target.wants/$service"
	done
fi
if [ "$PROFILE" = headless ]; then
	rm -f "$TARGET_DIR/etc/systemd/system/multi-user.target.wants/dmx-panel.service"
fi

if [ -x "$OEM_ROOT/usr/bin/dmx-panel" ] &&
   [ -f "$OEM_ROOT/usr/share/dmx-panel/VERSION" ]; then
	if [ ! -f "$TARGET_DIR/etc/systemd/system/dmx-panel.service" ]; then
		echo "post-build: dmx rootfs overlay missing dmx-panel.service" >&2
		exit 1
	fi
	install -d -m 0755 "$TARGET_DIR/etc/systemd/system/multi-user.target.wants"
	ln -sfn ../dmx-panel.service \
		"$TARGET_DIR/etc/systemd/system/multi-user.target.wants/dmx-panel.service"
else
	rm -f "$TARGET_DIR/etc/systemd/system/multi-user.target.wants/dmx-panel.service"
fi

# Enable the display demo, persistent storage, remote access and networking.
install -d -m 0755 "$TARGET_DIR/etc/systemd/system/multi-user.target.wants"
install -d -m 0755 "$TARGET_DIR/etc/systemd/system/local-fs.target.wants"
install -d -m 0755 "$TARGET_DIR/etc/systemd/system/sysinit.target.wants"
if [ "$PROFILE" != headless ]; then
    if [ ! -f "$TARGET_DIR/etc/systemd/system/a333-touch-diag.service" ] ||
       [ ! -x "$TARGET_DIR/usr/sbin/a333-touch-diag" ]; then
        echo "post-build: display rootfs overlay missing touch diagnostics" >&2
        exit 1
    fi
    ln -sfn ../a333-touch-diag.service \
        "$TARGET_DIR/etc/systemd/system/multi-user.target.wants/a333-touch-diag.service"
else
    rm -f "$TARGET_DIR/etc/systemd/system/multi-user.target.wants/a333-touch-diag.service"
fi
ln -sfn ../a333-userdata.service \
    "$TARGET_DIR/etc/systemd/system/multi-user.target.wants/a333-userdata.service"
ln -sfn ../a333-ble-wifi.service \
    "$TARGET_DIR/etc/systemd/system/multi-user.target.wants/a333-ble-wifi.service"
ln -sfn ../a333-bluetooth-uart.service \
    "$TARGET_DIR/etc/systemd/system/multi-user.target.wants/a333-bluetooth-uart.service"

rm -f "$TARGET_DIR/etc/systemd/system/network.service" \
    "$TARGET_DIR/etc/systemd/system/multi-user.target.wants/network.service"
rm -f "$TARGET_DIR/etc/systemd/system/multi-user.target.wants/systemd-networkd.service" \
    "$TARGET_DIR/etc/systemd/system/multi-user.target.wants/wpa_supplicant@wlan0.service"

# systemd installs this tmpfiles snippet even when networkd is disabled. Its
# systemd-network owner is intentionally absent in that configuration, which
# makes systemd-tmpfiles --create fail while generating the root filesystem.
rm -f "$TARGET_DIR/usr/lib/tmpfiles.d/systemd-network.conf"

# Override upstream presets that otherwise re-enable networkd after this script.
install -d -m 0755 "$TARGET_DIR/usr/lib/systemd/system-preset"
{
    printf '%s\n' \
        'disable network.service' \
        'disable systemd-network-generator.service' \
        'disable systemd-networkd.service' \
        'disable systemd-networkd.socket' \
        'disable systemd-networkd-wait-online.service' \
        'disable systemd-networkd-wait-online@.service' \
        'enable wpa_supplicant.service' \
        'enable NetworkManager.service' \
        'disable NetworkManager-wait-online.service' \
        'enable systemd-resolved.service' \
        'enable bluetooth.service' \
        'enable a333-bluetooth-uart.service' \
        'enable a333-ble-wifi.service' \
        'enable sshd.service'
} > "$TARGET_DIR/usr/lib/systemd/system-preset/00-a333.preset"

if [ -f "$TARGET_DIR/usr/lib/systemd/system/NetworkManager.service" ]; then
    ln -sfn /usr/lib/systemd/system/NetworkManager.service \
        "$TARGET_DIR/etc/systemd/system/multi-user.target.wants/NetworkManager.service"
else
    rm -f "$TARGET_DIR/etc/systemd/system/multi-user.target.wants/NetworkManager.service"
fi

# NetworkManager uses the shared D-Bus supplicant (-u), not a per-interface
# standalone supplicant with a competing network configuration.
if [ -f "$TARGET_DIR/usr/lib/systemd/system/wpa_supplicant.service" ]; then
    ln -sfn /usr/lib/systemd/system/wpa_supplicant.service \
        "$TARGET_DIR/etc/systemd/system/multi-user.target.wants/wpa_supplicant.service"
fi

ln -sfn /dev/null "$TARGET_DIR/etc/systemd/system/NetworkManager-wait-online.service"
rm -f "$TARGET_DIR/etc/systemd/system/network-online.target.wants/NetworkManager-wait-online.service"

if [ -f "$TARGET_DIR/usr/lib/systemd/system/systemd-resolved.service" ]; then
    ln -sfn /usr/lib/systemd/system/systemd-resolved.service \
        "$TARGET_DIR/etc/systemd/system/sysinit.target.wants/systemd-resolved.service"
fi

if [ -f "$TARGET_DIR/usr/lib/systemd/system/bluetooth.service" ]; then
    ln -sfn /usr/lib/systemd/system/bluetooth.service \
        "$TARGET_DIR/etc/systemd/system/multi-user.target.wants/bluetooth.service"
fi

if [ -x "$TARGET_DIR/usr/sbin/sshd" ]; then
    ln -sfn ../sshd.service \
        "$TARGET_DIR/etc/systemd/system/multi-user.target.wants/sshd.service"
else
    rm -f "$TARGET_DIR/etc/systemd/system/multi-user.target.wants/sshd.service"
fi

# NetworkManager profiles must survive A/B updates and remain writable when
# the base rootfs is made read-only. /var/lib is bind-mounted from userdata.
install -d -m 0755 "$TARGET_DIR/etc/NetworkManager"
install -d -m 0700 "$TARGET_DIR/var/lib/NetworkManager/system-connections"
rm -rf "$TARGET_DIR/etc/NetworkManager/system-connections"
ln -s /var/lib/NetworkManager/system-connections \
    "$TARGET_DIR/etc/NetworkManager/system-connections"

if [ -x "$TARGET_DIR/usr/bin/adbd" ]; then
    ln -sfn ../adbd.service \
        "$TARGET_DIR/etc/systemd/system/multi-user.target.wants/adbd.service"
else
    rm -f "$TARGET_DIR/etc/systemd/system/multi-user.target.wants/adbd.service"
fi
