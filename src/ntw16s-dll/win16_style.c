/* Project-authored experimental non-client presentation, not imported USER.
 * See T445 S1 provenance/design: original NTUSER server is a stopping boundary.
 */
#include "win16_style.h"
#include "layout.h"
#include "../lib/win16-control/frame_paint.h"
#include "../lib/win16-control/frame_view.h"
#include "../lib/win16-control/frame_state.h"
#include "../lib/win16-control/frame_lifecycle.h"
#include "../lib/win16-control/menu_iteration.h"
#include "../lib/win16-control/menu_message.h"
#include "../lib/win16-control/frame_message.h"
#include "../lib/win16-control/frame_dispatch.h"
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

static const win16_control_frame_lifecycle_services frame_lifecycle={style_proc,DwmGetWindowAttribute,DwmSetWindowAttribute,local_window,state,refresh_font,layout,activate_frame,
    win16_style_frame_scrollbars_install_owned,win16_style_frame_scrollbars_palette_owned,win16_style_control_remove_owned,
    win16_style_remove,leave_bar,paint};
BOOL win16_style_menu_owner_install_created(HWND w,const void *owner,const BOOL *cancelled)
{return win16_control_menu_owner_install(w,owner,cancelled,&frame_lifecycle);}
BOOL win16_style_install(HWND w,const win16_style_palette *p)
{return win16_control_frame_install(w,p,FALSE,NULL,NULL,&frame_lifecycle);}
BOOL win16_style_install_created(HWND w,const win16_style_palette *p,const void *owner,const BOOL *cancelled)
{return win16_control_frame_install(w,p,TRUE,owner,cancelled,&frame_lifecycle);}
BOOL win16_style_frame_remove_owned(HWND w,const void *owner)
{return win16_control_frame_remove_owned(w,owner,&frame_lifecycle);}
BOOL win16_style_remove(HWND w)
{return win16_control_frame_remove(w,&frame_lifecycle);}
BOOL win16_style_set_palette(HWND w,const win16_style_palette *p)
{return win16_control_frame_set_palette(w,p,&frame_lifecycle);}
BOOL win16_style_frame_terminal(HWND w)
{return win16_control_frame_terminal(w,&frame_lifecycle);}

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
    (void)id;
    const win16_control_frame_message_services message={measure_window,activate_frame,paint,paint_callback,paint_buffered_callback,win16_style_hit_test,refresh_font};
    const win16_control_frame_input_services input={menu_current,menu_begin,menu_end,layout,bar_at_screen,paint_menu_state,select_bar,leave_bar,open_bar,button_view};
    const win16_control_frame_dispatch_services services={&message,&input,paint_button_state,&tracking_menu,win16_style_frame_terminal};
    return win16_control_frame_dispatch(w,(style_window *)data,msg,wp,lp,&services);
}
