#include "late.h"
#include <stdlib.h>
typedef struct late_work {DWORD cookie;HDESK desktop;HMODULE module;} late_work;
typedef struct notification {const style_global_late_ops *ops;DWORD cookie;} notification;
static BOOL CALLBACK notify_window(HWND window,LPARAM parameter)
{
    const notification *n=(const notification *)parameter;DWORD pid=0;
    GetWindowThreadProcessId(window,&pid);
    if(pid==GetCurrentProcessId() && IsWindowVisible(window)){
        UINT message=RegisterWindowMessageW(n->ops->control);
        if(message)SendNotifyMessageW(window,message,n->cookie,n->ops->apply);
    }
    return TRUE;
}
void style_global_late_mark(style_global_late *state,DWORD cookie)
{win32_process_global_background_mark(&state->background,cookie);}
static void initialize(DWORD key,void *value,void *context)
{
    late_work *work=value;style_global_late *state=context;DWORD error=ERROR_INVALID_STATE;
    (void)key;
    if(SetThreadDesktop(work->desktop) && style_global_policy_active(state->ops->space,work->cookie))
        error=state->ops->initialize(work->module,work->cookie,state->ops->context);
    if(!error && style_global_policy_active(state->ops->space,work->cookie)){
        style_global_late_mark(state,work->cookie);
        notification n={state->ops,work->cookie};EnumWindows(notify_window,(LPARAM)&n);
    }
}
static void *prepare(DWORD cookie,void *context)
{
    (void)context;late_work *work=calloc(1,sizeof(*work));HMODULE module=NULL;
    if(!work)return NULL;
    work->desktop=GetThreadDesktop(GetCurrentThreadId());work->cookie=cookie;
    if(!work->desktop || !GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_PIN,
        (LPCWSTR)(void *)&style_global_late_schedule,&module)){free(work);return NULL;}
    work->module=module;return work;
}
static void destroy(void *task,void *context){(void)context;free(task);}
BOOL style_global_late_schedule(style_global_late *state,DWORD cookie)
{
    static const win32_process_global_background_ops ops={prepare,initialize,destroy};
    return win32_process_global_background_schedule(&state->background,cookie,&ops,state);
}
