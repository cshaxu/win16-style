#include "uwp_hook.h"
#include "lib-win32-style/application/adapters/uwp/target.h"
#include "lib-win32-style/access/runtime/window_hook.h"
#include "lib-win32-style/application/window/window_identity.h"
static BOOL accept_message(const CWPSTRUCT *message,void *context)
{
    const style_uwp_hook_spec *spec=context;
    return spec->accept(message->wParam,spec->context);
}
static DWORD perform(const CWPSTRUCT *message,void *context)
{
    const style_uwp_hook_spec *spec=context;DWORD core_pid=0;
    HWND core=(HWND)message->lParam;
    BOOL local=win32_style_window_current_thread(message->hwnd);
    GetWindowThreadProcessId(core,&core_pid);
    style_uwp_target target={.pid=core_pid,.core=core,.frame=message->hwnd};
    if(!local || !style_uwp_target_valid(&target))return ERROR_INVALID_WINDOW_HANDLE;
    return spec->perform(message->hwnd,core,message->wParam,spec->context);
}
void style_uwp_hook_handle(int code,LPARAM parameter,BOOL *busy,const style_uwp_hook_spec *spec)
{
    if(code<0 || *busy)return;
    const style_window_hook_spec handler={spec->message,spec->result_property,accept_message,perform,(void *)spec};
    style_window_hook_handle(code,parameter,busy,&handler);
}
