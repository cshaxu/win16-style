"""Compare native and styled CreateWindowInBand in an isolated package."""
import pathlib
import shutil
import subprocess
import sys

launcher, dll32, dll64, fixture = map(pathlib.Path, sys.argv[1:5])
package = pathlib.Path.cwd() / "package"
package.mkdir()
for source in (launcher, dll32, dll64, fixture):
    shutil.copy2(source, package / source.name)
native = subprocess.run([str(package / fixture.name)], capture_output=True, timeout=15)
styled = subprocess.run([str(package / launcher.name), "--wait", str(package / fixture.name), "styled"], capture_output=True, timeout=15)
print(f"native={native.returncode} {native.stdout!r}; styled={styled.returncode} {styled.stdout!r}")
if native.returncode == 77 and styled.returncode == 77:
    sys.exit(77)
invalid = b"BAND INVALID window=0 error=1407"
sys.exit(0 if native.returncode == 0 and styled.returncode == 0 and b"attached=0 expected=0" in native.stdout and b"attached=1 expected=1" in styled.stdout and b"BAND REMOVE removed=1 remaining=0" in styled.stdout and invalid in native.stdout and invalid in styled.stdout else 1)
