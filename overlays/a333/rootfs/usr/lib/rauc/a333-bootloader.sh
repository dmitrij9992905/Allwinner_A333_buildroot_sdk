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
	# Never overwrite a bad/uninitialized environment with fw_setenv's
	# built-in defaults: that loses the board's boot commands and A/B mapping.
	[ "$(env_get systemA)" = bootA ] && [ "$(env_get systemB)" = bootB ] || {
		echo "Invalid A333 environment; repair/initialize it before OTA" >&2
		return 1
	}
	# systemAB_next is consumed by sunxi_auto_switch_system() at boot.
	# systemAB_damage is the vendor tree's persistent health marker: the
	# selected slot is considered healthy when both values point to it.
	settings=$(mktemp /run/a333-rauc-env.XXXXXX)
	trap 'rm -f "$settings"' EXIT HUP INT TERM
	printf 'systemAB_next %s\nsystemAB_damage %s\nbootcount 0\n' "$slot" "$slot" > "$settings"
	fw_setenv -s "$settings"
}

set_state()
{
	slot="$1"
	state="$2"
	valid_slot "$slot"
	[ "$(env_get systemA)" = bootA ] && [ "$(env_get systemB)" = bootB ] || {
		echo "Invalid A333 environment; refusing slot state write" >&2
		return 1
	}
	case "$state" in
		good)
			fw_setenv systemAB_damage "$slot"
			fw_setenv bootcount 0
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
