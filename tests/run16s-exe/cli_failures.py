"""Actual CLI rollback before target entry, with isolated package mutations."""
import os
import ctypes
from pathlib import Path
import shutil
import subprocess
import sys

launcher,dll32,dll64,target32,target64=map(Path,sys.argv[1:])
root=Path(os.environ['WIN16STYLE_CAPTURE_DIR'])/'cli-failures'
root.mkdir()
for width,target in [(32,target32),(64,target64)]:
    for case in ('missing','wrong-width','corrupt'):
        folder=root/f'{width}-{case}'
        folder.mkdir()
        exe=folder/'run16s.exe'
        shutil.copy2(launcher,exe)
        for bits,source in [(32,dll32),(64,dll64)]:
            destination=folder/f'ntw16s{bits}.dll'
            if bits!=width or case!='missing':shutil.copy2(source,destination)
        changed=folder/f'ntw16s{width}.dll'
        if case=='wrong-width':shutil.copy2(dll64 if width==32 else dll32,changed)
        if case=='corrupt':changed.write_bytes(b'MZ truncated native image')
        env=dict(os.environ,WIN16STYLE_CAPTURE_DIR=str(folder))
        result=subprocess.run([str(exe),'--wait',str(target),'styled','two words','quote"inside',
                               'trailing\\','','中文 αβ'],env=env,capture_output=True,timeout=20)
        error=2 if case=='missing' else 193
        if result.returncode!=6 or f'({error})'.encode() not in result.stderr:
            raise SystemExit(f'CLI ROLLBACK FAIL width={width} {case} code={result.returncode} stderr={result.stderr!r}')
        if list(folder.glob('target-*.txt')):raise SystemExit(f'Target entered after failed installation: {width} {case}')
        print(f'CLI ROLLBACK PASS width={width} {case}, Windows error={error}, no target entry')

# Public CLI boundary: actual Windows native programs consume stdin, produce
# both output streams, and return a distinctive exit code. No product test flag.
healthy=root/'healthy'
healthy.mkdir()
for source,name in [(launcher,'run16s.exe'),(dll32,'ntw16s32.dll'),(dll64,'ntw16s64.dll')]:
    shutil.copy2(source,healthy/name)
windows=ctypes.create_unicode_buffer(32768)
count=ctypes.windll.kernel32.GetWindowsDirectoryW(windows,len(windows))
if not 0<count<len(windows):raise SystemExit('Windows directory query failed')
command='set /p WIN16STYLE_IO=& echo STDOUT:!WIN16STYLE_IO!& echo STDERR:ok 1>&2& exit /b 37'
for width,system in [(32,'SysWOW64'),(64,'System32')]:
    for wait in (False,True):
        arguments=[str(healthy/'run16s.exe'),*(['--wait'] if wait else []),
                   str(Path(windows.value)/system/'cmd.exe'),'/d','/v:on','/c',command]
        result=subprocess.run(arguments,input=b'piped input\r\n',capture_output=True,timeout=20)
        if (result.returncode!=(37 if wait else 0) or b'STDOUT:piped input' not in result.stdout
                or b'STDERR:ok' not in result.stderr):
            raise SystemExit(f'CLI IO/EXIT FAIL width={width} wait={wait}: {result.returncode} {result.stdout!r} {result.stderr!r}')
        print(f'CLI IO/EXIT PASS width={width} wait={wait}: stdin/stdout/stderr preserved, exit={result.returncode}')
