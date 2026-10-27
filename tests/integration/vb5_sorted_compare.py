"""Compare a source-reviewed real VB5 sorted multiselect list capture.

Use vb-intrinsic-observe real-vb5-sorted-list in a disposable native and styled
instance of the optional IconEditor demo. That mode deliberately terminates its
own job instead of invoking the demo's registry-writing QueryUnload handler.
"""
import argparse
from pathlib import Path
from vb_picture_compare import pixels


def compare(native, styled):
    a = pixels(native / "sorted-selected.bmp", 153, 121)
    b = pixels(styled / "sorted-selected.bmp", 153, 121)
    changed = sum(a[i:i + 3] != b[i:i + 3] for i in range(0, 153 * 121 * 4, 4))
    if not changed:
        raise ValueError("real sorted VB5 list did not show a style difference")
    print(f"VB5_REAL_SORTED_COMPARE_OK pixels_changed={changed} total=18513")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("native", type=Path)
    parser.add_argument("styled", type=Path)
    args = parser.parse_args()
    compare(args.native, args.styled)
