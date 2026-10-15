# Shared GPT lookup. Never infer a writable partition from a fixed pN number
# or stale bootargs: old images used p10 for userdata, new ones use p11.
export LC_ALL=C

a333_partition_number()
{
    storage_table=$(sgdisk --print "$1") || return 1
    storage_number=$(printf '%s\n' "$storage_table" |
        awk -v label="$2" '$1 ~ /^[0-9]+$/ && $NF == label {print $1}')
    case "$storage_number" in
        ''|*[!0-9]*) return 1 ;;
    esac
    [ "$storage_number" -gt 0 ] || return 1
    storage_info=$(sgdisk --info="$storage_number" "$1") || return 1
    storage_label=$(printf '%s\n' "$storage_info" | sed -n "s/^Partition name: '\(.*\)'$/\1/p")
    [ "$storage_label" = "$2" ] || return 1
    printf '%s\n' "$storage_number"
}

a333_partition_path()
{
    case "$1" in
        *[0-9]) printf '%sp%s\n' "$1" "$2" ;;
        *) printf '%s%s\n' "$1" "$2" ;;
    esac
}

a333_partition_device()
{
    storage_disk=${A333_STORAGE_DISK:-/dev/mmcblk0}
    [ -b "$storage_disk" ] || return 1
    storage_index=$(a333_partition_number "$storage_disk" "$1") || return 1
    storage_device=$(a333_partition_path "$storage_disk" "$storage_index")
    [ -b "$storage_device" ] || return 1
    printf '%s\n' "$storage_device"
}
