"""Compare real VB6 DisabledPicture output against the original sample bitmap.

The source-reviewed external sample is optional and is not a build dependency.
Pass its EXE, FRX, and fresh bare/styled observer capture directories.
"""
import argparse
import hashlib
import struct
from pathlib import Path

EXE_SHA = "fccc3d4e657843189b39fad5230f81bc1bfe3517dcd1a64d62ac0c15a1bdb296"
FRX_SHA = "e61c74a1693598138b92399c513f154c188772e212572abf51bc1a3b013a50bf"


def capture(path):
    data = path.read_bytes()
    if len(data) < 54 or data[:2] != b"BM":
        raise ValueError(f"invalid capture: {path}")
    offset = struct.unpack_from("<I", data, 10)[0]
    header, width, height, planes, bits, compression = struct.unpack_from("<IiiHHI", data, 14)
    if (header, width, height, planes, bits, compression) != (40, 49, -49, 1, 32, 0):
        raise ValueError(f"unexpected capture layout: {path}")
    if len(data) != offset + 49 * 49 * 4:
        raise ValueError(f"truncated capture: {path}")
    return [data[offset + i * 4:offset + i * 4 + 3] for i in range(49 * 49)]


def source_picture(frx, exe):
    raw, binary = frx.read_bytes(), exe.read_bytes()
    if hashlib.sha256(raw).hexdigest() != FRX_SHA or hashlib.sha256(binary).hexdigest() != EXE_SHA:
        raise ValueError("sample source or executable hash mismatch")
    start, length = 0x6952 + 12, 0xE2A
    bmp = raw[start:start + length]
    if len(bmp) != length or bmp not in binary:
        raise ValueError("disabled bitmap absent from pinned executable")
    if struct.unpack_from("<2sIHHI", bmp) != (b"BM", length, 0, 0, 1078):
        raise ValueError("unexpected source bitmap header")
    if struct.unpack_from("<IiiHHI", bmp, 14) != (40, 49, 49, 1, 8, 0):
        raise ValueError("unexpected source bitmap layout")
    palette = [bmp[54 + i * 4:57 + i * 4] for i in range(256)]
    return [palette[bmp[1078 + (48 - y) * 52 + x]] for y in range(49) for x in range(49)]


def compare(exe, frx, bare, styled):
    source = source_picture(frx, exe)
    expected_count = 0
    for directory in (bare, styled):
        normal = capture(directory / "tile-normal.bmp")
        disabled = capture(directory / "tile-disabled.bmp")
        restored = capture(directory / "tile-restored.bmp")
        if normal != restored:
            raise ValueError(f"normal appearance not restored: {directory}")
        changed = 0
        for y in range(3, 46):
            for x in range(3, 46):
                index = y * 49 + x
                if disabled[index] != source[index]:
                    raise ValueError(f"DisabledPicture pixel changed: {directory} ({x},{y})")
                changed += normal[index] != disabled[index]
                expected_count += 1
        if changed != 328:
            raise ValueError(f"distinct DisabledPicture not observed: {directory} ({changed})")
    print(f"VB6_DISABLED_PICTURE_COMPARE_OK cases=2 source_pixels={expected_count} "
          "distinct_pixels=328 restored=1")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("exe", type=Path)
    parser.add_argument("frx", type=Path)
    parser.add_argument("bare", type=Path)
    parser.add_argument("styled", type=Path)
    args = parser.parse_args()
    compare(args.exe, args.frx, args.bare, args.styled)
