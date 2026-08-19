#!/usr/bin/env python3

import struct
import sys
from pathlib import Path

CHECKSUM_OFFSET = 0x0C

TAG_OFFSET = 0xF1
TAG = bytes.fromhex("54 41 57 b6 f9 b6 d9")

# Tag занимает части двух little-endian uint32:
#   0xF0..0xF3
#   0xF4..0xF7
WORD_AREA_OFFSET = 0xF0
WORD_AREA_SIZE = 8


def words_sum(data: bytes) -> int:
    assert len(data) == 8

    a, b = struct.unpack("<II", data)
    return (a + b) & 0xFFFFFFFF


if len(sys.argv) != 2:
    print(f"Usage: {sys.argv[0]} boot0_sdcard.fex")
    sys.exit(2)


path = Path(sys.argv[1])

data = bytearray(path.read_bytes())

if len(data) < 0xF8:
    raise RuntimeError("BOOT0 file is unexpectedly small")


old_tag = bytes(data[TAG_OFFSET:TAG_OFFSET + len(TAG)])

print(f"BOOT0: {path}")
print(f"old tag: {old_tag.hex(' ')}")
print(f"new tag: {TAG.hex(' ')}")


if old_tag == TAG:
    print("chip tag already installed")
    sys.exit(0)


if old_tag != b"\x00" * len(TAG):
    raise RuntimeError(
        "Unexpected existing chip tag: "
        + old_tag.hex(" ")
    )


old_checksum = struct.unpack_from(
    "<I",
    data,
    CHECKSUM_OFFSET
)[0]


old_area = bytes(
    data[
        WORD_AREA_OFFSET:
        WORD_AREA_OFFSET + WORD_AREA_SIZE
    ]
)


new_area = bytearray(old_area)

relative_tag_offset = TAG_OFFSET - WORD_AREA_OFFSET

new_area[
    relative_tag_offset:
    relative_tag_offset + len(TAG)
] = TAG


old_sum = words_sum(old_area)
new_sum = words_sum(new_area)

delta = (new_sum - old_sum) & 0xFFFFFFFF

new_checksum = (
    old_checksum + delta
) & 0xFFFFFFFF


data[
    TAG_OFFSET:
    TAG_OFFSET + len(TAG)
] = TAG

struct.pack_into(
    "<I",
    data,
    CHECKSUM_OFFSET,
    new_checksum
)


path.write_bytes(data)


print(f"checksum: 0x{old_checksum:08x} -> 0x{new_checksum:08x}")
print(f"delta:    0x{delta:08x}")

print("done")
