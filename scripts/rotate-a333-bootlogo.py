#!/usr/bin/env python3
"""Rotate an uncompressed 24-bit BMP without external image packages."""

import argparse
import struct
from pathlib import Path


def read_bitmap(path: Path):
    data = path.read_bytes()
    if len(data) < 54 or data[:2] != b"BM":
        raise ValueError(f"{path}: not a Windows BMP")

    pixel_offset = struct.unpack_from("<I", data, 10)[0]
    dib_size = struct.unpack_from("<I", data, 14)[0]
    if dib_size < 40 or len(data) < 14 + dib_size:
        raise ValueError(f"{path}: unsupported BMP header")

    width, stored_height = struct.unpack_from("<ii", data, 18)
    planes, bits_per_pixel = struct.unpack_from("<HH", data, 26)
    compression = struct.unpack_from("<I", data, 30)[0]
    x_pixels_per_meter, y_pixels_per_meter = struct.unpack_from("<ii", data, 38)
    if width <= 0 or stored_height == 0 or planes != 1:
        raise ValueError(f"{path}: invalid BMP geometry")
    if bits_per_pixel != 24 or compression != 0:
        raise ValueError(f"{path}: only uncompressed 24-bit BMP is supported")

    height = abs(stored_height)
    row_stride = (width * 3 + 3) & ~3
    required_size = pixel_offset + row_stride * height
    if required_size > len(data):
        raise ValueError(f"{path}: truncated BMP pixel data")

    top_down = stored_height < 0
    rows = []
    for y in range(height):
        stored_y = y if top_down else height - 1 - y
        offset = pixel_offset + stored_y * row_stride
        row = data[offset : offset + width * 3]
        rows.append([row[x : x + 3] for x in range(0, len(row), 3)])
    return rows, x_pixels_per_meter, y_pixels_per_meter


def rotate_rows(rows, rotation):
    height = len(rows)
    width = len(rows[0])
    if rotation == 0:
        return [list(row) for row in rows]
    if rotation == 90:
        return [
            [rows[height - 1 - x][y] for x in range(height)]
            for y in range(width)
        ]
    if rotation == 180:
        return [list(reversed(row)) for row in reversed(rows)]
    if rotation == 270:
        return [
            [rows[x][width - 1 - y] for x in range(height)]
            for y in range(width)
        ]
    raise ValueError(f"unsupported rotation: {rotation}")


def write_bitmap(path: Path, rows, x_pixels_per_meter, y_pixels_per_meter):
    height = len(rows)
    width = len(rows[0])
    row_stride = (width * 3 + 3) & ~3
    padding = bytes(row_stride - width * 3)
    image_size = row_stride * height
    pixel_offset = 14 + 40
    file_size = pixel_offset + image_size

    header = struct.pack("<2sIHHI", b"BM", file_size, 0, 0, pixel_offset)
    dib = struct.pack(
        "<IiiHHIIiiII",
        40,
        width,
        height,
        1,
        24,
        0,
        image_size,
        x_pixels_per_meter,
        y_pixels_per_meter,
        0,
        0,
    )
    pixels = bytearray()
    for row in reversed(rows):
        pixels.extend(b"".join(row))
        pixels.extend(padding)
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_bytes(header + dib + pixels)


def main():
    parser = argparse.ArgumentParser(
        description="Rotate an A333 bootlogo BMP clockwise"
    )
    parser.add_argument(
        "--rotation", required=True, type=int, choices=(0, 90, 180, 270)
    )
    parser.add_argument("source", type=Path)
    parser.add_argument("destination", type=Path)
    args = parser.parse_args()

    rows, x_pixels_per_meter, y_pixels_per_meter = read_bitmap(args.source)
    rotated = rotate_rows(rows, args.rotation)
    write_bitmap(
        args.destination,
        rotated,
        x_pixels_per_meter,
        y_pixels_per_meter,
    )


if __name__ == "__main__":
    main()
