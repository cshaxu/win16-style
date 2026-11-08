"""Exercise XAML frame activation through an isolated run16s package."""
import pathlib
import shutil
import subprocess
import sys

launcher, dll32, dll64, fixture = map(pathlib.Path, sys.argv[1:5])
package = pathlib.Path.cwd() / "package"
package.mkdir()
for source in (launcher, dll32, dll64, fixture):
    shutil.copy2(source, package / source.name)
result = subprocess.run(
    [str(package / launcher.name), "--wait", str(package / fixture.name)],
    capture_output=True,
    timeout=15,
)
print(f"exit={result.returncode} stdout={result.stdout!r} stderr={result.stderr!r}")
sys.exit(0 if result.returncode == 0 and b"DEFER waiting=0" in result.stdout else 1)
