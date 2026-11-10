#include "run16s-exe/elevation.h"
#include "lib-win32/security/elevation.h"
#include <stdio.h>
#include <wchar.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"UAC UNIT FAIL line=%d error=%lu\n",__LINE__,GetLastError());return 1;}}while(0)
static DWORD mode,calls;
static BOOL request_valid;
static BOOL expected_wait=TRUE;
static BOOL WINAPI shell_probe(SHELLEXECUTEINFOW *info)
{
    WCHAR cwd[32768];GetCurrentDirectoryW(32768,cwd);++calls;
    request_valid=info->cbSize==sizeof(*info)&&!wcscmp(info->lpVerb,L"runas")&&
        info->lpFile && wcsstr(info->lpFile,L"uac-unit") && !wcscmp(info->lpDirectory,cwd)&&
        (info->fMask&SEE_MASK_NOCLOSEPROCESS) && (info->fMask&SEE_MASK_NOASYNC)&&
        !wcscmp(info->lpParameters,expected_wait?L"--elevated --wait \"C:\\space dir\\target.exe\" \"\" \"a b\" tail\\":L"--elevated \"C:\\space dir\\target.exe\" \"\" \"a b\" tail\\");
    if(mode==1){SetLastError(ERROR_CANCELLED);return FALSE;}
    if(mode==2){SetLastError(ERROR_ACCESS_DENIED);return FALSE;}
    if(mode==3)return TRUE;
    if(mode==4){SetLastError(0);return FALSE;}
    WCHAR system[MAX_PATH],command[MAX_PATH+64];GetSystemDirectoryW(system,MAX_PATH);
    swprintf(command,MAX_PATH+64,L"\"%ls\\cmd.exe\" /d /c exit /b 73",system);
    PROCESS_INFORMATION p={0};STARTUPINFOW si={.cb=sizeof(si)};
    if(!CreateProcessW(NULL,command,NULL,NULL,FALSE,CREATE_NO_WINDOW,NULL,NULL,&si,&p))return FALSE;
    info->hProcess=p.hProcess;CloseHandle(p.hThread);return TRUE;
}
static BOOL WINAPI global_shell_probe(SHELLEXECUTEINFOW *info)
{
    ++calls;
    request_valid=info->cbSize==sizeof(*info) && !wcscmp(info->lpVerb,L"runas") &&
        info->lpFile && wcsstr(info->lpFile,L"uac-unit") && !wcscmp(info->lpParameters,L"--global-elevated 12345") &&
        (info->fMask&SEE_MASK_NOCLOSEPROCESS) && (info->fMask&SEE_MASK_NOASYNC) && (info->fMask&SEE_MASK_NO_CONSOLE);
    if(mode==1){SetLastError(ERROR_CANCELLED);return FALSE;}
    if(mode==3)return TRUE;
    WCHAR system[MAX_PATH],command[MAX_PATH+64];GetSystemDirectoryW(system,MAX_PATH);
    swprintf(command,MAX_PATH+64,L"\"%ls\\cmd.exe\" /d /c exit /b 73",system);
    PROCESS_INFORMATION p={0};STARTUPINFOW si={.cb=sizeof(si)};
    if(!CreateProcessW(NULL,command,NULL,NULL,FALSE,CREATE_NO_WINDOW,NULL,NULL,&si,&p))return FALSE;
    info->hProcess=p.hProcess;CloseHandle(p.hThread);return TRUE;
}
int wmain(void)
{
    DWORD error,exit_code=999,before=0,after=0;
    HANDLE borrowed=CreateEventW(NULL,TRUE,FALSE,NULL);CHECK(borrowed);
    SHELLEXECUTEINFOW invalid={.cbSize=sizeof(invalid),.hProcess=borrowed};
    CHECK(win32_security_shell_wait(&invalid,shell_probe,FALSE,&exit_code)==ERROR_INVALID_PARAMETER);
    CHECK(WaitForSingleObject(borrowed,0)==WAIT_TIMEOUT);CloseHandle(borrowed);
    (void)win32_security_is_elevated(&error);CHECK(!error);
    /* Warm COM caches before the per-call handle ownership check. */
    CHECK(!run16s_elevate(L"C:\\space dir\\target.exe",L"\"\" \"a b\" tail\\",TRUE,shell_probe,&exit_code));
    CHECK(request_valid&&exit_code==73);
    CHECK(GetProcessHandleCount(GetCurrentProcess(),&before));
    for(mode=0;mode<5;mode++){
        error=run16s_elevate(L"C:\\space dir\\target.exe",L"\"\" \"a b\" tail\\",TRUE,shell_probe,&exit_code);
        CHECK(request_valid);
        CHECK(error==(mode==0?0:mode==1?ERROR_CANCELLED:mode==2?ERROR_ACCESS_DENIED:mode==3?ERROR_INVALID_HANDLE:ERROR_GEN_FAILURE));
    }
    mode=0;expected_wait=FALSE;
    CHECK(!run16s_elevate(L"C:\\space dir\\target.exe",L"\"\" \"a b\" tail\\",FALSE,shell_probe,&exit_code));CHECK(request_valid);
    CHECK(GetProcessHandleCount(GetCurrentProcess(),&after));CHECK(before==after&&calls==7);
    WCHAR long_arguments[32768];wmemset(long_arguments,L'x',32767);long_arguments[32767]=0;
    CHECK(run16s_elevate(L"C:\\target.exe",long_arguments,FALSE,shell_probe,&exit_code)==ERROR_FILENAME_EXCED_RANGE);
    CHECK(calls==7);
    mode=0;CHECK(!run16s_elevate_global(12345,global_shell_probe,&exit_code) && request_valid && exit_code==73);
    mode=1;CHECK(run16s_elevate_global(12345,global_shell_probe,&exit_code)==ERROR_CANCELLED && request_valid);
    mode=3;CHECK(run16s_elevate_global(12345,global_shell_probe,&exit_code)==ERROR_INVALID_HANDLE && request_valid);
    CHECK(run16s_elevate_global(0,global_shell_probe,&exit_code)==ERROR_INVALID_PARAMETER);
    puts("UAC UNIT PASS cases=8 cancel=1223 denied=5 missing_handle=6 exit=73 handles=stable");
    puts("GLOBAL UAC UNIT PASS success=73 cancel=1223 missing_handle=6 invalid=87");return 0;
}
