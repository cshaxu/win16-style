#include "hook.h"
#include "lib-style/uwp/native.h"
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
LRESULT CALLBACK Ntw16sUwpFrameHook(int code,WPARAM parameter,LPARAM message)
{
    if(code>=0&&!busy){
        const CWPSTRUCT *m=(const CWPSTRUCT *)message;
        if(m && m->message==RegisterWindowMessageW(NTW16S_UWP_MESSAGE) &&
           (m->wParam==NTW16S_UWP_APPLY || m->wParam==NTW16S_UWP_APPLY_FULLSCREEN || m->wParam==NTW16S_UWP_REMOVE)){
            busy=TRUE;DWORD error=ERROR_INVALID_WINDOW_HANDLE;HWND core=(HWND)m->lParam;
            DWORD owner=0;DWORD thread=GetWindowThreadProcessId(m->hwnd,&owner);
            if(thread==GetCurrentThreadId()&&owner==GetCurrentProcessId()&&
               class_is(m->hwnd,L"ApplicationFrameWindow")&&class_is(core,L"Windows.UI.Core.CoreWindow")&&
               GetAncestor(core,GA_ROOT)==m->hwnd){
                HMODULE module=NULL;SetLastError(0);
                BOOL ok;
                if(m->wParam==NTW16S_UWP_REMOVE)ok=win16_uwp_frame_remove(m->hwnd);
                else if(win16_uwp_frame_surface(m->hwnd))ok=TRUE;
                else ok=GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_PIN,
                    (PCWSTR)(uintptr_t)Ntw16sUwpFrameHook,&module)&&apply_frame(m->hwnd,m->wParam==NTW16S_UWP_APPLY_FULLSCREEN);
                error=ok?ERROR_SUCCESS:GetLastError();if(!ok&&!error)error=ERROR_NOT_SUPPORTED;
            }
            SetPropW(m->hwnd,NTW16S_UWP_RESULT,(HANDLE)(uintptr_t)((ULONGLONG)error+1));
            busy=FALSE;
        }
    }
    return CallNextHookEx(NULL,code,parameter,message);
}
