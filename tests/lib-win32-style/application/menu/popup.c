#include "lib-win32-style/application/menu/popup.h"
#include <stdio.h>

#define CHECK(x) do{if(!(x)){fprintf(stderr,"POPUP line=%d error=%lu\n",__LINE__,GetLastError());return 1;}}while(0)
typedef struct presentation {
    style_menu_popup_scope *scope;
    HMENU menu;
    unsigned accepted,paints,disposed;
    BOOL end_in_accept,end_in_paint,inside,disposed_inside;
} presentation;
static BOOL accept_popup(void *context,HWND window)
{
    presentation *s=context;MENUBARINFO info={.cbSize=sizeof(info)};
    ++s->accepted;
    if(s->end_in_accept){s->inside=TRUE;style_menu_popup_end(s->scope);s->inside=FALSE;return TRUE;}
    return GetMenuBarInfo(window,OBJID_CLIENT,0,&info) && info.hMenu==s->menu;
}
static BOOL paint(style_menu_popup *popup,HDC dc,BOOL window_coordinates)
{
    presentation *s=style_menu_popup_context(popup);(void)dc;(void)window_coordinates;
    ++s->paints;
    if(s->end_in_paint){
        s->inside=TRUE;style_menu_popup_end(s->scope);
        s->disposed_inside=s->disposed!=0 || style_menu_popup_live(popup);
        s->inside=FALSE;
    }
    return TRUE;
}
static BOOL selection(style_menu_popup *popup,ULONGLONG *value)
{(void)popup;*value=1;return TRUE;}
static void dispose(void *context)
{presentation *s=context;s->disposed_inside|=s->inside;++s->disposed;}
static const style_menu_popup_ops first={accept_popup,NULL,paint,selection,dispose,FALSE,TRUE};
static const style_menu_popup_ops second={accept_popup,NULL,paint,selection,dispose,FALSE,FALSE};
static BOOL timed_out,observed;
static presentation *running;
static void CALLBACK cancel(HWND owner,UINT message,UINT_PTR id,DWORD time)
{
    (void)owner;(void)message;(void)id;(void)time;timed_out=TRUE;EndMenu();
}
static LRESULT CALLBACK owner_proc(HWND owner,UINT message,WPARAM a,LPARAM b)
{
    if(message==WM_ENTERIDLE && a==MSGF_MENU){
        observed=TRUE;
        if(running && !running->disposed)style_menu_popup_idle(running->scope);
        EndMenu();return 0;
    }
    return DefWindowProcW(owner,message,a,b);
}
typedef struct foreign { HWND window;style_menu_popup_scope *scope;BOOL rejected,end_rejected; } foreign;
static DWORD WINAPI other_thread(void *context)
{
    foreign *f=context;
    f->rejected=style_menu_popup_begin(f->window,&first,NULL)==NULL && GetLastError()==ERROR_INVALID_THREAD_ID;
    f->end_rejected=!style_menu_popup_end(f->scope) && GetLastError()==ERROR_INVALID_PARAMETER;
    return 0;
}
int main(void)
{
    WNDCLASSW cls={.lpfnWndProc=owner_proc,.hInstance=GetModuleHandleW(NULL),.lpszClassName=L"PublicPopupOwner"};
    CHECK(RegisterClassW(&cls));
    HWND owner=CreateWindowW(cls.lpszClassName,L"Public menu lifecycle",WS_OVERLAPPEDWINDOW,20,20,320,200,NULL,NULL,cls.hInstance,NULL);
    CHECK(owner);ShowWindow(owner,SW_SHOWNOACTIVATE);
    CHECK(!style_menu_popup_begin(owner,NULL,NULL) && GetLastError()==ERROR_INVALID_PARAMETER);
    presentation a={0},b={0};
    a.scope=style_menu_popup_begin(owner,&first,&a);CHECK(a.scope);
    b.scope=style_menu_popup_begin(owner,&second,&b);CHECK(b.scope);
    CHECK(style_menu_popup_scope_context(a.scope)==&a && style_menu_popup_scope_context(b.scope)==&b);
    foreign f={owner,b.scope,FALSE,FALSE};HANDLE thread=CreateThread(NULL,0,other_thread,&f,0,NULL);CHECK(thread);
    CHECK(WaitForSingleObject(thread,2000)==WAIT_OBJECT_0 && CloseHandle(thread) && f.rejected && f.end_rejected);
    CHECK(style_menu_popup_end(a.scope) && a.disposed==1 && !b.disposed);
    CHECK(style_menu_popup_scope_context(b.scope)==&b && style_menu_popup_end(b.scope) && b.disposed==1);
    for(unsigned mode=0;mode<3;++mode){
        presentation s={.end_in_accept=mode==1,.end_in_paint=mode==2};
        s.menu=CreatePopupMenu();CHECK(s.menu && AppendMenuW(s.menu,MF_STRING,1,L"Owned item"));
        s.scope=style_menu_popup_begin(owner,&first,&s);CHECK(s.scope);running=&s;
        timed_out=observed=FALSE;CHECK(SetTimer(owner,99,1500,cancel));
        TrackPopupMenuEx(s.menu,TPM_RETURNCMD|TPM_NOANIMATION,40,80,owner,NULL);
        KillTimer(owner,99);running=NULL;
        CHECK(!timed_out && observed && s.accepted && !s.disposed_inside);
        if(!mode){CHECK(s.paints && !s.disposed);CHECK(style_menu_popup_end(s.scope));}
        else CHECK(s.disposed==1 && (mode==1 || s.paints));
        CHECK(s.disposed==1 && !style_menu_popup_scope_context(s.scope));
        CHECK(DestroyMenu(s.menu));
    }
    CHECK(DestroyWindow(owner) && UnregisterClassW(cls.lpszClassName,cls.hInstance));
    puts("PUBLIC_POPUP_OK nested=1 non_top=1 foreign_thread=1 native=1 accept_teardown=1 paint_teardown=1");return 0;
}
