"""Compare private captures from the reviewed VbPng graphical-control sample.

Run vb-intrinsic-observe in real-graphical-picture mode against bare and
styled instances into separate fresh directories, then pass those paths here.
The external sample is optional and is not a build or package dependency.
"""
import argparse
import struct
from pathlib import Path

CASES = ((3, "normal", 125, 125), (3, "disabled", 125, 125),
         (4, "normal", 125, 121), (4, "disabled", 125, 121),
         (6, "normal", 129, 53), (6, "checked", 129, 53),
         (6, "disabled", 129, 53))


def pixels(path, width, height):
    data = path.read_bytes()
    if len(data) < 54 or data[:2] != b"BM":
        raise ValueError(f"invalid bitmap: {path}")
    offset = struct.unpack_from("<I", data, 10)[0]
    header, w, h, planes, bits, compression = struct.unpack_from("<IiiHHI", data, 14)
    if (header, w, h, planes, bits, compression) != (40, width, -height, 1, 32, 0):
        raise ValueError(f"unexpected 96-DPI bitmap layout: {path}")
    if len(data) != offset + width * height * 4:
        raise ValueError(f"truncated or oversized bitmap: {path}")
    return data[offset:]


def compare(bare, styled):
    total = 0
    for index, state, width, height in CASES:
        name = f"control-{index}-{state}.bmp"
        left, right = pixels(bare / name, width, height), pixels(styled / name, width, height)
        if left == right:
            raise ValueError(f"style did not change the full control image: {name}")
        colors = set()
        for y in range(5, height - 20):
            for x in range(5, width - 5):
                n = (y * width + x) * 4
                color = left[n:n + 3]
                if color != right[n:n + 3]:
                    raise ValueError(f"picture-region pixel changed: {name} ({x},{y})")
                colors.add(color)
                total += 1
        if len(colors) < 4:
            raise ValueError(f"picture region lacks expected image data: {name}")
    print(f"VB_REAL_PICTURE_COMPARE_OK cases={len(CASES)} pixels={total} normal_selected_disabled=1")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("bare", type=Path)
    parser.add_argument("styled", type=Path)
    args = parser.parse_args()
    compare(args.bare, args.styled)
