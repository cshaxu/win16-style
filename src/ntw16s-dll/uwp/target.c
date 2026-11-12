#include "target.h"
#include "hook.h"
#include "lib-style/uwp/native.h"
#include "lib-win32/process-image/image.h"
#include "lib-win32/style/window_dispatch.h"
DWORD ntw16s_uwp_target_change(const style_uwp_target *app,PCWSTR path,BOOL apply)
{
    if(!app||!path||!*path||!style_uwp_target_valid(app))return ERROR_INVALID_WINDOW_HANDLE;
    DWORD host_pid=0,thread=GetWindowThreadProcessId(app->frame,&host_pid),error;
    HANDLE host=OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION,FALSE,host_pid);
    if(!host)return GetLastError();
    WORD machine=0;error=win32_process_image_machine(host,&machine);CloseHandle(host);
    if(error)return error;
    if(machine!=IMAGE_FILE_MACHINE_AMD64)return ERROR_NOT_SUPPORTED;
    WPARAM action=NTW16S_UWP_REMOVE;
    if(apply){
        HWND auxiliary=NULL,title=style_uwp_title(app->frame,&auxiliary);
        if(!title)return ERROR_NOT_SUPPORTED;
        HRESULT capability=style_uwp_fullscreen_available(title);
        if(FAILED(capability))return (DWORD)capability;
        action=capability==S_OK?NTW16S_UWP_APPLY_FULLSCREEN:NTW16S_UWP_APPLY;
    }
    error=win32_process_image_validate(path,machine,TRUE);if(error)return error;
    return win32_style_window_dispatch(app->frame,thread,path,"Ntw16sUwpFrameHook",
        NTW16S_UWP_MESSAGE,NTW16S_UWP_RESULT,action,(LPARAM)app->core,3000);
}
