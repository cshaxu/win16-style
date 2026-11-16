#!/usr/bin/env python3
"""Reject product and concrete-style dependencies from shared style code."""
import argparse
from pathlib import Path
import re
import subprocess
import sys

ROOT = Path(__file__).resolve().parent.parent
FORBIDDEN = re.compile(r"ntw16s|ntw2ks|ntwxps|ntw7s|ntwvistas|ntwshk|run16s|win16|win2k|win2000|winxp|win7|vista|win3x|luna|aero", re.I)
COMPONENTS = ("src/lib-win32-style/access", "src/lib-win32-style/application")


def check_text(path, text):
    return [f"{path}: product or concrete-style identifier"] if FORBIDDEN.search(text) else []


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--self-test", action="store_true")
    parser.add_argument("--nm")
    parser.add_argument("--archive", action="append", default=[])
    args = parser.parse_args()
    if bool(args.nm) != bool(args.archive):
        parser.error("--nm and --archive must be supplied together")
    if args.self_test:
        assert check_text("fixture", '#include "ntw16s-dll/private.h"\n')
        assert check_text("fixture", "void win2k_frame(void);\n")
        assert check_text("fixture", '#include "lib-win32-style/appearance/win3x/window/frame.h"\n')
        assert check_text("fixture", "void vista_caption(void);\n")
        assert check_text("fixture", '#include "lib-win32-style/appearance/luna/frame.h"\n')
        assert check_text("fixture", '#include "lib-win32-style/appearance/aero/frame.h"\n')
        assert check_text("fixture", "void win7_frame(void);\n")
        assert not check_text("fixture", '#include "lib-win32-style/application/window/frame.h"\n')
    errors = []
    for component in COMPONENTS:
        if not (ROOT / component).is_dir():
            errors.append(f"{component}: shared component directory missing")
        sources = [p for p in sorted((ROOT / component).rglob("*")) if p.is_file() and p.suffix in (".c", ".h")]
        if not sources:
            errors.append(f"{component}: no shared sources checked")
        for path in sources:
            errors.extend(check_text(path.relative_to(ROOT), path.read_text(encoding="utf-8")))
    for archive in args.archive:
        result = subprocess.run([args.nm, "--undefined-only", archive], text=True, capture_output=True)
        if result.returncode:
            errors.append(f"{archive}: nm failed: {result.stderr.strip()}")
        elif FORBIDDEN.search(result.stdout):
            errors.append(f"{archive}: product or concrete-style undefined symbol")
    if errors:
        print("\n".join(errors), file=sys.stderr)
        return 1
    print(f"STYLE BOUNDARY PASS source={len(COMPONENTS)} archives={len(args.archive)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
