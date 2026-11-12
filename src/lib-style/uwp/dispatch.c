#include "dispatch.h"
#include "lib-win32/process-image/image.h"
#include "lib-win32/style/window_dispatch.h"
DWORD style_uwp_dispatch_change(const style_uwp_target *app,PCWSTR path,BOOL apply,const style_uwp_dispatch_spec *spec)
{
    if(!app||!path||!*path||!style_uwp_target_valid(app))return ERROR_INVALID_WINDOW_HANDLE;
    if(!spec||!spec->action||!spec->hook_export||!spec->message||!spec->result_property)return ERROR_INVALID_PARAMETER;
    DWORD host_pid=0,thread=GetWindowThreadProcessId(app->frame,&host_pid),error;
    HANDLE host=OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION,FALSE,host_pid);
    if(!host)return GetLastError();
    WORD machine=0;error=win32_process_image_machine(host,&machine);CloseHandle(host);
    if(error)return error;
    if(machine!=spec->machine)return ERROR_NOT_SUPPORTED;
    WPARAM action=0;error=spec->action(app,apply,&action,spec->context);if(error)return error;
    error=win32_process_image_validate(path,machine,TRUE);if(error)return error;
    return win32_style_window_dispatch(app->frame,thread,path,spec->hook_export,
        spec->message,spec->result_property,action,(LPARAM)app->core,spec->timeout);
}
