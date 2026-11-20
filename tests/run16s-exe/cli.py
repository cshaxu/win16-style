"""Exercise actual CLI diagnostics without allowing a target to execute."""
import os
from pathlib import Path
import re
import shutil
import subprocess
import sys
import tempfile

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
def check(launcher,arguments,expected,message=b''):
    run=subprocess.run([str(launcher),*arguments],capture_output=True,timeout=10)
    if run.returncode!=expected or message not in run.stderr:
        raise SystemExit(f'CLI FAIL {arguments}: {run.returncode}, expected {expected}, {run.stderr!r}')

for arguments,expected in cases:
    check(exe,arguments,expected)

with tempfile.TemporaryDirectory(dir=scratch) as temporary:
    fixture=Path(temporary).resolve()
    launcher=fixture/exe.name
    shutil.copy2(exe,launcher)
    terminal=fixture/'WindowsTerminal.exe'
    terminal.write_bytes(bad.read_bytes())
    arguments=['Sample',str(terminal)]
    theme_dir=fixture/'Sample'
    for source,expected in (('Win16',4),('win2k',6)):
        shutil.copytree(exe.parent/source,theme_dir)
        theme=next(theme_dir.glob('*.theme'))
        descriptor=re.sub(rb'(?m)^\[WindowAttachment\]\r?\n[^\[]*',b'',theme.read_bytes())
        host_section=b'\r\n[WindowAttachment]\r\nVersion=1\r\nExplorer=0\r\nConsole=0\r\nTerminal='
        theme.write_bytes(descriptor+host_section+b'0\r\n')
        check(launcher,arguments,10,b'theme window attachment')
        theme.write_bytes(descriptor+host_section+b'1\r\n')
        check(launcher,arguments,expected)
        shutil.rmtree(theme_dir)

print('CLI PASS usage/help/wait argument, absent target and malformed native image; no target executed')
print('CLI attachment policy PASS renamed themes, requested host capability and renderer compatibility')
