#include "lib-style/uwp/hook.h"
#include <stdint.h>
#include <stdio.h>
static BOOL busy;static unsigned calls;static style_uwp_hook_spec spec;
static BOOL accepts(WPARAM action,void *context){(void)context;return action==1;}
static DWORD perform(HWND frame,HWND core,WPARAM action,void *context)
{
    (void)context;++calls;
    CWPSTRUCT nested={.hwnd=frame,.message=RegisterWindowMessageW(spec.message),.wParam=action,.lParam=(LPARAM)core};
    style_uwp_hook_handle(0,(LPARAM)&nested,&busy,&spec);
    return ERROR_ACCESS_DENIED;
}
int main(void)
{
    HINSTANCE instance=GetModuleHandleW(NULL);
    WNDCLASSW frame_class={.lpfnWndProc=DefWindowProcW,.hInstance=instance,.lpszClassName=L"ApplicationFrameWindow"};
    WNDCLASSW core_class={.lpfnWndProc=DefWindowProcW,.hInstance=instance,.lpszClassName=L"Windows.UI.Core.CoreWindow"};
    if(!RegisterClassW(&frame_class) || !RegisterClassW(&core_class))return 1;
    HWND frame=CreateWindowW(frame_class.lpszClassName,L"",WS_POPUP,0,0,50,50,NULL,NULL,instance,NULL);
    HWND core=frame?CreateWindowW(core_class.lpszClassName,L"",WS_CHILD,0,0,20,20,frame,NULL,instance,NULL):NULL;
    if(!frame || !core)return 2;
    spec=(style_uwp_hook_spec){L"Style.UwpHook.Test",L"Style.UwpHook.Result",accepts,perform,NULL};
    CWPSTRUCT message={.hwnd=frame,.message=RegisterWindowMessageW(spec.message),.wParam=1,.lParam=(LPARAM)core};
    style_uwp_hook_handle(0,(LPARAM)&message,&busy,&spec);
    BOOL ok=calls==1 && !busy && (uintptr_t)GetPropW(frame,spec.result_property)==ERROR_ACCESS_DENIED+1;
    RemovePropW(frame,spec.result_property);message.lParam=0;
    style_uwp_hook_handle(0,(LPARAM)&message,&busy,&spec);
    ok=ok && calls==1 && (uintptr_t)GetPropW(frame,spec.result_property)==ERROR_INVALID_WINDOW_HANDLE+1;
    RemovePropW(frame,spec.result_property);DestroyWindow(frame);
    UnregisterClassW(core_class.lpszClassName,instance);UnregisterClassW(frame_class.lpszClassName,instance);
    if(!ok)return 3;
    puts("UWP_HOOK_OK valid=1 invalid=1 recursion=1 cleanup=1");return 0;
}
