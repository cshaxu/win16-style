#include "lib-win32-style/application/adapters/win32/runtime_identity.h"
#include <stdio.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"RUNTIME_IDENTITY FAIL line=%d error=%lu\n",__LINE__,GetLastError());return 1;}}while(0)
static style_instance instance;
static style_runtime_frame_state frame;
static style_runtime_rich_state rich;
static style_runtime_frame_ops operations;
static HWND parent,other;
static unsigned mode,installs,removes,failed;
static BOOL rich_role;
static BOOL frame_install(HWND w,void *context,const BOOL *cancelled)
{
    ++installs;if(context!=&instance || cancelled!=&instance.detached || !(rich_role?rich.installed:frame.verified))failed=1;
    if(rich_role)style_runtime_rich_settle(w,WM_PAINT,&instance,&rich,MCS_MULTISELECT,&operations);
    else style_runtime_box_settle(w,&instance,&frame,TRUE,&operations);
    if(mode==2)instance.detached=TRUE;
    if(mode==3 && !SetParent(w,other))failed=1;
    SetLastError(0x1111);return mode!=1;
}
static BOOL frame_remove(HWND w,const void *context)
{
    (void)w;++removes;if(context!=&instance || (rich_role?rich.installed:frame.verified))failed=1;
    SetLastError(0x2222);return TRUE;
}
static LRESULT CALLBACK native(HWND w,UINT m,WPARAM a,LPARAM b){return DefWindowProcW(w,m,a,b);}
static HWND child(PCWSTR name,DWORD ex,DWORD style)
{return CreateWindowExW(ex,name,L"",WS_CHILD|style,0,0,200,100,parent,NULL,GetModuleHandleW(NULL),NULL);}
int main(void)
{
    static const WCHAR *names[]={L"IdentityParent",L"TScrollBox",L"TControlBar",L"TRichEdit",L"THotKey",L"TMonthCalendar",L"TPopupListbox"};
    HMODULE module=GetModuleHandleW(NULL);
    for(unsigned i=0;i<ARRAYSIZE(names);++i){WNDCLASSW cls={.hInstance=module,.lpfnWndProc=native,.lpszClassName=names[i]};CHECK(RegisterClassW(&cls));}
    parent=CreateWindowW(names[0],L"",WS_POPUP,0,0,300,200,NULL,NULL,module,NULL);CHECK(parent);
    other=CreateWindowW(names[0],L"",WS_POPUP,0,0,300,200,NULL,NULL,module,NULL);CHECK(other);
    operations=(style_runtime_frame_ops){&instance,frame_install,frame_remove};
    HWND box=child(L"TScrollBox",WS_EX_CLIENTEDGE,0);CHECK(box);
    CHECK(style_runtime_scrollbox_identity(box,module,parent));
    CHECK(!style_runtime_scrollbox_identity(box,NULL,parent) && !style_runtime_scrollbox_identity(box,module,other));
    SetWindowLongPtrW(box,GWL_EXSTYLE,WS_EX_CLIENTEDGE|WS_EX_LAYOUTRTL);CHECK(!style_runtime_scrollbox_identity(box,module,parent));
    SetWindowLongPtrW(box,GWL_EXSTYLE,0);CHECK(!style_runtime_scrollbox_identity(box,module,parent));
    SetWindowLongPtrW(box,GWL_EXSTYLE,WS_EX_CLIENTEDGE);
    for(mode=0;mode<=3;++mode){
        CHECK(GetParent(box)==parent || SetParent(box,parent));
        instance=(style_instance){0};frame=(style_runtime_frame_state){.candidate=TRUE,.owner=module,.parent=parent};installs=removes=0;
        style_runtime_box_settle(box,&instance,&frame,FALSE,&operations);CHECK(!installs && !frame.verified);
        SetLastError(0x9876);style_runtime_box_settle(box,&instance,&frame,TRUE,&operations);
        CHECK(GetLastError()==0x9876 && !failed && installs==1 && removes==(mode==1 || mode==3));
        CHECK(frame.verified==(mode==0 || mode==2));
    }
    CHECK(DestroyWindow(box));
    HWND bar=child(L"TControlBar",0,0);CHECK(bar);instance=(style_instance){0};frame=(style_runtime_frame_state){.candidate=TRUE,.owner=module,.parent=parent,.verified=TRUE};
    CHECK(style_runtime_controlbar_identity(bar,&instance,&frame));
    instance.detached=TRUE;CHECK(!style_runtime_controlbar_identity(bar,&instance,&frame));instance.detached=FALSE;
    frame.owner=NULL;CHECK(!style_runtime_controlbar_identity(bar,&instance,&frame));frame.owner=module;
    style_runtime_frame_retire(bar,&frame);CHECK(!frame.retired && frame.verified);
    CHECK(SetParent(bar,other));style_runtime_frame_retire(bar,&frame);CHECK(frame.retired && !frame.verified);
    CHECK(SetParent(bar,parent));CHECK(!style_runtime_controlbar_identity(bar,&instance,&frame));CHECK(DestroyWindow(bar));
    rich_role=TRUE;
    for(unsigned role=0;role<3;++role){
        HWND w=child(names[3+role],WS_EX_CLIENTEDGE,0);CHECK(w);
        for(mode=0;mode<=3;++mode){
            CHECK(GetParent(w)==parent || SetParent(w,parent));
            instance=(style_instance){0};rich=(style_runtime_rich_state){.candidate=TRUE,.unicode=TRUE,.hotkey=role==1,.month=role==2,.owner=module,.parent=parent};installs=removes=0;
            CHECK(style_runtime_rich_identity(w,&instance,&rich,MCS_MULTISELECT));
            rich.owner=NULL;CHECK(!style_runtime_rich_identity(w,&instance,&rich,MCS_MULTISELECT));rich.owner=module;
            style_runtime_rich_settle(w,WM_PAINT,&instance,&rich,MCS_MULTISELECT,&operations);CHECK(!installs && !rich.installed);
            rich.witness=TRUE;style_runtime_rich_settle(w,WM_PRINTCLIENT,&instance,&rich,MCS_MULTISELECT,&operations);CHECK(!installs && !rich.installed);
            SetLastError(0x7654);style_runtime_rich_settle(w,WM_PAINT,&instance,&rich,MCS_MULTISELECT,&operations);
            CHECK(GetLastError()==0x7654 && !failed && installs==(role==2?0u:1u) && removes==(role!=2 && (mode==1 || mode==3)));
            CHECK(rich.installed==(role==2 || mode==0 || mode==2));
        }
        CHECK(DestroyWindow(w));
    }
    HWND month=child(L"TMonthCalendar",0,MCS_MULTISELECT);CHECK(month);
    instance=(style_instance){0};rich=(style_runtime_rich_state){.candidate=TRUE,.month=TRUE,.unicode=TRUE,.witness=TRUE,.owner=module,.parent=parent};
    CHECK(style_runtime_rich_identity(month,&instance,&rich,0));CHECK(!style_runtime_rich_identity(month,&instance,&rich,MCS_MULTISELECT));
    installs=0;style_runtime_rich_settle(month,WM_PAINT,&instance,&rich,MCS_MULTISELECT,&operations);CHECK(!installs && !rich.installed);CHECK(DestroyWindow(month));
    CHECK(DestroyWindow(parent));CHECK(DestroyWindow(other));
    for(unsigned i=0;i<ARRAYSIZE(names);++i)CHECK(UnregisterClassW(names[i],module));
    puts("RUNTIME_IDENTITY PASS owned_class=1 parent=1 module=1 rtl=1 retire=1 publish_order=1 reentrant=1 failed_apply=1 detached=1 month_policy=1 last_error=1 cleanup=1");return 0;
}
