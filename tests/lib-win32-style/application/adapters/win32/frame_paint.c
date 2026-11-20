#include "lib-win32-style/application/adapters/win32/frame_paint.h"
#include <stdio.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"FRAME_PAINT FAIL line=%d error=%lu\n",__LINE__,GetLastError());return 1;}}while(0)
static style_runtime_frame_scope scope;
static style_runtime_frame_state state;
static style_instance instance;
static unsigned groups,panels,bars,bits,mode,failed;
static const COLORREF theme_edge=RGB(49,103,157);
static BOOL current=TRUE,owned=TRUE;
static BOOL frame_owned(HWND w,void *owner){return IsWindow(w) && owner==&instance && owned;}
static BOOL frame_current(style_runtime_frame_scope *s){return current && s==&scope;}
static LRESULT CALLBACK native(HWND w,UINT m,WPARAM a,LPARAM b){return DefWindowProcW(w,m,a,b);}
static void group_render(HDC dc,const RECT *bounds,const RECT *clip,const RECT *exclude,COLORREF edge,
    void (*done)(void *),void *context)
{
    (void)dc;(void)clip;++groups;if(edge!=theme_edge || bounds->left || bounds->right!=200 || bounds->bottom!=100 || (exclude && bounds->top!=10))failed=1;
    if(mode==1)instance.detached=TRUE;
    if(mode!=2)done(context);
}
static void panel_render(HDC dc,const POINT *points,BOOL (WINAPI *draw)(HDC,const POINT *,int),COLORREF edge,
    void (*done)(void *),void *context)
{(void)dc;++panels;if(!points || draw!=Polyline || edge!=theme_edge)failed=1;done(context);}
static BOOL bar_render(HDC dc,const RECT *rect,const RECT *clip)
{(void)dc;++bars;if(clip || rect->left || rect->top || rect->right!=200 || rect->bottom!=100)failed=1;if(mode==2)instance.detached=TRUE;return mode!=1;}
static BOOL bit_render(HWND w,HDC dc,const RECT *rect,const RECT *clip,UINT flags)
{
    (void)dc;++bits;if(!w || clip || flags!=ODS_SELECTED || rect->left || rect->top || rect->right!=200 || rect->bottom!=100)failed=1;
    if(mode==2)instance.detached=TRUE;
    return mode!=1;
}
int main(void)
{
    HMODULE module=GetModuleHandleW(NULL);static const WCHAR *names[]={L"FramePaintParent",L"TGroupBox",L"TPanel",L"TControlBar",L"TBitBtn"};
    for(unsigned i=0;i<ARRAYSIZE(names);++i){WNDCLASSW cls={.hInstance=module,.lpfnWndProc=native,.lpszClassName=names[i]};CHECK(RegisterClassW(&cls));}
    HWND parent=CreateWindowExW(WS_EX_NOACTIVATE,names[0],L"frame association test",WS_POPUP,20,20,320,200,NULL,NULL,module,NULL);CHECK(parent);ShowWindow(parent,SW_SHOWNOACTIVATE);
    HWND group=CreateWindowW(names[1],L"Group",WS_CHILD|WS_VISIBLE,0,0,200,100,parent,NULL,module,NULL);CHECK(group);
    HDC dc=GetDC(group);CHECK(dc);state=(style_runtime_frame_state){.candidate=TRUE};
    style_runtime_frame_paint paint={&scope,&instance,&state,&instance,group,frame_owned,frame_current};
    const style_runtime_frame_renderer renderer={group_render,panel_render,theme_edge};
    RECT background={0,10,200,100},text={6,0,66,20};
    style_runtime_group_frame(&paint,&renderer,dc,4,1,&text,NULL,L"Group",5,0,native,TRUE);CHECK(!groups && !state.verified);
    style_runtime_group_frame(&paint,&renderer,dc,4,1,&background,NULL,NULL,0,0,native,FALSE);CHECK(scope.group_background && !groups);
    current=FALSE;style_runtime_group_frame(&paint,&renderer,dc,4,1,&text,NULL,L"Group",5,0,native,TRUE);CHECK(!groups);current=TRUE;
    style_runtime_group_frame(&paint,&renderer,dc,4,1,&text,NULL,L"Group",5,0,NULL,TRUE);CHECK(!groups);
    style_runtime_group_frame(&paint,&renderer,dc,4,2,&text,NULL,L"Group",5,0,native,TRUE);CHECK(!groups);
    style_runtime_group_frame(&paint,&renderer,dc,4,1,&text,NULL,L"Group",5,0,native,TRUE);CHECK(groups==1 && state.verified && state.parent==parent && !failed);
    for(mode=1;mode<=2;++mode){state.verified=FALSE;instance.detached=FALSE;style_runtime_group_frame(&paint,&renderer,dc,4,1,&text,NULL,L"Group",5,0,native,TRUE);CHECK(!state.verified);}
    instance.detached=FALSE;mode=0;state.retired=TRUE;unsigned before=groups;style_runtime_group_frame(&paint,&renderer,dc,4,1,&text,NULL,L"Group",5,0,native,TRUE);CHECK(groups==before);
    CHECK(ReleaseDC(group,dc));CHECK(DestroyWindow(group));
    HWND panel=CreateWindowW(names[2],L"",WS_CHILD|WS_VISIBLE,0,0,200,100,parent,NULL,module,NULL);CHECK(panel);
    dc=GetDC(panel);CHECK(dc);scope=(style_runtime_frame_scope){0};state=(style_runtime_frame_state){.candidate=TRUE};paint.window=panel;
    COLORREF hi=GetSysColor(COLOR_BTNHIGHLIGHT),lo=GetSysColor(COLOR_BTNSHADOW);CHECK(hi!=lo);
    HPEN high=CreatePen(PS_SOLID,1,hi),low=CreatePen(PS_SOLID,1,lo);CHECK(high && low);HGDIOBJ prior=SelectObject(dc,high);CHECK(prior);
    const POINT first[3]={{0,99},{0,0},{199,0}},second[3]={{199,0},{199,99},{-1,99}};
    style_runtime_panel_polyline(&paint,&renderer,dc,first,3,native,Polyline);CHECK(scope.panel_first && !panels);
    SelectObject(dc,low);style_runtime_panel_polyline(&paint,&renderer,dc,second,3,native,Polyline);CHECK(!scope.panel_first && panels==1 && state.verified && state.parent==parent && !failed);
    SelectObject(dc,high);style_runtime_panel_polyline(&paint,&renderer,dc,first,3,native,Polyline);CHECK(scope.panel_first);
    style_runtime_panel_polyline(&paint,&renderer,dc,second,2,native,Polyline);CHECK(!scope.panel_first && panels==1);
    SelectObject(dc,prior);CHECK(DeleteObject(high) && DeleteObject(low));CHECK(ReleaseDC(panel,dc));CHECK(DestroyWindow(panel));
    HWND bar=CreateWindowW(names[3],L"",WS_CHILD|WS_VISIBLE,0,0,200,100,parent,NULL,module,NULL);CHECK(bar);
    dc=GetDC(bar);CHECK(dc);RECT bounds={0,0,200,100};
    CHECK(style_runtime_controlbar_edge_allowed(EDGE_ETCHED) && !style_runtime_controlbar_edge_allowed(0xff));
    for(mode=0;mode<3;++mode){
        state=(style_runtime_frame_state){.candidate=TRUE,.owner=module,.parent=parent};instance=(style_instance){0};bars=0;
        style_runtime_controlbar_edge(bar,&instance,&state,dc,&bounds,EDGE_RAISED,BF_RECT,native,bar_render);CHECK(!bars);
        style_runtime_controlbar_edge(bar,&instance,&state,dc,&bounds,EDGE_RAISED,BF_RECT|BF_ADJUST,NULL,bar_render);CHECK(!bars);
        style_runtime_controlbar_edge(bar,&instance,&state,dc,&bounds,EDGE_RAISED,BF_RECT|BF_ADJUST,native,bar_render);CHECK(bars==1 && state.verified==(mode==0) && !failed);
    }
    CHECK(ReleaseDC(bar,dc));CHECK(DestroyWindow(bar));
    HWND bit=CreateWindowW(names[4],L"",WS_CHILD|BS_OWNERDRAW,0,0,200,100,parent,NULL,module,NULL);CHECK(bit);dc=GetDC(bit);HDC wrong=CreateCompatibleDC(dc);CHECK(dc && wrong);
    style_runtime_verification verification;style_control_runtime_report report={.module=module,.parent=parent};
    for(mode=0;mode<3;++mode){
        instance=(style_instance){0};verification=(style_runtime_verification){0};bits=0;
        const style_runtime_bit_frame_paint view={bit,dc,bounds,ODS_SELECTED,&instance,&verification,&report};
        style_runtime_bit_frame(&view,dc,2,&bounds,NULL,native,bit_render);style_runtime_bit_frame(&view,dc,1,&bounds,NULL,NULL,bit_render);style_runtime_bit_frame(&view,wrong,1,&bounds,NULL,native,bit_render);CHECK(!bits);
        style_runtime_bit_frame(&view,dc,1,&bounds,NULL,native,bit_render);CHECK(bits==1 && verification.verified==(mode==0) && !failed);
    }
    CHECK(DeleteDC(wrong) && ReleaseDC(bit,dc) && DestroyWindow(bit) && DestroyWindow(parent));
    for(unsigned i=0;i<ARRAYSIZE(names);++i)CHECK(UnregisterClassW(names[i],module));
    puts("FRAME_PAINT PASS native_pair=1 caller=1 current=1 enabled=1 parent_commit=1 renderer_failure=1 detached=1 panel_pair=1 controlbar=1 bit_native=1 bit_commit=1 no_style_renderer=1 cleanup=1");return 0;
}
