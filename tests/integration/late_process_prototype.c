#define _WIN32_WINNT 0x0A00
#include <windows.h>
#include <stdio.h>
#include <wchar.h>
#include "ntw16s-dll/injection/intercept.h"
#include "ntw16s-dll/injection/late_patch.h"
#include "ntw16s-dll/injection/minhook/src/buffer.h"
ntw16s_context ntw16s_process_context;
static ntw16s_late_patch hooks[2];static ntw16s_late_patch *items[2];static DWORD count;
static volatile LONG running=1,ticks;
/* Test-only adapter: exercise the production CreateProcess detours with the
 * experimental late transaction, without calling the startup bootstrap. */
DWORD ntw16s_hooks_attach(void *slot,void *replacement)
{
    if(count>=ARRAYSIZE(hooks))return ERROR_INSUFFICIENT_BUFFER;
    DWORD error=ntw16s_late_patch_prepare(&hooks[count],slot,replacement);
    if(!error){items[count]=&hooks[count];++count;}return error;
}
static DWORD WINAPI background(void *p)
{(void)p;while(InterlockedCompareExchange(&running,0,0)){InterlockedIncrement(&ticks);Sleep(1);}return 0;}
int wmain(int argc,WCHAR **argv)
{
    if(argc!=4 || (wcscmp(argv[3],L"A") && wcscmp(argv[3],L"W")))return 2;
    DWORD length=GetFullPathNameW(argv[2],NTW16S_PATH_CAP,ntw16s_process_context.directory,NULL);
    if(!length || length>=NTW16S_PATH_CAP)return 3;
    ntw16s_process_context.flags=NTW16S_POLICY_STYLE|NTW16S_POLICY_PROPAGATE;
    HANDLE thread=CreateThread(NULL,0,background,NULL,0,NULL);if(!thread)return 4;
    while(!InterlockedCompareExchange(&ticks,0,0))Sleep(1);
    InitializeBuffer();LONG status=ntw16s_attach_processes();if(status || count!=2)return 5;
    ntw16s_late_guard guard={0};DWORD error=ERROR_BUSY;
    for(unsigned i=0;i<100 && (error==ERROR_BUSY||error==ERROR_RETRY);++i){error=ntw16s_late_patch_install_batch(&guard,items,count);if(error==ERROR_BUSY||error==ERROR_RETRY)Sleep(1);}
    if(error){printf("LATE PROCESS install=%lu\n",error);return 6;}
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
    printf("LATE PROCESS bits=%u api=%ls hooks=%lu created=%d exit=%lu joined=%d\n",(unsigned)(sizeof(void*)*8),argv[3],count,made,code,joined==WAIT_OBJECT_0);
    /* Test process exits with detours and trampoline storage still live. */
    return made && code==0 && joined==WAIT_OBJECT_0?0:1;
}
