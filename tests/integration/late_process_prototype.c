#define _WIN32_WINNT 0x0A00
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>
#include "ntw16s-dll/injection/intercept.h"
#include "ntw16s-dll/injection/late_process.h"
ntw16s_context ntw16s_process_context;
static volatile LONG running=1,ticks;
/* The ordinary-startup backend is not linked into this isolated fixture. */
DWORD ntw16s_hooks_attach(void *slot,void *replacement)
{(void)slot;(void)replacement;return ERROR_NOT_SUPPORTED;}
static DWORD WINAPI background(void *p)
{(void)p;while(InterlockedCompareExchange(&running,0,0)){InterlockedIncrement(&ticks);Sleep(1);}return 0;}
int wmain(int argc,WCHAR **argv)
{
    if(argc==2 && !wcscmp(argv[1],L"existing")){
        BYTE before_w[16],before_a[16];memcpy(before_w,(const void *)CreateProcessW,sizeof(before_w));memcpy(before_a,(const void *)CreateProcessA,sizeof(before_a));
        ntw16s_process_context.flags=NTW16S_POLICY_STYLE|NTW16S_POLICY_PROPAGATE;
        DWORD status=ntw16s_late_process_init(L"C:\\fixture");
        BOOL unchanged=!memcmp(before_w,(const void *)CreateProcessW,sizeof(before_w)) && !memcmp(before_a,(const void *)CreateProcessA,sizeof(before_a));
        printf("LATE EXISTING bits=%u status=%lu unchanged=%d\n",(unsigned)(sizeof(void*)*8),status,unchanged);
        return !status && unchanged?0:1;
    }
    if(argc!=4 || (wcscmp(argv[3],L"A") && wcscmp(argv[3],L"W")))return 2;
    WCHAR directory[NTW16S_PATH_CAP];DWORD length=GetFullPathNameW(argv[2],NTW16S_PATH_CAP,directory,NULL);
    if(!length || length>=NTW16S_PATH_CAP)return 3;
    HANDLE thread=CreateThread(NULL,0,background,NULL,0,NULL);if(!thread)return 4;
    while(!InterlockedCompareExchange(&ticks,0,0))Sleep(1);
    DWORD error=ntw16s_late_process_init(directory);
    if(error){printf("LATE PROCESS install=%lu\n",error);return 6;}
    DWORD repeat=ntw16s_late_process_init(directory);if(repeat)return 5;
    PROCESS_INFORMATION child={0};BOOL made=FALSE;
    if(!wcscmp(argv[3],L"W")){
        WCHAR command[32768];STARTUPINFOW si={.cb=sizeof(si)};
        if(swprintf(command,ARRAYSIZE(command),L"\"%ls\"",argv[1])<0)return 7;
        made=CreateProcessW(argv[1],command,NULL,NULL,TRUE,0,NULL,NULL,&si,&child);
    }else{
        char image[32768],command[32768];STARTUPINFOA si={.cb=sizeof(si)};BOOL replaced=FALSE,utf8=GetACP()==CP_UTF8;
        if(!WideCharToMultiByte(CP_ACP,utf8?0:WC_NO_BEST_FIT_CHARS,argv[1],-1,image,sizeof(image),NULL,utf8?NULL:&replaced)||replaced)return 8;
        if(snprintf(command,sizeof(command),"\"%s\"",image)<0)return 8;
        made=CreateProcessA(image,command,NULL,NULL,TRUE,0,NULL,NULL,&si,&child);
    }
    DWORD code=99;if(made){if(WaitForSingleObject(child.hProcess,10000)==WAIT_OBJECT_0)GetExitCodeProcess(child.hProcess,&code);else{TerminateProcess(child.hProcess,ERROR_TIMEOUT);WaitForSingleObject(child.hProcess,1000);}CloseHandle(child.hThread);CloseHandle(child.hProcess);}
    InterlockedExchange(&running,0);DWORD joined=WaitForSingleObject(thread,2000);CloseHandle(thread);
    printf("LATE PROCESS bits=%u api=%ls init=1 repeat=1 created=%d exit=%lu joined=%d\n",(unsigned)(sizeof(void*)*8),argv[3],made,code,joined==WAIT_OBJECT_0);
    /* Test process exits with detours and trampoline storage still live. */
    return made && code==0 && joined==WAIT_OBJECT_0?0:1;
}
