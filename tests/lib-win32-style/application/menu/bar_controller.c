#include "lib-win32-style/application/menu/bar_controller.h"
#include "lib-win32-style/application/menu/classic_menu_layout.h"
#include <stdio.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"BAR_CONTROLLER line=%d error=%lu\n",__LINE__,GetLastError());return 1;}}while(0)
typedef struct fixture {style_instance instance;style_menu_owner owner;HWND window;unsigned paints,commands,disposed;BOOL detach_init,detach_read,pinned_read,timed_out;struct fixture *next;unsigned next_calls;} fixture;
static void dispose(void *context){++((fixture *)context)->disposed;}
static LRESULT CALLBACK procedure(HWND w,UINT m,WPARAM a,LPARAM b)
{
    fixture *f=(fixture *)GetWindowLongPtrW(w,GWLP_USERDATA);
    if(f && m==WM_INITMENU && f->detach_init){f->instance.detached=TRUE;return 0;}
    if(f && m==WM_ENTERMENULOOP && f->next){PostMessageW(w,WM_KEYDOWN,VK_RIGHT,1);return 0;}
    if(f && m==WM_NEXTMENU && f->next){MDINEXTMENU *next=(MDINEXTMENU *)b;++f->next_calls;next->hwndNext=f->next->window;next->hmenuNext=GetMenu(next->hwndNext);return 0;}
    if(f && m==WM_TIMER){f->timed_out=TRUE;EndMenu();return 0;}
    if(f && m==WM_COMMAND){++f->commands;return 0;}
    return DefWindowProcW(w,m,a,b);
}
static BOOL measure(void *context,HDC dc,const MENUITEMINFOW *item,SIZE *size)
{(void)context;(void)dc;(void)item;*size=(SIZE){60,20};return TRUE;}
static BOOL read(const style_menu_bar_target *t,style_menu_bar_snapshot *bar)
{
    fixture *f=t->context;if(f->detach_read){f->pinned_read=f->instance.calls>0;f->instance.detached=TRUE;return FALSE;}
    const style_menu_bar_profile profile={.measure=measure};
    RECT outer;if(!GetWindowRect(t->window,&outer))return FALSE;
    return style_menu_bar_read(t->window,GetMenu(t->window),(POINT){outer.left,outer.top},300,&profile,NULL,bar);
}
static int hit(const style_menu_bar_target *t,POINT point)
{
    style_menu_bar_snapshot bar;if(!read(t,&bar))return STYLE_MENU_NO_ITEM;
    int index=style_classic_menu_hit(bar.rects,bar.count,point);style_menu_bar_free(&bar);
    return index<0?STYLE_MENU_NO_ITEM:index;
}
static void paint(const style_menu_bar_target *t){++((fixture *)t->context)->paints;}
static const style_menu_bar_ops columns;
static BOOL lookup(HWND window,style_menu_bar_target *target)
{
    fixture *f=(fixture *)GetWindowLongPtrW(window,GWLP_USERDATA);if(!f)return FALSE;
    *target=(style_menu_bar_target){window,&f->instance,&f->owner,f,&columns};return TRUE;
}
static const style_menu_bar_ops columns={read,hit,paint,lookup,STYLE_MENU_TRACK_COLUMNS};
static void dispatch(void)
{MSG m;while(PeekMessageW(&m,NULL,0,0,PM_REMOVE)){TranslateMessage(&m);DispatchMessageW(&m);}}
int main(void)
{
    WNDCLASSW cls={.hInstance=GetModuleHandleW(NULL),.lpszClassName=L"PublicBarController",.lpfnWndProc=procedure};CHECK(RegisterClassW(&cls));
    for(unsigned mode=0;mode<2;++mode){
        fixture f={0};style_instance_init(&f.instance,&f,dispose);
        HMENU menu=CreateMenu();CHECK(menu && AppendMenuW(menu,MF_STRING,1,L"&Run") && AppendMenuW(menu,MF_STRING|MF_GRAYED,2,L"&Disabled") && AppendMenuW(menu,MF_STRING,3,L"&Other"));
        f.window=CreateWindowW(cls.lpszClassName,L"",WS_OVERLAPPEDWINDOW,20,20,360,200,NULL,menu,cls.hInstance,NULL);CHECK(f.window);
        SetWindowLongPtrW(f.window,GWLP_USERDATA,(LONG_PTR)&f);
        const style_menu_bar_ops ops={.read=read,.hit=hit,.paint=paint,.navigation=mode?STYLE_MENU_TRACK_NATIVE_POPUPS:STYLE_MENU_TRACK_COLUMNS};
        const style_menu_bar_target target={f.window,&f.instance,&f.owner,&f,&ops};
        CHECK(style_menu_bar_input(&target,WM_SYSCOMMAND,SC_KEYMENU,0) && f.owner.bar.keyboard && f.owner.bar.selected==0);
        CHECK(style_menu_bar_input(&target,WM_KEYDOWN,VK_RIGHT,0) && f.owner.bar.selected==(mode?2:1));
        CHECK(style_menu_bar_input(&target,WM_KEYDOWN,VK_ESCAPE,0) && !f.owner.bar.keyboard);
        CHECK(style_menu_bar_input(&target,WM_SYSCOMMAND,SC_KEYMENU,L'r'));dispatch();CHECK(f.commands==1 && f.instance.calls==0);
        CHECK(!style_menu_tracking_current());
        CHECK(style_menu_bar_choose(&target,0));
        CHECK(!style_menu_bar_input(&target,WM_KILLFOCUS,0,0) && !f.owner.bar.keyboard);
        f.detach_init=TRUE;style_menu_bar_open(target,0);
        CHECK(f.instance.detached && !f.instance.calls && f.disposed==1 && !style_menu_tracking_current());
        CHECK(DestroyWindow(f.window));
    }
    fixture source={0},destination={.detach_read=TRUE};
    style_instance_init(&source.instance,&source,dispose);style_instance_init(&destination.instance,&destination,dispose);
    HMENU root=CreateMenu(),popup=CreatePopupMenu(),other=CreateMenu();CHECK(root && popup && other);
    CHECK(AppendMenuW(popup,MF_STRING,1,L"Item") && AppendMenuW(root,MF_POPUP,(UINT_PTR)popup,L"&Source") && AppendMenuW(other,MF_STRING,2,L"Other"));
    source.window=CreateWindowW(cls.lpszClassName,L"Source",WS_OVERLAPPEDWINDOW,20,20,360,200,NULL,root,cls.hInstance,NULL);
    destination.window=CreateWindowW(cls.lpszClassName,L"Destination",WS_OVERLAPPEDWINDOW,20,20,360,200,NULL,other,cls.hInstance,NULL);
    CHECK(source.window && destination.window);source.next=&destination;
    SetWindowLongPtrW(source.window,GWLP_USERDATA,(LONG_PTR)&source);SetWindowLongPtrW(destination.window,GWLP_USERDATA,(LONG_PTR)&destination);
    ShowWindow(source.window,SW_SHOWNORMAL);
    style_menu_bar_target target={source.window,&source.instance,&source.owner,&source,&columns};
    CHECK(style_menu_bar_choose(&target,0) && SetTimer(source.window,99,1500,NULL));
    style_menu_bar_open(target,0);KillTimer(source.window,99);
    CHECK(!source.timed_out && source.next_calls==1 && destination.pinned_read && destination.disposed==1);
    CHECK(!source.instance.calls && !destination.instance.calls && !style_menu_tracking_current());
    style_menu_bar_finish(&target,TRUE);CHECK(DestroyWindow(source.window) && DestroyWindow(destination.window));dispatch();
    CHECK(UnregisterClassW(cls.lpszClassName,cls.hInstance));
    puts("BAR_CONTROLLER two navigation policies, mnemonic/commands, focus/cancel, reentrant initialization and pinned NEXTMENU teardown PASS");return 0;
}
