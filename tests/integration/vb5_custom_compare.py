"""Compare captures from the reviewed optional VB5 custom-draw sample.

Run vb-intrinsic-observe in real-vb5-custom mode against bare and styled
instances into separate fresh directories, then pass those directories here.
The external sample and its private COM dependencies are not required by build.
"""
import argparse
import struct
from pathlib import Path


def bitmap(path):
    data = path.read_bytes()
    if len(data) < 54 or data[:2] != b"BM":
        raise ValueError(f"invalid bitmap: {path}")
    offset = struct.unpack_from("<I", data, 10)[0]
    header, width, height, planes, bits, compression = struct.unpack_from("<IiiHHI", data, 14)
    if (header, width, height, planes, bits, compression) != (40, 85, -33, 1, 32, 0):
        raise ValueError(f"unexpected 96-DPI bitmap layout: {path}")
    if len(data) != offset + width * abs(height) * 4:
        raise ValueError(f"truncated or oversized bitmap: {path}")
    return data


def compare(bare, styled):
    images = {}
    for state in ("normal", "checked", "restored"):
        name = f"custom-{state}.bmp"
        left, right = bitmap(bare / name), bitmap(styled / name)
        if left != right:
            raise ValueError(f"application-drawn image changed: {name}")
        images[state] = left
    if images["normal"] == images["checked"]:
        raise ValueError("sample did not render a distinct checked state")
    print("VB5_CUSTOM_COMPARE_OK images=3 exact=1 checked_distinct=1")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("bare", type=Path)
    parser.add_argument("styled", type=Path)
    args = parser.parse_args()
    compare(args.bare, args.styled)
