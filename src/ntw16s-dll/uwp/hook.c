#include "hook.h"
#include "lib-style/uwp/native.h"
#include "lib-style/uwp/hook.h"
#include "lib-style/win16-window-uwp/frame.h"
#include <stdint.h>
#include <wchar.h>
static _Thread_local BOOL busy;
static BOOL class_is(HWND w,PCWSTR name)
{WCHAR value[80];return GetClassNameW(w,value,ARRAYSIZE(value))&&!wcscmp(value,name);}
static void application_menu(HWND frame,void *context)
{
    HWND child=(HWND)context;RECT r;
    if(GetParent(child)!=frame || !IsWindowVisible(child) ||
       !class_is(child,L"ApplicationFrameTitleBarWindow") || !GetClientRect(child,&r))return;
    LPARAM point=MAKELPARAM(r.right/2,r.bottom/2);
    PostMessageW(child,WM_LBUTTONDOWN,MK_LBUTTON,point);
    PostMessageW(child,WM_LBUTTONUP,0,point);
}
static BOOL apply_frame(HWND frame,BOOL fullscreen)
{
    HWND auxiliary=NULL,title=style_uwp_title(frame,&auxiliary);
    if(!title){SetLastError(ERROR_NOT_SUPPORTED);return FALSE;}
    win16_uwp_frame_action actions[2];unsigned count=0;
    if(auxiliary)actions[count++]=(win16_uwp_frame_action){L"Application menu...",application_menu,auxiliary};
    if(fullscreen)actions[count++]=(win16_uwp_frame_action){L"Toggle full screen",style_uwp_fullscreen,title};
    return win16_uwp_frame_apply_actions(frame,title,actions,count);
}
static BOOL accept_action(WPARAM action,void *context)
{(void)context;return action==NTW16S_UWP_APPLY || action==NTW16S_UWP_APPLY_FULLSCREEN || action==NTW16S_UWP_REMOVE;}
static DWORD perform_action(HWND frame,HWND core,WPARAM action,void *context)
{
    (void)core;(void)context;HMODULE module=NULL;SetLastError(0);BOOL ok;
    if(action==NTW16S_UWP_REMOVE)ok=win16_uwp_frame_remove(frame);
    else if(win16_uwp_frame_surface(frame))ok=TRUE;
    else ok=GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_PIN,
        (PCWSTR)(uintptr_t)Ntw16sUwpFrameHook,&module)&&apply_frame(frame,action==NTW16S_UWP_APPLY_FULLSCREEN);
    DWORD error=ok?ERROR_SUCCESS:GetLastError();return !ok&&!error?ERROR_NOT_SUPPORTED:error;
}
LRESULT CALLBACK Ntw16sUwpFrameHook(int code,WPARAM parameter,LPARAM message)
{
    const style_uwp_hook_spec spec={NTW16S_UWP_MESSAGE,NTW16S_UWP_RESULT,accept_action,perform_action,NULL};
    style_uwp_hook_handle(code,message,&busy,&spec);
    return CallNextHookEx(NULL,code,parameter,message);
}
