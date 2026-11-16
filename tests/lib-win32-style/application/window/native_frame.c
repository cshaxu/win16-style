#include "lib-win32-style/application/window/native_frame.h"
#include <stdio.h>

static BOOL enabled=TRUE,fail_get,fail_set,detached;
static unsigned get_calls,set_calls,changed;
static DWORD policy;
static style_native_frame *queued_state;
static unsigned refreshes;
static HRESULT WINAPI get(HWND window,DWORD attribute,PVOID value,DWORD size)
{
    (void)window;
    ++get_calls;
    if(attribute!=DWMWA_NCRENDERING_ENABLED || size!=sizeof(BOOL))return E_INVALIDARG;
    if(fail_get)return E_FAIL;
    *(BOOL *)value=enabled;return S_OK;
}
static HRESULT WINAPI set(HWND window,DWORD attribute,LPCVOID value,DWORD size)
{
    (void)window;
    ++set_calls;
    if(attribute!=DWMWA_NCRENDERING_POLICY || size!=sizeof(DWORD))return E_INVALIDARG;
    policy=*(const DWORD *)value;return fail_set?E_FAIL:S_OK;
}
static LRESULT CALLBACK procedure(HWND window,UINT message,WPARAM wp,LPARAM lp)
{
    if(queued_state && message==queued_state->refresh_message && style_native_frame_take_refresh(queued_state))++refreshes;
    if(message==WM_WINDOWPOSCHANGED)++changed;
    return DefWindowProcW(window,message,wp,lp);
}
int main(void)
{
    HINSTANCE module=GetModuleHandleW(NULL);
    WNDCLASSW cls={.hInstance=module,.lpfnWndProc=procedure,.lpszClassName=L"NativeFrameStateTest"};
    if(!RegisterClassW(&cls))return 1;
    HWND window=CreateWindowW(cls.lpszClassName,L"Native frame state",WS_OVERLAPPEDWINDOW,
        30,30,300,200,NULL,NULL,module,NULL);
    HWND child=window?CreateWindowW(cls.lpszClassName,L"Child",WS_CHILD,0,0,30,30,window,NULL,module,NULL):NULL;
    if(!window || !child){if(window)DestroyWindow(window);UnregisterClassW(cls.lpszClassName,module);return 2;}
    int result=0;style_native_frame state={0};
    style_native_frame_capture(&state,window,get,FALSE);
    if(!state.saved || state.policy!=DWMNCRP_ENABLED || get_calls!=1){result=3;goto done;}
    state.pending=TRUE;
    if(FAILED(style_native_frame_restore_policy(window,&state,set)) || set_calls){result=4;goto done;}
    state.pending=FALSE;
    if(FAILED(style_native_frame_restore_policy(window,&state,set)) || set_calls!=1 || policy!=DWMNCRP_ENABLED){result=5;goto done;}
    fail_set=TRUE;
    if(style_native_frame_restore_policy(window,&state,set)!=E_FAIL){result=6;goto done;}
    fail_set=FALSE;enabled=FALSE;
    style_native_frame_capture(&state,window,get,FALSE);
    if(!state.saved || state.policy!=DWMNCRP_DISABLED){result=7;goto done;}
    fail_get=TRUE;
    style_native_frame_capture(&state,window,get,FALSE);
    unsigned before=set_calls;
    if(state.saved || FAILED(style_native_frame_restore_policy(window,&state,set)) || set_calls!=before){result=8;goto done;}
    style_native_frame_capture(&state,window,get,TRUE);
    if(!state.saved || state.policy!=DWMNCRP_USEWINDOWSTYLE ||
       FAILED(style_native_frame_restore_policy(window,&state,set)) || policy!=DWMNCRP_USEWINDOWSTYLE){result=9;goto done;}
    before=get_calls;style_native_frame_capture(&state,child,get,TRUE);
    if(state.saved || get_calls!=before){result=10;goto done;}
    before=set_calls;detached=TRUE;
    style_native_frame_activate(child,&state,&detached,NULL,FALSE,set);
    if(set_calls!=before){result=11;goto done;}
    style_native_frame_activate(window,&state,&detached,NULL,FALSE,set);
    if(set_calls!=before+1 || policy!=DWMNCRP_DISABLED){result=12;goto done;}
    state.pending=TRUE;before=changed;
    style_native_frame_restore_geometry(window,&state,NULL,FALSE);
    if(changed!=before){result=13;goto done;}
    state.pending=FALSE;state.keep_outer=TRUE;
    RECT outer,after;GetWindowRect(window,&outer);
    style_native_frame_restore_geometry(window,&state,NULL,TRUE);
    GetWindowRect(window,&after);
    if(!EqualRect(&outer,&after) || changed==before){result=14;goto done;}
    queued_state=&state;
    UINT refresh=RegisterWindowMessageW(L"NativeFrameStateTest.Refresh");
    if(!refresh){result=16;goto done;}
    style_native_frame_queue(&state,window,0);
    if(state.refresh_pending){result=17;goto done;}
    style_native_frame_queue(&state,window,refresh);style_native_frame_queue(&state,window,refresh);
    MSG message;
    while(PeekMessageW(&message,window,refresh,refresh,PM_REMOVE))DispatchMessageW(&message);
    if(refreshes!=1 || state.refresh_pending || state.refresh_message!=refresh){result=18;goto done;}
    queued_state=NULL;
    DestroyWindow(child);child=NULL;before=set_calls;
    if(FAILED(style_native_frame_restore_policy(child,&state,set)) || set_calls!=before){result=15;goto done;}
done:
    DestroyWindow(window);UnregisterClassW(cls.lpszClassName,module);
    printf("NATIVE FRAME STATE bits=%u result=%d get=%u set=%u geometry=%u\n",
        (unsigned)(sizeof(void *)*8),result,get_calls,set_calls,changed);
    return result;
}
