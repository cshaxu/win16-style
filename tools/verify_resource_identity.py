"""Verify original graphical resources without generating or rewriting artwork.

Pinned ntvdm64 source identity is checked locally; no sibling/network access.
"""
import argparse
import hashlib
from pathlib import Path

EXPECTED = {'src/ntw16s-dll/combo_glyph.h': '75b05a50bf02bbc8fcbd6c6f84948fcdbf82fa7f0fbbfc590a6e4eb94c3ffe34', 'src/ntw16s-dll/control_glyphs.h': '75031b93af2382f8ce1f47d971f32efdfe00cb7000e0db09ef878a1b50adff83', 'src/ntw16s-dll/frame_buttons.h': 'e7a472d76858c3d01fb1501a34d958ccd8337c8d8ccc36893c17ef9222bac0c5', 'src/ntw16s-dll/menu_glyphs.h': '03922a10b6a7b46e1792852b93f1e1590d45546a57a63cead41fb7b0f0eefa4e', 'src/ntw16s-dll/resources.md': '211d1d1d85e65bef001990fb229bd5959e272e1992b95d9086acc7c67716b6a7', 'src/ntw16s-dll/scroll_glyphs.h': 'a748e7d89d52abf6225403244971da442c5b391ab877d1f248ddf2ca96001d40'}

def verify(files):
    for path, expected in EXPECTED.items():
        if hashlib.sha256(files[path]).hexdigest() != expected:
            raise ValueError('Original resource bytes differ: '+path)

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--root',type=Path,default=Path(__file__).resolve().parents[1])
    parser.add_argument('--self-test',action='store_true')
    args=parser.parse_args()
    files={p:(args.root/p).read_bytes() for p in EXPECTED}
    verify(files)
    if args.self_test:
        for path in EXPECTED:
            changed=dict(files);changed[path]=files[path]+b' '
            try:verify(changed)
            except ValueError:pass
            else:raise AssertionError('Altered resource accepted: '+path)
        print('Original resource SelfTest PASS: 6 byte substitutions rejected')
    print('Original resource identity PASS: 5 headers and original provenance, byte-exact')

if __name__=='__main__':main()
