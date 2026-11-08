"""Run the native Shell handle fixture from an isolated two-architecture package."""
import pathlib
import shutil
import subprocess
import sys

launcher, dll32, dll64, fixture = map(pathlib.Path, sys.argv[1:5])
mode = sys.argv[5] if len(sys.argv) > 5 else "--handle-control"
if mode not in ("--handle-control", "--invalid", "--invalid-handle"):
    raise ValueError("unsupported fixture mode")
package = pathlib.Path.cwd() / "package"
package.mkdir()
for source in (launcher, dll32, dll64, fixture):
    shutil.copy2(source, package / source.name)
native = subprocess.run([str(package / fixture.name), mode], timeout=15, capture_output=True)
styled = subprocess.run([str(package / launcher.name), "--wait", str(package / fixture.name), mode], timeout=15, capture_output=True)
print(f"native={native.returncode} {native.stdout!r}; styled={styled.returncode} {styled.stdout!r}")
sys.exit(0 if native.returncode == 0 and styled.returncode == 0 and native.stdout == styled.stdout else 1)
