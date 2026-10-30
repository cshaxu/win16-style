"""Optional end-to-end copy-count check using only Microsoft Print to PDF."""
import argparse
import re
import subprocess
import sys
import time
import tempfile
from pathlib import Path

parser = argparse.ArgumentParser()
parser.add_argument('--driver', required=True, type=Path)
parser.add_argument('--launcher', required=True, type=Path)
parser.add_argument('--target32', required=True, type=Path)
parser.add_argument('--target64', required=True, type=Path)
parser.add_argument('--pdfinfo', required=True, type=Path)
parser.add_argument('--scratch', required=True, type=Path)
parser.add_argument('--native', action='store_true')
args = parser.parse_args()
repo = Path(__file__).resolve().parents[2]
build = (repo / 'build').resolve()
scratch = args.scratch.resolve()
if not scratch.is_dir() or not scratch.is_relative_to(build):
    parser.error('scratch must be an existing directory under this checkout build/')
if not args.pdfinfo.is_file():
    print('VIRTUAL_PRINT_SKIP pdfinfo_unavailable')
    sys.exit(77)
for path in (args.driver, args.launcher, args.target32, args.target64):
    if not path.is_file():
        parser.error(f'missing executable: {path}')

with tempfile.TemporaryDirectory(prefix='virtual-print-', dir=scratch) as tmp:
    for bits, target in (('32', args.target32), ('64', args.target64)):
        for encoding in ('ansi', 'wide'):
            for action in ('print', 'apply'):
                output = Path(tmp) / f'{bits}-{encoding}-{action}.pdf'
                command = [str(args.driver), '--private', '--direct' if args.native else str(args.launcher),
                           str(target), action, str(output), encoding]
                try:
                    result = subprocess.run(command, capture_output=True, text=True, timeout=45)
                except subprocess.TimeoutExpired:
                    print(f'VIRTUAL_PRINT_FAIL {bits} {encoding} {action} driver_timeout')
                    sys.exit(1)
                if result.returncode == 77 and 'VIRTUAL_SKIP ' in result.stdout:
                    print('VIRTUAL_PRINT_SKIP printer_unavailable_or_configuration_mismatch')
                    sys.exit(77)
                identity = re.search(r'^VIRTUAL_RESULT bits=(\d+) ansi=(\d+) ', result.stdout, re.MULTILINE)
                if not identity or identity.group(1) != bits or identity.group(2) != ('1' if encoding == 'ansi' else '0'):
                    print(f'VIRTUAL_PRINT_FAIL {bits} {encoding} {action} target_identity_mismatch')
                    print(result.stdout, result.stderr)
                    sys.exit(1)
                if result.returncode or 'VIRTUAL_END page=1 text=1 endpage=1 enddoc=1' not in result.stdout:
                    print(f'VIRTUAL_PRINT_FAIL {bits} {encoding} {action} exit={result.returncode}')
                    print(result.stdout, result.stderr)
                    sys.exit(1)
                deadline = time.monotonic() + 10
                info = None
                match = None
                while time.monotonic() < deadline:
                    if output.is_file():
                        try:
                            info = subprocess.run([str(args.pdfinfo), str(output)], capture_output=True, text=True, timeout=5)
                        except subprocess.TimeoutExpired:
                            print(f'VIRTUAL_PRINT_FAIL {bits} {encoding} {action} pdfinfo_timeout')
                            sys.exit(1)
                        match = re.search(r'^Pages:\s*(\d+)\s*$', info.stdout, re.MULTILINE)
                        if info.returncode == 0 and match:
                            break
                    time.sleep(0.2)
                if info is None:
                    print(f'VIRTUAL_PRINT_FAIL {bits} {encoding} {action} output_missing')
                    sys.exit(1)
                if info.returncode or not match or int(match.group(1)) != 3:
                    print(f'VIRTUAL_PRINT_FAIL {bits} {encoding} {action} pages={match.group(1) if match else "unknown"}')
                    print(info.stderr)
                    sys.exit(1)
                print(f'VIRTUAL_PRINT_PASS {bits} {encoding} {action} pages=3')
