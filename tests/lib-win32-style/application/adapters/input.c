#include "lib-win32-style/application/adapters/input.h"
#include <stdio.h>
#include "lib-win32-style/application/adapters/surface.h"

#define CHECK(x) do {if(!(x)){fprintf(stderr,"UWP INPUT FAIL line=%d\n",__LINE__);return 1;}} while(0)
typedef struct fixture {style_uwp_frame frame;BOOL detach_on_refresh;int invoked;UINT event;unsigned refreshes;} fixture;
static WPARAM drag_hit;static LPARAM drag_point;
static LRESULT CALLBACK owner_proc(HWND window,UINT message,WPARAM wp,LPARAM lp)
{
    if(message==WM_NCLBUTTONDOWN){drag_hit=wp;drag_point=lp;return 0;}
    return DefWindowProcW(window,message,wp,lp);
}
static LRESULT hit(void *context,POINT point)
{(void)context;return point.x<20?HTMAXBUTTON:point.x<40?HTCAPTION:HTCLIENT;}
static BOOL button(void *context,int part)
{(void)context;return part==HTMAXBUTTON;}
static void invoke(void *context,int part,UINT event)
{fixture *f=context;f->invoked=part;f->event=event;}
static BOOL geometry(void *context,RECT *outer,RECT *hole,POINT *anchor)
{(void)context;*outer=(RECT){0,0,120,80};*hole=(RECT){5,25,115,75};*anchor=(POINT){5,25};return TRUE;}
static void paint(void *context,HDC dc)
{fixture *f=context;(void)dc;++f->refreshes;if(f->detach_on_refresh)f->frame.host.instance.detached=TRUE;}
int main(void)
{
    HINSTANCE instance=GetModuleHandleW(NULL);
    WNDCLASSW cls={.hInstance=instance,.lpfnWndProc=owner_proc,.lpszClassName=L"StyleUwpInputOwner"};
    CHECK(RegisterClassW(&cls));
    HWND owner=CreateWindowW(cls.lpszClassName,L"",WS_OVERLAPPEDWINDOW|WS_VISIBLE,0,0,120,80,NULL,NULL,instance,NULL);
    HWND surface=owner?CreateWindowExW(WS_EX_LAYERED,L"STATIC",L"",WS_POPUP,0,0,40,20,owner,NULL,instance,NULL):NULL;
    CHECK(owner && surface);fixture f={0};
    const style_uwp_frame_ops operations={.geometry=geometry,.paint=paint,.hit=hit,.button=button,.invoke=invoke};
    f.frame=(style_uwp_frame){.host={.owner=owner,.surface=surface},.context=&f,.ops=&operations};
    LRESULT result=0;
    CHECK(style_uwp_input_message(&f.frame,WM_MOUSEACTIVATE,0,0,&result) && result==MA_NOACTIVATE);
    CHECK(style_uwp_input_message(&f.frame,WM_NCHITTEST,0,0,&result) && result==HTCLIENT);
    CHECK(!style_uwp_input_message(&f.frame,WM_NULL,0,0,&result));
    CHECK(style_uwp_input_message(&f.frame,WM_LBUTTONDOWN,0,MAKELPARAM(1,1),&result) && f.frame.pressed==HTMAXBUTTON && f.frame.hot);
    CHECK(style_uwp_input_message(&f.frame,WM_MOUSEMOVE,0,MAKELPARAM(30,1),&result) && !f.frame.hot);
    CHECK(style_uwp_input_message(&f.frame,WM_MOUSEMOVE,0,MAKELPARAM(1,1),&result) && f.frame.hot);
    CHECK(style_uwp_input_message(&f.frame,WM_LBUTTONUP,0,MAKELPARAM(1,1),&result) && !f.frame.pressed && !f.frame.hot && f.invoked==HTMAXBUTTON && f.event==WM_LBUTTONUP);
    POINT drag={30,1};CHECK(ClientToScreen(surface,&drag));
    CHECK(style_uwp_input_message(&f.frame,WM_LBUTTONDOWN,0,MAKELPARAM(30,1),&result) && drag_hit==HTCAPTION && drag_point==MAKELPARAM(drag.x,drag.y));
    f.invoked=0;CHECK(style_uwp_input_message(&f.frame,WM_LBUTTONDOWN,0,MAKELPARAM(1,1),&result));
    CHECK(style_uwp_input_message(&f.frame,WM_CANCELMODE,0,0,&result) && !f.frame.pressed && !f.invoked);
    CHECK(style_uwp_input_message(&f.frame,WM_LBUTTONDBLCLK,0,MAKELPARAM(1,1),&result) && f.event==WM_LBUTTONDBLCLK);
    CHECK(style_uwp_input_message(&f.frame,WM_RBUTTONUP,0,MAKELPARAM(1,1),&result) && f.event==WM_RBUTTONUP);
    f.invoked=0;CHECK(style_uwp_input_message(&f.frame,WM_LBUTTONDOWN,0,MAKELPARAM(1,1),&result));
    f.detach_on_refresh=TRUE;
    CHECK(style_uwp_input_message(&f.frame,WM_LBUTTONUP,0,MAKELPARAM(1,1),&result) && f.frame.host.instance.detached && !f.invoked && !f.frame.pressed);
    style_bitmap_dispose(&f.frame.bitmap);
    CHECK(DestroyWindow(surface) && DestroyWindow(owner) && UnregisterClassW(cls.lpszClassName,instance));
    puts("UWP_INPUT_OK button=1 hot=1 drag=1 cancel=1 double=1 right=1 detach=1");return 0;
}
