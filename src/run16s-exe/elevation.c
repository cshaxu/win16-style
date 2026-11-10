#include "elevation.h"
#include "lib-win32/security/elevation.h"
#include <objbase.h>
#include <stdlib.h>
#include <wchar.h>
#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <stdint.h>

void run16s_rebind_console_stderr(void)
{
    /* A process may have stale CRT handles after attaching to a console.
     * Keep stderr open if binding fails. */
    HANDLE console=CreateFileW(L"CONOUT$",GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE,
        NULL,OPEN_EXISTING,0,NULL);
    if(console==INVALID_HANDLE_VALUE)return;
    int fd=_open_osfhandle((intptr_t)console,_O_WRONLY|_O_TEXT);
    if(fd<0){CloseHandle(console);return;}
    if(fd!=2){(void)_dup2(fd,2);_close(fd);}
    clearerr(stderr);
}
BOOL run16s_global_interactive_console(void)
{
    DWORD mode=0,pids[1];HANDLE error=GetStdHandle(STD_ERROR_HANDLE);
    return error && error!=INVALID_HANDLE_VALUE && GetFileType(error)==FILE_TYPE_CHAR &&
        GetConsoleMode(error,&mode) && GetConsoleProcessList(pids,1)>0;
}
DWORD run16s_elevate_global(DWORD parent_pid,run16s_shell_execute execute,DWORD *exit_code)
{
    WCHAR self[32768],directory[32768],parameters[80];DWORD n,error=ERROR_SUCCESS;
    if(!parent_pid || !execute || !exit_code)return ERROR_INVALID_PARAMETER;
    n=GetModuleFileNameW(NULL,self,ARRAYSIZE(self));if(!n || n>=ARRAYSIZE(self))return n?ERROR_FILENAME_EXCED_RANGE:GetLastError();
    n=GetCurrentDirectoryW(ARRAYSIZE(directory),directory);if(!n || n>=ARRAYSIZE(directory))return n?ERROR_FILENAME_EXCED_RANGE:GetLastError();
    if(swprintf(parameters,ARRAYSIZE(parameters),L"--global-elevated %lu",parent_pid)<0)return ERROR_INSUFFICIENT_BUFFER;
    SHELLEXECUTEINFOW info={.cbSize=sizeof(info),.fMask=SEE_MASK_NOCLOSEPROCESS|SEE_MASK_NOASYNC|SEE_MASK_NO_CONSOLE,
        .lpVerb=L"runas",.lpFile=self,.lpParameters=parameters,.lpDirectory=directory,.nShow=SW_HIDE};
    error=win32_security_shell_wait(&info,execute,TRUE,exit_code);
    return error;
}
DWORD run16s_elevate(PCWSTR image,PCWSTR arguments,BOOL wait,BOOL win2k,
    run16s_shell_execute execute,DWORD *exit_code)
{
    WCHAR self[32768],directory[32768];DWORD n=GetModuleFileNameW(NULL,self,32768);
    if(!n||n>=32768)return n?ERROR_FILENAME_EXCED_RANGE:GetLastError();
    n=GetCurrentDirectoryW(32768,directory);
    if(!n||n>=32768)return n?ERROR_FILENAME_EXCED_RANGE:GetLastError();
    /* Pin the target resolved by the original caller. Only argv[0] is
     * canonicalized; the argument tail is copied verbatim, never reparsed. */
    size_t length=wcslen(image)+wcslen(arguments)+40;
    if(length>32767)return ERROR_FILENAME_EXCED_RANGE;
    WCHAR *parameters=calloc(length,sizeof(WCHAR));
    if(!parameters)return ERROR_NOT_ENOUGH_MEMORY;
    swprintf(parameters,length,L"%ls--elevated %ls\"%ls\"%ls%ls",
        win2k?L"--2k ":L"",wait?L"--wait ":L"",image,*arguments?L" ":L"",arguments);
    SHELLEXECUTEINFOW info={.cbSize=sizeof(info),
        .fMask=SEE_MASK_NOCLOSEPROCESS|SEE_MASK_NOASYNC|SEE_MASK_FLAG_NO_UI,
        .lpVerb=L"runas",.lpFile=self,.lpParameters=parameters,
        .lpDirectory=directory,.nShow=SW_SHOWNORMAL};
    DWORD error=win32_security_shell_wait(&info,execute,FALSE,exit_code);
    free(parameters);return error;
}
