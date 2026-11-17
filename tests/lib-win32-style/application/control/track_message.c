#include "lib-win32-style/application/control/notify.h"
#include "lib-win32-style/application/control/control_scope.h"
#include <stdio.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"TRACK_MESSAGE FAIL line=%d error=%lu\n",__LINE__,GetLastError());return 1;}}while(0)
static style_instance instance;
static style_control_notify_state state;
static style_control_track_calls calls;
static HWND destination;
static void *notify_owner;
static unsigned disposed,refreshes,forwarded,bad;
static BOOL forwarded_style,detach_native,destroy_native;
static void dispose(void *context){(void)context;++disposed;}
static LRESULT CALLBACK notify(HWND w,UINT m,WPARAM a,LPARAM b,UINT_PTR id,DWORD_PTR data)
{(void)id;(void)data;return DefSubclassProc(w,m,a,b);}
static LRESULT CALLBACK application(HWND w,UINT m,WPARAM a,LPARAM b,UINT_PTR id,DWORD_PTR data)
{
    (void)id;(void)data;
    if(m==WM_STYLECHANGED){++forwarded;if(detach_native)style_instance_detach(&instance);if(destroy_native)DestroyWindow(w);forwarded_style=TRUE;return 123;}
    if(m==WM_SYSCOLORCHANGE){++refreshes;if(!forwarded_style)++bad;}
    if(m==WM_APP+1){SetParent(w,destination);return 321;}
    return DefSubclassProc(w,m,a,b);
}
static LRESULT CALLBACK track(HWND w,UINT m,WPARAM a,LPARAM b,UINT_PTR id,DWORD_PTR data)
{
    (void)id;style_instance *owner=(void *)data;style_instance_retain(owner);LRESULT result;
    if(m==WM_NCDESTROY){style_instance_detach(owner);result=DefSubclassProc(w,m,a,b);}
    else result=style_control_track_message(w,owner,&state,m,a,b,notify,notify_owner,&calls);
    style_instance_release(owner);return result;
}
static HWND create(HWND parent)
{
    state=(style_control_notify_state){0};style_instance_init(&instance,NULL,dispose);style_instance_retain(&instance);
    HWND w=CreateWindowW(TRACKBAR_CLASSW,L"",WS_CHILD,0,0,100,30,parent,NULL,GetModuleHandleW(NULL),NULL);
    if(!w || !SetWindowSubclass(w,application,37,0) || !style_instance_attach(&instance,w,track,NULL) ||
       !style_control_notify_bind(w,&state,notify,notify_owner))return NULL;
    return w;
}
int main(void)
{
    INITCOMMONCONTROLSEX init={sizeof(init),ICC_BAR_CLASSES};CHECK(InitCommonControlsEx(&init));
    int marker=1;notify_owner=&marker;
    HWND parent=CreateWindowW(L"STATIC",L"",WS_POPUP,0,0,160,100,NULL,NULL,GetModuleHandleW(NULL),NULL);CHECK(parent);
    destination=CreateWindowW(L"STATIC",L"",WS_POPUP,0,0,160,100,NULL,NULL,GetModuleHandleW(NULL),NULL);CHECK(destination);
    HWND w=create(parent);CHECK(w && style_control_subclass_owned(parent,notify,(UINT_PTR)w,&marker));
    STYLESTRUCT style={(DWORD)GetWindowLongPtrW(w,GWL_STYLE),(DWORD)GetWindowLongPtrW(w,GWL_STYLE)};
    forwarded_style=FALSE;unsigned before=refreshes;CHECK(SendMessageW(w,WM_STYLECHANGED,GWL_STYLE,(LPARAM)&style)==123);
    CHECK(forwarded==1 && refreshes==before+1 && !bad && state.parent==parent && !calls.current);
    CHECK(SendMessageW(w,WM_APP+1,0,0)==321 && GetParent(w)==destination && !state.parent && !state.internal);
    CHECK(!style_control_subclass_owned(parent,notify,(UINT_PTR)w,&marker) && !style_control_subclass_owned(destination,notify,(UINT_PTR)w,&marker));
    CHECK(refreshes>=before+2 && !instance.detached && instance.calls==1 && !calls.current);
    CHECK(DestroyWindow(w) && instance.detached);style_instance_release(&instance);CHECK(disposed==1);
    w=create(parent);CHECK(w);detach_native=TRUE;before=refreshes;
    CHECK(SendMessageW(w,WM_STYLECHANGED,GWL_STYLE,(LPARAM)&style)==123 && instance.detached && refreshes==before && instance.calls==1);
    style_control_notify_unbind(&state,notify,&marker);CHECK(DestroyWindow(w));style_instance_release(&instance);CHECK(disposed==2);detach_native=FALSE;
    w=create(parent);CHECK(w);destroy_native=TRUE;before=refreshes;
    CHECK(SendMessageW(w,WM_STYLECHANGED,GWL_STYLE,(LPARAM)&style)==123 && !IsWindow(w) && instance.detached && refreshes==before && instance.calls==1);
    style_control_notify_unbind(&state,notify,&marker);style_instance_release(&instance);CHECK(disposed==3 && !calls.current && !bad);
    CHECK(DestroyWindow(destination) && DestroyWindow(parent));
    puts("TRACK_MESSAGE PASS native_trackbar=1 native_first=1 style_cache=1 reparent=1 owned_unbind=1 no_rebind=1 distinct_owner=1 detached=1 destroyed=1 final_release=1 no_style=1 cleanup=1");return 0;
}
