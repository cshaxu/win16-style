"""Compare the reviewed VB5 popup demo's two graphical Picture buttons.

Use vb-intrinsic-observe real-vb5-picture-buttons with the native and styled
launchers into separate fresh capture directories. The research sample and its
private unregistered dependencies are optional and are not build dependencies.
The probe does not click the buttons, whose handlers open external webpages.
"""
import argparse
from pathlib import Path
from vb_picture_compare import pixels


def compare(bare, styled):
    total = 0
    for index in range(2):
        for state in ("normal", "disabled", "restored"):
            name = f"button-{index}-{state}.bmp"
            original = pixels(bare / name, 129, 33)
            actual = pixels(styled / name, 129, 33)
            if original == actual:
                raise ValueError(f"style did not change the frame: {name}")
            colors = set()
            for y in range(4, 29):
                for x in range(4, 125):
                    pos = (y * 129 + x) * 4
                    rgb = original[pos:pos + 3]
                    if rgb != actual[pos:pos + 3]:
                        raise ValueError(f"picture-region pixel changed: {name} ({x},{y})")
                    colors.add(rgb)
                    total += 1
            if len(colors) < 3:
                raise ValueError(f"picture region lacks image detail: {name}")
        for folder in (bare, styled):
            normal = pixels(folder / f"button-{index}-normal.bmp", 129, 33)
            disabled = pixels(folder / f"button-{index}-disabled.bmp", 129, 33)
            restored = pixels(folder / f"button-{index}-restored.bmp", 129, 33)
            if normal == disabled or normal != restored:
                raise ValueError(f"button {index} failed disabled/restored check: {folder}")
    print(f"VB5_REAL_PICTURE_COMPARE_OK buttons=2 states=3 interior_pixels={total} no_button_click=1")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("bare", type=Path)
    parser.add_argument("styled", type=Path)
    args = parser.parse_args()
    compare(args.bare, args.styled)
