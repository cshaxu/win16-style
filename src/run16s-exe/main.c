#include "ntw16s-dll/injection/injection.h"
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
/* MinGW CRT must preserve native arguments. Globbing otherwise interprets the
 * '?' in explicit extended paths (\\?\) before target resolution. */
int _dowildcard=0;
/* Locate the raw next argument without reserializing the target command line. */
static PCWSTR next_argument(PCWSTR p)
{
    BOOL quoted=FALSE;
    while(*p==L' ' || *p==L'\t')++p;
    while(*p){
        unsigned slashes=0;
        if(!quoted && (*p==L' ' || *p==L'\t'))break;
        while(*p==L'\\'){++slashes;++p;}
        if(*p==L'"'){if(!(slashes&1))quoted=!quoted;++p;}
        else if(*p && (quoted || (*p!=L' ' && *p!=L'\t')))++p;
    }
    while(*p==L' ' || *p==L'\t')++p;
    return p;
}
static int diagnostic(int code,DWORD error,PCWSTR phase)
{
    WCHAR message[512]={0};
    FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS,NULL,error,0,message,512,NULL);
    fwprintf(stderr,L"run16s: %ls failed (%lu): %ls\n",phase,error,message);return code;
}
int wmain(int argc,WCHAR **argv)
{
    int first=1;BOOL wait=FALSE;DWORD error,exit_code=0;
    WCHAR image[NTW16S_PATH_CAP];ntw16s_context context;
    STARTUPINFOW startup={.cb=sizeof(startup)};PROCESS_INFORMATION child={0};
    PCWSTR raw=next_argument(GetCommandLineW());PWSTR command;
    if(argc>1 && !wcscmp(argv[1],L"--wait")){wait=TRUE;first=2;raw=next_argument(raw);}
    if(argc<=first || !wcscmp(argv[first],L"--help")){
        fwprintf(argc<=first?stderr:stdout,L"Usage: run16s.exe [--wait] <application> [arguments]\n");
        return argc<=first?2:0;
    }
    DWORD length=SearchPathW(NULL,argv[first],L".exe",NTW16S_PATH_CAP,image,NULL);
    if(!length || length>=NTW16S_PATH_CAP)return diagnostic(3,length?ERROR_FILENAME_EXCED_RANGE:GetLastError(),L"target resolution");
    error=ntw16s_validate_image(image,IMAGE_FILE_MACHINE_AMD64,FALSE);
    if(error==ERROR_BAD_EXE_FORMAT)error=ntw16s_validate_image(image,IMAGE_FILE_MACHINE_I386,FALSE);
    if(error)return diagnostic(4,error,L"native target validation");
    error=ntw16s_context_paths(&context,NULL);if(error)return diagnostic(6,error,L"package directory");
    command=_wcsdup(raw);if(!command)return diagnostic(5,ERROR_NOT_ENOUGH_MEMORY,L"command allocation");
    if(!CreateProcessW(image,command,NULL,NULL,FALSE,CREATE_SUSPENDED,NULL,NULL,&startup,&child)){
        error=GetLastError();free(command);return diagnostic(5,error,L"target creation");
    }
    free(command);error=ntw16s_install(child.hProcess,&context,CreateProcessW);
    int failure=6;
    if(!error){DWORD previous;failure=7;previous=ResumeThread(child.hThread);
        if(previous!=1)error=previous==(DWORD)-1?GetLastError():ERROR_INVALID_STATE;}
    if(error){
        TerminateProcess(child.hProcess,error);WaitForSingleObject(child.hProcess,5000);
        CloseHandle(child.hThread);CloseHandle(child.hProcess);return diagnostic(failure,error,L"installation/resume");
    }
    CloseHandle(child.hThread);
    if(wait && (WaitForSingleObject(child.hProcess,INFINITE)!=WAIT_OBJECT_0 || !GetExitCodeProcess(child.hProcess,&exit_code))){
        error=GetLastError();CloseHandle(child.hProcess);return diagnostic(8,error,L"target wait");
    }
    CloseHandle(child.hProcess);return (int)exit_code;
}
