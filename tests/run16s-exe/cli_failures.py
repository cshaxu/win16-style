"""Actual CLI rollback before target entry, with isolated package mutations."""
import os
import ctypes
from ctypes import wintypes
from pathlib import Path
import shutil
import subprocess
import sys

class ProcessEntry(ctypes.Structure):
    _fields_=[('size',wintypes.DWORD),('usage',wintypes.DWORD),('pid',wintypes.DWORD),
              ('heap',ctypes.c_size_t),('module',wintypes.DWORD),('threads',wintypes.DWORD),
              ('parent',wintypes.DWORD),('priority',wintypes.LONG),('flags',wintypes.DWORD),
              ('image',wintypes.WCHAR*260)]

kernel=ctypes.WinDLL('kernel32',use_last_error=True)
kernel.CreateToolhelp32Snapshot.argtypes=[wintypes.DWORD,wintypes.DWORD]
kernel.CreateToolhelp32Snapshot.restype=wintypes.HANDLE
kernel.Process32FirstW.argtypes=[wintypes.HANDLE,ctypes.POINTER(ProcessEntry)]
kernel.Process32NextW.argtypes=[wintypes.HANDLE,ctypes.POINTER(ProcessEntry)]
kernel.CloseHandle.argtypes=[wintypes.HANDLE]

def process_present(image):
    snapshot=kernel.CreateToolhelp32Snapshot(2,0)
    if snapshot==ctypes.c_void_p(-1).value:raise ctypes.WinError(ctypes.get_last_error())
    try:
        entry=ProcessEntry();entry.size=ctypes.sizeof(entry)
        more=kernel.Process32FirstW(snapshot,ctypes.byref(entry))
        while more:
            if entry.image.casefold()==image.casefold():return True
            more=kernel.Process32NextW(snapshot,ctypes.byref(entry))
        error=ctypes.get_last_error()
        if error!=18:raise ctypes.WinError(error)
        return False
    finally:
        kernel.CloseHandle(snapshot)

if not process_present(Path(sys.executable).name):raise SystemExit('Process snapshot self-check failed')

launcher,dll32,dll64,target32,_=map(Path,sys.argv[1:])
root=Path(os.environ['WIN16STYLE_CAPTURE_DIR'])/'cli-failures'
root.mkdir()
def stage(folder):
    folder.mkdir()
    for source in (launcher,dll32,dll64):
        shutil.copy2(source,folder/source.name)
    for family in ('Win16','Win2K'):
        shutil.copytree(launcher.parent/family,folder/family)

# The x64 launcher imports ntwshk64.dll. Only the x86 payload can be
# invalidated while the launcher still reaches target-creation rollback.
for family in ('Win16','Win2K'):
    for case in ('missing','wrong-width','corrupt'):
        folder=root/f'{family}-32-{case}'
        stage(folder)
        changed=folder/dll32.name
        if case=='missing':changed.unlink()
        if case=='wrong-width':shutil.copy2(dll64,changed)
        if case=='corrupt':changed.write_bytes(b'MZ truncated native image')
        owned_target=folder/f'cli-{os.getpid()}-{family}-32-{case}.exe'
        shutil.copy2(target32,owned_target)
        if process_present(owned_target.name):raise SystemExit(f'Target name already in use: {owned_target.name}')
        env=dict(os.environ,WIN16STYLE_CAPTURE_DIR=str(folder))
        result=subprocess.run([str(folder/launcher.name),family,str(owned_target),
                               'styled','two words','quote"inside','trailing\\','','中文 αβ'],
                              env=env,capture_output=True,timeout=20)
        error=2 if case=='missing' else 193
        if result.returncode!=6 or b'installation/resume' not in result.stderr or f'({error})'.encode() not in result.stderr:
            raise SystemExit(f'CLI ROLLBACK FAIL style={family} {case} code={result.returncode} stderr={result.stderr!r}')
        if list(folder.glob('target-*.txt')):raise SystemExit(f'Target entered after failed installation: {family} {case}')
        if process_present(owned_target.name):raise SystemExit(f'Target remains after rollback: {family} {case}')
        print(f'{family} ROLLBACK PASS width=32 {case}, Windows error={error}, no target entry, no target process')

# Public CLI boundary: actual Windows native programs consume stdin, produce
# both output streams, and return a distinctive exit code. No product test flag.
healthy=root/'healthy'
stage(healthy)
windows=ctypes.create_unicode_buffer(32768)
count=ctypes.windll.kernel32.GetWindowsDirectoryW(windows,len(windows))
if not 0<count<len(windows):raise SystemExit('Windows directory query failed')
command='set /p WIN16STYLE_IO=& echo STDOUT:!WIN16STYLE_IO!& echo STDERR:ok 1>&2& exit /b 37'
for width,system in [(32,'SysWOW64'),(64,'System32')]:
    for family in ('Win16','Win2K'):
        arguments=[str(healthy/'run16s.exe'),family,
                   str(Path(windows.value)/system/'cmd.exe'),'/d','/v:on','/c',command]
        result=subprocess.run(arguments,input=b'piped input\r\n',capture_output=True,timeout=20)
        if (result.returncode!=37 or b'STDOUT:piped input' not in result.stdout
                or b'STDERR:ok' not in result.stderr):
            raise SystemExit(f'CLI IO/EXIT FAIL width={width} wait={family}: {result.returncode} {result.stdout!r} {result.stderr!r}')
        print(f'CLI IO/EXIT PASS width={width} wait={family}: stdin/stdout/stderr preserved, exit={result.returncode}')
