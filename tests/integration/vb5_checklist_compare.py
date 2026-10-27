"""Verify real VB5 checkbox-list captures from vb-intrinsic-observe.

Run the optional real-vb5-checklist mode on native and styled instances of the
source-reviewed AppCommand demo at 96 DPI, into fresh private directories.
The executable and its unregistered COM dependencies remain research-only.
"""
import argparse
import re
from pathlib import Path
from vb_picture_compare import pixels

ROOT = Path(__file__).resolve().parents[2]
CASES = (("vb5-before.bmp", 0), ("vb5-after.bmp", 14),
         ("vb5-disabled.bmp", 0), ("vb5-disabled-checked.bmp", 14))


def expected_glyph(column):
    data = (ROOT / "src/ntw16s-dll/control_glyphs.h").read_text(encoding="utf-8")
    rows = re.findall(r'"([01]{56})"', data)[:13]
    if len(rows) != 13:
        raise ValueError("original checkbox resource unavailable")
    result = []
    for y in range(11):
        sy = next(i for i in range(13) if (i * 11 + 6) // 13 <= y < ((i + 1) * 11 + 6) // 13)
        for x in range(11):
            sx = next(i for i in range(14) if (i * 11 + 7) // 14 <= x < ((i + 1) * 11 + 7) // 14)
            result.append(b"\x00\x00\x00" if rows[sy][column + sx] == "0" else b"\xff\xff\xff")
    return result


def glyph(data):
    return [data[((y + 4) * 405 + x + 2) * 4:((y + 4) * 405 + x + 2) * 4 + 3]
            for y in range(11) for x in range(11)]


def compare(native, styled):
    for name, column in CASES:
        original = pixels(native / name, 405, 214)
        actual = pixels(styled / name, 405, 214)
        wanted = expected_glyph(column)
        if glyph(original) == wanted:
            raise ValueError(f"native image is not a useful negative control: {name}")
        if glyph(actual) != wanted:
            raise ValueError(f"original resource glyph mismatch: {name}")
    for folder in (native, styled):
        before = pixels(folder / "vb5-before.bmp", 405, 214)
        after = pixels(folder / "vb5-after.bmp", 405, 214)
        restored = pixels(folder / "vb5-restored.bmp", 405, 214)
        if before == after or before != restored:
            raise ValueError(f"selection or restoration not visible: {folder}")
    print("VB5_REAL_CHECKLIST_COMPARE_OK states=4 resource_pixels=484 selection_restore=1")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("native", type=Path)
    parser.add_argument("styled", type=Path)
    args = parser.parse_args()
    compare(args.native, args.styled)
