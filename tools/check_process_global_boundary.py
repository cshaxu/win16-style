#!/usr/bin/env python3
"""Reject product-private dependencies from shared process management sources."""
from pathlib import Path
import argparse
import re
import subprocess
import sys

FORBIDDEN = re.compile(r"ntw16s|run16s|win16(?:style)?", re.I)
INCLUDE = re.compile(r"^\s*#\s*include\s*[<\"]([^>\"]+)[>\"]", re.M)
ROOT = Path(__file__).resolve().parent.parent


def check_text(path, text):
    errors = []
    if FORBIDDEN.search(text):
        errors.append(f"{path}: product identifier")
    for include in INCLUDE.findall(text):
        if FORBIDDEN.search(include):
            errors.append(f"{path}: product include {include}")
    return errors


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
        assert check_text("fixture", 'void run16s_adapter(void);\n')
        assert not check_text("fixture", '#include "lib-win32/process-image/image.h"\n')
    errors = []
    for component in ("process-global", "process-hook", "process-image", "handle"):
        for path in sorted((ROOT / "src" / "lib-win32" / component).rglob("*")):
            if path.is_file() and path.suffix in (".c", ".h"):
                errors.extend(check_text(path.relative_to(ROOT), path.read_text(encoding="utf-8")))
    if args.nm:
        for archive in args.archive:
            result = subprocess.run([args.nm, "--undefined-only", archive], text=True, capture_output=True)
            if result.returncode:
                errors.append(f"{archive}: nm failed: {result.stderr.strip()}")
            elif FORBIDDEN.search(result.stdout):
                errors.append(f"{archive}: product-private undefined symbol")
    if errors:
        print("\n".join(errors), file=sys.stderr)
        return 1
    print("PROCESS GLOBAL BOUNDARY PASS source=4" + (f" archives={len(args.archive)}" if args.nm else ""))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
