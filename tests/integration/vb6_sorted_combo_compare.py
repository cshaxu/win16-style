"""Compare the sorted VB6 combo drop-list captures from the optional real probe.

Use vb-intrinsic-observe real-vb6-sorted-combo in separate private copies of
reviewed CBMXfer, once native and once with the installed style. The sample and
its private COM dependency are research-only and not a build prerequisite.
"""
import argparse
from pathlib import Path
from vb_picture_compare import pixels


def compare(native, styled):
    a = pixels(native / "combo-list.bmp", 281, 41)
    b = pixels(styled / "combo-list.bmp", 281, 41)
    changed = sum(a[i:i + 3] != b[i:i + 3] for i in range(0, 281 * 41 * 4, 4))
    if not changed:
        raise ValueError("real VB6 sorted combo list did not show a style difference")
    print(f"VB6_SORTED_COMBO_COMPARE_OK pixels_changed={changed} total=11521")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("native", type=Path)
    parser.add_argument("styled", type=Path)
    args = parser.parse_args()
    compare(args.native, args.styled)
