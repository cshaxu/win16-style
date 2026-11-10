"""Exercise the real launcher with product DLLs and targets in distinct directories."""
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile

if len(sys.argv) != 9 or sys.argv[1] not in ('--default', '--16', '--2k'):
    raise SystemExit('expected style, launcher, four DLLs, x86 target and x64 target')
style = sys.argv[1]
launcher, style32, style64, shared32, shared64, target32, target64 = map(
    lambda value: Path(value).resolve(strict=True), sys.argv[2:])
root = Path(os.environ['WIN16STYLE_CAPTURE_DIR']).resolve(strict=True)
with tempfile.TemporaryDirectory(prefix='shared dll ', dir=root) as temp:
    base = Path(temp)
    package, app, working = (base / name for name in ('product files', 'target files', 'working directory'))
    for directory in (package, app, working):
        directory.mkdir()
    for source in (launcher, style32, style64, shared32, shared64):
        shutil.copyfile(source, package / source.name)
    targets = []
    for source, name in ((target32, 'target32.exe'), (target64, 'target64.exe')):
        destination = app / name
        shutil.copyfile(source, destination)
        targets.append(destination)
    for parent in targets:
        for child in targets:
            options = [] if style == '--default' else [style]
            result = subprocess.run([str(package / launcher.name), *options, '--wait', str(parent), str(child)],
                                    cwd=working, capture_output=True, timeout=25)
            if result.returncode:
                raise SystemExit(f'SHARED PATH FAIL {parent.name}->{child.name} exit={result.returncode} '
                                 f'out={result.stdout!r} err={result.stderr!r}')
            print(f'SHARED PATH PASS {parent.name}->{child.name}')
print('SHARED PATH PASS separate_package_target_cwd=1 widths=4')
