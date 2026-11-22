#include "lib-win32-style/application/control/scroll_geometry.h"
#include "lib-win32-style/application/control/scroll_read.h"
#include "lib-win32-style/application/control/scroll_track.h"
#include "lib-win32-style/application/control/scroll_state.h"
#include "lib-win32-style/application/control/scroll_resources.h"
#include "lib-win32-style/application/control/scroll_controller.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

static const style_scroll_policy win16_scroll_policy={200,50,{STYLE_SCROLL_DRAG_LOGICAL,4,1},style_scroll_hit};
static const style_scroll_defaults outline_defaults={STYLE_SCROLL_EXTENT_CROSS_AXIS,STYLE_SCROLL_THUMB_FIXED};
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
static BOOL no_embedded_layout(HWND w,void *context,LONG object,style_scroll_layout *layout,POINT *origin)
{(void)w;(void)context;(void)object;(void)layout;(void)origin;return FALSE;}
#define CHECK(x) do { ++checks; if(!(x)){fprintf(stderr,"SCROLL GEOMETRY FAIL %d: %s\n",__LINE__,#x);exit(1);} } while(0)
static void tracking_tests(void)
{
    const style_scroll_fixed_metrics metrics={17,17,1};RECT bar={0,0,17,200},limit={-4,-1,21,201};
    style_scroll_layout l,from_native;style_scroll_track t={0};style_scroll_actions a;
    style_scroll_policy repeat=win16_scroll_policy;
    style_scroll_repeat_configure(&repeat,317,67);
    CHECK(repeat.initial_repeat_ms==317 && repeat.repeat_ms==67);
    style_scroll_repeat_configure(&repeat,0,0);
    UINT system=GetDoubleClickTime()*4/5;
    CHECK(repeat.initial_repeat_ms==max(1,system) && repeat.repeat_ms==max(1,system/8));
    style_scroll_resources caret={.caret_shown=TRUE};
    style_scroll_caret_destroy(NULL,&caret);CHECK(!caret.caret && !caret.caret_shown);
    CHECK(style_scroll_horizontal_thumb_messages(NULL,fake_info));
    fake_max=65536;CHECK(!style_scroll_horizontal_thumb_messages(NULL,fake_info));
    fake_page=2;CHECK(style_scroll_horizontal_thumb_messages(NULL,fake_info));
    fake_min=-1;CHECK(!style_scroll_horizontal_thumb_messages(NULL,fake_info));
    fake_min=0;fake_max=100;fake_page=1;
    CHECK(style_scroll_list_axis_owned(NULL,OBJID_VSCROLL,fake_info));
    CHECK(style_scroll_list_axis_owned(NULL,OBJID_HSCROLL,fake_info));
    CHECK(!style_scroll_edit_thumb_messages(NULL,OBJID_VSCROLL,fake_info));
    CHECK(style_scroll_disabled(NULL,OBJID_CLIENT)==ESB_DISABLE_BOTH);
    WNDCLASSW cls={.lpfnWndProc=action_parent,.hInstance=GetModuleHandleW(NULL),.lpszClassName=L"S24ScrollActionTest"};
    CHECK(RegisterClassW(&cls)!=0);
    HWND parent=CreateWindowExW(0,cls.lpszClassName,L"",WS_OVERLAPPED,0,0,100,100,NULL,NULL,cls.hInstance,NULL);
    HWND child=parent?CreateWindowExW(0,L"SCROLLBAR",L"",WS_CHILD|SBS_VERT,0,0,20,60,parent,NULL,cls.hInstance,NULL):NULL;
    CHECK(parent && child);
    EnableWindow(child,FALSE);CHECK(style_scroll_disabled(child,OBJID_CLIENT)==ESB_DISABLE_BOTH);
    EnableWindow(child,TRUE);
    CHECK(style_scroll_read_control_layout(child,fake_info,(style_scroll_metrics){0},outline_defaults,&from_native));
    CHECK(from_native.vertical && from_native.first.bottom-from_native.first.top==20);
    style_instance instance={0};style_control_state owner={0};
    style_scroll_controller controller={.epoch=5,.report=77,.object=OBJID_CLIENT};
    const style_scroll_ops operations={NULL,&win16_scroll_policy,fake_info,no_embedded_layout,action_draw,action_nonclient,action_caret};
    controller.resources.previous_capture=parent;style_scroll_finish(child,&instance,&controller,&operations,FALSE);
    CHECK(controller.resources.previous_capture==parent && controller.epoch==5);
    style_scroll_actions notices={.count=2,.timer_ms=STYLE_SCROLL_KEEP_TIMER};
    notices.notices[0].command=SB_LINEUP;notices.notices[1].command=SB_ENDSCROLL;observed_epoch=&controller.epoch;
    style_scroll_apply_actions(child,&instance,&controller,&operations,&notices,5);
    CHECK(received_notices==1 && controller.epoch==6 && !controller.reporting && controller.report==77);
    received_notices=0;controller.epoch=5;
    CHECK(style_scroll_control_input(child,&instance,&controller,&operations,WM_KEYDOWN,VK_DOWN,0));
    CHECK(controller.key && received_notices==1 && last_notice==SB_LINEDOWN);
    CHECK(style_scroll_control_input(child,&instance,&controller,&operations,WM_KEYUP,VK_DOWN,0));
    CHECK(!controller.key && received_notices==2 && last_notice==SB_ENDSCROLL);
    CHECK(!style_scroll_control_input(child,&instance,&controller,&operations,WM_KEYDOWN,VK_F2,0));
    CHECK(!style_scroll_embedded_input(child,&instance,&owner,&controller,&operations,WM_KEYDOWN,VK_DOWN,0));
    controller.resources.frame=TRUE;
    CHECK(!style_scroll_embedded_input(child,&instance,&owner,&controller,&operations,WM_KEYDOWN,VK_DOWN,0));
    CHECK(style_scroll_embedded_input(child,&instance,&owner,&controller,&operations,WM_NCLBUTTONDOWN,HTVSCROLL,0));
    controller.resources.frame=FALSE;
    DestroyWindow(child);DestroyWindow(parent);CHECK(UnregisterClassW(cls.lpszClassName,cls.hInstance));
    POINT point={8,100};
    SCROLLINFO native={.cbSize=sizeof(native),.nMin=0,.nMax=109,.nPage=10,.nPos=50};
    CHECK(style_scroll_measure_metrics(&bar,TRUE,96,&native,(style_scroll_metrics){0},outline_defaults,&from_native));
    CHECK(from_native.maximum==100 && from_native.position==50 && from_native.first.bottom==17);
    CHECK(from_native.thumb.bottom-from_native.thumb.top==17);
    const style_scroll_metrics mixed={STYLE_SCROLL_EXTENT_CROSS_AXIS,0,STYLE_SCROLL_THUMB_PAGE,0,0,{0}};
    CHECK(style_scroll_measure_metrics(&bar,TRUE,96,&native,mixed,outline_defaults,&l));
    CHECK(l.maximum==from_native.maximum && l.thumb.bottom-l.thumb.top==15 && l.minimum_pixel==17);
    CHECK(style_scroll_hit(&l,(POINT){8,l.thumb.top},0)==STYLE_SCROLL_THUMB);
    const style_scroll_defaults native_defaults={STYLE_SCROLL_EXTENT_SYSTEM,STYLE_SCROLL_THUMB_PAGE};
    style_scroll_layout expected;
    CHECK(style_scroll_measure_page(&bar,TRUE,GetSystemMetricsForDpi(SM_CYVSCROLL,96),&native,&expected));
    CHECK(style_scroll_measure_metrics(&bar,TRUE,96,&native,(style_scroll_metrics){0},native_defaults,&l));
    CHECK(EqualRect(&l.first,&expected.first) && EqualRect(&l.thumb,&expected.thumb));
    RECT dragged=style_scroll_thumb_at_pixel(&l,l.thumb.top+13);
    CHECK(dragged.top==l.thumb.top+13 && dragged.bottom==l.thumb.bottom+13 && dragged.left==l.thumb.left);
    RECT horizontal={0,0,200,17};
    CHECK(style_scroll_measure_metrics(&horizontal,FALSE,96,&native,(style_scroll_metrics){0},outline_defaults,&l));
    dragged=style_scroll_thumb_at_pixel(&l,l.thumb.left+11);
    CHECK(dragged.left==l.thumb.left+11 && dragged.right==l.thumb.right+11 && dragged.top==l.thumb.top);
    native.nPage=200;CHECK(style_scroll_measure_metrics(&bar,TRUE,96,&native,(style_scroll_metrics){0},outline_defaults,&from_native) && from_native.maximum==0);
    CHECK(!style_scroll_measure_metrics(NULL,TRUE,96,&native,(style_scroll_metrics){0},outline_defaults,&from_native));
    CHECK(!style_scroll_measure_metrics(&bar,TRUE,96,NULL,(style_scroll_metrics){0},outline_defaults,&from_native));
    CHECK(style_scroll_measure_fixed(&bar,TRUE,&metrics,0,100,50,&l));
    CHECK(style_scroll_begin(&t,&l,0,point,&limit,&a,&win16_scroll_policy));
    CHECK(t.active && t.part==STYLE_SCROLL_THUMB && t.grab_offset==8 && t.pixel==92);
    CHECK(a.count==1 && a.notices[0].command==SB_THUMBTRACK && a.notices[0].position==50 && !a.timer_ms);
    CHECK(!style_scroll_begin(&t,&l,0,point,&limit,&a,&win16_scroll_policy) && t.active && t.pixel==92);
    point.y=130;style_scroll_motion(&t,&l,0,point,FALSE,&a,&win16_scroll_policy);
    CHECK(t.pixel==122 && t.position==70 && a.count==1 && a.notices[0].command==SB_THUMBTRACK);
    style_scroll_motion(&t,&l,0,point,FALSE,&a,&win16_scroll_policy);
    CHECK(!a.count && !a.repaint && a.timer_ms==STYLE_SCROLL_KEEP_TIMER);
    point.x=-5;style_scroll_motion(&t,&l,0,point,FALSE,&a,&win16_scroll_policy);
    CHECK(!t.inside && t.pixel==92 && t.position==50 && a.count==1);
    point.x=8;style_scroll_motion(&t,&l,0,point,FALSE,&a,&win16_scroll_policy);
    CHECK(t.inside && t.pixel==122 && t.position==70 && a.count==1);
    style_scroll_end(&t,TRUE,&a);
    CHECK(!t.active && t.pixel==92 && !a.timer_ms && a.count==2);
    CHECK(a.notices[0].command==SB_THUMBPOSITION && a.notices[0].position==70 && a.notices[1].command==SB_ENDSCROLL);
    style_scroll_end(&t,FALSE,&a);CHECK(!a.count && !a.timer_ms);
    point.x=8;point.y=5;CHECK(style_scroll_begin(&t,&l,0,point,&limit,&a,&win16_scroll_policy));
    CHECK(a.count==1 && a.notices[0].command==SB_LINEUP && a.timer_ms==200);
    style_scroll_motion(&t,&l,0,point,FALSE,&a,&win16_scroll_policy);
    CHECK(!a.count && a.timer_ms==STYLE_SCROLL_KEEP_TIMER);
    style_scroll_motion(&t,&l,0,point,TRUE,&a,&win16_scroll_policy);
    CHECK(a.count==1 && a.notices[0].command==SB_LINEUP && a.timer_ms==50);
    point.x=30;style_scroll_motion(&t,&l,0,point,FALSE,&a,&win16_scroll_policy);
    CHECK(!t.inside && !a.count && !a.timer_ms && a.repaint);
    point.x=8;style_scroll_motion(&t,&l,0,point,FALSE,&a,&win16_scroll_policy);
    CHECK(t.inside && a.count==1 && a.timer_ms==50);
    style_scroll_motion(&t,&l,ESB_DISABLE_UP,point,FALSE,&a,&win16_scroll_policy);
    CHECK(!t.inside && !a.count && !a.timer_ms);
    style_scroll_end(&t,FALSE,&a);CHECK(a.count==1 && a.notices[0].command==SB_ENDSCROLL && !t.active);
    CHECK(!style_scroll_begin(&t,&l,ESB_DISABLE_BOTH,point,&limit,&a,&win16_scroll_policy) && !t.active);
    point.y=50;CHECK(style_scroll_begin(&t,&l,0,point,&limit,&a,&win16_scroll_policy));
    CHECK(a.count==1 && a.notices[0].command==SB_PAGEUP);
    CHECK(style_scroll_measure_fixed(&bar,TRUE,&metrics,0,100,0,&l));
    style_scroll_motion(&t,&l,0,point,TRUE,&a,&win16_scroll_policy);
    CHECK(!t.inside && !a.count && !a.timer_ms);
    style_scroll_end(&t,FALSE,&a);CHECK(!t.active && a.count==1);
    puts("SCROLL TRACK PASS: grab offset, outside/back, original cancel notices, 200/50ms repeat, no motion timer starvation, page-stop, disabled and idempotent end");
}
int main(void)
{
    const style_scroll_fixed_metrics vga={17,17,1};
    style_scroll_layout l,h;RECT v={10,20,27,220},r={20,10,220,27};int p,length;
    /* Hand-calculated original-profile vectors: 17-pixel arrows/thumb and
     * the one-pixel shared boundary, not native SCROLLBARINFO as an oracle. */
    CHECK(style_scroll_measure_fixed(&v,TRUE,&vga,0,100,0,&l));
    CHECK(l.first.bottom==37 && l.last.top==203 && l.minimum_pixel==36 && l.travel==151);
    CHECK(l.thumb.top==36 && l.thumb.bottom==53 && l.thumb_fits);
    {POINT p1={12,36},p2={12,37},p3={12,53},p4={27,37};
        CHECK(style_scroll_hit(&l,p1,0)==STYLE_SCROLL_LINE_BEFORE);
        CHECK(style_scroll_hit(&l,p2,0)==STYLE_SCROLL_THUMB);
        CHECK(style_scroll_hit(&l,p3,0)==STYLE_SCROLL_PAGE_AFTER);
        CHECK(style_scroll_hit(&l,p4,0)==STYLE_SCROLL_NONE);
        CHECK(style_scroll_hit(&l,p1,ESB_DISABLE_UP)==STYLE_SCROLL_NONE);
        CHECK(style_scroll_hit(&l,p2,ESB_DISABLE_BOTH)==STYLE_SCROLL_NONE);}
    CHECK(style_scroll_measure_fixed(&v,TRUE,&vga,0,100,50,&l));
    CHECK(l.thumb.top==112 && l.thumb.bottom==129);
    CHECK(style_scroll_measure_fixed(&v,TRUE,&vga,0,100,100,&l));
    CHECK(l.thumb.top==187 && l.thumb.bottom==204);
    CHECK(style_scroll_position(l.minimum_pixel,l.travel,l.minimum,l.maximum,36)==0 && style_scroll_position(l.minimum_pixel,l.travel,l.minimum,l.maximum,187)==100);
    CHECK(style_scroll_position(l.minimum_pixel,l.travel,l.minimum,l.maximum,INT_MIN)==0 && style_scroll_position(l.minimum_pixel,l.travel,l.minimum,l.maximum,INT_MAX)==100);
    for(length=2;length<=400;++length){
        LONG previous=LONG_MIN;v.bottom=v.top+length;r.right=r.left+length;
        for(p=-101;p<=101;++p){
            CHECK(style_scroll_measure_fixed(&v,TRUE,&vga,-100,100,p,&l));
            CHECK(style_scroll_measure_fixed(&r,FALSE,&vga,-100,100,p,&h));
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
                LONG position=style_scroll_position(l.minimum_pixel,l.travel,l.minimum,l.maximum,p);
                CHECK(position>=-100 && position<=100 && position>=previous);previous=position;
            }
        }
    }
    v.bottom=220;
    CHECK(style_scroll_measure_fixed(&v,TRUE,&vga,LONG_MIN,LONG_MAX,LONG_MAX,&l));
    CHECK(l.thumb.top==187);
    CHECK(style_scroll_position(l.minimum_pixel,l.travel,l.minimum,l.maximum,36)==LONG_MIN && style_scroll_position(l.minimum_pixel,l.travel,l.minimum,l.maximum,187)==LONG_MAX);
    CHECK(style_scroll_measure_fixed(&v,TRUE,&vga,5,5,500,&l) && l.position==5);
    CHECK(style_scroll_position(l.minimum_pixel,l.travel,l.minimum,l.maximum,100)==5);
    CHECK(!style_scroll_measure_fixed(&v,TRUE,&vga,1,0,0,&l));
    CHECK(!style_scroll_measure_fixed(NULL,TRUE,&vga,0,100,0,&l));
    v.top=LONG_MIN;v.bottom=LONG_MAX;
    CHECK(!style_scroll_measure_fixed(&v,TRUE,&vga,0,100,0,&l));
    tracking_tests();
    printf("SCROLL GEOMETRY PASS %u checks: original vectors, shared borders, short bars, transpose, monotonic mapping, native-width bounds\n",checks);
    return 0;
}
