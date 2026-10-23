"""Launch both native widths from explicit extended paths longer than MAX_PATH."""
import os
from pathlib import Path
import shutil
import subprocess
import sys

launcher, dll32, dll64, target32, target64 = map(Path, sys.argv[1:])
scratch = Path(os.environ['WIN16STYLE_CAPTURE_DIR'])/'long-path'
scratch.mkdir()
package = scratch/'package'
package.mkdir()
for source, name in [(launcher, 'run16s.exe'), (dll32, 'ntw16s32.dll'), (dll64, 'ntw16s64.dll')]:
    shutil.copy2(source, package/name)
deep = Path('\\\\?\\'+str(scratch.resolve()))
for letter in 'xyz':
    deep = deep/('segment-'+letter*75)
deep.mkdir(parents=True)
for width, source in [(32, target32), (64, target64)]:
    target = deep/f'long target {width}.exe'
    shutil.copy2(source, target)
    assert len(str(target)) > 300
    marker = scratch/f'entry-{width}.txt'
    result = subprocess.run([str(package/'run16s.exe'), '--wait', str(target), '--entry', str(marker)],
                            capture_output=True, timeout=30)
    assert result.returncode == 42 and marker.exists(), (width, result.returncode, result.stderr)
    print(f'LONG PATH PASS width={width} characters={len(str(target))}, actual CLI/install/entry/exit42')
