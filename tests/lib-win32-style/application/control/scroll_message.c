#include "lib-win32-style/application/control/scroll_message.h"
#include <stdio.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"SCROLL_MESSAGE line=%d error=%lu\n",__LINE__,GetLastError());return 1;}}while(0)
typedef struct fixture {
    style_instance instance;style_control_state state;style_scroll_controller scroll;
    BOOL frame,sizebox,stop;unsigned draws,caret,borders,bars,overlays,disposed,keys;
    UINT message;WPARAM parameter;LPARAM value;
} fixture;
static style_scroll_ops operations;
static void dispose(void *context){++((fixture *)context)->disposed;}
static BOOL layout(HWND w,void *c,LONG o,style_scroll_layout *l,POINT *p)
{(void)w;(void)c;(void)o;(void)l;(void)p;return FALSE;}
static style_scroll_part hit(const style_scroll_layout *l,POINT p,UINT d)
{(void)l;(void)p;(void)d;return STYLE_SCROLL_NONE;}
static BOOL info(HWND w,int bar,LPSCROLLINFO s){return GetScrollInfo(w,bar,s);}
static void draw(HWND w,void *c,HDC dc,LONG o,BOOL v){(void)w;(void)dc;(void)o;(void)v;++((fixture *)c)->draws;}
static void caret(HWND w,void *c){(void)w;++((fixture *)c)->caret;}
static void border(HWND w,void *c,HDC dc){(void)w;(void)dc;++((fixture *)c)->borders;}
static void bars(HWND w,void *c,HDC dc){(void)w;(void)dc;++((fixture *)c)->bars;}
static void overlay(HWND w,void *c,HDC dc){(void)w;(void)dc;++((fixture *)c)->overlays;}
static void sizebox(HWND w,void *c,HDC dc,BOOL buffered){(void)w;(void)dc;(void)buffered;++((fixture *)c)->draws;}
static LRESULT CALLBACK witness(HWND w,UINT m,WPARAM a,LPARAM b,UINT_PTR id,DWORD_PTR data)
{
    (void)id;fixture *f=(fixture *)data;
    if(m==WM_PRINT){SetViewportOrgEx((HDC)a,80,81,NULL);if(f->stop)(void)style_instance_detach(&f->instance);return 17;}
    if(m==SBM_SETPOS || m==SBM_SETSCROLLINFO || m==SBM_SETRANGE){f->message=m;f->parameter=a;f->value=b;return 81;}
    if(m==SBM_GETSCROLLINFO){((SCROLLINFO *)b)->nTrackPos=12;return TRUE;}
    if(m==WM_KEYDOWN){++f->keys;return 77;}
    return DefSubclassProc(w,m,a,b);
}
static LRESULT CALLBACK procedure(HWND w,UINT m,WPARAM a,LPARAM b,UINT_PTR id,DWORD_PTR data)
{
    (void)id;fixture *f=(fixture *)data;style_instance_retain(&f->instance);LRESULT result;
    if(m==WM_NCDESTROY){(void)style_instance_detach(&f->instance);result=DefSubclassProc(w,m,a,b);}
    else if(f->frame){const style_scroll_frame_ops frame={f,TRUE,border,overlay};result=style_scroll_frame_message(w,&f->instance,&f->state,&f->scroll,m,a,b,&operations,&frame);}
    else if(f->sizebox){const style_control_draw render={f,sizebox};result=style_scroll_sizebox_message(w,m,a,b,&render);}
    else result=style_scroll_control_message(w,&f->instance,&f->scroll,&m,&a,&b,&operations);
    style_instance_release(&f->instance);return result;
}
int main(void)
{
    HINSTANCE module=GetModuleHandleW(NULL);HWND root=CreateWindowW(L"STATIC",L"",WS_OVERLAPPED,0,0,200,240,NULL,NULL,module,NULL);
    HWND window=CreateWindowW(L"SCROLLBAR",L"",WS_CHILD|SBS_VERT,0,0,20,200,root,NULL,module,NULL);CHECK(root && window);
    fixture f={0};style_instance_init(&f.instance,&f,dispose);const style_scroll_policy policy={170,31,2,2,hit,NULL};
    operations=(style_scroll_ops){&f,&policy,info,layout,draw,bars,caret};
    CHECK(SetWindowSubclass(window,witness,1,(DWORD_PTR)&f) && style_instance_attach(&f.instance,window,procedure,NULL));
    CHECK(SendMessageW(window,SBM_SETPOS,44,TRUE)==81 && f.message==SBM_SETPOS && f.parameter==44 && !f.value);
    SCROLLINFO scroll={.cbSize=sizeof(scroll),.fMask=SIF_TRACKPOS};
    CHECK(SendMessageW(window,SBM_SETSCROLLINFO,TRUE,(LPARAM)&scroll)==81 && !f.parameter && f.value==(LPARAM)&scroll);
    CHECK(SendMessageW(window,SBM_SETRANGEREDRAW,3,90)==81 && f.message==SBM_SETRANGE && f.parameter==3 && f.value==90);
    f.scroll.reporting=TRUE;f.scroll.report=200000;CHECK(SendMessageW(window,SBM_GETSCROLLINFO,0,(LPARAM)&scroll) && scroll.nTrackPos==200000);f.scroll.reporting=FALSE;
    EnableWindow(window,FALSE);CHECK(SendMessageW(window,WM_KEYDOWN,VK_DOWN,0)==0 && !f.keys && !f.scroll.key);EnableWindow(window,TRUE);
    HDC dc=CreateCompatibleDC(NULL);CHECK(dc);f.sizebox=TRUE;unsigned count=f.draws;
    SendMessageW(window,WM_PRINTCLIENT,(WPARAM)dc,0);CHECK(f.draws==count+1 && SendMessageW(window,WM_ERASEBKGND,(WPARAM)dc,0)==1);
    f.sizebox=FALSE;f.frame=TRUE;f.scroll.resources.frame=TRUE;f.scroll.resources.native_border=TRUE;
    SetViewportOrgEx(dc,3,4,NULL);unsigned borders=f.borders,before=f.bars,overlays=f.overlays;
    CHECK(SendMessageW(window,WM_PRINT,(WPARAM)dc,PRF_NONCLIENT)==17);
    POINT origin;CHECK(GetViewportOrgEx(dc,&origin) && origin.x==3 && origin.y==4 && f.borders==borders+1 && f.bars==before+1 && f.overlays==overlays+1);
    f.stop=TRUE;borders=f.borders;before=f.bars;
    CHECK(SendMessageW(window,WM_PRINT,(WPARAM)dc,PRF_NONCLIENT)==17 && f.instance.detached && f.disposed==1);
    CHECK(GetViewportOrgEx(dc,&origin) && origin.x==3 && origin.y==4 && f.borders==borders && f.bars==before);
    CHECK(RemoveWindowSubclass(window,witness,1) && DeleteDC(dc) && DestroyWindow(root));
    puts("SCROLL_MESSAGE native result/argument rewriting, full track position, disabled native input, sizebox and frame DC/retirement PASS");return 0;
}
