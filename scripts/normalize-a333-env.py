#!/usr/bin/env python3
"""Convert a backed-up vendor single environment to standard fw_*env format.

Never opens block devices for writing. Validate the saved CRC and preserve all
variables; the caller must separately verify and install the resulting file.
"""
import argparse
import binascii
import pathlib
import struct

ENV_SIZE = 0x20000


def normalize(blob):
    if len(blob) != ENV_SIZE:
        raise ValueError("expected exactly 128 KiB of single environment")
    data = blob[4:]
    crc = struct.unpack("<I", blob[:4])[0]
    if binascii.crc32(data) != crc:
        raise ValueError("invalid single-environment CRC; refusing conversion")
    if data.startswith(b"\0"):
        data = data[1:]
    if not data or data.startswith(b"\0") or b"\0\0" not in data:
        raise ValueError("empty or unterminated environment")
    variables = data.split(b"\0\0", 1)[0]
    for entry in variables.split(b"\0"):
        if b"=" not in entry or not entry.split(b"=", 1)[0]:
            raise ValueError("invalid environment entry")
    data = (variables + b"\0\0").ljust(ENV_SIZE - 4, b"\0")
    return struct.pack("<I", binascii.crc32(data)) + data


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("input", type=pathlib.Path)
    parser.add_argument("output", type=pathlib.Path)
    args = parser.parse_args()
    if not args.input.is_file():
        parser.error("input must be a regular backup file, not a block device")
    try:
        blob = normalize(args.input.read_bytes())
        with args.output.open("xb") as stream:
            stream.write(blob)
    except (ValueError, OSError) as exc:
        parser.error(str(exc))
    print(f"Validated environment written to {args.output}; no device modified")


if __name__ == "__main__":
    main()
