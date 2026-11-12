#include <windows.h>
#include <stdio.h>
#include <wchar.h>
#include "lib-win32/process-global/window.h"

static unsigned injected,walk_injected;static BOOL inject_sweep;static DWORD walk_error=ERROR_ACCESS_DENIED;
static DWORD WINAPI fail_start_wait(DWORD count,const HANDLE *handles,BOOL all,DWORD timeout);
static BOOL fail_window_walk(DWORD thread,ULONGLONG deadline,win32_process_global_visit_window visit,void *context);
#define WaitForMultipleObjects fail_start_wait
#define win32_process_global_window_walk fail_window_walk
#include "../../src/run16s-exe/global.c"
#undef WaitForMultipleObjects
#undef win32_process_global_window_walk

static DWORD WINAPI fail_start_wait(DWORD count,const HANDLE *handles,BOOL all,DWORD timeout)
{
    (void)count;(void)handles;(void)all;(void)timeout;
    if(inject_sweep)return WaitForMultipleObjects(count,handles,all,timeout);
    ++injected;SetLastError(ERROR_ACCESS_DENIED);return WAIT_FAILED;
}
static BOOL fail_window_walk(DWORD thread,ULONGLONG deadline,win32_process_global_visit_window visit,void *context)
{
    if(!inject_sweep)return win32_process_global_window_walk(thread,deadline,visit,context);
    ++walk_injected;SetLastError(walk_error);return FALSE;
}
int wmain(int argc,WCHAR **argv)
{
    if(argc==2 && !wcscmp(argv[1],L"--unsupported-style")){
        int result=run16s_global_with_parent(NULL,0xffffffffu);
        printf("GLOBAL STYLE UNSUPPORTED result=%d\n",result);return result==10?0:4;
    }
    if(argc==2 && !wcscmp(argv[1],L"--timeout")){
        inject_sweep=TRUE;walk_error=ERROR_TIMEOUT;
        DWORD result=sweep(1,1,0,NTW16S_GLOBAL_APPLY);
        printf("GLOBAL SWEEP TIMEOUT result=%lu injected=%u\n",result,walk_injected);
        return result==ERROR_SUCCESS && walk_injected==1?0:4;
    }
    inject_sweep=argc==2 && !wcscmp(argv[1],L"--sweep");
    if(argc>1 && !inject_sweep)return 3;
    WCHAR desktop[128],name[NTW16S_GLOBAL_NAME_CAP];DWORD needed=0;
    if(!GetUserObjectInformationW(GetThreadDesktop(GetCurrentThreadId()),UOI_NAME,desktop,sizeof(desktop),&needed) ||
       wcsncmp(desktop,L"Win16S-Dialog-",14) || !ntw16s_global_name(name,ARRAYSIZE(name)))return 1;
    int result=run16s_global_with_parent(NULL,NTW16S_GLOBAL_STYLE_16);
    HANDLE mapping=OpenFileMappingW(FILE_MAP_READ,FALSE,name);DWORD error=mapping?ERROR_SUCCESS:GetLastError();
    if(mapping)CloseHandle(mapping);
    BOOL gone=!mapping && error==ERROR_FILE_NOT_FOUND;
    printf("GLOBAL START %s FAILURE result=%d injected=%u gone=%d\n",inject_sweep?"SWEEP":"WAIT",result,inject_sweep?walk_injected:injected,gone);
    return result==10 && (inject_sweep?walk_injected==2:injected==1) && gone?0:2;
}
