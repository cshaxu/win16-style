#include "lib-win32-style/application/control/edge.h"
#include <stdio.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"EDGE FAIL line=%d error=%lu\n",__LINE__,GetLastError());return 1;}}while(0)
static style_instance instance;
static style_control_edge_state state;
static style_control_edge_kind kind;
static unsigned mode,installs,removes,qualifications,queries,failed;
static BOOL admitted=TRUE,query_retire;
static HWND other;
static style_control_edge_ops operations;
static BOOL edge_eligible(HWND w,HWND parent,style_control_edge_kind role,void *context)
{++qualifications;if(context!=&state || role!=kind)failed=1;return admitted && GetParent(w)==parent;}
static BOOL edge_install(HWND w,void *context,const BOOL *cancelled)
{
    ++installs;if(context!=&state || cancelled!=&instance.detached || !state.attempted || state.installed!=(kind!=STYLE_EDGE_GRID))failed=1;
    style_control_edge_settle(w,WM_PAINT,&instance,&state,kind,&operations);
    if(mode==2)instance.detached=TRUE;
    if(mode==4 && !SetParent(w,other))failed=1;
    SetLastError(0x1111);return mode==0 || mode==4;
}
static BOOL edge_remove(HWND w,const void *context)
{(void)w;if(context!=&state || state.installed)failed=1;++removes;SetLastError(0x2222);return TRUE;}
static BOOL edge_scroll(HWND w,int axis,LPSCROLLINFO info)
{++queries;if(query_retire)instance.detached=TRUE;return GetScrollInfo(w,axis,info);}
static LRESULT CALLBACK native(HWND w,UINT m,WPARAM a,LPARAM b){return DefWindowProcW(w,m,a,b);}
int main(void)
{
    HWND parent=CreateWindowW(L"STATIC",L"",WS_POPUP,0,0,300,200,NULL,NULL,GetModuleHandleW(NULL),NULL);CHECK(parent);
    other=CreateWindowW(L"STATIC",L"",WS_POPUP,0,0,300,200,NULL,NULL,GetModuleHandleW(NULL),NULL);CHECK(other);
    operations=(style_control_edge_ops){&state,edge_eligible,edge_install,edge_remove,edge_scroll};
    for(kind=STYLE_EDGE_DATA;kind<=STYLE_EDGE_USER;++kind)for(mode=0;mode<=4;++mode){
        instance=(style_instance){0};state=(style_control_edge_state){.candidate=TRUE};installs=removes=qualifications=0;admitted=mode!=3;
        HWND child=CreateWindowW(L"STATIC",L"",WS_CHILD,0,0,200,30,parent,NULL,GetModuleHandleW(NULL),NULL);CHECK(child);
        style_control_edge_settle(child,WM_PRINTCLIENT,&instance,&state,kind,&operations);CHECK(!state.attempted);
        SetLastError(0x9876);style_control_edge_settle(child,WM_PAINT,&instance,&state,kind,&operations);CHECK(GetLastError()==0x9876 && !failed);
        CHECK(installs==(mode==3?0u:1u));
        CHECK(removes==(mode==1 || (mode==4 && kind==STYLE_EDGE_DATA)?1u:0u));
        CHECK(state.installed==(mode==0 || mode==2 || (mode==4 && kind==STYLE_EDGE_USER)));
        if(mode==3){CHECK(!state.attempted);admitted=TRUE;mode=0;style_control_edge_settle(child,WM_PAINT,&instance,&state,kind,&operations);CHECK(state.installed);mode=3;}
        CHECK(DestroyWindow(child));
    }
    kind=STYLE_EDGE_GRID;WNDCLASSW cls={.hInstance=GetModuleHandleW(NULL),.lpfnWndProc=native,.lpszClassName=L"TValueListEditor"};CHECK(RegisterClassW(&cls));
    for(mode=0;mode<=3;++mode){
        instance=(style_instance){0};state=(style_control_edge_state){.candidate=TRUE,.parent=parent,.owner=mode==3?NULL:cls.hInstance};installs=removes=queries=0;
        HWND grid=CreateWindowExW(WS_EX_CLIENTEDGE,cls.lpszClassName,L"",WS_CHILD,0,0,200,100,parent,NULL,cls.hInstance,NULL);CHECK(grid);
        SetLastError(0x7654);style_control_edge_settle(grid,WM_PAINT,&instance,&state,kind,&operations);CHECK(GetLastError()==0x7654 && !failed);
        CHECK(state.attempted && !queries && installs==(mode==3?0u:1u) && removes==(mode==1?1u:0u));
        CHECK(state.installed==(mode==0));state.owner=cls.hInstance;style_control_edge_settle(grid,WM_PAINT,&instance,&state,kind,&operations);CHECK(installs==(mode==3?0u:1u));CHECK(DestroyWindow(grid));
    }
    instance=(style_instance){0};state=(style_control_edge_state){.candidate=TRUE,.parent=parent,.owner=cls.hInstance};installs=removes=queries=0;mode=0;
    HWND grid=CreateWindowExW(WS_EX_CLIENTEDGE,cls.lpszClassName,L"",WS_CHILD|WS_VSCROLL,0,0,200,100,parent,NULL,cls.hInstance,NULL);CHECK(grid);
    SCROLLINFO scroll={.cbSize=sizeof(scroll),.fMask=SIF_RANGE|SIF_POS,.nMin=0,.nMax=100,.nPos=50};SetScrollInfo(grid,SB_VERT,&scroll,FALSE);
    style_control_edge_settle(grid,WM_PAINT,&instance,&state,kind,&operations);CHECK(queries==1 && installs==1 && state.installed && !failed);
    instance=(style_instance){0};state=(style_control_edge_state){.candidate=TRUE,.parent=parent,.owner=cls.hInstance};installs=queries=0;query_retire=TRUE;
    style_control_edge_settle(grid,WM_PAINT,&instance,&state,kind,&operations);CHECK(queries==1 && !installs && state.attempted && !state.installed && instance.detached);
    CHECK(DestroyWindow(grid));CHECK(UnregisterClassW(cls.lpszClassName,cls.hInstance));CHECK(DestroyWindow(parent));CHECK(DestroyWindow(other));
    puts("EDGE PASS delayed_parent=1 publish_order=1 data_recheck=1 user_native_order=1 failed_apply=1 reentrant=1 grid_scroll=1 query_retire=1 last_error=1 cleanup=1");return 0;
}
