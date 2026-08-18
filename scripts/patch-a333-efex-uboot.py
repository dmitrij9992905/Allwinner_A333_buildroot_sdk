#!/usr/bin/env python3
"""Prepare an Allwinner A333 U-Boot image for USB EFEX boot."""

from __future__ import annotations

import argparse
import struct
from pathlib import Path


STAMP_VALUE = 0x5F0A6C39
CHECKSUM_OFFSET = 0x0C
LENGTH_OFFSET = 0x14
WORK_MODE_OFFSET = 0xE0
DRAM_SCAN_SIZE_OFFSET = 0x4F8


def checksum(data: bytearray, length: int) -> int:
    old = struct.unpack_from("<I", data, CHECKSUM_OFFSET)[0]
    struct.pack_into("<I", data, CHECKSUM_OFFSET, STAMP_VALUE)
    value = sum(struct.unpack_from(f"<{length // 4}I", data, 0)) & 0xFFFFFFFF
    struct.pack_into("<I", data, CHECKSUM_OFFSET, old)
    return value


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("source", type=Path)
    parser.add_argument("destination", type=Path)
    parser.add_argument("--work-mode", type=lambda value: int(value, 0), default=0x10)
    parser.add_argument("--dram-mib", type=int, default=2048)
    args = parser.parse_args()

    data = bytearray(args.source.read_bytes())
    if len(data) < DRAM_SCAN_SIZE_OFFSET + 4:
        raise SystemExit(f"{args.source}: file is too small for an A333 U-Boot header")
    if not data[4:12].startswith(b"uboot"):
        raise SystemExit(f"{args.source}: missing Allwinner U-Boot magic")

    length = struct.unpack_from("<I", data, LENGTH_OFFSET)[0]
    if length == 0 or length > len(data) or length % 4:
        raise SystemExit(
            f"{args.source}: invalid header length 0x{length:x} for {len(data):#x}-byte file"
        )
    if not 0 <= args.work_mode <= 0xFFFFFFFF:
        raise SystemExit("work mode must fit in an unsigned 32-bit value")
    if not 0 <= args.dram_mib <= 0xFFFFFFFF:
        raise SystemExit("DRAM size must fit in an unsigned 32-bit value")

    old_mode = struct.unpack_from("<I", data, WORK_MODE_OFFSET)[0]
    old_dram = struct.unpack_from("<I", data, DRAM_SCAN_SIZE_OFFSET)[0]
    old_sum = struct.unpack_from("<I", data, CHECKSUM_OFFSET)[0]
    struct.pack_into("<I", data, WORK_MODE_OFFSET, args.work_mode)
    struct.pack_into("<I", data, DRAM_SCAN_SIZE_OFFSET, args.dram_mib)

    new_sum = checksum(data, length)
    struct.pack_into("<I", data, CHECKSUM_OFFSET, new_sum)
    if checksum(data, length) != new_sum:
        raise SystemExit("internal checksum verification failed")

    args.destination.parent.mkdir(parents=True, exist_ok=True)
    args.destination.write_bytes(data)
    print(
        "patched U-Boot: "
        f"work_mode 0x{old_mode:x}->0x{args.work_mode:x}, "
        f"dram {old_dram}->{args.dram_mib} MiB, "
        f"checksum 0x{old_sum:08x}->0x{new_sum:08x}"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
