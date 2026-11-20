"""Actual cmd -> Console launcher -> held native target; no synthetic key input."""
import ctypes
from ctypes import wintypes as W
import os
import json
from pathlib import Path
import subprocess
import sys

launcher,probe32,probe64,gui=map(lambda p:Path(p).resolve(),sys.argv[1:])
root=Path(os.environ['WIN16STYLE_CAPTURE_DIR'])/'wait-contract'
root.mkdir()
k=ctypes.WinDLL('kernel32',use_last_error=True)
k.CreateEventW.argtypes=[ctypes.c_void_p,W.BOOL,W.BOOL,W.LPCWSTR];k.CreateEventW.restype=W.HANDLE
k.SetEvent.argtypes=[W.HANDLE];k.CloseHandle.argtypes=[W.HANDLE]
k.WaitForSingleObject.argtypes=[W.HANDLE,W.DWORD];k.WaitForSingleObject.restype=W.DWORD
k.OpenProcess.argtypes=[W.DWORD,W.BOOL,W.DWORD];k.OpenProcess.restype=W.HANDLE
k.GetExitCodeProcess.argtypes=[W.HANDLE,ctypes.POINTER(W.DWORD)]
cmd=Path(os.environ['SystemRoot'])/'System32/cmd.exe'
for index,(probe,blocking) in enumerate([(probe32,True),(probe64,True),(gui,False)]):
    case=root/str(index);case.mkdir();report=case/'ready.txt';after=case/'returned.txt'
    ready_name=f'Local\\Win16WaitReady-{os.getpid()}-{index}'
    release_name=f'Local\\Win16WaitRelease-{os.getpid()}-{index}'
    ready=k.CreateEventW(None,True,False,ready_name);release=k.CreateEventW(None,True,False,release_name)
    assert ready and release
    line=f'"{launcher}" Win16 "{probe}" "{report}" "{ready_name}" "{release_name}"'
    tail=f'{line} & set "code=!errorlevel!" & echo returned>"{after}" & exit /b !code!'
    command=f'"{cmd}" /d /v:on /s /c "{tail}"'
    startup=subprocess.STARTUPINFO();startup.dwFlags=subprocess.STARTF_USESHOWWINDOW;startup.wShowWindow=0
    process=subprocess.Popen(command,executable=str(cmd),stdout=subprocess.PIPE,stderr=subprocess.PIPE,startupinfo=startup)
    target=None;collected=False
    try:
        assert k.WaitForSingleObject(ready,10000)==0,('target not ready',index,process.poll())
        lines=report.read_text().splitlines();pid,parent,window,count=map(int,lines[0].split());members=set(map(int,lines[1:]))
        target=k.OpenProcess(0x100000|0x1000,False,pid);assert target
        (case/'identity.json').write_text(json.dumps(dict(target=pid,launcher=parent,cmd=process.pid,console_window=window,members=sorted(members)),indent=2))
        assert k.WaitForSingleObject(target,0)==258
        if blocking:
            assert k.WaitForSingleObject(int(process._handle),1000)==258 and not after.exists(),('parent cmd returned before target release',index)
        else:
            assert process.wait(timeout=5)==0 and after.exists(),('GUI default did not return',index)
            assert k.WaitForSingleObject(target,0)==258,('GUI target did not survive launcher',index)
        if probe!=gui:
            # Headless/CREATE_NO_WINDOW consoles can have a NULL HWND. The
            # public membership API still proves the same console identity.
            assert count>=3 and {pid,parent,process.pid}<=members,('console membership differs',index,lines)
        else:assert not window and not count,('GUI target acquired console',lines)
        assert k.SetEvent(release)
        out,err=process.communicate(timeout=10);collected=True
        assert process.returncode==(42 if blocking else 0),(index,process.returncode,out,err)
        assert k.WaitForSingleObject(target,10000)==0
        code=W.DWORD();assert k.GetExitCodeProcess(target,ctypes.byref(code)) and code.value==42
        (case/'stdout.log').write_bytes(out);(case/'stderr.log').write_bytes(err)
        print(f'WAIT CASE PASS case={index} console={probe!=gui} blocking={blocking} cmd={process.pid} target={pid} members={count} code={process.returncode}',flush=True)
    finally:
        k.SetEvent(release)
        if not collected:
            try:out,err=process.communicate(timeout=25)
            except subprocess.TimeoutExpired:process.kill();out,err=process.communicate()
            (case/'stdout.log').write_bytes(out);(case/'stderr.log').write_bytes(err)
        if target:k.CloseHandle(target)
        k.CloseHandle(ready);k.CloseHandle(release)
print('WAIT CONTRACT PASS 3 cases: x86/x64 console sync42, GUI async0, shared console, injected target')
