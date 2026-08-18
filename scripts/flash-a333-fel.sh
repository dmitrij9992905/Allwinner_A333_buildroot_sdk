#!/bin/sh
set -eu

PROJECT_ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
IMAGES_DIR="$PROJECT_ROOT/output/images"
XFEL_BIN=""
TIMEOUT=120
XFEL_TIMEOUT=20
DRAM_MIB=2048
WORK_MODE=0x10
DRY_RUN=0
EFEX_COMMAND=""

usage() {
	cat <<'EOF'
Usage:
  scripts/flash-a333-fel.sh [options] [-- efex-flasher arguments...]

The script uploads FES1, a checksum-corrected U-Boot and its vendor parameters
to an A333 in FEL. After U-Boot starts in USB EFEX mode, an external EFEX
flasher must write the full image. xfel itself cannot write A333 eMMC.

Options:
  --images DIR       Buildroot images directory (default: output/images)
  --xfel FILE        A537/A333-aware xfel binary
  --timeout SEC      FEL wait timeout (default: 120)
  --xfel-timeout SEC USB command timeout (default: 20)
  --dram-mib N       DRAM scan size written to U-Boot (default: 2048)
  --work-mode VALUE  U-Boot mode, normally 0x10 (USB product/EFEX)
  --efex-command CMD Run CMD after EFEX handoff
  --dry-run          Prepare and validate the patched U-Boot only
  -h, --help         Show this help

Examples:
  sudo scripts/flash-a333-fel.sh --dry-run
  sudo scripts/flash-a333-fel.sh --xfel ../xfel/xfel
  sudo scripts/flash-a333-fel.sh --efex-command 'sudo ./LiveSuit'

The EFEX GUI must be given:
  output/images/a333-helperboard-full.img
EOF
}

while [ "$#" -gt 0 ]; do
	case "$1" in
		--images) IMAGES_DIR=$2; shift 2 ;;
		--xfel) XFEL_BIN=$2; shift 2 ;;
		--timeout) TIMEOUT=$2; shift 2 ;;
		--xfel-timeout) XFEL_TIMEOUT=$2; shift 2 ;;
		--dram-mib) DRAM_MIB=$2; shift 2 ;;
		--work-mode) WORK_MODE=$2; shift 2 ;;
		--efex-command) EFEX_COMMAND=$2; shift 2 ;;
		--dry-run) DRY_RUN=1; shift ;;
		-h|--help) usage; exit 0 ;;
		--) shift; [ "$#" -gt 0 ] || { echo "missing EFEX command" >&2; exit 2; }; EFEX_COMMAND="$*"; break ;;
		*) echo "unknown option: $1" >&2; usage >&2; exit 2 ;;
	esac
done

IMAGES_DIR=$(CDPATH= cd -- "$IMAGES_DIR" && pwd)
FEL_DIR="$IMAGES_DIR/.a333-pack/pack_out"
FES1="$FEL_DIR/fes1.fex"
UBOOT="$FEL_DIR/u-boot.fex"
CONFIG="$FEL_DIR/config.fex"
BOARD="$FEL_DIR/board.fex"
DTB="$FEL_DIR/sunxi.fex"
PATCHER="$PROJECT_ROOT/scripts/patch-a333-efex-uboot.py"

for file in "$FES1" "$UBOOT" "$DTB" "$CONFIG" "$BOARD" "$PATCHER"; do
	[ -f "$file" ] || { echo "missing required file: $file" >&2; exit 1; }
done

if [ -z "$XFEL_BIN" ]; then
	if [ -x "$PROJECT_ROOT/../xfel/xfel" ]; then
		XFEL_BIN="$PROJECT_ROOT/../xfel/xfel"
	elif command -v xfel >/dev/null 2>&1; then
		XFEL_BIN=$(command -v xfel)
	else
		echo "A537/A333-aware xfel was not found; use --xfel /path/to/xfel" >&2
		exit 1
	fi
fi
[ -x "$XFEL_BIN" ] || { echo "xfel is not executable: $XFEL_BIN" >&2; exit 1; }
command -v timeout >/dev/null 2>&1 || { echo "required command not found: timeout" >&2; exit 1; }
if ! timeout "${XFEL_TIMEOUT}s" "$XFEL_BIN" --help 2>&1 | grep -q -- "--no-version"; then
	echo "xfel lacks --no-version support; rebuild the patched tool in ../xfel" >&2
	exit 1
fi

PATCHED_UBOOT=$(mktemp "${TMPDIR:-/tmp}/a333-efex-uboot.XXXXXX.fex")
trap 'rm -f "$PATCHED_UBOOT"' EXIT INT TERM
python3 "$PATCHER" "$UBOOT" "$PATCHED_UBOOT" \
	--work-mode "$WORK_MODE" --dram-mib "$DRAM_MIB"

echo "FEL payloads:"
echo "  FES1:   $FES1 -> 0x0004c000"
echo "  U-Boot: $PATCHED_UBOOT -> 0x4a000000"
echo "  DTB:    $DTB -> 0x4a200000"
echo "  config: $CONFIG -> 0x4a300000"
echo "  board:  $BOARD -> 0x4a380000"
echo "  image:  $IMAGES_DIR/a333-helperboard-full.img"

if [ "$DRY_RUN" -eq 1 ]; then
	echo "dry-run: no USB operation performed"
	exit 0
fi

if [ "$(id -u)" -ne 0 ]; then
	echo "USB FEL access requires root; run this script with sudo." >&2
	exit 1
fi

run_xfel() {
	if [ "$(id -u)" -eq 0 ]; then
		timeout "${XFEL_TIMEOUT}s" "$XFEL_BIN" "$@"
	else
		timeout "${XFEL_TIMEOUT}s" sudo "$XFEL_BIN" "$@"
	fi
}

wait_for_fel() {
	end=$(( $(date +%s) + TIMEOUT ))
	while [ "$(date +%s)" -lt "$end" ]; do
		# lsusb is only a presence check.  xfel is still required below to
		# claim the interface and verify the A333 FEL protocol.
		if ! command -v lsusb >/dev/null 2>&1 || lsusb -d 1f3a:efe8 >/dev/null 2>&1; then
			version_output=$(run_xfel version 2>&1 || true)
			if printf '%s\n' "$version_output" | grep -Eq 'AWUSBFEX|ID=0x00191900'; then
				printf '%s\n' "$version_output"
				return 0
			fi
		fi
		sleep 0.25
	done
	if command -v lsusb >/dev/null 2>&1 && lsusb -d 1f3a:efe8 >/dev/null 2>&1; then
		echo "A333 FEL USB device is present, but xfel could not claim or query it:" >&2
		run_xfel version >&2 || true
		echo "Check that this script is run with sudo and that no other FEL/awusb client owns the device." >&2
	else
		echo "No USB device 1f3a:efe8 was detected." >&2
	fi
	echo "timeout waiting for A333 FEL device" >&2
	return 1
}

wait_for_usb_device() {
	end=$(( $(date +%s) + TIMEOUT ))
	while [ "$(date +%s)" -lt "$end" ]; do
		if ! command -v lsusb >/dev/null 2>&1 || lsusb -d 1f3a:efe8 >/dev/null 2>&1; then
			return 0
		fi
		sleep 0.25
	done
	echo "No USB device 1f3a:efe8 after FES1" >&2
	return 1
}

echo "Waiting for A333 FEL..."
wait_for_fel
echo "Uploading FES1..."
run_xfel write 0x0004c000 "$FES1"
run_xfel exec 0x0004c000

# FES1 initializes DRAM and keeps the USB VID/PID, but it no longer has to
# answer the FEL `version` request. Querying `version` here can hang.
# The vendor flow sends the second-stage payload directly after enumeration.
echo "Waiting for USB device after FES1..."
sleep 10
wait_for_usb_device
sleep 10
echo "Uploading USB EFEX U-Boot and parameters..."
run_xfel --no-version write 0x4a000000 "$PATCHED_UBOOT"
run_xfel --no-version write 0x4a200000 "$DTB"
run_xfel --no-version write 0x4a300000 "$CONFIG"
run_xfel --no-version write 0x4a380000 "$BOARD"
run_xfel --no-version exec 0x4a000000

echo "FEL stage complete; U-Boot should now be in USB EFEX (work_mode=$WORK_MODE)."
echo "Start the vendor EFEX flasher and select:"
echo "  $IMAGES_DIR/a333-helperboard-full.img"

if [ -n "$EFEX_COMMAND" ]; then
	echo "Running EFEX command: $EFEX_COMMAND"
	sh -c "$EFEX_COMMAND"
else
	echo "No EFEX command supplied; stopping before any eMMC write."
	exit 10
fi
