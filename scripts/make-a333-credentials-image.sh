#!/bin/sh
set -eu
umask 077
if [ "$#" -lt 1 ] || [ "$#" -gt 2 ]; then
    echo "Usage: $0 <credentials.ext4> [per-device source directory]" >&2
    exit 2
fi
project=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
output=$1
source=${2:-${A333_CREDENTIALS_ROOT:-$project/configs/boards/a333/helperboard-a333/credentials-skel}}
[ -d "$source" ] || { echo "credentials: source directory unavailable" >&2; exit 1; }
# Do not follow links to secrets outside the supplied payload or import devices.
special=$(find "$source" ! -type d ! -type f -print -quit)
[ -z "$special" ] || { echo "credentials: only regular files/directories are allowed" >&2; exit 1; }
[ ! -L "$output" ] && { [ ! -e "$output" ] || [ -f "$output" ]; } || exit 1
fakeroot=${HOST_DIR:+$HOST_DIR/bin/fakeroot}
[ -n "$fakeroot" ] && [ -x "$fakeroot" ] || fakeroot=$(command -v fakeroot)
work=$(mktemp -d "${TMPDIR:-/tmp}/a333-credentials.XXXXXXXX")
trap 'rm -rf -- "$work"' EXIT HUP INT TERM
mkdir "$work/root"
cp -a "$source/." "$work/root/"
find "$work/root" -type d -exec chmod 0700 {} +
find "$work/root" -type f -exec chmod 0600 {} +
# The source directory belongs to the factory operator. Only the temporary
# copy is assigned root ownership under fakeroot; source files are untouched.
"$fakeroot" -- sh -eu -c '
    chown -R 0:0 "$1/root"
    mke2fs -q -t ext4 -O ^has_journal -m 0 -L credentials \
        -E root_owner=0:0 -d "$1/root" "$1/credentials.ext4" 16M
    # mke2fs -d copies children but keeps the new root inode at its default
    # 0755; explicitly make the filesystem root private as well.
    debugfs -w -R "set_inode_field / mode 040700" "$1/credentials.ext4" >/dev/null
    chmod 0600 "$1/credentials.ext4"
' sh "$work"
[ "$(stat -c %s "$work/credentials.ext4")" -eq 16777216 ]
mv -f -- "$work/credentials.ext4" "$output"
echo "credentials: created 16 MiB ext4 image (root-owned, no journal): $output"
