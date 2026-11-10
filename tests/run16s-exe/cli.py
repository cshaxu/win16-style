"""Exercise actual CLI diagnostics without allowing a target to execute."""
import os
from pathlib import Path
import subprocess
import sys

exe=Path(sys.argv[1]).resolve()
scratch=Path(os.environ['WIN16STYLE_CAPTURE_DIR'])
bad=scratch/'not-a-native-image.exe'
bad.write_bytes(b'MZ\0not a PE')
cases=[([],2),(['--help'],0),(['--wait'],2),
       (['does-not-exist-win16style.exe'],3),([str(bad)],4)]
cases += [(['--16',*arguments],expected) for arguments,expected in cases]
cases += [(['--2k','--help'],0),(['--2k','--global'],10),(['--2k','--app','x'],10),
          (['--16','--2k'],2),(['--16','--16'],2),(['--2k','--16'],2)]
for arguments,expected in cases:
    run=subprocess.run([str(exe),*arguments],capture_output=True,timeout=10)
    if run.returncode!=expected:
        raise SystemExit(f'CLI FAIL {arguments}: {run.returncode}, expected {expected}, {run.stderr!r}')
print('CLI PASS usage/help/wait argument, absent target and malformed native image; no target executed')
