#include "global_late.h"
#include "global_session.h"
#include "injection/injection.h"
#include "injection/late_process.h"
#include <stdlib.h>
typedef struct late_work {DWORD cookie;HDESK desktop;HMODULE module;} late_work;
static volatile LONG running,completed_cookie;
static BOOL CALLBACK notify_window(HWND window,LPARAM parameter)
{
    DWORD pid=0;GetWindowThreadProcessId(window,&pid);
    if(pid==GetCurrentProcessId() && IsWindowVisible(window)){
        UINT message=RegisterWindowMessageW(NTW16S_GLOBAL_CONTROL);
        if(message)SendNotifyMessageW(window,message,(WPARAM)parameter,NTW16S_GLOBAL_APPLY);
    }
    return TRUE;
}
void ntw16s_global_late_mark_ready(DWORD cookie)
{if(cookie)InterlockedExchange(&completed_cookie,(LONG)cookie);}
static DWORD WINAPI initialize(void *value)
{
    late_work *work=(late_work *)value;DWORD error=ERROR_INVALID_STATE;
    if(SetThreadDesktop(work->desktop) && ntw16s_global_policy_active(work->cookie)){
        ntw16s_context *paths=(ntw16s_context *)calloc(1,sizeof(*paths));
        if(paths){error=ntw16s_context_paths(paths,work->module);if(!error)error=ntw16s_late_process_init(paths->directory,work->cookie);free(paths);}
    }
    if(!error && ntw16s_global_policy_active(work->cookie)){
        ntw16s_global_late_mark_ready(work->cookie);
        EnumWindows(notify_window,(LPARAM)work->cookie);
    }
    InterlockedExchange(&running,0);free(work);return error;
}
BOOL ntw16s_global_late_schedule(DWORD cookie)
{
    if(!cookie)return FALSE;
    if((DWORD)InterlockedCompareExchange(&completed_cookie,0,0)==cookie)return TRUE;
    if(InterlockedCompareExchange(&running,1,0)!=0)return FALSE;
    late_work *work=(late_work *)calloc(1,sizeof(*work));HMODULE module=NULL;
    if(!work)goto fail;
    work->desktop=GetThreadDesktop(GetCurrentThreadId());work->cookie=cookie;
    if(!work->desktop || !GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_PIN,
        (LPCWSTR)(void *)&ntw16s_global_late_schedule,&module))goto fail;
    work->module=module;HANDLE thread=CreateThread(NULL,0,initialize,work,0,NULL);
    if(!thread)goto fail;
    CloseHandle(thread);return FALSE;
fail:
    free(work);InterlockedExchange(&running,0);return FALSE;
}
