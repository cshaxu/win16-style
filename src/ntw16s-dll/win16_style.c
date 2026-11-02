/* Project-authored experimental non-client presentation, not imported USER.
 * See T445 S1 provenance/design: original NTUSER server is a stopping boundary.
 */
#include "win16_style.h"
#include "layout.h"
#include "../lib/win16-control/frame_paint.h"
#include "../lib/win16-control/frame_view.h"
#include "../lib/win16-control/frame_layout.h"
#include "menu_bar.h"
#include "menu_items.h"
#include "menu_scope.h"
#include "controls.h"
#include "font.h"
#include "injection/unwind.h"
#include <commctrl.h>
#include <dwmapi.h>
#include <windowsx.h>
#include <stdlib.h>
#include <string.h>

void win16_style_menu_system(win16_style_menu_scope *scope,HMENU menu,HFONT font,UINT dpi);

typedef struct menu_loop {
    win16_style_menu_scope *scope;
    unsigned depth;
    struct menu_loop *previous;
} menu_loop;
typedef struct style_window {
    win16_style_palette palette;
    HFONT title_font,menu_font; /* Title unchanged; menu owns a System variant. */
    UINT dpi, calls;
    BOOL active, detached, hot, system_menu, dialog, menu_only;
    int pressed,bar_selected,bar_direction,bar_hover;
    BOOL bar_keyboard;
    HWND bar_focus;
    win16_style_menu_item *menu_items;
    menu_loop *menus;
    unsigned menu_depth;
    DWORD old_policy;
    BOOL policy_saved, pending_frame;
    const void *creation_owner;
} style_window;

static LRESULT CALLBACK style_proc(HWND,UINT,WPARAM,LPARAM,UINT_PTR,DWORD_PTR);
static LRESULT CALLBACK style_proc_body(HWND,UINT,WPARAM,LPARAM,UINT_PTR,DWORD_PTR);
static void dispose(style_window *);

typedef struct print_children {
    HWND parent;
    HDC dc;
    POINT origin;
    LPARAM flags;
    BOOL owned;
} print_children;

static BOOL CALLBACK print_child(HWND child,LPARAM data)
{
    print_children *p=(print_children *)data;RECT r;int saved;
    if(!IsWindow(p->parent))return FALSE;
    if((p->owned?GetWindow(child,GW_OWNER):GetParent(child))!=p->parent || !IsWindowVisible(child))return TRUE;
    if(!GetWindowRect(child,&r) || !(saved=SaveDC(p->dc)))return TRUE;
    OffsetViewportOrgEx(p->dc,r.left-p->origin.x,r.top-p->origin.y,NULL);
    IntersectClipRect(p->dc,0,0,r.right-r.left,r.bottom-r.top);
    SendMessageW(child,WM_PRINT,(WPARAM)p->dc,p->flags|PRF_NONCLIENT|PRF_CLIENT|PRF_ERASEBKGND);
    RestoreDC(p->dc,saved);return TRUE;
}

/* Custom nonclient geometry cannot delegate native frame printing. Preserve
 * DWP's frame/client/child ordering using the public HWND/DC boundaries.
 * See the S4 print audit for the original xxxDWPPrint reference. */
static void print_client_and_children(HWND w,style_window *s,HDC dc,LPARAM flags)
{
    RECT wr,client;POINT origin={0,0};int saved;print_children children;
    if(!GetWindowRect(w,&wr) || !GetClientRect(w,&client) || !ClientToScreen(w,&origin))return;
    children.parent=w;children.dc=dc;children.origin=origin;children.flags=flags;children.owned=FALSE;
    if(flags&PRF_NONCLIENT){children.origin.x=wr.left;children.origin.y=wr.top;}
    if(flags&PRF_CLIENT){
        saved=SaveDC(dc);if(!saved)return;
        OffsetViewportOrgEx(dc,origin.x-children.origin.x,origin.y-children.origin.y,NULL);
        IntersectClipRect(dc,0,0,client.right,client.bottom);
        if(flags&PRF_ERASEBKGND)SendMessageW(w,WM_ERASEBKGND,(WPARAM)dc,0);
        if(!s->detached)SendMessageW(w,WM_PRINTCLIENT,(WPARAM)dc,flags);
        RestoreDC(dc,saved);
    }
    if(!s->detached && (flags&PRF_CHILDREN))EnumChildWindows(w,print_child,(LPARAM)&children);
    if(!s->detached && (flags&PRF_OWNED)){
        children.owned=TRUE;EnumThreadWindows(GetCurrentThreadId(),print_child,(LPARAM)&children);
    }
}
typedef struct menu_frame {
    HWND window;
    RECT button;
    BOOL double_click,escape;
    LPARAM point;
    HMENU root,active;
    int direction;
    style_window *style;
    int selected,target;
} menu_frame;
/* -3 means outside the bar/system cell; -2 denotes the system cell. */
static int bar_at_screen(HWND,style_window *,POINT);
/* Bound only while this thread runs a dialog's native system-menu loop.
 * A stack binding preserves reentrant menus without a global mouse hook. */
static _Thread_local menu_frame *tracking_menu;
static BOOL bar_arrow(menu_frame *frame,UINT key)
{
    int i,count,selected=-1;MENUITEMINFOW info={.cbSize=sizeof(info)};RECT chosen,other;
    HMENU menu=frame->active;
    if(!frame->root || !menu || (key!=VK_LEFT && key!=VK_RIGHT))return FALSE;
    if(key==VK_LEFT && menu!=frame->root)return FALSE; /* native closes hierarchy */
    count=GetMenuItemCount(menu);
    for(i=0;i<count;++i){info.fMask=MIIM_STATE|MIIM_SUBMENU;
        if(GetMenuItemInfoW(menu,i,TRUE,&info) && (info.fState&MFS_HILITE)){selected=i;break;}}
    if(selected>=0){
        if(key==VK_RIGHT && info.hSubMenu && !(info.fState&MFS_DISABLED))return FALSE;
        /* Native tracking keeps ownership of navigation between columns. */
        if(!GetMenuItemRect(frame->window,menu,selected,&chosen))return FALSE;
        for(i=0;i<count;++i)if(GetMenuItemRect(frame->window,menu,i,&other) &&
            ((key==VK_LEFT && other.right<=chosen.left) ||
             (key==VK_RIGHT && other.left>=chosen.right)))return FALSE;
    }
    return TRUE;
}
static LRESULT CALLBACK menu_filter(int code,WPARAM wp,LPARAM lp)
{
    menu_frame *frame=tracking_menu;
    if(code==HC_ACTION && (wp&PM_REMOVE) && frame){
        MSG *m=(MSG *)lp;
        if((m->message==WM_KEYDOWN || m->message==WM_SYSKEYDOWN) &&
           m->wParam==VK_ESCAPE && frame->active==frame->root)frame->escape=TRUE;
        if(frame->style && (m->message==WM_LBUTTONDOWN || m->message==WM_NCLBUTTONDOWN)){
            POINT p={GET_X_LPARAM(m->lParam),GET_Y_LPARAM(m->lParam)};
            int hit=bar_at_screen(frame->window,frame->style,p);
            if(hit!=-3){
                /* A second click on the current bar item dismisses it;
                 * another item switches without leaving a queued click. */
                frame->target=hit==frame->selected?-3:hit;
                m->message=WM_NULL;m->wParam=0;m->lParam=0;EndMenu();
            }
        }
        if(frame->style && (m->message==WM_MOUSEMOVE || m->message==WM_NCMOUSEMOVE)){
            POINT p={GET_X_LPARAM(m->lParam),GET_Y_LPARAM(m->lParam)};
            int hit=bar_at_screen(frame->window,frame->style,p);
            if(hit!=-3 && hit!=frame->selected){
                frame->target=hit;m->message=WM_NULL;m->wParam=0;m->lParam=0;EndMenu();
            }
        }
        if((m->message==WM_KEYDOWN || m->message==WM_SYSKEYDOWN) && bar_arrow(frame,(UINT)m->wParam)){
            frame->direction=m->wParam==VK_LEFT?-1:1;
            m->message=WM_NULL;m->wParam=0;m->lParam=0;EndMenu();
        }
        if(m->hwnd==frame->window &&
           (m->message==WM_NCLBUTTONDBLCLK || m->message==WM_LBUTTONDBLCLK)){
            POINT p={GET_X_LPARAM(m->lParam),GET_Y_LPARAM(m->lParam)};
            /* Menu capture carries screen coordinates even for WM_LBUTTON*.
             * USER may consume this double click as native-caption input
             * before calling WH_MSGFILTER, so observe queue removal instead. */
            if(PtInRect(&frame->button,p)){
                frame->double_click=TRUE;frame->point=MAKELPARAM(p.x,p.y);
                m->message=WM_NULL;m->wParam=0;m->lParam=0;EndMenu();
            }
        }
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
    style_window *s;win16_style_layout l;RECT r;POINT point;
    SCROLLBARINFO scroll={.cbSize=sizeof(scroll)};
    POINT screen={GET_X_LPARAM(lp),GET_Y_LPARAM(lp)};
    DWORD flags;
    if(!result || !local_window(w))return FALSE;
    s=state(w);
    if(!s || !layout(w,s,&l))return FALSE;
    flags=(DWORD)GetWindowLongPtrW(w,GWL_STYLE);
    if((flags&WS_HSCROLL) && GetScrollBarInfo(w,OBJID_HSCROLL,&scroll) && PtInRect(&scroll.rcScrollBar,screen)){
        *result=HTHSCROLL;return TRUE;}
    if((flags&WS_VSCROLL) && GetScrollBarInfo(w,OBJID_VSCROLL,&scroll) && PtInRect(&scroll.rcScrollBar,screen)){
        *result=HTVSCROLL;return TRUE;}
    if(!GetWindowRect(w,&r))return FALSE;
    point.x=screen.x-r.left;point.y=screen.y-r.top;
    *result=PtInRect(&l.menu,point)?HTBORDER:win16_style_hit(&l,point);
    return TRUE;
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
static void paint_menu_state(HWND w,style_window *s)
{const win16_control_frame_view view=paint_view(s);win16_control_frame_menu_state(w,&view);}
static void paint_button_state(HWND w,style_window *s,int hit)
{const win16_control_frame_view view=paint_view(s);win16_control_frame_button_state(w,&view,hit);}

static void leave_bar(HWND w,style_window *s,BOOL restore_focus)
{
    HWND prior=s->bar_focus;
    s->bar_keyboard=FALSE;s->bar_focus=NULL;s->bar_selected=-1;
    if(restore_focus && prior && IsWindow(prior) && GetFocus()==w)SetFocus(prior);
    if(!s->detached)paint_menu_state(w,s);
}

static void select_bar(HWND w,style_window *s,int direction)
{
    int count;win16_style_layout l;win16_style_bar bar;POINT origin;
    if(!layout(w,s,&l))return;
    origin.x=l.menu.left;
    origin.y=l.menu.top;
    if(!win16_style_bar_read_for_window(w,GetMenu(w),s->menu_font,s->dpi,origin,l.menu.right-l.menu.left,&bar))return;
    count=(int)bar.count;win16_style_bar_free(&bar);
    if(count<=0)return;
    if(!s->bar_keyboard){s->bar_focus=GetFocus();s->bar_keyboard=TRUE;
        s->bar_selected=0;SetFocus(w);}
    else if(direction){
        BOOL system=(GetWindowLongPtrW(w,GWL_STYLE)&WS_SYSMENU)!=0;
        if(s->bar_selected==-2)s->bar_selected=direction>0?0:count-1;
        else {int next=s->bar_selected+direction;
            s->bar_selected=(next<0 || next>=count)?(system?-2:(next+count)%count):next;}
    }
    if(!s->detached)paint_menu_state(w,s);
}

static int bar_at_screen(HWND w,style_window *s,POINT point)
{
    win16_style_layout l;win16_style_bar bar;RECT outer;POINT origin;int hit;
    if(s->detached || !layout(w,s,&l) || !GetWindowRect(w,&outer))return -3;
    point.x-=outer.left;point.y-=outer.top;
    if(PtInRect(&l.system,point))return -2;
    if(!PtInRect(&l.menu,point))return -3;
    origin.x=l.menu.left;origin.y=l.menu.top;
    if(!win16_style_bar_read_for_window(w,GetMenu(w),s->menu_font,s->dpi,origin,l.menu.right-l.menu.left,&bar))return -3;
    hit=win16_style_menu_hit(bar.rects,bar.count,point);win16_style_bar_free(&bar);
    return hit<0?-3:hit;
}

static int open_bar_once(HWND w,style_window *s,int index,int *hover)
{
    win16_style_layout l;win16_style_bar bar;POINT origin,anchor;RECT outer;
    HMENU popup;UINT command;BOOL merged_system;
    menu_frame frame={0},*previous=tracking_menu;HHOOK hook=NULL;
    *hover=-3;frame.target=-3;
    if(index==-2){
        s->bar_direction=0;s->bar_hover=-3;
        SendMessageW(w,WM_SYSCOMMAND,SC_KEYMENU,L' ');
        *hover=s->bar_hover;
        return s->bar_direction;
    }
    SendMessageW(w,WM_INITMENU,(WPARAM)GetMenu(w),0);
    if(s->detached || !layout(w,s,&l) || IsRectEmpty(&l.menu))return 0;
    origin.x=l.menu.left;origin.y=l.menu.top;
    if(!win16_style_bar_read_for_window(w,GetMenu(w),s->menu_font,s->dpi,origin,l.menu.right-l.menu.left,&bar))return 0;
    if(index<0 || (UINT)index>=bar.count || (bar.items[index].state&MFS_DISABLED)){
        win16_style_bar_free(&bar);return 0;}
    popup=bar.items[index].popup;command=bar.items[index].id;
    merged_system=bar.items[index].control==HBMMENU_SYSTEM;
    GetWindowRect(w,&outer);anchor.x=outer.left+bar.rects[index].left;
    anchor.y=outer.top+bar.rects[index].bottom;
    if(popup){
        frame.window=w;frame.root=frame.active=popup;
        frame.style=s;frame.selected=index;
        if(merged_system){frame.button=bar.rects[index];OffsetRect(&frame.button,outer.left,outer.top);}
        tracking_menu=&frame;hook=SetWindowsHookExW(WH_GETMESSAGE,menu_filter,NULL,GetCurrentThreadId());
    }
    win16_style_bar_free(&bar);s->bar_selected=index;paint_menu_state(w,s);
    if(s->bar_keyboard){
        if(popup)command=(UINT)TrackPopupMenuEx(popup,TPM_LEFTALIGN|TPM_TOPALIGN|TPM_RETURNCMD,
            anchor.x,anchor.y,w,NULL);
        if(!s->detached){
            if(command){leave_bar(w,s,TRUE);if(!s->detached)PostMessageW(w,WM_COMMAND,command,0);}
            else if(!frame.direction && frame.target==-3 && !frame.escape)leave_bar(w,s,TRUE);
            else paint_menu_state(w,s); /* Keyboard Escape returns to the selected bar. */
        }
    }else{
        if(popup)TrackPopupMenuEx(popup,TPM_LEFTALIGN|TPM_TOPALIGN,anchor.x,anchor.y,w,NULL);
        else PostMessageW(w,WM_COMMAND,command,0);
        if(!s->detached){s->bar_selected=-1;paint_menu_state(w,s);}
    }
    if(popup){
        if(hook)UnhookWindowsHookEx(hook);
        tracking_menu=previous;
        if(frame.double_click && !s->detached)
            PostMessageW(w,WM_NCLBUTTONDBLCLK,HTMENU,frame.point);
    }
    *hover=frame.target;return frame.direction;
}

static void open_bar(HWND w,style_window *s,int index)
{
    int direction,hover;BOOL mouse_origin=!s->bar_keyboard;
    HWND original_focus=s->bar_keyboard?s->bar_focus:GetFocus();
    ++s->calls; /* The iterator may change HWND; pin its current style state. */
    for(;;){
        win16_style_layout l;win16_style_bar bar;POINT origin;BOOL popup;int count=0;
        direction=open_bar_once(w,s,index,&hover);
        if(s->detached || (!direction && hover==-3))break;
        if(GetMenu(w) && layout(w,s,&l)){
            origin.x=l.menu.left;origin.y=l.menu.top;
            if(win16_style_bar_read_for_window(w,GetMenu(w),s->menu_font,s->dpi,origin,l.menu.right-l.menu.left,&bar)){
                count=(int)bar.count;win16_style_bar_free(&bar);}}
        if(direction && (index==-2 || (index==0 && direction<0) || (index==count-1 && direction>0))){
            MDINEXTMENU next={0};style_window *target;int next_index;
            next.hmenuIn=index==-2?GetSystemMenu(w,FALSE):GetMenu(w);
            SendMessageW(w,WM_NEXTMENU,direction<0?VK_LEFT:VK_RIGHT,(LPARAM)&next);
            if(s->detached)break;
            if(next.hwndNext && next.hmenuNext && IsWindow(next.hwndNext) && IsMenu(next.hmenuNext) &&
                GetWindowThreadProcessId(next.hwndNext,NULL)==GetCurrentThreadId() &&
                (target=state(next.hwndNext))!=NULL){
                if(next.hmenuNext==GetSystemMenu(next.hwndNext,FALSE))next_index=-2;
                else if(next.hmenuNext==GetMenu(next.hwndNext)){
                    if(!layout(next.hwndNext,target,&l))break;
                    origin.x=l.menu.left;origin.y=l.menu.top;
                    if(!win16_style_bar_read_for_window(next.hwndNext,next.hmenuNext,target->menu_font,target->dpi,origin,l.menu.right-l.menu.left,&bar))break;
                    next_index=direction>0?0:(int)bar.count-1;win16_style_bar_free(&bar);
                    if(next_index<0)break;
                }else break;
                ++target->calls;
                leave_bar(w,s,FALSE);
                if(!--s->calls && s->detached)dispose(s);
                w=next.hwndNext;s=target;index=next_index;
                if(s->detached)break;
                SetFocus(w);
                if(s->detached)break;
                s->bar_focus=original_focus;s->bar_keyboard=TRUE;s->bar_selected=index;
                paint_menu_state(w,s);continue;
            }
        }
        /* Switch selection, but never execute a direct bar command merely
         * because an arrow reached it. Enter still owns command invocation. */
        if(!s->bar_keyboard)select_bar(w,s,0);
        if(hover!=-3){s->bar_selected=hover;paint_menu_state(w,s);}
        else {s->bar_selected=index;select_bar(w,s,direction);}
        index=s->bar_selected;
        if(index==-2)continue;
        if(!layout(w,s,&l))break;
        origin.x=l.menu.left;
        origin.y=l.menu.top;
        if(!win16_style_bar_read_for_window(w,GetMenu(w),s->menu_font,s->dpi,origin,l.menu.right-l.menu.left,&bar))break;
        popup=index>=0 && (UINT)index<bar.count && bar.items[index].popup!=NULL;
        win16_style_bar_free(&bar);if(!popup)break;
    }
    /* Switching a mouse-opened popup uses the keyboard iterator internally,
     * but must not leave Alt/F10 mode armed after cancellation. */
    if(mouse_origin && !s->detached)leave_bar(w,s,TRUE);
    if(!--s->calls && s->detached)dispose(s);
}

static BOOL refresh_font(HWND w,style_window *s)
{
    HFONT title,menu;
    UINT dpi=GetDpiForWindow(w);if(!dpi)dpi=96;
    /* Point heights are converted once in the window's DPI context. USER
     * virtualizes legacy/unaware windows separately; do not scale them again. */
    title=CreateFontW(-MulDiv(10,(int)dpi,72),0,0,0,FW_BOLD,FALSE,FALSE,FALSE,
        DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,NONANTIALIASED_QUALITY,
        DEFAULT_PITCH,L"MS Sans Serif");
    menu=win16_style_menu_font_acquire(dpi);
    if(!title || !menu){if(title)DeleteObject(title);win16_style_system_release(menu);return FALSE;}
    if(s->title_font)DeleteObject(s->title_font);
    win16_style_system_release(s->menu_font);
    s->dpi=dpi;s->title_font=title;s->menu_font=menu;return TRUE;
}

static void frame_changed(HWND w)
{
    SetWindowPos(w,NULL,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER|
        SWP_NOACTIVATE|SWP_FRAMECHANGED);
    RedrawWindow(w,NULL,NULL,RDW_FRAME|RDW_INVALIDATE|RDW_UPDATENOW);
}

static void client_screen(HWND w,RECT *r)
{
    POINT p={0,0};GetClientRect(w,r);ClientToScreen(w,&p);OffsetRect(r,p.x,p.y);
}

static void restore_client(HWND w,const RECT *wanted)
{
    RECT r,outer;POINT origin;client_screen(w,&r);GetWindowRect(w,&outer);
    origin.x=outer.left+wanted->left-r.left;origin.y=outer.top+wanted->top-r.top;
    if(GetWindowLongPtr(w,GWL_STYLE)&WS_CHILD)ScreenToClient(GetParent(w),&origin);
    SetWindowPos(w,NULL,origin.x,origin.y,
        outer.right-outer.left+(wanted->right-wanted->left)-(r.right-r.left),
        outer.bottom-outer.top+(wanted->bottom-wanted->top)-(r.bottom-r.top),
        SWP_NOZORDER|SWP_NOACTIVATE);
}

static void end_menu_scope(style_window *s)
{
    menu_loop *loop=s->menus;
    if(loop){s->menus=loop->previous;win16_style_menu_end(loop->scope);free(loop);}
}

static void dispose(style_window *s)
{
    while(s->menus)end_menu_scope(s);
    win16_style_menu_restore(&s->menu_items);
    if(s->title_font)DeleteObject(s->title_font);
    win16_style_system_release(s->menu_font);
    free(s);
}

static void activate_frame(HWND w,style_window *s,const RECT *client,BOOL normal)
{
    DWORD policy=DWMNCRP_DISABLED;
    if(!(GetWindowLongPtr(w,GWL_STYLE)&WS_CHILD))
        DwmSetWindowAttribute(w,DWMWA_NCRENDERING_POLICY,&policy,sizeof(policy));
    if(!s->detached)frame_changed(w);
    if(!s->detached && normal)restore_client(w,client);
}

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
    s=(style_window *)calloc(1,sizeof(*s));
    if(!s){SetLastError(ERROR_NOT_ENOUGH_MEMORY);return FALSE;}
    s->bar_selected=-1;s->creation_owner=owner;s->calls=1;
    s->pending_frame=creating && normal && !(style&WS_VISIBLE);
    {WCHAR name[32];s->dialog=GetClassNameW(w,name,ARRAYSIZE(name)) && !lstrcmpW(name,L"#32770");}
    if(p)s->palette=*p;else win16_style_default_palette(&s->palette);
    s->active=(ex&WS_EX_MDICHILD)?
        (HWND)SendMessageW(GetParent(w),WM_MDIGETACTIVE,0,0)==w:GetActiveWindow()==w;
    if(!refresh_font(w,s)){dispose(s);SetLastError(ERROR_INVALID_DATA);return FALSE;}
    {win16_style_layout checked;
        if(!layout(w,s,&checked)){dispose(s);SetLastError(ERROR_NOT_SUPPORTED);return FALSE;}}
    client_screen(w,&client);
    s->policy_saved=!(style&WS_CHILD) && SUCCEEDED(DwmGetWindowAttribute(w,DWMWA_NCRENDERING_POLICY,
        &s->old_policy,sizeof(s->old_policy)));
    if(cancelled && *cancelled){dispose(s);SetLastError(ERROR_CANCELLED);return FALSE;}
    if(!SetWindowSubclass(w,style_proc,STYLE_ID,(DWORD_PTR)s)){dispose(s);return FALSE;}
    /* Bars may be added by range/visibility changes after attachment. Keep
     * one frame tracker ready instead of conditionally installing it later. */
    if(!win16_style_frame_scrollbars_install_owned(w,&s->palette,s,&s->detached)){
        DWORD error=GetLastError();
        if(state(w)==s)win16_style_remove(w);
        if(!--s->calls && s->detached)dispose(s);
        SetLastError(error);return FALSE;
    }
    /* Initial minimized/maximized creation is valid too. Native USER owns
     * its saved normal placement; never resize that state to preserve the
     * transient icon/maximized client rectangle. Restore uses normal layout. */
    if(!s->pending_frame)activate_frame(w,s,&client,normal);
    --s->calls;
    if(s->detached){dispose(s);SetLastError(cancelled && *cancelled?ERROR_CANCELLED:ERROR_INVALID_WINDOW_HANDLE);return FALSE;}
    return TRUE;
}

/* A transient menu owner never takes over its hidden window's frame. */
BOOL win16_style_menu_owner_install_created(HWND w,const void *owner,const BOOL *cancelled)
{
 if(!local_window(w)||state(w)){SetLastError(ERROR_INVALID_STATE);return FALSE;}
 style_window *s=calloc(1,sizeof(*s));if(!s){SetLastError(ERROR_NOT_ENOUGH_MEMORY);return FALSE;}
 s->creation_owner=owner;s->menu_only=TRUE;s->dpi=GetDpiForWindow(w);if(!s->dpi)s->dpi=96;
 win16_style_default_palette(&s->palette);s->menu_font=win16_style_menu_font_acquire(s->dpi);
 if(!s->menu_font || (cancelled&&*cancelled)){dispose(s);SetLastError(ERROR_CANCELLED);return FALSE;}
 if(!SetWindowSubclass(w,style_proc,STYLE_ID,(DWORD_PTR)s)){DWORD error=GetLastError();dispose(s);SetLastError(error);return FALSE;}
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
    if(s->menu_only){if(!RemoveWindowSubclass(w,style_proc,STYLE_ID))return FALSE;++s->calls;s->detached=TRUE;while(s->menus)end_menu_scope(s);win16_style_menu_restore(&s->menu_items);if(!--s->calls)dispose(s);return TRUE;}
    normal=!IsIconic(w)&&!IsZoomed(w);client_screen(w,&client);
    if(!RemoveWindowSubclass(w,style_proc,STYLE_ID))return FALSE;
    ++s->calls;
    s->detached=TRUE;
    win16_style_control_remove_owned(w,s);
    while(s->menus)end_menu_scope(s);
    win16_style_menu_restore(&s->menu_items);
    if(s->bar_keyboard)leave_bar(w,s,TRUE);
    if(s->pressed && GetCapture()==w)ReleaseCapture();
    if(IsWindow(w) && !s->pending_frame && s->policy_saved)DwmSetWindowAttribute(w,DWMWA_NCRENDERING_POLICY,
        &s->old_policy,sizeof(s->old_policy));
    if(IsWindow(w) && !s->pending_frame){frame_changed(w);if(normal && IsWindow(w))restore_client(w,&client);}
    if(!--s->calls)dispose(s);
    return TRUE;
}

BOOL win16_style_set_palette(HWND w,const win16_style_palette *p)
{
    style_window *s;
    if(!p){SetLastError(ERROR_INVALID_PARAMETER);return FALSE;}
    if(!local_window(w))return FALSE;
    s=state(w);if(!s){SetLastError(ERROR_NOT_FOUND);return FALSE;}
    ++s->calls;s->palette=*p;win16_style_frame_scrollbars_palette_owned(w,p,s);paint(w,s,NULL);
    if(!--s->calls && s->detached)dispose(s);
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
    while(s->menus)end_menu_scope(s);
    if(!--s->calls && s->detached)dispose(s);
    return TRUE;
}

typedef struct menu_dispatch_scope { style_window *state;BOOL returned; } menu_dispatch_scope;
static void menu_dispatch_end(void *data)
{
    menu_dispatch_scope *scope=data;style_window *s=scope->state;
    if(!scope->returned && !--s->calls && s->detached)dispose(s);
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
        if(wp && s->pending_frame){
            RECT client;BOOL normal=!IsIconic(w)&&!IsZoomed(w);
            client_screen(w,&client);s->pending_frame=FALSE;
            activate_frame(w,s,&client,normal);
            if(s->detached)break;
        }
        goto forward;
    case WM_CTLCOLORDLG:
    case WM_CTLCOLORSTATIC:
        if(s->dialog){
            /* Native dialogs still own their contents and notifications.
             * Supply a coherent client palette, including native SysLink;
             * ordinary application windows retain their painting contract. */
            HDC dc=(HDC)wp;
            SetBkColor(dc,s->palette.light);
            SetTextColor(dc,s->palette.edge);
            SetDCBrushColor(dc,s->palette.light);
            result=(LRESULT)GetStockObject(DC_BRUSH);break;
        }
        goto forward;
    case WM_MEASUREITEM:
        if(win16_style_menu_size(s->menu_items,(MEASUREITEMSTRUCT *)lp,s->menu_font,s->dpi)){result=TRUE;break;}
        goto forward;
    case WM_DRAWITEM:
        {win16_style_palette palette;colors(s,&palette);
        if(win16_style_menu_draw(s->menu_items,(DRAWITEMSTRUCT *)lp,s->menu_font,s->dpi,&palette)){result=TRUE;break;}}
        goto forward;
    case WM_MENUCHAR:
        /* Application-owned items retain their existing mnemonic handler. */
        result=DefSubclassProc(w,msg,wp,lp);
        if(!s->detached && HIWORD(result)==MNC_IGNORE)
            win16_style_menu_char(s->menu_items,(HMENU)lp,(WCHAR)LOWORD(wp),&result);
        break;
    case WM_KEYDOWN:
        if(s->bar_keyboard){
            if(wp==VK_LEFT || wp==VK_RIGHT){select_bar(w,s,wp==VK_LEFT?-1:1);break;}
            if(wp==VK_ESCAPE || wp==VK_F10){leave_bar(w,s,TRUE);break;}
            if(wp==VK_DOWN || wp==VK_RETURN){open_bar(w,s,s->bar_selected);break;}
        }
        goto forward;
    case WM_KILLFOCUS:
        if(s->bar_keyboard)leave_bar(w,s,FALSE);
        goto forward;
    case WM_NCCALCSIZE:
        r=wp?((NCCALCSIZE_PARAMS *)lp)->rgrc[0]:*(RECT *)lp;
        if(measure_window(w,s,&r,&l)){
            /* USER maintains scrollbar-presence state during native NC
             * calculation. Let that owner update, then supply our client
             * rectangle; no native nonclient drawing is requested here. */
            DefSubclassProc(w,msg,wp,lp);
            if(s->detached)break;
            OffsetRect(&l.client,r.left,r.top);
            if(wp)((NCCALCSIZE_PARAMS *)lp)->rgrc[0]=l.client;else *(RECT *)lp=l.client;
            break;
        }
        goto forward;
    case WM_NCPAINT:
        if(wp!=1 && wp){
            /* USER owns the message region. GetDCEx takes ownership of its
             * argument, so pass a copy and restrict painting to the actual
             * update rather than repainting the whole caption on popups. */
            HRGN region=CreateRectRgn(0,0,0,0);HDC dc=NULL;
            if(region && CombineRgn(region,(HRGN)wp,NULL,RGN_COPY)!=ERROR){
                dc=GetDCEx(w,region,DCX_WINDOW|DCX_INTERSECTRGN|DCX_CACHE);
                if(dc){paint_nonclient_buffered(w,s,dc);ReleaseDC(w,dc);region=NULL;}
            }
            if(region)DeleteObject(region);
            if(!dc)paint(w,s,NULL);
        }else {HDC dc=GetWindowDC(w);if(dc){paint_nonclient_buffered(w,s,dc);ReleaseDC(w,dc);}}
        break;
    case WM_SETCURSOR:
        /* Default NC cursor handling can also repaint native caption buttons.
         * Only our frame belongs to us; preserve the application's client
         * cursor and error handling. No hover artwork exists in Win3.1. */
        if(!IsIconic(w) && (HWND)wp==w && (short)LOWORD(lp)>HTCLIENT){
            LPCWSTR cursor=IDC_ARROW;
            switch((short)LOWORD(lp)){
            case HTLEFT: case HTRIGHT: cursor=IDC_SIZEWE;break;
            case HTTOP: case HTBOTTOM: cursor=IDC_SIZENS;break;
            case HTSIZE: case HTTOPLEFT: case HTBOTTOMRIGHT: cursor=IDC_SIZENWSE;break;
            case HTTOPRIGHT: case HTBOTTOMLEFT: cursor=IDC_SIZENESW;break;
            }
            SetCursor(LoadCursorW(NULL,cursor));result=TRUE;break;
        }
        goto forward;
    case STYLE_NCUAHDRAWCAPTION:
    case STYLE_NCUAHDRAWFRAME:
        if(IsIconic(w))goto forward;
        break; /* Drawing notifications do not contain mouse coordinates. */
    case STYLE_UAHDRAWMENU:
    case STYLE_UAHDRAWMENUITEM:
        if(IsIconic(w) || !GetMenu(w))goto forward;
        break; /* Native hover must not paint a second, host-font menu item. */
    case WM_NCMOUSEMOVE:
        if(s->bar_keyboard && !tracking_menu){
            POINT cursor={GET_X_LPARAM(lp),GET_Y_LPARAM(lp)};int hit=bar_at_screen(w,s,cursor);
            if(hit!=-3 && hit!=s->bar_selected){
                win16_style_bar bar;POINT origin;BOOL popup=hit==-2;
                s->bar_selected=hit;paint_menu_state(w,s);
                if(hit>=0 && layout(w,s,&l)){origin.x=l.menu.left;origin.y=l.menu.top;
                    if(win16_style_bar_read_for_window(w,GetMenu(w),s->menu_font,s->dpi,origin,l.menu.right-l.menu.left,&bar)){
                        popup=(UINT)hit<bar.count && bar.items[hit].popup!=NULL;
                        win16_style_bar_free(&bar);}}
                if(popup)open_bar(w,s,hit);
            }
        }
        if(IsIconic(w))goto forward;
        break;
    case WM_NCMOUSEHOVER:
    case WM_NCMOUSELEAVE:
        if(IsIconic(w))goto forward; /* S3 retains native minimized presentation. */
        /* DWMNCRP_DISABLED does not disable USER/UxTheme painting. In
         * particular default NC hover paints native caption buttons without
         * asking our WM_NCPAINT owner, potentially into our client area.
         * Win3.1 buttons have no hover drawing. Press tracking remains in
         * WM_MOUSEMOVE below; menu/drag/size commands still go to USER. */
        break;
    case WM_NCACTIVATE:
        s->active=(BOOL)wp;
        if(IsIconic(w))goto forward;
        /* Own activation drawing too. Forwarding even with lp=-1 leaves
         * native themed-caption pixels below the shorter custom caption.
         * TRUE permits deactivation; WM_ACTIVATE/focus still go downstream. */
        result=TRUE;paint(w,s,NULL);break;
    case WM_NCHITTEST:
        if(win16_style_hit_test(w,lp,&result))break;
        goto forward;
    case WM_INITMENUPOPUP:
        /* The owner must see its original item data when it refreshes a
         * submenu that has already been opened during this menu loop. */
        win16_style_menu_restore_popup((HMENU)wp,&s->menu_items);
        result=DefSubclassProc(w,msg,wp,lp);
        if(!s->detached && !win16_style_menu_prepare((HMENU)wp,&s->menu_items,s->menu_depth))
            win16_style_menu_restore_popup((HMENU)wp,&s->menu_items);
        if(!s->detached && HIWORD(lp)){
            if(s->menus)win16_style_menu_system(s->menus->scope,(HMENU)wp,s->menu_font,s->dpi);
            s->system_menu=TRUE;paint_button_state(w,s,HTSYSMENU);
        }
        break;
    case WM_SYSCOMMAND:
        if((wp&0xfff0)==SC_KEYMENU && !lp && GetMenu(w)){
            if(s->bar_keyboard)leave_bar(w,s,TRUE);else select_bar(w,s,0);
            break;
        }
        if((wp&0xfff0)==SC_KEYMENU && lp && lp!=L' ' && GetMenu(w) && layout(w,s,&l)){
            win16_style_bar bar;POINT origin={l.menu.left,l.menu.top};int index=-1;
            if(win16_style_bar_read_for_window(w,GetMenu(w),s->menu_font,s->dpi,origin,l.menu.right-l.menu.left,&bar)){
                UINT i;WCHAR key=(WCHAR)lp;CharUpperBuffW(&key,1);
                for(i=0;i<bar.count && index<0;++i){const WCHAR *text=bar.items[i].text;
                    if(bar.items[i].state&MFS_DISABLED)continue;
                    for(;*text && *text!=L'\t';++text)if(*text==L'&' && text[1]){
                        WCHAR letter=*++text;if(letter==L'&')continue;CharUpperBuffW(&letter,1);
                        if(letter==key){index=(int)i;break;}}
                }
                win16_style_bar_free(&bar);
            }
            if(index>=0){
                if(!s->bar_keyboard)select_bar(w,s,0);
                if(!s->detached)open_bar(w,s,index);
            }
            break;
        }
        if((wp&0xfff0)==SC_KEYMENU && lp==L' ' && layout(w,s,&l)){
            MENUBARINFO menu={.cbSize=sizeof(menu)};
            if((s->bar_keyboard && s->bar_selected==-2) ||
                (GetMenuBarInfo(w,OBJID_SYSMENU,0,&menu) && IsRectEmpty(&menu.rcBar))){
                menu_frame frame={0},*previous=tracking_menu;HHOOK hook;
                GetWindowRect(w,&r);frame.window=w;frame.button=l.system;
                if(s->bar_keyboard && s->bar_selected==-2)
                    {frame.root=frame.active=GetSystemMenu(w,FALSE);
                     frame.style=s;frame.selected=-2;frame.target=-3;}
                OffsetRect(&frame.button,r.left,r.top);tracking_menu=&frame;
                hook=SetWindowsHookExW(WH_GETMESSAGE,menu_filter,NULL,GetCurrentThreadId());
                result=DefSubclassProc(w,msg,wp,lp);
                if(hook)UnhookWindowsHookEx(hook);
                tracking_menu=previous;
                if(frame.root){
                    s->bar_direction=frame.direction;s->bar_hover=frame.target;
                    if(!s->detached && !frame.direction && frame.target==-3 && !frame.escape)
                        leave_bar(w,s,TRUE);
                }
                /* Outside the menu loop USER handles SC_CLOSE availability
                 * and the application still receives its normal NC message. */
                if(frame.double_click && !s->detached)
                    PostMessageW(w,WM_NCLBUTTONDBLCLK,HTSYSMENU,frame.point);
                break;
            }
        }
        goto forward;
    case WM_MENUSELECT:
        if(tracking_menu && tracking_menu->window==w && lp)
            tracking_menu->active=(HMENU)lp;
        goto forward;
    case WM_ENTERMENULOOP:
        ++s->menu_depth;
        {menu_loop *loop=calloc(1,sizeof(*loop));
            if(loop){loop->scope=win16_style_menu_begin(&s->palette);
                if(loop->scope){loop->depth=s->menu_depth;loop->previous=s->menus;s->menus=loop;}
                else free(loop);}}
        goto forward;
    case WM_EXITMENULOOP:
        win16_style_menu_restore_depth(s->menu_depth,&s->menu_items);
        /* An allocation/hook failure at an inner entry must not pop an
         * outer successful scope when that unstyled inner loop exits. */
        if(s->menus && s->menus->depth==s->menu_depth)end_menu_scope(s);
        if(s->menu_depth)--s->menu_depth;
        result=DefSubclassProc(w,msg,wp,lp);
        if(!s->detached && s->system_menu){s->system_menu=FALSE;paint_button_state(w,s,HTSYSMENU);}
        break;
    case WM_NCLBUTTONDBLCLK:
        if(wp==HTMENU && layout(w,s,&l)){
            win16_style_bar bar;POINT origin={l.menu.left,l.menu.top};int item;BOOL close=FALSE;
            GetWindowRect(w,&r);point.x=GET_X_LPARAM(lp)-r.left;point.y=GET_Y_LPARAM(lp)-r.top;
            if(win16_style_bar_read_for_window(w,GetMenu(w),s->menu_font,s->dpi,origin,l.menu.right-l.menu.left,&bar)){
                item=win16_style_menu_hit(bar.rects,bar.count,point);
                if(item>=0 && bar.items[item].control==HBMMENU_SYSTEM && bar.items[item].popup){
                    UINT state=GetMenuState(bar.items[item].popup,SC_CLOSE,MF_BYCOMMAND);
                    close=state!=(UINT)-1 && !(state&(MF_DISABLED|MF_GRAYED));
                }
                win16_style_bar_free(&bar);
            }
            /* DefFrameProc routes this command to the maximized MDI child.
             * Sending SC_CLOSE to the frame itself would close the parent. */
            if(close)PostMessageW(w,WM_COMMAND,SC_CLOSE,0);
            break;
        }
        goto forward;
    case WM_NCLBUTTONDOWN:
        if(s->bar_keyboard && !tracking_menu)leave_bar(w,s,TRUE);
        if(wp==HTMENU && layout(w,s,&l)){
            win16_style_bar bar;POINT origin={l.menu.left,l.menu.top};int item;
            GetWindowRect(w,&r);point.x=GET_X_LPARAM(lp)-r.left;point.y=GET_Y_LPARAM(lp)-r.top;
            if(win16_style_bar_read_for_window(w,GetMenu(w),s->menu_font,s->dpi,origin,l.menu.right-l.menu.left,&bar)){
                item=win16_style_menu_hit(bar.rects,bar.count,point);
                win16_style_bar_free(&bar);
                if(item>=0){
                    if(!s->detached)open_bar(w,s,item);
                }
            }
            break;
        }
        if(wp==HTSYSMENU){
            MENUBARINFO menu={.cbSize=sizeof(menu)};
            /* Modern dialog frames can have a system menu but no native
             * icon rectangle. Their native mouse-menu entry has no button
             * to track; use the same system-menu entry as Alt+Space. */
            if(GetMenuBarInfo(w,OBJID_SYSMENU,0,&menu) && IsRectEmpty(&menu.rcBar)){
                /* Do not enter a keyboard menu with the mouse still down:
                 * its subsequent release would immediately dismiss it. */
                s->pressed=HTSYSMENU;s->hot=TRUE;SetCapture(w);paint_button_state(w,s,HTSYSMENU);break;
            }
        }
        /* Native system-menu tracking owns the second click, cancellation
         * and disabled SC_CLOSE semantics. TrackPopupMenu is not equivalent
         * to the non-client menu loop. Double clicks also go to the owner. */
        if(wp==HTMINBUTTON || wp==HTMAXBUTTON){
            s->pressed=(int)wp;s->hot=TRUE;SetCapture(w);paint_button_state(w,s,(int)wp);break;
        }
        goto forward;
    case WM_LBUTTONDOWN:
        if(s->bar_keyboard && !tracking_menu)leave_bar(w,s,TRUE);
        goto forward;
    case WM_MOUSEMOVE:
    case WM_LBUTTONUP:
        if(s->pressed && layout(w,s,&l)){
            int pressed=s->pressed;BOOL was_hot=s->hot;
            point.x=GET_X_LPARAM(lp);point.y=GET_Y_LPARAM(lp);ClientToScreen(w,&point);
            GetWindowRect(w,&r);point.x-=r.left;point.y-=r.top;
            s->hot=win16_style_hit(&l,point)==pressed;
            if(msg==WM_LBUTTONUP){
                BOOL invoke=s->hot;s->pressed=0;ReleaseCapture();
                if(invoke)PostMessageW(w,WM_SYSCOMMAND,pressed==HTSYSMENU?SC_KEYMENU:
                    pressed==HTMINBUTTON?SC_MINIMIZE:(IsZoomed(w)?SC_RESTORE:SC_MAXIMIZE),
                    pressed==HTSYSMENU?L' ':0);
            }
            if(!s->detached && (msg==WM_LBUTTONUP || was_hot!=s->hot))paint_button_state(w,s,pressed);
            break;
        }
        goto forward;
    case WM_CANCELMODE:
    case WM_CAPTURECHANGED:
        if(msg==WM_CANCELMODE && tracking_menu && tracking_menu->window==w)EndMenu();
        if(msg==WM_CANCELMODE && s->bar_keyboard)leave_bar(w,s,TRUE);
        if(!s->pressed)goto forward;
        {int pressed=s->pressed;s->pressed=0;s->hot=FALSE;
        if(msg==WM_CANCELMODE && GetCapture()==w)ReleaseCapture();
        if(!s->detached)paint_button_state(w,s,pressed);}goto forward;
    case WM_PRINT:
        if((lp&PRF_CHECKVISIBLE) && !IsWindowVisible(w))break;
        if(lp&PRF_NONCLIENT)paint(w,s,(HDC)wp);
        if(!s->detached)print_client_and_children(w,s,(HDC)wp,lp);
        break;
    case WM_SETTEXT:
        result=DefSubclassProc(w,msg,wp,lp);
        if(!s->detached)paint(w,s,NULL);
        break;
    case WM_HSCROLL:
    case WM_VSCROLL:
        result=DefSubclassProc(w,msg,wp,lp);
        /* A nonzero lParam identifies a child SCROLLBAR notification, not a
         * change to this window's nonclient bars. */
        /* Styled frame scroll updates repaint the affected bar via the API
         * binding; child notifications never invalidate unrelated bars. */
        break;
    case WM_DPICHANGED:
        result=DefSubclassProc(w,msg,wp,lp);
        if(!s->detached){refresh_font(w,s);frame_changed(w);}break;
    case WM_SETTINGCHANGE:
    case WM_SYSCOLORCHANGE:
    case WM_THEMECHANGED:
        result=DefSubclassProc(w,msg,wp,lp);
        if(!s->detached)paint(w,s,NULL);
        break;
    case WM_NCDESTROY:
        (void)win16_style_frame_terminal(w);
        result=DefSubclassProc(w,msg,wp,lp);break;
    default:
forward:
        result=DefSubclassProc(w,msg,wp,lp);break;
    }
    if(!--s->calls && s->detached)dispose(s);
    return result;
}
