#include "lib-win32/win16-control/scroll_layout.h"
#include "lib-win32/win16-control/scroll_native.h"
#include "lib-win32/win16-control/scroll_track.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

static unsigned checks;
static int fake_max=100;static int fake_min=0;static UINT fake_page=1;
static BOOL fake_info(HWND w,int bar,LPSCROLLINFO info)
{(void)w;if(bar!=SB_HORZ && bar!=SB_CTL)return FALSE;info->nMin=fake_min;info->nMax=fake_max;info->nPage=fake_page;return TRUE;}
static unsigned *observed_epoch;static int received_notices;static UINT last_notice;
static LRESULT CALLBACK action_parent(HWND w,UINT m,WPARAM a,LPARAM b)
{if(m==WM_HSCROLL || m==WM_VSCROLL){++received_notices;last_notice=LOWORD(a);++*observed_epoch;return 0;}return DefWindowProcW(w,m,a,b);}
static void action_draw(HWND w,void *p,HDC dc,LONG object,BOOL vertical)
{(void)w;(void)p;(void)dc;(void)object;(void)vertical;}
static void action_nonclient(HWND w,void *p,HDC dc){(void)w;(void)p;(void)dc;}
static void action_caret(HWND w,void *p){(void)w;(void)p;}
static BOOL no_owner(void *context){(void)context;return FALSE;}
static BOOL no_embedded_layout(HWND w,LONG object,win16_scroll_layout *layout,POINT *origin)
{(void)w;(void)object;(void)layout;(void)origin;return FALSE;}
#define CHECK(x) do { ++checks; if(!(x)){fprintf(stderr,"SCROLL GEOMETRY FAIL %d: %s\n",__LINE__,#x);exit(1);} } while(0)
static void tracking_tests(void)
{
    const win16_scroll_metrics metrics={17,17,1};RECT bar={0,0,17,200},limit={-4,-1,21,201};
    win16_scroll_layout l,from_native;win16_scroll_track t={0};win16_scroll_actions a;
    BOOL detached=TRUE,created=FALSE,shown=TRUE;SIZE caret_size={0};
    const win16_scroll_caret_view caret={&detached,&created,&shown,&caret_size,&t,NULL,NULL};
    win16_scroll_caret_sync(NULL,&caret);CHECK(!created && shown);
    win16_scroll_caret_destroy(NULL,&caret);CHECK(!created && !shown);
    CHECK(win16_scroll_horizontal_thumb_messages(NULL,fake_info));
    fake_max=65536;CHECK(!win16_scroll_horizontal_thumb_messages(NULL,fake_info));
    fake_page=2;CHECK(win16_scroll_horizontal_thumb_messages(NULL,fake_info));
    fake_min=-1;CHECK(!win16_scroll_horizontal_thumb_messages(NULL,fake_info));
    fake_min=0;fake_max=100;fake_page=1;
    CHECK(win16_scroll_list_axis_owned(NULL,OBJID_VSCROLL,fake_info));
    CHECK(win16_scroll_list_axis_owned(NULL,OBJID_HSCROLL,fake_info));
    CHECK(!win16_scroll_edit_thumb_messages(NULL,OBJID_VSCROLL,fake_info));
    CHECK(win16_scroll_disabled(NULL,OBJID_CLIENT)==ESB_DISABLE_BOTH);
    WNDCLASSW cls={.lpfnWndProc=action_parent,.hInstance=GetModuleHandleW(NULL),.lpszClassName=L"S24ScrollActionTest"};
    CHECK(RegisterClassW(&cls)!=0);
    HWND parent=CreateWindowExW(0,cls.lpszClassName,L"",WS_OVERLAPPED,0,0,100,100,NULL,NULL,cls.hInstance,NULL);
    HWND child=parent?CreateWindowExW(0,L"SCROLLBAR",L"",WS_CHILD|SBS_VERT,0,0,20,60,parent,NULL,cls.hInstance,NULL):NULL;
    CHECK(parent && child);
    CHECK(win16_scroll_control_layout(child,fake_info,&from_native));
    CHECK(from_native.vertical && from_native.first.bottom-from_native.first.top==20);
    BOOL action_detached=FALSE,reporting=FALSE,frame=FALSE,edit=FALSE,list=FALSE;
    unsigned epoch=5;LONG report=77,object=OBJID_CLIENT;UINT_PTR timer=0;win16_scroll_track action_track={0};
    const win16_scroll_actions_view view={NULL,&action_detached,&reporting,&frame,&edit,&list,&epoch,&report,&object,
        &timer,&action_track,action_draw,action_nonclient,action_caret};
    HWND prior_capture=parent;win16_scroll_finish(child,&view,&prior_capture,FALSE);
    CHECK(prior_capture==parent && epoch==5);
    win16_scroll_actions notices={.count=2,.timer_ms=WIN16_SCROLL_KEEP_TIMER};
    notices.notices[0].command=SB_LINEUP;notices.notices[1].command=SB_ENDSCROLL;observed_epoch=&epoch;
    win16_scroll_apply_actions(child,&view,&notices,5);
    CHECK(received_notices==1 && epoch==6 && !reporting && report==77);
    BOOL key=FALSE;win16_scroll_input_view input={view,&prior_capture,&key,NULL};
    received_notices=0;epoch=5;
    CHECK(win16_scroll_control_input(child,&input,WM_KEYDOWN,VK_DOWN,0));
    CHECK(key && received_notices==1 && last_notice==SB_LINEDOWN);
    CHECK(win16_scroll_control_input(child,&input,WM_KEYUP,VK_DOWN,0));
    CHECK(!key && received_notices==2 && last_notice==SB_ENDSCROLL);
    CHECK(!win16_scroll_control_input(child,&input,WM_KEYDOWN,VK_F2,0));
    const win16_scroll_embedded_view embedded={view,&prior_capture,no_owner,no_embedded_layout,fake_info};
    CHECK(!win16_scroll_embedded_input(child,&embedded,WM_KEYDOWN,VK_DOWN,0));
    frame=TRUE;
    CHECK(!win16_scroll_embedded_input(child,&embedded,WM_KEYDOWN,VK_DOWN,0));
    CHECK(win16_scroll_embedded_input(child,&embedded,WM_NCLBUTTONDOWN,HTVSCROLL,0));
    frame=FALSE;
    DestroyWindow(child);DestroyWindow(parent);CHECK(UnregisterClassW(cls.lpszClassName,cls.hInstance));
    POINT point={8,100};
    SCROLLINFO native={.cbSize=sizeof(native),.nMin=0,.nMax=109,.nPage=10,.nPos=50};
    CHECK(win16_scroll_measure_native(&bar,TRUE,96,&native,&from_native));
    CHECK(from_native.maximum==100 && from_native.position==50 && from_native.first.bottom==17);
    native.nPage=200;CHECK(win16_scroll_measure_native(&bar,TRUE,96,&native,&from_native) && from_native.maximum==0);
    CHECK(!win16_scroll_measure_native(NULL,TRUE,96,&native,&from_native));
    CHECK(!win16_scroll_measure_native(&bar,TRUE,96,NULL,&from_native));
    CHECK(win16_scroll_measure(&bar,TRUE,&metrics,0,100,50,&l));
    CHECK(win16_scroll_begin(&t,&l,0,point,&limit,&a));
    CHECK(t.active && t.part==WIN16_SCROLL_THUMB && t.grab_offset==8 && t.pixel==92);
    CHECK(a.count==1 && a.notices[0].command==SB_THUMBTRACK && a.notices[0].position==50 && !a.timer_ms);
    CHECK(!win16_scroll_begin(&t,&l,0,point,&limit,&a) && t.active && t.pixel==92);
    point.y=130;win16_scroll_motion(&t,&l,0,point,FALSE,&a);
    CHECK(t.pixel==122 && t.position==70 && a.count==1 && a.notices[0].command==SB_THUMBTRACK);
    win16_scroll_motion(&t,&l,0,point,FALSE,&a);
    CHECK(!a.count && !a.repaint && a.timer_ms==WIN16_SCROLL_KEEP_TIMER);
    point.x=-5;win16_scroll_motion(&t,&l,0,point,FALSE,&a);
    CHECK(!t.inside && t.pixel==92 && t.position==50 && a.count==1);
    point.x=8;win16_scroll_motion(&t,&l,0,point,FALSE,&a);
    CHECK(t.inside && t.pixel==122 && t.position==70 && a.count==1);
    win16_scroll_end(&t,TRUE,&a);
    CHECK(!t.active && t.pixel==92 && !a.timer_ms && a.count==2);
    CHECK(a.notices[0].command==SB_THUMBPOSITION && a.notices[0].position==70 && a.notices[1].command==SB_ENDSCROLL);
    win16_scroll_end(&t,FALSE,&a);CHECK(!a.count && !a.timer_ms);
    point.x=8;point.y=5;CHECK(win16_scroll_begin(&t,&l,0,point,&limit,&a));
    CHECK(a.count==1 && a.notices[0].command==SB_LINEUP && a.timer_ms==200);
    win16_scroll_motion(&t,&l,0,point,FALSE,&a);
    CHECK(!a.count && a.timer_ms==WIN16_SCROLL_KEEP_TIMER);
    win16_scroll_motion(&t,&l,0,point,TRUE,&a);
    CHECK(a.count==1 && a.notices[0].command==SB_LINEUP && a.timer_ms==50);
    point.x=30;win16_scroll_motion(&t,&l,0,point,FALSE,&a);
    CHECK(!t.inside && !a.count && !a.timer_ms && a.repaint);
    point.x=8;win16_scroll_motion(&t,&l,0,point,FALSE,&a);
    CHECK(t.inside && a.count==1 && a.timer_ms==50);
    win16_scroll_motion(&t,&l,ESB_DISABLE_UP,point,FALSE,&a);
    CHECK(!t.inside && !a.count && !a.timer_ms);
    win16_scroll_end(&t,FALSE,&a);CHECK(a.count==1 && a.notices[0].command==SB_ENDSCROLL && !t.active);
    CHECK(!win16_scroll_begin(&t,&l,ESB_DISABLE_BOTH,point,&limit,&a) && !t.active);
    point.y=50;CHECK(win16_scroll_begin(&t,&l,0,point,&limit,&a));
    CHECK(a.count==1 && a.notices[0].command==SB_PAGEUP);
    CHECK(win16_scroll_measure(&bar,TRUE,&metrics,0,100,0,&l));
    win16_scroll_motion(&t,&l,0,point,TRUE,&a);
    CHECK(!t.inside && !a.count && !a.timer_ms);
    win16_scroll_end(&t,FALSE,&a);CHECK(!t.active && a.count==1);
    puts("SCROLL TRACK PASS: grab offset, outside/back, original cancel notices, 200/50ms repeat, no motion timer starvation, page-stop, disabled and idempotent end");
}
int main(void)
{
    const win16_scroll_metrics vga={17,17,1};
    win16_scroll_layout l,h;RECT v={10,20,27,220},r={20,10,220,27};int p,length;
    /* Hand-calculated original-profile vectors: 17-pixel arrows/thumb and
     * the one-pixel shared boundary, not native SCROLLBARINFO as an oracle. */
    CHECK(win16_scroll_measure(&v,TRUE,&vga,0,100,0,&l));
    CHECK(l.first.bottom==37 && l.last.top==203 && l.minimum_pixel==36 && l.travel==151);
    CHECK(l.thumb.top==36 && l.thumb.bottom==53 && l.thumb_fits);
    {POINT p1={12,36},p2={12,37},p3={12,53},p4={27,37};
        CHECK(win16_scroll_hit(&l,p1,0)==WIN16_SCROLL_LINE_BEFORE);
        CHECK(win16_scroll_hit(&l,p2,0)==WIN16_SCROLL_THUMB);
        CHECK(win16_scroll_hit(&l,p3,0)==WIN16_SCROLL_PAGE_AFTER);
        CHECK(win16_scroll_hit(&l,p4,0)==WIN16_SCROLL_NONE);
        CHECK(win16_scroll_hit(&l,p1,ESB_DISABLE_UP)==WIN16_SCROLL_NONE);
        CHECK(win16_scroll_hit(&l,p2,ESB_DISABLE_BOTH)==WIN16_SCROLL_NONE);}
    CHECK(win16_scroll_measure(&v,TRUE,&vga,0,100,50,&l));
    CHECK(l.thumb.top==112 && l.thumb.bottom==129);
    CHECK(win16_scroll_measure(&v,TRUE,&vga,0,100,100,&l));
    CHECK(l.thumb.top==187 && l.thumb.bottom==204);
    CHECK(win16_scroll_position(&l,36)==0 && win16_scroll_position(&l,187)==100);
    CHECK(win16_scroll_position(&l,INT_MIN)==0 && win16_scroll_position(&l,INT_MAX)==100);
    for(length=2;length<=400;++length){
        LONG previous=LONG_MIN;v.bottom=v.top+length;r.right=r.left+length;
        for(p=-101;p<=101;++p){
            CHECK(win16_scroll_measure(&v,TRUE,&vga,-100,100,p,&l));
            CHECK(win16_scroll_measure(&r,FALSE,&vga,-100,100,p,&h));
            CHECK(l.first.bottom==h.first.right && l.last.top==h.last.left);
            CHECK(l.thumb.top==h.thumb.left && l.thumb.bottom==h.thumb.right);
            CHECK(l.thumb.bottom-l.thumb.top==17);
            CHECK(l.position>=-100 && l.position<=100);
            CHECK(l.thumb.top>=previous);previous=l.thumb.top;
            CHECK(l.thumb_fits==((l.last.top-l.first.bottom)>=17));
            CHECK(l.thumb_draggable==((l.last.top-l.first.bottom)>17));
        }
        if(l.thumb_fits){
            previous=LONG_MIN;
            for(p=l.minimum_pixel-1;p<=l.minimum_pixel+l.travel+1;++p){
                LONG position=win16_scroll_position(&l,p);
                CHECK(position>=-100 && position<=100 && position>=previous);previous=position;
            }
        }
    }
    v.bottom=220;
    CHECK(win16_scroll_measure(&v,TRUE,&vga,LONG_MIN,LONG_MAX,LONG_MAX,&l));
    CHECK(l.thumb.top==187);
    CHECK(win16_scroll_position(&l,36)==LONG_MIN && win16_scroll_position(&l,187)==LONG_MAX);
    CHECK(win16_scroll_measure(&v,TRUE,&vga,5,5,500,&l) && l.position==5);
    CHECK(win16_scroll_position(&l,100)==5);
    CHECK(!win16_scroll_measure(&v,TRUE,&vga,1,0,0,&l));
    CHECK(!win16_scroll_measure(NULL,TRUE,&vga,0,100,0,&l));
    v.top=LONG_MIN;v.bottom=LONG_MAX;
    CHECK(!win16_scroll_measure(&v,TRUE,&vga,0,100,0,&l));
    tracking_tests();
    printf("SCROLL GEOMETRY PASS %u checks: original vectors, shared borders, short bars, transpose, monotonic mapping, native-width bounds\n",checks);
    return 0;
}
