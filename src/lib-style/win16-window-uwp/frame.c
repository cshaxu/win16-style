#include "frame.h"
#include "frame_layout.h"
#include "lib-style/uwp/surface.h"
#include "lib-style/uwp/host.h"
#include "lib-style/uwp/menu.h"
#include "lib-style/common/frame.h"
#include "lib-win32/style/window_identity.h"
#include "lib-style/win16-window/frame_view.h"
#include "lib-style/win16-common/font.h"
#include "lib-style/win16-window/menu_message.h"
#include <commctrl.h>
#include <stdint.h>
#include <stdlib.h>
#include <windowsx.h>

typedef struct frame_action { win16_uwp_frame_action value; UINT command; } frame_action;

typedef struct uwp_frame {
    style_uwp_host host;
    BOOL updating,active,hot,tracking;
    BOOL no_menu,no_keyboard;
    int pressed,no_selection;
    UINT dpi;
    HFONT title_font,menu_font;
    style_bitmap bitmap;
    win16_style_layout layout;
    style_window *menu;
    HWND menu_window;
    unsigned action_count;
    frame_action actions[];
} uwp_frame;

static BOOL surface_message(void *,HWND,UINT,WPARAM,LPARAM,LRESULT *);

static BOOL local(HWND w)
{
    if(!win32_style_window_current_thread(w)){
        SetLastError(ERROR_INVALID_THREAD_ID);return FALSE;
    }
    return TRUE;
}
static uwp_frame *lookup(HWND w)
{
    DWORD_PTR value=0;
    return GetWindowSubclass(w,style_uwp_host_owner_proc,(UINT_PTR)style_uwp_host_owner_proc,&value)?
        (uwp_frame *)((style_uwp_host *)value)->context:NULL;
}
static void dispose(void *context)
{
    uwp_frame *s=context;
    style_bitmap_dispose(&s->bitmap);
    if(s->title_font)DeleteObject(s->title_font);
    s->menu->menu_font=NULL; /* The frame owns the font lease. */
    win16_control_frame_state_dispose(s->menu);
    win16_style_system_release(s->menu_font);
    free(s);
}
static void before_title_remove(void *context)
{
    uwp_frame *s=context;
    if(s->tracking)EndMenu();
}
static void after_title_remove(void *context)
{
    uwp_frame *s=context;
    s->menu->detached=TRUE;
    while(s->menu->menus)win16_control_frame_state_end_menu(s->menu);
    style_menu_restore(&s->menu->menu_items);
}
static BOOL geometry(uwp_frame *s,RECT *outer)
{
    RECT content;BOOL zoomed;
    if(!style_uwp_frame_bounds(s->host.owner,s->host.title,outer,&content,&zoomed))return FALSE;
    SIZE size={outer->right-outer->left,outer->bottom-outer->top};
    UINT dpi=GetDpiForWindow(s->host.owner);
    if(!win16_uwp_frame_layout(size,dpi,(DWORD)GetWindowLongPtrW(s->host.owner,GWL_STYLE),zoomed,content,&s->layout))return FALSE;
    if(dpi!=s->dpi && !win16_control_refresh_frame_fonts(s->host.owner,&s->dpi,&s->title_font,&s->menu_font))return FALSE;
    s->menu->dpi=s->dpi;s->menu->menu_font=s->menu_font;
    return !s->host.detached;
}
static BOOL cached_layout(HWND w,void *context,win16_style_layout *layout)
{(void)w;*layout=((uwp_frame *)context)->layout;return TRUE;}
static void colors(void *context,win16_style_palette *palette)
{(void)context;win16_style_default_palette(palette);}
static void no_scrollbars(HWND w,HDC dc,const win16_style_palette *palette)
{(void)w;(void)dc;(void)palette;}
static BOOL refresh_geometry(void *context,RECT *outer,RECT *hole)
{
    uwp_frame *s=context;
    if(!geometry(s,outer))return FALSE;
    *hole=s->layout.client;return TRUE;
}
static void refresh_paint(void *context,HDC dc)
{
    uwp_frame *s=context;
    win16_control_frame_view view={s,&s->host.detached,&s->active,&s->menu_font,&s->title_font,&s->dpi,
        &s->no_selection,&s->no_menu,&s->no_keyboard,&s->pressed,&s->hot,cached_layout,colors,no_scrollbars};
    win16_control_paint_frame(s->host.owner,&view,dc);
}
static BOOL refresh(void *context)
{
    uwp_frame *s=context;
    const style_uwp_surface_view view={s->host.owner,&s->host.surface,&s->host.detached,&s->updating,
        &s->bitmap,s,refresh_geometry,refresh_paint};
    return style_uwp_surface_refresh(&view);
}
static LRESULT point_hit(uwp_frame *s,LPARAM point)
{
    POINT p={GET_X_LPARAM(point),GET_Y_LPARAM(point)};
    return win16_uwp_frame_hit(&s->layout,p);
}
static LPARAM screen_point(uwp_frame *s,LPARAM point)
{
    POINT p={GET_X_LPARAM(point),GET_Y_LPARAM(point)};ClientToScreen(s->host.surface,&p);
    return MAKELPARAM(p.x,p.y);
}
static void invoke(uwp_frame *s,int hit)
{
    UINT command=hit==HTMINBUTTON?SC_MINIMIZE:hit==HTMAXBUTTON?
        (IsZoomed(s->host.owner)?SC_RESTORE:SC_MAXIMIZE):SC_KEYMENU;
    PostMessageW(s->host.owner,WM_SYSCOMMAND,command,command==SC_KEYMENU?L' ':0);
}
static void menu_present(uwp_frame *s,HWND window)
{
    if(s->host.detached || !s->tracking || s->menu_window)return;
    style_uwp_menu_present(window,GetSystemMenu(s->host.owner,FALSE),&s->menu_window);
}
static void system_menu(uwp_frame *s)
{
    if(s->tracking || s->host.detached)return;
    HMENU menu=GetSystemMenu(s->host.owner,FALSE);RECT surface;
    if(!menu || !GetWindowRect(s->host.surface,&surface))return;
    s->tracking=TRUE;
    SendMessageW(s->host.owner,WM_INITMENU,(WPARAM)menu,0);
    if(s->host.detached){s->tracking=FALSE;return;}
    /* TrackPopupMenu does not apply the root window's system-command states.
     * Add only temporary restrictions; retain application-disabled commands. */
    style_uwp_menu_state menu_state;
    style_uwp_menu_restrict(s->host.owner,menu,&menu_state);
    BOOL actions_ready=TRUE;
    for(unsigned i=0;i<s->action_count;++i){
        frame_action *action=&s->actions[i];
        action->command=style_uwp_menu_add(menu,action->value.label,action);
        if(!action->command){actions_ready=FALSE;break;}
    }
    UINT command=0;unsigned selected=s->action_count;
    /* Never present a replacement menu with a required action missing. */
    if(actions_ready){
        s->no_menu=TRUE;refresh(s);
        if(!s->host.detached)command=TrackPopupMenuEx(menu,TPM_LEFTALIGN|TPM_TOPALIGN|TPM_RETURNCMD|TPM_NOANIMATION,
            surface.left+s->layout.system.left,surface.top+s->layout.system.bottom,
            s->host.surface,NULL);
    }
    if(s->menu_window){
        style_uwp_menu_unpresent(s->menu_window,menu);
        s->menu_window=NULL;
    }
    if(!s->host.detached && GetSystemMenu(s->host.owner,FALSE)==menu)style_uwp_menu_restore(menu,&menu_state);
    for(unsigned i=0;i<s->action_count;++i){
        frame_action *action=&s->actions[i];
        if(!action->command)continue;
        if(command==action->command)selected=i;
        style_uwp_menu_remove(menu,action->command,action);
        action->command=0;
    }
    s->tracking=FALSE;s->no_menu=FALSE;
    if(!s->host.detached){
        refresh(s);
        if(!s->host.detached && command){
            if(selected<s->action_count)s->actions[selected].value.invoke(s->host.owner,s->actions[selected].value.context);
            else PostMessageW(s->host.owner,WM_SYSCOMMAND,command,0);
        }
    }
}
static void surface_menu_button(HWND w,style_window *menu,int hit)
{(void)w;(void)menu;(void)hit;}
static BOOL surface_message(void *context,HWND w,UINT message,WPARAM a,LPARAM b,LRESULT *output)
{
    uwp_frame *s=context;LRESULT result=0;BOOL handled=TRUE;
    if(message==WM_ENTERMENULOOP)win16_control_frame_state_enter_menu(s->menu);
    if(win16_control_menu_message(w,s->menu,message,a,b,surface_menu_button,&result)){
        *output=result;return TRUE;
    }
    switch(message){
    case WM_ENTERIDLE:if(a==MSGF_MENU)menu_present(s,(HWND)b);break;
    case WM_MOUSEACTIVATE:result=MA_NOACTIVATE;break;
    case WM_SETCURSOR: {
        POINT point;
        if(!GetCursorPos(&point)||!ScreenToClient(w,&point)){handled=FALSE;break;}
        int hit=(int)win16_uwp_frame_hit(&s->layout,point);
        PCWSTR cursor=style_frame_resize_cursor(hit);
        SetCursor(LoadCursorW(NULL,cursor));
        result=TRUE;break;
    }
    case WM_NCHITTEST:result=HTCLIENT;break; /* The window region excludes client content. */
    case WM_LBUTTONDOWN: {
        int hit=(int)point_hit(s,b);SetForegroundWindow(s->host.owner);
        if(s->host.detached)break;
        if(hit==HTSYSMENU || hit==HTMINBUTTON || hit==HTMAXBUTTON){s->pressed=hit;s->hot=TRUE;SetCapture(w);refresh(s);}
        else if(hit==HTCAPTION || (hit>=HTLEFT && hit<=HTBOTTOMRIGHT))
            SendMessageW(s->host.owner,WM_NCLBUTTONDOWN,(WPARAM)hit,screen_point(s,b));
        break;
    }
    case WM_LBUTTONDBLCLK: {
        int hit=(int)point_hit(s,b);
        if(hit==HTSYSMENU)PostMessageW(s->host.owner,WM_SYSCOMMAND,SC_CLOSE,0);
        else if(hit==HTCAPTION && (GetWindowLongPtrW(s->host.owner,GWL_STYLE)&WS_MAXIMIZEBOX))invoke(s,HTMAXBUTTON);
        break;
    }
    case WM_RBUTTONUP:
        if(point_hit(s,b)==HTCAPTION || point_hit(s,b)==HTSYSMENU)invoke(s,HTSYSMENU);
        break;
    case WM_MOUSEMOVE:
        if(s->pressed){BOOL hot=point_hit(s,b)==s->pressed;if(hot!=s->hot){s->hot=hot;refresh(s);}}
        break;
    case WM_LBUTTONUP:
        if(s->pressed){int hit=s->pressed;BOOL act=point_hit(s,b)==hit;s->pressed=0;s->hot=FALSE;
            ReleaseCapture();if(!s->host.detached){refresh(s);if(act&&!s->host.detached)invoke(s,hit);}}
        break;
    case WM_CANCELMODE:
    case WM_CAPTURECHANGED:
        if(s->pressed){s->pressed=0;s->hot=FALSE;if(GetCapture()==w)ReleaseCapture();refresh(s);}
        break;
    default:handled=FALSE;break;
    }
    *output=result;return handled;
}
static void menu_button(HWND w,style_window *menu,int hit)
{
    (void)hit;uwp_frame *s=lookup(w);
    if(s&&!s->host.detached){s->no_menu=menu->system_menu;refresh(s);}
}
static BOOL owner_message(void *context,HWND w,UINT message,WPARAM a,LPARAM b,LRESULT *result)
{
    uwp_frame *s=context;
    if(!s->host.detached && message==WM_CANCELMODE && s->tracking)EndMenu();
    if(!s->host.detached && message==WM_SYSCOMMAND &&
       (((a&0xfff0)==SC_KEYMENU && (b==L' '||b==0)) || (a&0xfff0)==SC_MOUSEMENU)){
        system_menu(s);*result=0;return TRUE;
    }
    if(!s->host.detached){
        if(message==WM_ENTERMENULOOP)win16_control_frame_state_enter_menu(s->menu);
        return win16_control_menu_message(w,s->menu,message,a,b,menu_button,result);
    }
    return FALSE;
}
static void owner_activation(void *context,UINT message,WPARAM value)
{uwp_frame *s=context;s->active=message==WM_NCACTIVATE?(BOOL)value:LOWORD(value)!=WA_INACTIVE;}
static BOOL apply_actions(HWND w,HWND title,const win16_uwp_frame_action *actions,unsigned count)
{
    /* There are 256 available 16-aligned command slots below SC_SIZE. */
    if(count>256 || (count&&!actions)){SetLastError(ERROR_INVALID_PARAMETER);return FALSE;}
    for(unsigned i=0;i<count;++i)if(!actions[i].label || !*actions[i].label || !actions[i].invoke){SetLastError(ERROR_INVALID_PARAMETER);return FALSE;}
    if(!local(w) || (title && !local(title)))return FALSE;
    if(lookup(w)){SetLastError(ERROR_ALREADY_EXISTS);return FALSE;}
    if(title && GetParent(title)!=w){SetLastError(ERROR_INVALID_PARAMETER);return FALSE;}
    uwp_frame *s=calloc(1,sizeof(*s)+(size_t)count*sizeof(frame_action));if(!s){SetLastError(ERROR_NOT_ENOUGH_MEMORY);return FALSE;}
    s->menu=win16_control_frame_state_create();
    if(!s->menu){free(s);SetLastError(ERROR_NOT_ENOUGH_MEMORY);return FALSE;}
    win16_style_default_palette(&s->menu->palette);
    s->action_count=count;
    for(unsigned i=0;i<count;++i)s->actions[i].value=actions[i];
    s->host=(style_uwp_host){.owner=w,.title=title,.surface_message=surface_message,.context=s,
        .owner_message=owner_message,.owner_activation=owner_activation,
        .before_title_remove=before_title_remove,.after_title_remove=after_title_remove,
        .dispose=dispose,.calls=1};
    s->no_selection=-1;s->active=GetForegroundWindow()==w;
    BOOL ok=style_uwp_host_start(&s->host,refresh);
    DWORD error=GetLastError();
    style_uwp_host_release(&s->host);SetLastError(error);return ok;
}
BOOL win16_uwp_frame_apply_actions(HWND w,HWND title,const win16_uwp_frame_action *actions,unsigned count)
{
    if(!title){SetLastError(ERROR_INVALID_PARAMETER);return FALSE;}
    return apply_actions(w,title,actions,count);
}
BOOL win16_uwp_frame_apply_nonclient(HWND w)
{return apply_actions(w,NULL,NULL,0);}
BOOL win16_uwp_frame_apply_action(HWND w,HWND title,const win16_uwp_frame_action *action)
{return win16_uwp_frame_apply_actions(w,title,action,action?1:0);}
BOOL win16_uwp_frame_apply(HWND w,HWND title)
{return win16_uwp_frame_apply_actions(w,title,NULL,0);}
BOOL win16_uwp_frame_remove(HWND w)
{
    if(!local(w))return FALSE;
    uwp_frame *s=lookup(w);if(!s){SetLastError(ERROR_NOT_FOUND);return FALSE;}
    style_uwp_host_retain(&s->host);BOOL ok=style_uwp_host_detach(&s->host);style_uwp_host_release(&s->host);return ok;
}
HWND win16_uwp_frame_surface(HWND w)
{
    if(!local(w))return NULL;
    uwp_frame *s=lookup(w);return s&&!s->host.detached?s->host.surface:NULL;
}
