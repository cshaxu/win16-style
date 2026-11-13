#include "lib-style/uwp/input.h"
#include <stdio.h>

#define CHECK(x) do {if(!(x)){fprintf(stderr,"UWP INPUT FAIL line=%d\n",__LINE__);return 1;}} while(0)
typedef struct fixture {BOOL detached,hot,detach_on_refresh;int pressed,invoked;UINT event;unsigned refreshes;} fixture;
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
static BOOL refresh(void *context)
{fixture *f=context;++f->refreshes;if(f->detach_on_refresh)f->detached=TRUE;return !f->detached;}
int main(void)
{
    HINSTANCE instance=GetModuleHandleW(NULL);
    WNDCLASSW cls={.hInstance=instance,.lpfnWndProc=owner_proc,.lpszClassName=L"StyleUwpInputOwner"};
    CHECK(RegisterClassW(&cls));
    HWND owner=CreateWindowW(cls.lpszClassName,L"",WS_OVERLAPPEDWINDOW,0,0,120,80,NULL,NULL,instance,NULL);
    HWND surface=owner?CreateWindowW(L"STATIC",L"",WS_POPUP,0,0,40,20,owner,NULL,instance,NULL):NULL;
    CHECK(owner && surface);fixture f={0};
    const style_uwp_input_view view={owner,surface,&f.detached,&f.pressed,&f.hot,&f,hit,button,invoke,refresh};
    LRESULT result=0;
    CHECK(style_uwp_input_message(&view,WM_MOUSEACTIVATE,0,0,&result) && result==MA_NOACTIVATE);
    CHECK(style_uwp_input_message(&view,WM_NCHITTEST,0,0,&result) && result==HTCLIENT);
    CHECK(!style_uwp_input_message(&view,WM_NULL,0,0,&result));
    CHECK(style_uwp_input_message(&view,WM_LBUTTONDOWN,0,MAKELPARAM(1,1),&result) && f.pressed==HTMAXBUTTON && f.hot);
    CHECK(style_uwp_input_message(&view,WM_MOUSEMOVE,0,MAKELPARAM(30,1),&result) && !f.hot);
    CHECK(style_uwp_input_message(&view,WM_MOUSEMOVE,0,MAKELPARAM(1,1),&result) && f.hot);
    CHECK(style_uwp_input_message(&view,WM_LBUTTONUP,0,MAKELPARAM(1,1),&result) && !f.pressed && !f.hot && f.invoked==HTMAXBUTTON && f.event==WM_LBUTTONUP);
    POINT drag={30,1};CHECK(ClientToScreen(surface,&drag));
    CHECK(style_uwp_input_message(&view,WM_LBUTTONDOWN,0,MAKELPARAM(30,1),&result) && drag_hit==HTCAPTION && drag_point==MAKELPARAM(drag.x,drag.y));
    f.invoked=0;CHECK(style_uwp_input_message(&view,WM_LBUTTONDOWN,0,MAKELPARAM(1,1),&result));
    CHECK(style_uwp_input_message(&view,WM_CANCELMODE,0,0,&result) && !f.pressed && !f.invoked);
    CHECK(style_uwp_input_message(&view,WM_LBUTTONDBLCLK,0,MAKELPARAM(1,1),&result) && f.event==WM_LBUTTONDBLCLK);
    CHECK(style_uwp_input_message(&view,WM_RBUTTONUP,0,MAKELPARAM(1,1),&result) && f.event==WM_RBUTTONUP);
    f.invoked=0;CHECK(style_uwp_input_message(&view,WM_LBUTTONDOWN,0,MAKELPARAM(1,1),&result));
    f.detach_on_refresh=TRUE;
    CHECK(style_uwp_input_message(&view,WM_LBUTTONUP,0,MAKELPARAM(1,1),&result) && f.detached && !f.invoked && !f.pressed);
    CHECK(DestroyWindow(surface) && DestroyWindow(owner) && UnregisterClassW(cls.lpszClassName,instance));
    puts("UWP_INPUT_OK button=1 hot=1 drag=1 cancel=1 double=1 right=1 detach=1");return 0;
}
