"""Exercise actual CLI diagnostics without allowing a target to execute."""
import os
from pathlib import Path
import subprocess
import sys

exe=Path(sys.argv[1]).resolve()
scratch=Path(os.environ['WIN16STYLE_CAPTURE_DIR'])
bad=scratch/'not-a-native-image.exe'
bad.write_bytes(b'MZ\0not a PE')
cases=[([],2),(['--help'],2),(['--wait'],2),(['notepad.exe'],2),
       (['notepad.exe',' /?'],6),(['no-such-theme-run16s','notepad.exe'],6),
       (['--global','unexpected'],2),(['--global','Win16'],2)]
for theme in ('Win16','win2k'):
    cases += [([theme],2),([theme,''],2),([theme,'--global'],2),
              ([theme,'--wait','notepad.exe'],2),([theme,'--app','x'],2),
              ([theme,'does-not-exist-win16style.exe'],3),([theme,str(bad)],4)]
cases += [([removed,mode],2) for removed in ('--16','--2k') for mode in ('--help','--global')]
for arguments,expected in cases:
    run=subprocess.run([str(exe),*arguments],capture_output=True,timeout=10)
    if run.returncode!=expected:
        raise SystemExit(f'CLI FAIL {arguments}: {run.returncode}, expected {expected}, {run.stderr!r}')
print('CLI PASS usage/help/wait argument, absent target and malformed native image; no target executed')
