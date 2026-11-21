#include "lib-win32-style/application/control/progress_snapshot.h"
#include "lib-win32-style/application/control/control_protocol.h"
#include <stdio.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"PROGRESS SNAPSHOT FAIL line=%d error=%lu\n",__LINE__,GetLastError());return 1;}}while(0)
static style_instance instance;
static style_control_native_state state;
static unsigned action,queries;
static BOOL allow=TRUE;
static BOOL supported(HWND w){(void)w;return allow;}
static LRESULT CALLBACK query(HWND w,UINT m,WPARAM a,LPARAM b,UINT_PTR id,DWORD_PTR data)
{
    (void)id;(void)data;LRESULT result=DefSubclassProc(w,m,a,b);
    if(m==PBM_GETPOS){++queries;if(action==1)++state.epoch;else if(action==2)instance.detached=TRUE;}
    return result;
}
int main(void)
{
    INITCOMMONCONTROLSEX init={sizeof(init),ICC_PROGRESS_CLASS};CHECK(InitCommonControlsEx(&init));
    HWND root=CreateWindowW(L"STATIC",L"",WS_POPUP,0,0,250,100,NULL,NULL,GetModuleHandleW(NULL),NULL);CHECK(root);
    HWND progress=CreateWindowW(PROGRESS_CLASSW,L"",WS_CHILD,0,0,200,30,root,NULL,GetModuleHandleW(NULL),NULL);CHECK(progress);
    SendMessageW(progress,PBM_SETRANGE32,10,210);SendMessageW(progress,PBM_SETPOS,75,0);SendMessageW(progress,PBM_SETSTATE,PBST_PAUSED,0);
    SendMessageW(progress,PBM_SETBARCOLOR,0,RGB(12,34,56));SendMessageW(progress,PBM_SETBKCOLOR,0,RGB(78,90,12));
    style_control_native_init(&state);style_progress_snapshot snapshot;
    CHECK(style_progress_snapshot_read(progress,&instance,&state,supported,&snapshot));
    CHECK(snapshot.range.iLow==10 && snapshot.range.iHigh==210 && snapshot.position==75 && snapshot.state==PBST_PAUSED && snapshot.bar==RGB(12,34,56) && snapshot.background==RGB(78,90,12));
    CHECK(SetWindowSubclass(progress,query,1,0));
    action=1;CHECK(!style_progress_snapshot_read(progress,&instance,&state,supported,&snapshot));
    action=2;CHECK(!style_progress_snapshot_read(progress,&instance,&state,supported,&snapshot));instance.detached=FALSE;
    action=0;allow=FALSE;CHECK(!style_progress_snapshot_read(progress,&instance,&state,supported,&snapshot));
    allow=TRUE;CHECK(style_progress_snapshot_read(progress,&instance,&state,supported,&snapshot) && queries==4);
    CHECK(SendMessageW(progress,PBM_GETPOS,0,0)==75 && SendMessageW(progress,PBM_GETSTATE,0,0)==PBST_PAUSED);
    CHECK(style_progress_native_supported(progress) && !style_progress_bound_supported(progress));
    style_control_protocol *protocol=style_control_protocol_bind(progress);
    CHECK(protocol && style_progress_bound_supported(progress));
    SetWindowLongPtrW(progress,GWL_STYLE,WS_CHILD|PBS_SMOOTH|PBS_SMOOTHREVERSE);
    CHECK(style_progress_bound_supported(progress) && !style_progress_native_supported(progress));
    SetWindowLongPtrW(progress,GWL_STYLE,WS_CHILD|PBS_MARQUEE);
    CHECK(!style_progress_bound_supported(progress) && !style_progress_native_supported(progress));
    SetWindowLongPtrW(progress,GWL_STYLE,WS_CHILD);
    SetWindowLongPtrW(progress,GWL_EXSTYLE,WS_EX_STATICEDGE);
    CHECK(!style_progress_bound_supported(progress) && style_progress_native_supported(progress));
    SetWindowLongPtrW(progress,GWL_EXSTYLE,WS_EX_WINDOWEDGE|WS_EX_DLGMODALFRAME);
    CHECK(style_progress_bound_supported(progress) && !style_progress_native_supported(progress));
    SetWindowLongPtrW(progress,GWL_EXSTYLE,WS_EX_LAYOUTRTL);
    CHECK(!style_progress_bound_supported(progress) && !style_progress_native_supported(progress));
    style_control_protocol_release(protocol);

    CHECK(DestroyWindow(root));puts("PROGRESS SNAPSHOT PASS values=1 colors=1 epoch=1 retire=1 eligibility=1 native_unchanged=1 cleanup=1");
    return 0;
}
