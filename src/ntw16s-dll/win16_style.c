/* Project-authored experimental non-client presentation, not imported USER.
 * See T445 S1 provenance/design: original NTUSER server is a stopping boundary.
 */
#include "win16_style.h"
#include "layout.h"
#include "../lib/win16-control/frame_paint.h"
#include "../lib/win16-control/frame_view.h"
#include "../lib/win16-control/frame_state.h"
#include "../lib/win16-control/menu_iteration.h"
#include "../lib/win16-control/menu_message.h"
#include "../lib/win16-control/frame_message.h"
#include "../lib/win16-control/frame_input.h"
#include "../lib/win16-control/frame_layout.h"
#include "../lib/win16-control/present.h"
#include "menu_bar.h"
#include "menu_items.h"
#include "menu_scope.h"
#include "controls.h"
#include "font.h"
#include "injection/unwind.h"
#include <commctrl.h>
#include <dwmapi.h>
#include <windowsx.h>
#include <string.h>

static LRESULT CALLBACK style_proc(HWND,UINT,WPARAM,LPARAM,UINT_PTR,DWORD_PTR);
static LRESULT CALLBACK style_proc_body(HWND,UINT,WPARAM,LPARAM,UINT_PTR,DWORD_PTR);

/* -3 means outside the bar/system cell; -2 denotes the system cell. */
static int bar_at_screen(HWND,style_window *,POINT);
/* Bound only while this thread runs a dialog's native system-menu loop.
 * A stack binding preserves reentrant menus without a global mouse hook. */
static _Thread_local menu_frame *tracking_menu;
static int bar_input_hit(void *context,POINT point)
{menu_frame *frame=context;return bar_at_screen(frame->window,frame->style,point);}
static LRESULT CALLBACK menu_filter(int code,WPARAM wp,LPARAM lp)
{
    menu_frame *frame=tracking_menu;
    if(code==HC_ACTION && (wp&PM_REMOVE) && frame){
        const win16_control_menu_input_view view={&frame->window,&frame->root,&frame->active,&frame->button,
            frame,frame->style?bar_input_hit:NULL,&frame->selected,&frame->target,&frame->direction,
            &frame->escape,&frame->double_click,&frame->point};
        win16_control_menu_input((MSG *)lp,&view);
    }
    return CallNextHookEx(NULL,code,wp,lp);
}
#define STYLE_ID ((UINT_PTR)style_proc)
/* UxTheme's non-client draw requests are not in the public SDK. They are
 * observed compatibility messages, not structures we interpret or APIs we
 * invoke. An attached custom frame must not delegate their drawing to USER.
 * See resources.md for the boundary and provenance. */
#define STYLE_NCUAHDRAWCAPTION 0x00ae
#define STYLE_NCUAHDRAWFRAME   0x00af
/* Host UAH menu drawing bypasses WM_NCPAINT and precedes NC mouse dispatch.
 * No private payload is interpreted: our menu-bar painter owns these pixels. */
#define STYLE_UAHDRAWMENU     0x0091
#define STYLE_UAHDRAWMENUITEM 0x0092

static style_window *state(HWND w)
{
    DWORD_PTR data=0;
    if(GetWindowSubclass(w,style_proc,STYLE_ID,&data))return (style_window *)data;
    return NULL;
}

static BOOL local_window(HWND w)
{
    if(GetWindowThreadProcessId(w,NULL)!=GetCurrentThreadId()){
        SetLastError(ERROR_INVALID_THREAD_ID);return FALSE;
    }
    return TRUE;
}

static BOOL measure_window(HWND w,style_window *s,const RECT *outer,win16_style_layout *l)
{return win16_control_measure_window(w,&s->dpi,&s->menu_font,&s->detached,outer,l);}

static BOOL layout(HWND w,style_window *s,win16_style_layout *l)
{
    RECT r;
    return GetWindowRect(w,&r) && measure_window(w,s,&r,l);
}

BOOL win16_style_hit_test(HWND w,LPARAM lp,LRESULT *result)
{
    style_window *s;win16_style_layout l;
    if(!result || !local_window(w))return FALSE;
    s=state(w);
    if(!s || !layout(w,s,&l))return FALSE;
    return win16_control_frame_hit_test(w,&l,lp,result);
}

static void colors(style_window *s,win16_style_palette *p)
{win16_control_resolve_palette(&s->palette,p);}

static BOOL paint_layout(HWND w,void *context,win16_style_layout *result)
{return layout(w,(style_window *)context,result);}
static void paint_colors(void *context,win16_style_palette *palette)
{colors((style_window *)context,palette);}
static win16_control_frame_view paint_view(style_window *s)
{
    const win16_control_frame_view view={s,&s->detached,&s->active,&s->menu_font,&s->title_font,
        &s->dpi,&s->bar_selected,&s->system_menu,&s->bar_keyboard,&s->pressed,&s->hot,
        paint_layout,paint_colors,win16_style_scrollbars_paint};
    return view;
}
static void paint(HWND w,style_window *s,HDC target)
{const win16_control_frame_view view=paint_view(s);win16_control_paint_frame(w,&view,target);}

static void paint_nonclient_buffered(HWND w,style_window *s,HDC dc)
{const win16_control_frame_view view=paint_view(s);win16_control_frame_buffered(w,&view,dc);}
static void paint_callback(HWND w,void *context,HDC dc){paint(w,(style_window *)context,dc);}
static void paint_buffered_callback(HWND w,void *context,HDC dc){paint_nonclient_buffered(w,(style_window *)context,dc);}
static void paint_menu_state(HWND w,style_window *s)
{const win16_control_frame_view view=paint_view(s);win16_control_frame_menu_state(w,&view);}
static void paint_button_state(HWND w,style_window *s,int hit)
{const win16_control_frame_view view=paint_view(s);win16_control_frame_button_state(w,&view,hit);}
static void button_paint_callback(HWND w,void *context,int hit){paint_button_state(w,(style_window *)context,hit);}
static win16_control_frame_button_view button_view(style_window *s)
{
    const win16_control_frame_button_view view={s,&s->pressed,&s->hot,&s->detached,paint_layout,button_paint_callback};
    return view;
}

static void bar_paint_callback(HWND w,void *context){paint_menu_state(w,(style_window *)context);}
static win16_control_bar_state_view bar_view(style_window *s)
{
    const win16_control_bar_state_view view={s,&s->detached,&s->bar_keyboard,&s->bar_focus,
        &s->bar_selected,&s->menu_font,&s->dpi,paint_layout,bar_paint_callback};
    return view;
}
static void leave_bar(HWND w,style_window *s,BOOL restore_focus)
{const win16_control_bar_state_view view=bar_view(s);win16_control_menu_bar_leave(w,&view,restore_focus);}
static void select_bar(HWND w,style_window *s,int direction)
{const win16_control_bar_state_view view=bar_view(s);win16_control_menu_bar_select(w,&view,direction);}

static int bar_at_screen(HWND w,style_window *s,POINT point)
{
    win16_style_layout l;
    if(s->detached || !layout(w,s,&l))return -3;
    return win16_control_menu_bar_hit_screen(w,&l,s->menu_font,s->dpi,point);
}

static menu_frame *menu_current(void)
{return tracking_menu;}
static void menu_begin(menu_frame *frame)
{tracking_menu=frame;frame->hook=SetWindowsHookExW(WH_GETMESSAGE,menu_filter,NULL,GetCurrentThreadId());}
static void menu_end(menu_frame *frame)
{if(frame->hook)UnhookWindowsHookEx(frame->hook);tracking_menu=frame->previous;}

static void open_bar(HWND w,style_window *s,int index)
{
    const win16_control_menu_bar_services services={menu_current,menu_begin,menu_end,layout,state,leave_bar,paint_menu_state,select_bar};
    win16_control_open_menu_bar(w,s,index,&services);
}

static BOOL refresh_font(HWND w,style_window *s)
{return win16_control_refresh_frame_fonts(w,&s->dpi,&s->title_font,&s->menu_font);}

static void activate_frame(HWND w,style_window *s,const RECT *client,BOOL normal)
{win16_control_frame_activate(w,&s->detached,client,normal,DwmSetWindowAttribute);}

static BOOL install_frame(HWND w,const win16_style_palette *p,BOOL creating,const void *owner,const BOOL *cancelled)
{
    style_window *s;
    DWORD style,ex;
    BOOL normal;
    RECT client;
    if(!local_window(w))return FALSE;
    if(state(w)){SetLastError(ERROR_ALREADY_EXISTS);return FALSE;}
    style=(DWORD)GetWindowLongPtr(w,GWL_STYLE);ex=(DWORD)GetWindowLongPtr(w,GWL_EXSTYLE);
    if(ex&(WS_EX_CLIENTEDGE|WS_EX_TOOLWINDOW|WS_EX_LAYERED)){
        SetLastError(ERROR_NOT_SUPPORTED);return FALSE;
    }
    normal=!IsIconic(w)&&!IsZoomed(w);
    s=win16_control_frame_state_create();
    if(!s){SetLastError(ERROR_NOT_ENOUGH_MEMORY);return FALSE;}
    s->bar_selected=-1;s->creation_owner=owner;s->calls=1;
    s->pending_frame=creating && normal && !(style&WS_VISIBLE);
    {WCHAR name[32];s->dialog=GetClassNameW(w,name,ARRAYSIZE(name)) && !lstrcmpW(name,L"#32770");}
    if(p)s->palette=*p;else win16_style_default_palette(&s->palette);
    s->active=(ex&WS_EX_MDICHILD)?
        (HWND)SendMessageW(GetParent(w),WM_MDIGETACTIVE,0,0)==w:GetActiveWindow()==w;
    if(!refresh_font(w,s)){win16_control_frame_state_dispose(s);SetLastError(ERROR_INVALID_DATA);return FALSE;}
    {win16_style_layout checked;
        if(!layout(w,s,&checked)){win16_control_frame_state_dispose(s);SetLastError(ERROR_NOT_SUPPORTED);return FALSE;}}
    win16_control_client_screen(w,&client);
    s->policy_saved=!(style&WS_CHILD) && SUCCEEDED(DwmGetWindowAttribute(w,DWMWA_NCRENDERING_POLICY,
        &s->old_policy,sizeof(s->old_policy)));
    if(cancelled && *cancelled){win16_control_frame_state_dispose(s);SetLastError(ERROR_CANCELLED);return FALSE;}
    if(!SetWindowSubclass(w,style_proc,STYLE_ID,(DWORD_PTR)s)){win16_control_frame_state_dispose(s);return FALSE;}
    /* Bars may be added by range/visibility changes after attachment. Keep
     * one frame tracker ready instead of conditionally installing it later. */
    if(!win16_style_frame_scrollbars_install_owned(w,&s->palette,s,&s->detached)){
        DWORD error=GetLastError();
        if(state(w)==s)win16_style_remove(w);
        if(!--s->calls && s->detached)win16_control_frame_state_dispose(s);
        SetLastError(error);return FALSE;
    }
    /* Initial minimized/maximized creation is valid too. Native USER owns
     * its saved normal placement; never resize that state to preserve the
     * transient icon/maximized client rectangle. Restore uses normal layout. */
    if(!s->pending_frame)activate_frame(w,s,&client,normal);
    --s->calls;
    if(s->detached){win16_control_frame_state_dispose(s);SetLastError(cancelled && *cancelled?ERROR_CANCELLED:ERROR_INVALID_WINDOW_HANDLE);return FALSE;}
    return TRUE;
}

/* A transient menu owner never takes over its hidden window's frame. */
BOOL win16_style_menu_owner_install_created(HWND w,const void *owner,const BOOL *cancelled)
{
 if(!local_window(w)||state(w)){SetLastError(ERROR_INVALID_STATE);return FALSE;}
 style_window *s=win16_control_frame_state_create();if(!s){SetLastError(ERROR_NOT_ENOUGH_MEMORY);return FALSE;}
 s->creation_owner=owner;s->menu_only=TRUE;s->dpi=GetDpiForWindow(w);if(!s->dpi)s->dpi=96;
 win16_style_default_palette(&s->palette);s->menu_font=win16_style_menu_font_acquire(s->dpi);
 if(!s->menu_font || (cancelled&&*cancelled)){win16_control_frame_state_dispose(s);SetLastError(ERROR_CANCELLED);return FALSE;}
 if(!SetWindowSubclass(w,style_proc,STYLE_ID,(DWORD_PTR)s)){DWORD error=GetLastError();win16_control_frame_state_dispose(s);SetLastError(error);return FALSE;}
 return TRUE;
}

BOOL win16_style_install(HWND w,const win16_style_palette *p)
{return install_frame(w,p,FALSE,NULL,NULL);}

BOOL win16_style_install_created(HWND w,const win16_style_palette *p,const void *owner,const BOOL *cancelled)
{return install_frame(w,p,TRUE,owner,cancelled);}

BOOL win16_style_frame_remove_owned(HWND w,const void *owner)
{
    style_window *s=state(w);
    if(!owner || !s || s->creation_owner!=owner)return FALSE;
    return win16_style_remove(w);
}

BOOL win16_style_remove(HWND w)
{
    style_window *s;RECT client;BOOL normal;
    if(!local_window(w))return FALSE;
    s=state(w);if(!s){SetLastError(ERROR_NOT_FOUND);return FALSE;}
    if(s->menu_only){if(!RemoveWindowSubclass(w,style_proc,STYLE_ID))return FALSE;++s->calls;s->detached=TRUE;while(s->menus)win16_control_frame_state_end_menu(s);win16_style_menu_restore(&s->menu_items);if(!--s->calls)win16_control_frame_state_dispose(s);return TRUE;}
    normal=!IsIconic(w)&&!IsZoomed(w);win16_control_client_screen(w,&client);
    if(!RemoveWindowSubclass(w,style_proc,STYLE_ID))return FALSE;
    ++s->calls;
    s->detached=TRUE;
    win16_style_control_remove_owned(w,s);
    while(s->menus)win16_control_frame_state_end_menu(s);
    win16_style_menu_restore(&s->menu_items);
    if(s->bar_keyboard)leave_bar(w,s,TRUE);
    if(s->pressed && GetCapture()==w)ReleaseCapture();
    win16_control_frame_restore(w,&s->pending_frame,&s->policy_saved,&s->old_policy,&client,normal,DwmSetWindowAttribute);
    if(!--s->calls)win16_control_frame_state_dispose(s);
    return TRUE;
}

BOOL win16_style_set_palette(HWND w,const win16_style_palette *p)
{
    style_window *s;
    if(!p){SetLastError(ERROR_INVALID_PARAMETER);return FALSE;}
    if(!local_window(w))return FALSE;
    s=state(w);if(!s){SetLastError(ERROR_NOT_FOUND);return FALSE;}
    ++s->calls;s->palette=*p;win16_style_frame_scrollbars_palette_owned(w,p,s);paint(w,s,NULL);
    if(!--s->calls && s->detached)win16_control_frame_state_dispose(s);
    return TRUE;
}
BOOL win16_style_frame_terminal(HWND w)
{
    style_window *s;
    s=state(w);if(!s)return TRUE;
    if(!RemoveWindowSubclass(w,style_proc,STYLE_ID)){
        SetLastError(ERROR_INVALID_STATE);return FALSE;
    }
    ++s->calls;s->detached=TRUE;
    while(s->menus)win16_control_frame_state_end_menu(s);
    if(!--s->calls && s->detached)win16_control_frame_state_dispose(s);
    return TRUE;
}

typedef struct menu_dispatch_scope { style_window *state;BOOL returned; } menu_dispatch_scope;
static void menu_dispatch_end(void *data)
{
    menu_dispatch_scope *scope=data;style_window *s=scope->state;
    if(!scope->returned && !--s->calls && s->detached)win16_control_frame_state_dispose(s);
}
static NTW16S_UNWIND_FUNCTION LRESULT CALLBACK style_proc(HWND w,UINT msg,WPARAM wp,LPARAM lp,
    UINT_PTR id,DWORD_PTR data)
{
    style_window *s=(style_window *)data;ntw16s_unwind_scope unwind;
    if(!s->menu_only)return style_proc_body(w,msg,wp,lp,id,data);
    menu_dispatch_scope scope={s,FALSE};LRESULT result;
    NTW16S_UNWIND_BEGIN(unwind,menu_dispatch_end,&scope);
    result=style_proc_body(w,msg,wp,lp,id,data);scope.returned=TRUE;
    NTW16S_UNWIND_END(unwind);return result;
}
static LRESULT CALLBACK style_proc_body(HWND w,UINT msg,WPARAM wp,LPARAM lp,
    UINT_PTR id,DWORD_PTR data)
{
    style_window *s=(style_window *)data;
    LRESULT result=0;win16_style_layout l;POINT point;RECT r;
    (void)id;++s->calls;
    if(s->menu_only && msg!=WM_ENTERMENULOOP && msg!=WM_EXITMENULOOP && msg!=WM_INITMENUPOPUP && msg!=WM_MENUCHAR && msg!=WM_MEASUREITEM && msg!=WM_DRAWITEM && msg!=WM_NCDESTROY)goto forward;
    /* Maximized MDI children merge their controls into the frame menu.
     * Preserve DefMDIChildProc's native geometry/dispatch for that state;
     * merged-control retro painting is a separate S4 menu surface. */
    if((GetWindowLongPtr(w,GWL_EXSTYLE)&WS_EX_MDICHILD) && IsZoomed(w) &&
       msg!=WM_NCDESTROY)goto forward;
    /* Our menu bar is custom nonclient chrome. USER hot-tracks HTMENU before
     * NC mouse dispatch (classic/WOW has no UAH draw callback to intercept).
     * Expose neutral nonclient hit-test ownership and route clicks by our
     * measured menu rectangle; never alter the application's HMENU. */
    if((msg==WM_NCLBUTTONDOWN || msg==WM_NCLBUTTONDBLCLK) && wp==HTBORDER && layout(w,s,&l)){
        GetWindowRect(w,&r);point.x=GET_X_LPARAM(lp)-r.left;point.y=GET_Y_LPARAM(lp)-r.top;
        if(PtInRect(&l.menu,point))wp=HTMENU;
    }
    switch(msg){
    case WM_SHOWWINDOW:
    case WM_CTLCOLORDLG:
    case WM_CTLCOLORSTATIC:
    case WM_NCCALCSIZE:
    case WM_NCPAINT:
    case WM_SETCURSOR:
    case STYLE_NCUAHDRAWCAPTION:
    case STYLE_NCUAHDRAWFRAME:
    case STYLE_UAHDRAWMENU:
    case STYLE_UAHDRAWMENUITEM:
    case WM_NCACTIVATE:
    case WM_NCHITTEST:
    case WM_PRINT:
    case WM_SETTEXT:
    case WM_HSCROLL:
    case WM_VSCROLL:
    case WM_DPICHANGED:
    case WM_SETTINGCHANGE:
    case WM_SYSCOLORCHANGE:
    case WM_THEMECHANGED: {
        const win16_control_frame_message_services services={measure_window,activate_frame,paint,paint_callback,paint_buffered_callback,win16_style_hit_test,refresh_font};
        if(win16_control_frame_message(w,s,msg,wp,lp,&services,&result))break;
        goto forward;
    }
    case WM_MEASUREITEM:
    case WM_DRAWITEM:
    case WM_MENUCHAR:
    case WM_INITMENUPOPUP:
    case WM_EXITMENULOOP:
        if(win16_control_menu_message(w,s,msg,wp,lp,paint_button_state,&result))break;
        goto forward;
    case WM_KEYDOWN:
    case WM_KILLFOCUS:
    case WM_NCMOUSEMOVE:
    case WM_NCMOUSEHOVER:
    case WM_NCMOUSELEAVE:
    case WM_NCLBUTTONDBLCLK:
    case WM_NCLBUTTONDOWN:
    case WM_LBUTTONDOWN:
    case WM_MOUSEMOVE:
    case WM_LBUTTONUP:
    case WM_CANCELMODE:
    case WM_CAPTURECHANGED: {
        const win16_control_frame_input_services services={menu_current,menu_begin,menu_end,layout,bar_at_screen,paint_menu_state,select_bar,leave_bar,open_bar,button_view};
        if(win16_control_frame_input(w,s,msg,wp,lp,&services,&result))break;
        goto forward;
    }
    case WM_SYSCOMMAND:
        {const win16_control_frame_input_services services={menu_current,menu_begin,menu_end,layout,bar_at_screen,paint_menu_state,select_bar,leave_bar,open_bar,button_view};
         if(win16_control_frame_input(w,s,msg,wp,lp,&services,&result))break;}
        goto forward;
    case WM_MENUSELECT:
        if(tracking_menu && tracking_menu->window==w && lp)
            tracking_menu->active=(HMENU)lp;
        goto forward;
    case WM_ENTERMENULOOP:
        win16_control_frame_state_enter_menu(s);
        goto forward;
    case WM_NCDESTROY:
        (void)win16_style_frame_terminal(w);
        result=DefSubclassProc(w,msg,wp,lp);break;
    default:
forward:
        result=DefSubclassProc(w,msg,wp,lp);break;
    }
    if(!--s->calls && s->detached)win16_control_frame_state_dispose(s);
    return result;
}
