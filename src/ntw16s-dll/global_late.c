#include "global_late.h"
#include "global_session.h"
#include "lib-win32/process-global/background.h"
#include "injection/injection.h"
#include "injection/late_process.h"
#include <stdlib.h>
typedef struct late_work {DWORD cookie;HDESK desktop;HMODULE module;} late_work;
static win32_process_global_background background;
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
{win32_process_global_background_mark(&background,cookie);}
static void initialize(DWORD key,void *value,void *context)
{
    late_work *work=(late_work *)value;DWORD error=ERROR_INVALID_STATE;(void)key;(void)context;
    if(SetThreadDesktop(work->desktop) && ntw16s_global_policy_active(work->cookie)){
        ntw16s_context *paths=(ntw16s_context *)calloc(1,sizeof(*paths));
        if(paths){error=ntw16s_context_paths(paths,work->module);if(!error)error=ntw16s_late_process_init(paths->directory,work->cookie);free(paths);}
    }
    if(!error && ntw16s_global_policy_active(work->cookie)){
        ntw16s_global_late_mark_ready(work->cookie);
        EnumWindows(notify_window,(LPARAM)work->cookie);
    }

}
static void *prepare(DWORD cookie,void *context)
{
    (void)context;late_work *work=(late_work *)calloc(1,sizeof(*work));HMODULE module=NULL;
    if(!work)return NULL;
    work->desktop=GetThreadDesktop(GetCurrentThreadId());work->cookie=cookie;
    if(!work->desktop || !GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_PIN,
        (LPCWSTR)(void *)&ntw16s_global_late_schedule,&module)){free(work);return NULL;}
    work->module=module;return work;
}
static void destroy(void *task,void *context){(void)context;free(task);}
BOOL ntw16s_global_late_schedule(DWORD cookie)
{
    static const win32_process_global_background_ops ops={prepare,initialize,destroy};
    return win32_process_global_background_schedule(&background,cookie,&ops,NULL);
}
