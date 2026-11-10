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
for bits, source in ((32, dll32), (64, dll64)):
    shared = source.with_name(f"ntwshk{bits}.dll")
    shutil.copy2(shared, package / shared.name)
for kind in ("legacy", "modern"):
    result = subprocess.run(
        [str(package / launcher.name), "--wait", str(package / fixture.name), kind],
        capture_output=True,
        timeout=15,
    )
    print(f"{kind}: exit={result.returncode} stdout={result.stdout!r} stderr={result.stderr!r}")
    if result.returncode != 0 or b"DEFER waiting=0" not in result.stdout:
        sys.exit(1)
