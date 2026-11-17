#include "lib-win32-style/application/control/scroll_controller.h"
#include <commctrl.h>
#include <stdio.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"CONTROLLER line=%d error=%lu\n",__LINE__,GetLastError());return 1;}}while(0)
typedef struct fixture {
    style_instance instance;
    style_control_state state;
    style_scroll_controller scroll;
    style_scroll_layout layout;
    HWND child;
    unsigned notices,last,caret;
    BOOL epoch_stop,destroy_on_notice,reporting;
} fixture;
static fixture *active;
static void dispose(void *context){(void)context;}
static LRESULT CALLBACK parent_proc(HWND w,UINT m,WPARAM a,LPARAM b)
{
    if(m==WM_VSCROLL && active){
        fixture *f=active;++f->notices;f->last=LOWORD(a);f->reporting=f->scroll.reporting;
        if(f->epoch_stop)++f->scroll.epoch;
        if(f->destroy_on_notice)DestroyWindow(f->child);
        return 0;
    }
    return DefWindowProcW(w,m,a,b);
}
static LRESULT CALLBACK child_proc(HWND w,UINT m,WPARAM a,LPARAM b,UINT_PTR id,DWORD_PTR data)
{
    (void)id;fixture *f=(fixture *)data;
    if(m==WM_NCDESTROY)(void)style_instance_detach(&f->instance);
    return DefSubclassProc(w,m,a,b);
}
static style_scroll_part hit(const style_scroll_layout *layout,POINT point,UINT disabled)
{
    if(disabled==ESB_DISABLE_BOTH || !PtInRect(&layout->bar,point))return STYLE_SCROLL_NONE;
    return PtInRect(&layout->first,point)?STYLE_SCROLL_LINE_BEFORE:STYLE_SCROLL_THUMB;
}
static BOOL measure(HWND w,void *context,LONG object,style_scroll_layout *layout,POINT *origin)
{
    (void)w;(void)object;*layout=((fixture *)context)->layout;
    if(origin)*origin=(POINT){0,0};
    return TRUE;
}
static BOOL native_info(HWND w,int bar,LPSCROLLINFO info){return GetScrollInfo(w,bar,info);}
static void draw(HWND w,void *context,HDC dc,LONG object,BOOL vertical)
{(void)w;(void)context;(void)dc;(void)object;(void)vertical;}
static void nonclient(HWND w,void *context,HDC dc){(void)w;(void)context;(void)dc;}
static void caret(HWND w,void *context){(void)w;++((fixture *)context)->caret;}
int main(void)
{
    WNDCLASSW cls={.lpfnWndProc=parent_proc,.hInstance=GetModuleHandleW(NULL),.lpszClassName=L"PublicScrollController"};
    CHECK(RegisterClassW(&cls));HWND parent=CreateWindowW(cls.lpszClassName,L"",WS_OVERLAPPED,0,0,200,240,NULL,NULL,cls.hInstance,NULL);CHECK(parent);
    fixture f={.scroll.report=77};active=&f;
    f.child=CreateWindowW(L"SCROLLBAR",L"",WS_CHILD|SBS_VERT,0,0,20,200,parent,NULL,cls.hInstance,NULL);CHECK(f.child);
    SCROLLINFO info={.cbSize=sizeof(info),.fMask=SIF_RANGE|SIF_PAGE|SIF_POS,.nMin=0,.nMax=100,.nPage=1,.nPos=50};SetScrollInfo(f.child,SB_CTL,&info,FALSE);
    f.layout=(style_scroll_layout){.bar={0,0,20,200},.first={0,0,20,20},.thumb={0,80,20,100},.minimum_pixel=20,.travel=120,.maximum=100,.position=50,.vertical=TRUE,.thumb_fits=TRUE,.thumb_draggable=TRUE};
    style_instance_init(&f.instance,&f,dispose);CHECK(style_instance_attach(&f.instance,f.child,child_proc,NULL));style_instance_retain(&f.instance);
    const style_scroll_policy policy={170,31,2,2,hit,NULL};const style_scroll_ops ops={&f,&policy,native_info,measure,draw,nonclient,caret};
    CHECK(style_scroll_control_input(f.child,&f.instance,&f.scroll,&ops,WM_KEYDOWN,VK_DOWN,0));
    CHECK(f.scroll.key && f.notices==1 && f.last==SB_LINEDOWN && f.reporting && !f.scroll.reporting && f.scroll.report==77);
    CHECK(style_scroll_control_input(f.child,&f.instance,&f.scroll,&ops,WM_KEYUP,VK_DOWN,0));CHECK(!f.scroll.key && f.notices==2 && f.last==SB_ENDSCROLL);
    CHECK(style_scroll_control_input(f.child,&f.instance,&f.scroll,&ops,WM_LBUTTONDOWN,0,MAKELPARAM(10,10)));
    CHECK(f.scroll.track.active && f.scroll.resources.timer && GetCapture()==f.child);
    style_scroll_finish(f.child,&f.instance,&f.scroll,&ops,FALSE);
    CHECK(!f.scroll.track.active && !f.scroll.resources.timer && GetCapture()!=f.child && f.last==SB_ENDSCROLL);
    style_scroll_actions actions={.count=2,.timer_ms=STYLE_SCROLL_KEEP_TIMER};actions.notices[0].command=SB_LINEUP;actions.notices[1].command=SB_ENDSCROLL;
    unsigned before=f.notices;f.epoch_stop=TRUE;style_scroll_apply_actions(f.child,&f.instance,&f.scroll,&ops,&actions,f.scroll.epoch);
    CHECK(f.notices==before+1 && !f.scroll.reporting && f.scroll.report==77);
    f.epoch_stop=FALSE;f.destroy_on_notice=TRUE;before=f.notices;unsigned paints=f.caret;
    style_scroll_apply_actions(f.child,&f.instance,&f.scroll,&ops,&actions,f.scroll.epoch);
    CHECK(f.instance.detached && !IsWindow(f.child) && f.notices==before+1 && f.caret==paints && !f.scroll.reporting && f.scroll.report==77);
    style_instance_release(&f.instance);CHECK(DestroyWindow(parent) && UnregisterClassW(cls.lpszClassName,cls.hInstance));
    puts("CONTROLLER real scrollbar notices, capture/timer lifecycle, epoch stop and callback destruction PASS");return 0;
}
