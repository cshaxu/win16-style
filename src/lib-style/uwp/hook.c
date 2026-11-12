#include "hook.h"
#include "target.h"
#include <stdint.h>
void style_uwp_hook_handle(int code,LPARAM message,BOOL *busy,const style_uwp_hook_spec *spec)
{
    if(code<0 || *busy)return;
    const CWPSTRUCT *m=(const CWPSTRUCT *)message;
    if(!m || m->message!=RegisterWindowMessageW(spec->message) || !spec->accept(m->wParam,spec->context))return;
    *busy=TRUE;DWORD error=ERROR_INVALID_WINDOW_HANDLE;HWND core=(HWND)m->lParam;
    DWORD owner=0,core_pid=0,thread=GetWindowThreadProcessId(m->hwnd,&owner);
    GetWindowThreadProcessId(core,&core_pid);
    style_uwp_target target={.pid=core_pid,.core=core,.frame=m->hwnd};
    if(thread==GetCurrentThreadId() && owner==GetCurrentProcessId() && style_uwp_target_valid(&target))
        error=spec->perform(m->hwnd,core,m->wParam,spec->context);
    SetPropW(m->hwnd,spec->result_property,(HANDLE)(uintptr_t)((ULONGLONG)error+1));
    *busy=FALSE;
}
