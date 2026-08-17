#!/bin/sh
set -eu

PATH=/usr/sbin:/usr/bin:/sbin:/bin
export PATH

env_get()
{
	name="$1"
	fw_printenv -n "$name" 2>/dev/null || true
}

valid_slot()
{
	case "$1" in
		A|B) return 0 ;;
		*) return 1 ;;
	esac
}

other_slot()
{
	case "$1" in
		A) printf '%s\n' B ;;
		B) printf '%s\n' A ;;
		*) return 1 ;;
	esac
}

set_primary()
{
	slot="$1"
	valid_slot "$slot"
	# systemAB_next is consumed by sunxi_auto_switch_system() at boot.
	# systemAB_damage is the vendor tree's persistent health marker: the
	# selected slot is considered healthy when both values point to it.
	fw_setenv systemAB_next "$slot"
	fw_setenv systemAB_damage "$slot"
	fw_setenv bootcount 0
}

set_state()
{
	slot="$1"
	state="$2"
	valid_slot "$slot"
	case "$state" in
		good)
			fw_setenv systemAB_damage "$slot"
			;;
		bad)
			fw_setenv systemAB_damage "$(other_slot "$slot")"
			;;
		*) return 1 ;;
	esac
}

case "${1:-}" in
	get-primary)
		slot="$(env_get systemAB_next)"
		valid_slot "$slot" && printf '%s\n' "$slot"
		;;
	set-primary)
		[ "$#" -eq 2 ] && set_primary "$2"
		;;
	get-state)
		[ "$#" -eq 2 ] || exit 1
		slot="$2"
		valid_slot "$slot"
		[ "$(env_get systemAB_damage)" = "$slot" ] && printf '%s\n' good || printf '%s\n' bad
		;;
	set-state)
		[ "$#" -eq 3 ] && set_state "$2" "$3"
		;;
	get-current)
		slot="$(sed -n 's/.*[[:space:]]rauc\.slot=\([^[:space:]]*\).*/\1/p' /proc/cmdline)"
		valid_slot "$slot" && printf '%s\n' "$slot"
		;;
	*)
		echo "Usage: $0 {get-primary|set-primary A|B|get-state A|B|set-state A|B good|bad|get-current}" >&2
		exit 2
		;;
esac
