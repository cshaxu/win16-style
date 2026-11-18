#include "lib-win32-style/application/control/notify.h"
#include "lib-win32-style/application/control/control_scope.h"
#include "../../../ntw16s-dll/exception_probe.h"
#include <stdio.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"NOTIFY FAIL line=%d error=%lu\n",__LINE__,GetLastError());return 1;}}while(0)
static style_instance instance;
static style_control_notify_state state;
static style_control_track_calls track_calls;
static unsigned calls,draws,disposed,active;
static BOOL retire,paint_supported=TRUE;
static LRESULT application;
static void dispose(void *context){(void)context;++disposed;}
static BOOL supported(HWND w){return w==state.window && paint_supported;}
static BOOL part(HWND w,const NMCUSTOMDRAW *draw,const void *context)
{(void)w;(void)draw;(void)context;++draws;return TRUE;}
static LRESULT CALLBACK app(HWND w,UINT m,WPARAM a,LPARAM b,UINT_PTR id,DWORD_PTR data)
{
    (void)id;(void)data;
    if(m==WM_NOTIFY){++calls;if(retire)instance.detached=TRUE;SetLastError(0x4321);return application;}
    return DefSubclassProc(w,m,a,b);
}
static LRESULT CALLBACK notify(HWND w,UINT m,WPARAM a,LPARAM b,UINT_PTR id,DWORD_PTR data)
{
    (void)data;const style_control_notify_ops ops={.supported=supported,.part=part};
    return style_control_notify_message(w,m,a,b,id,&instance,&state,notify,NULL,&ops);
}
static LRESULT CALLBACK native(HWND w,UINT m,WPARAM a,LPARAM b)
{
    if(m==WM_APP+1 || m==WM_APP+2){if(track_calls.current && track_calls.current->owner==&instance)++active;
        if(m==WM_APP+2)RaiseException(NTW16S_TEST_EXCEPTION_CODE,0,0,NULL);
        SetLastError(0x8765);return 123;}
    return DefWindowProcW(w,m,a,b);
}
static LRESULT CALLBACK dispatch(HWND w,UINT m,WPARAM a,LPARAM b,UINT_PTR id,DWORD_PTR data)
{
    (void)id;(void)data;return style_control_track_dispatch(&track_calls,&instance,w,m,a,b);
}
int main(void)
{
    SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOGPFAULTERRORBOX);
    HWND parent=CreateWindowW(L"STATIC",L"",WS_POPUP,0,0,40,40,NULL,NULL,GetModuleHandleW(NULL),NULL);CHECK(parent);
    HWND child=CreateWindowW(L"STATIC",L"",WS_CHILD,0,0,20,20,parent,NULL,GetModuleHandleW(NULL),NULL);CHECK(child);
    style_instance_init(&instance,NULL,dispose);CHECK(SetWindowSubclass(parent,app,1,0));
    CHECK(style_control_notify_bind(child,&state,notify,&instance));
    CHECK(style_control_subclass_owned(parent,notify,(UINT_PTR)child,&instance));
    style_control_notify_unbind(&state,notify,&state);CHECK(style_control_subclass_owned(parent,notify,(UINT_PTR)child,&instance));state.parent=parent;
    NMCUSTOMDRAW draw={.hdr={.hwndFrom=child,.code=NM_CUSTOMDRAW},.dwDrawStage=CDDS_PREPAINT};
    application=CDRF_SKIPDEFAULT;CHECK(SendMessageW(parent,WM_NOTIFY,0,(LPARAM)&draw)==CDRF_SKIPDEFAULT && !state.internal && !instance.calls);
    application=CDRF_DODEFAULT;CHECK(SendMessageW(parent,WM_NOTIFY,0,(LPARAM)&draw)==(CDRF_NOTIFYITEMDRAW|CDRF_NOTIFYPOSTPAINT) && state.internal && GetLastError()==0x4321);
    unsigned previous=calls;draw.dwDrawStage=CDDS_ITEMPREPAINT;CHECK(SendMessageW(parent,WM_NOTIFY,0,(LPARAM)&draw)==CDRF_SKIPDEFAULT && draws==1 && calls==previous);
    draw.dwDrawStage=CDDS_POSTPAINT;CHECK(SendMessageW(parent,WM_NOTIFY,0,(LPARAM)&draw)==CDRF_DODEFAULT && !state.internal);
    draw.hdr.hwndFrom=parent;application=CDRF_SKIPDEFAULT;CHECK(SendMessageW(parent,WM_NOTIFY,0,(LPARAM)&draw)==CDRF_SKIPDEFAULT && draws==1);
    draw.hdr.hwndFrom=child;draw.dwDrawStage=CDDS_PREPAINT;application=CDRF_DODEFAULT;retire=TRUE;
    CHECK(SendMessageW(parent,WM_NOTIFY,0,(LPARAM)&draw)==CDRF_DODEFAULT && !state.internal && disposed==1 && !instance.calls);
    retire=FALSE;style_instance_init(&instance,NULL,dispose);
    WNDCLASSW cls={.hInstance=GetModuleHandleW(NULL),.lpfnWndProc=native,.lpszClassName=L"NotifyDispatch"};CHECK(RegisterClassW(&cls));
    HWND target=CreateWindowW(cls.lpszClassName,L"",WS_CHILD,0,0,20,20,parent,NULL,cls.hInstance,NULL);CHECK(target);
    CHECK(SetWindowSubclass(target,dispatch,1,0));style_control_track_scope prior={0};track_calls.current=&prior;
    CHECK(SendMessageW(target,WM_APP+1,0,0)==123 && track_calls.current==&prior && active==1 && GetLastError()==0x8765);
    CHECK(test_unwind(target,WM_APP+2,0,0) && track_calls.current==&prior && active==2);
    CHECK(DestroyWindow(parent));CHECK(!state.parent && !state.internal);CHECK(UnregisterClassW(cls.lpszClassName,cls.hInstance));
    puts("NOTIFY PASS native_result=1 private_stage=1 ownership=1 retire=1 scope=1 native_unwind=1 last_error=1 cleanup=1");return 0;
}
