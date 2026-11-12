#include "hook.h"
#include "target.h"
#include "lib-win32/style/window_hook.h"
#include "lib-win32/style/window_identity.h"
void style_uwp_hook_handle(int code,LPARAM parameter,BOOL *busy,const style_uwp_hook_spec *spec)
{
    if(code<0 || *busy)return;
    const CWPSTRUCT *message=win32_style_window_hook_match(code,parameter,*busy,spec->message);
    if(!message || !spec->accept(message->wParam,spec->context))return;
    *busy=TRUE;
    DWORD error=ERROR_INVALID_WINDOW_HANDLE;HWND core=(HWND)message->lParam;
    BOOL local=win32_style_window_current_thread(message->hwnd);DWORD core_pid=0;
    GetWindowThreadProcessId(core,&core_pid);
    style_uwp_target target={.pid=core_pid,.core=core,.frame=message->hwnd};
    if(local && style_uwp_target_valid(&target))
        error=spec->perform(message->hwnd,core,message->wParam,spec->context);
    win32_style_window_hook_finish(message->hwnd,spec->result_property,error);
    *busy=FALSE;
}
