#include "lib-style/common/control_vcl_identity.h"
#include "../lib-style/win16-control/vb_text.h"
#include "../lib-style/win16-control/control_class.h"
#include "ntw16s.h"
#include "native_api.h"
#include "theme.h"
#include "static_text.h"
#include "../lib-style/win16-control/basic_paint.h"
#include "../lib-style/win16-control/combo_paint.h"
#include "../lib-style/win16-control/native_combo_identity.h"
#include "../lib-style/win16-control/cool_grip_identity.h"
#include "lib-style/common/control_custom_draw.h"
#include "../lib-style/win16-control/vcl_native_paint.h"
#include "../lib-style/win16-control/text_policy.h"
#include "../lib-style/win16-control/vcl_frame_policy.h"
#include "../lib-style/win16-control/choice_paint.h"
#include "../lib-style/win16-control/special_paint.h"
#include "lib-style/common/control_vcl_spin.h"
#include "../lib-style/win16-control/spin_scope.h"
#include "../lib-style/win16-control/status_frame.h"
#include "../lib-style/win16-control/status_font.h"
#include "../lib-style/win16-control/status_text.h"
#include "lib-style/common/control_status.h"
#include "../lib-style/win16-control/vcl_identity.h"
#include "../lib-style/win16-control/attachment_state.h"
#include "../lib-style/win16-control/vcl_extra_frame.h"
#include "../lib-style/win16-control/month_paint.h"
#include "../lib-style/win16-window-uwp/frame.h"
#include "lib-style/common/message_observer.h"
#include "lib-win32/style/window_hook.h"
#include "lib-win32/style/window_identity.h"
#include "lib-win32/style/creation_pending.h"
#include "../lib-win32/process-hook/unwind.h"
#include "status_theme.h"
#include "../lib-style/win16-control/month_text.h"
#include <commctrl.h>
#include <uxtheme.h>
#include <vssym32.h>
#include <oleacc.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <limits.h>
#include <stdint.h>
#include "attachment.h"
#include "window_thread.h"

/* This coordinator composes the one existing renderer. It observes only
 * creation inside explicit caller scopes; it never scans a desktop or infers
 * task policy from an owner, title, executable name or a process-wide switch. */
typedef win32_style_creation_pending pending_window;
struct ntw16s_creation_scope {
    struct ntw16s_creation_scope *previous;
    DWORD thread;
    BOOL enabled;
#ifdef NTW16S_EXPERIMENTAL_CREATION_SUBCLASS
    BOOL owns_return_hook,suspended_return_hook;
#endif
    pending_window *pending;
    ntw16s_creation_result result;
};

static _Thread_local ntw16s_creation_scope *current;
static _Thread_local HWND lookup_popup_creator;
typedef struct cool_observation {struct cool_observation *previous;attachment *owner;HWND window;WNDPROC native;} cool_observation;
static _Thread_local cool_observation *cool_observing;
typedef struct static_paint_scope {struct static_paint_scope *previous;attachment *owner;HWND window;ntw16s_vb_paint *owned_paint;void *status_prior,*spin_prior;BOOL completed;win16_control_month_state month;win16_control_vcl_frame_scope frame;} static_paint_scope;
static _Thread_local static_paint_scope *static_paint;
static _Thread_local ntw16s_static_call *static_call;
static _Thread_local HHOOK creation_return_hook;
static _Thread_local HHOOK creation_before_hook;
#ifdef NTW16S_EXPERIMENTAL_CREATION_SUBCLASS
static LRESULT CALLBACK creation_return(int,WPARAM,LPARAM);
static LRESULT creation_observe(int,WPARAM,LPARAM,BOOL);
static _Thread_local void (*creation_dialog_observer)(HWND);
void (*ntw16s_creation_dialog_observer(void (*observer)(HWND)))(HWND)
{
    void (*previous)(HWND)=creation_dialog_observer;
    creation_dialog_observer=observer;return previous;
}
static LRESULT CALLBACK creation_subclass(HWND w,UINT message,WPARAM a,LPARAM b,UINT_PTR id,DWORD_PTR data)
{
    (void)id;(void)data;
    LRESULT result=DefSubclassProc(w,message,a,b);
    if(message==WM_CREATE || message==WM_INITDIALOG || message==WM_SHOWWINDOW ||
        message==WM_WINDOWPOSCHANGED || message==WM_NCDESTROY){
        CWPRETSTRUCT returned={.lResult=result,.lParam=b,.wParam=a,.message=message,.hwnd=w};
        creation_observe(0,0,(LPARAM)&returned,TRUE);
    }
    if(message==WM_INITDIALOG && IsWindow(w) && creation_dialog_observer){
        DWORD error=GetLastError();creation_dialog_observer(w);SetLastError(error);
    }
    if(message==WM_NCDESTROY || message==WM_INITDIALOG ||
        (message==WM_CREATE && !style_control_class_is(w,L"#32770")))
        RemoveWindowSubclass(w,creation_subclass,(UINT_PTR)creation_subclass);
    return result;
}
#endif
static _Thread_local checklist_draw_scope *checklist_current;
typedef struct bit_draw_scope {struct bit_draw_scope *previous;attachment *owner;HWND window;HDC dc;RECT item;UINT state;BOOL completed;} bit_draw_scope;
static _Thread_local bit_draw_scope *bit_current;


/* Status frames are drawn by the native common control before text, icons
 * and WM_DRAWITEM. A per-message snapshot makes nested unrelated drawing a
 * barrier and allows a changing or destroyed control to fall back safely. */
typedef struct status_draw_scope {
    struct status_draw_scope *previous;
    attachment *owner; HWND window; HDC dc; HMODULE native;
    unsigned epoch,count,text_count; BOOL simple; int simple_length; WCHAR simple_text[513]; RECT panes[64],text_panes[64];
} status_draw_scope;
static _Thread_local status_draw_scope *status_current;
static _Thread_local ntw16s_status_theme_scope *status_theme_current;
static LONG status_hooks_ready;
void ntw16s_status_hooks_enable(void){InterlockedExchange(&status_hooks_ready,1);}

/* Scope is active only while dispatching the observed control's own paint or
 * pointer/enable messages. Unrelated nested attachment messages are barriers. */
static _Thread_local spin_draw_scope *spin_stack;
static _Thread_local spin_draw_scope *spin_current;
void ntw16s_modern_spin_dc_created(HDC a,HDC b){win16_control_spin_dc_created(spin_current,a,b);}
void ntw16s_modern_spin_theme_begin(HDC dc,const RECT *r){win16_control_spin_theme_begin(spin_current,dc,r);}
void ntw16s_modern_spin_dc_deleted(HDC dc){win16_control_spin_dc_deleted(spin_stack,dc);}
void ntw16s_modern_spin_selected(HDC dc){win16_control_spin_selected(spin_stack,dc);}
void ntw16s_modern_spin_object_deleted(HGDIOBJ o){win16_control_spin_object_deleted(spin_stack,o);}
void ntw16s_modern_spin_theme(HDC dc,int part,int state,const RECT *r,const RECT *clip,const void *caller)
{win16_control_spin_theme(spin_current,dc,part,state,r,clip,caller);}
void ntw16s_modern_spin_blit(HDC dst,int x,int y,int w,int h,HDC src,int sx,int sy,DWORD rop)
{win16_control_spin_blit(spin_current,dst,x,y,w,h,src,sx,sy,rop);}
void ntw16s_spin_frame(HDC dc,const RECT *r,UINT type,UINT flags,const void *caller)
{win16_control_spin_frame(spin_current,dc,r,type,flags,caller);}

void ntw16s_bit_theme_frame(HDC dc,int part,const RECT *rect,const RECT *clip,const void *address)
{
    bit_draw_scope *s=bit_current;if(!s || !s->owner)return;attachment *a=s->owner;
    const win16_control_bit_frame_view v={a,s->window,s->dc,s->item,s->state,&a->detached,&a->bit_verified,&a->report_parent,&a->report_module};
    win16_control_bit_theme_frame(&v,dc,part,rect,clip,address);
}
static void inplace_draw(HDC dc,int part,int state,const RECT *r,const RECT *clip,const void *address,BOOL drop)
{
    static_paint_scope *s=static_paint;if(!s)return;
    const win16_control_vcl_paint_view v={s->window,&s->owner->inplace_candidate,&s->owner->detached,&s->owner->inplace_verified,&s->owner->inplace_owner,&s->owner->inplace_parent,style_control_vcl_inplace_or_lookup,style_control_vcl_lookup_identity,style_control_class_is};
    win16_control_vcl_inplace_draw(&v,dc,part,state,r,clip,address,drop);
}
void ntw16s_nav_theme(HDC dc,int part,int state,const RECT *r,const RECT *clip,const void *address)
{
    static_paint_scope *s=static_paint;if(!s)return;
    const win16_control_vcl_paint_view v={s->window,&s->owner->nav_candidate,&s->owner->detached,&s->owner->nav_verified,&s->owner->nav_owner,&s->owner->nav_parent,style_control_vcl_nav_identity,NULL,style_control_class_is};
    win16_control_vcl_nav_draw(&v,dc,part,state,r,clip,address);
}
BOOL ntw16s_inplace_paint(HDC dc)
{static_paint_scope *s=static_paint;return win16_attachment_inplace_paint(s?s->owner:NULL,s?s->window:NULL,dc);}
void ntw16s_inplace_button(HDC dc,int part,int state,const RECT *r,const RECT *clip,const void *address){inplace_draw(dc,part,state,r,clip,address,FALSE);}
void ntw16s_inplace_dropdown(HDC dc,int part,int state,const RECT *r,const RECT *clip,const void *address){inplace_draw(dc,part,state,r,clip,address,TRUE);}
static BOOL status_identity(HWND w,attachment *a,HMODULE *native)
{
    if(!a)return style_control_status_identity(w,NULL,native);
    const style_control_status_view view={&a->status_candidate,&a->detached,&a->status_witness_owner,&a->status_parent,L"TStatusBar"};
    return style_control_status_identity(w,&view,native);
}

static BOOL status_frame_identity(HWND w,void *owner,HMODULE *native)
{return status_identity(w,(attachment *)owner,native);}
static win16_control_status_text_view status_text_view(status_draw_scope *s)
{
    const win16_control_status_text_view v={s->owner,s,s->window,s->dc,s->native,s->epoch,s->text_count,s->simple,
        s->simple_length,s->simple_text,s->text_panes,s->owner?&s->owner->status_candidate:NULL,
        s->owner?&s->owner->detached:NULL,s->owner?&s->owner->status_epoch:NULL,
        s->owner?&s->owner->status_font:NULL,status_frame_identity};
    return v;
}
BOOL ntw16s_status_theme_begin(ntw16s_status_theme_scope *t,HDC dc,int part,int state,LPCWSTR text,int count,DWORD flags,const RECT *rect,const void *address)
{
    status_draw_scope *s=status_current;
    if(!t || (!s && !status_theme_current))return FALSE;
    memset(t,0,sizeof(*t));t->previous=status_theme_current;status_theme_current=t;
    if(!s)return TRUE;
    const win16_control_status_text_view v=status_text_view(s);
    return win16_control_status_theme_prepare(t,&v,dc,part,state,text,count,flags,rect,address);
}
void ntw16s_status_theme_end(void *data)
{ntw16s_status_theme_scope *t=data;if(status_theme_current==t)status_theme_current=t->previous;}
HFONT ntw16s_status_simple_font(HDC dc,LPCWSTR text,int count,UINT flags,const void *address,void **token)
{
    status_draw_scope *s=status_current;if(!s){if(token)*token=NULL;return NULL;}
    const win16_control_status_text_view v=status_text_view(s);
    return win16_control_status_text_font(&v,status_theme_current,dc,text,count,flags,address,token);
}
void ntw16s_status_simple_return(void *token,HFONT font)
{attachment *a=token;if(a)win16_control_status_font_return(&a->status_font,font);}


static win16_control_status_frame_view status_frame_view(status_draw_scope *s)
{
    const win16_control_status_frame_view view={s->owner,s->window,s->dc,s->native,s->epoch,s->count,
        s->owner?&s->owner->status_epoch:NULL,s->owner?&s->owner->detached:NULL,s->panes,status_frame_identity};
    return view;
}
void ntw16s_status_text_frame(HDC dc,const RECT *text,UINT flags,const void *address)
{status_draw_scope *s=status_current;if(s){const win16_control_status_frame_view v=status_frame_view(s);win16_control_status_text_frame(&v,dc,text,flags,address);}}
void ntw16s_status_icon_frame(HDC dc,int x,int y,int cx,int cy,HICON icon,const void *address)
{status_draw_scope *s=status_current;if(s){const win16_control_status_frame_view v=status_frame_view(s);win16_control_status_icon_frame(&v,dc,x,y,cx,cy,icon,address);}}
void ntw16s_status_frame(HDC dc,const RECT *rect,const RECT *clip,const void *caller)
{status_draw_scope *s=status_current;if(s){const win16_control_status_frame_view v=status_frame_view(s);win16_control_status_frame(&v,dc,rect,clip,caller);}}

BOOL ntw16s_checklist_draw_target(HDC dc,const RECT *r,const void *address)
{return win16_attachment_checklist_draw_target(checklist_current,dc,r,address);}
static LRESULT CALLBACK attachment_proc(HWND,UINT,WPARAM,LPARAM,UINT_PTR,DWORD_PTR);
static void report_try_bind(HWND,attachment *);
static void vcl_report_try_bind(HWND,attachment *);

static attachment *lookup(HWND w)
{
    DWORD_PTR value;
    return GetWindowSubclass(w,attachment_proc,(UINT_PTR)attachment_proc,&value)?(attachment *)value:NULL;
}
static BOOL mdi_host_attached(HWND host){return lookup(host)!=NULL;}
static BOOL mdi_identity(HWND w,HWND parent,HWND host,HMODULE owner)
{return style_control_vcl_mdi_identity(w,parent,host,owner,mdi_host_attached);}
static BOOL controlbar_identity(HWND w,attachment *a)
{
    if(!a)return FALSE;
    const win16_control_controlbar_view v={&a->detached,&a->controlbar_retired,&a->controlbar_parent,&a->controlbar_owner};
    return win16_control_controlbar_identity(w,&v);
}
static BOOL rich_identity(HWND w,attachment *a)
{
    if(!a)return FALSE;
    const win16_control_rich_view v={&a->detached,&a->rich_popup,&a->rich_hotkey,&a->rich_month,&a->rich_unicode,&a->rich_parent,&a->rich_owner};
    return win16_control_rich_identity(w,&v);
}

BOOL ntw16s_vcl_combo_candidate(HWND w)
{return win16_attachment_vcl_combo_candidate(w,lookup(w));}
BOOL ntw16s_vcl_combo_valid(HWND w)
{return win16_attachment_vcl_combo_valid(w,lookup(w));}
BOOL ntw16s_group_button_valid(HWND w)
{return win16_attachment_group_button_valid(w,lookup(w));}
BOOL ntw16s_group_push_button(HWND w)
{return win16_attachment_group_push_button(w,lookup(w));}
BOOL ntw16s_group_choice_button(HWND w)
{return win16_attachment_group_choice_button(w,lookup(w));}

BOOL ntw16s_vb_data_valid(HWND w)
{return win16_attachment_vb_data_valid(w,lookup(w));}
BOOL ntw16s_vb_data_attached(HWND w)
{return win16_attachment_vb_data_attached(w,lookup(w));}


BOOL ntw16s_vb_user_label_target(HWND w)
{return win16_attachment_vb_user_label_target(w,lookup(w));}
BOOL ntw16s_vb_user_edge_dormant(HWND w)
{return win16_attachment_vb_user_edge_dormant(w,lookup(w));}
BOOL ntw16s_vb_user_edge_valid(HWND w)
{return win16_attachment_vb_user_edge_valid(w,lookup(w));}
BOOL ntw16s_grid_valid(HWND w)
{return win16_attachment_grid_valid(w,lookup(w));}
BOOL ntw16s_vcl_toolbar_valid(HWND w)
{return win16_attachment_vcl_toolbar_valid(w,lookup(w));}
BOOL ntw16s_vcl_tab_valid(HWND w)
{return win16_attachment_vcl_tab_valid(w,lookup(w));}
/* Observe only the exact data edit forwarding its own paint to ANSI EDIT. */
void ntw16s_dbedit_ansi_witness(WNDPROC proc,HWND w,UINT message,const void *address)
{win16_attachment_dbedit_ansi_witness(proc,w,message,address,static_paint?static_paint->owner:NULL,static_paint?static_paint->window:NULL,lookup);}
/* Data-aware ANSI combos must forward their own paint to the native class. */
void ntw16s_dbcombo_ansi_witness(WNDPROC proc,HWND w,UINT message,const void *address)
{win16_attachment_dbcombo_ansi_witness(proc,w,message,address,static_paint?static_paint->owner:NULL,static_paint?static_paint->window:NULL,lookup);}
BOOL ntw16s_static_call_begin(ntw16s_static_call *scope,WNDPROC proc,HWND w,UINT message,const void *address)
{
    ZeroMemory(scope,sizeof(*scope));
    HMODULE native_owner=win16_attachment_observe_native(w,message,proc,address,
        static_paint?static_paint->owner:NULL,static_paint?static_paint->window:NULL,
        cool_observing?cool_observing->owner:NULL,cool_observing?cool_observing->window:NULL,
        cool_observing?&cool_observing->native:NULL,lookup);
    if(!native_owner)return FALSE;
    scope->previous=static_call;scope->window=w;scope->native_module=native_owner;
    scope->cancelled=&static_paint->owner->detached;scope->active=TRUE;
    static_call=scope;static_paint->owner->static_verified=TRUE;return TRUE;
}
BOOL ntw16s_static_call_suspend(ntw16s_static_call *scope)
{
    if(!static_call)return FALSE;
    ZeroMemory(scope,sizeof(*scope));scope->previous=static_call;scope->active=TRUE;static_call=scope;return TRUE;
}
void ntw16s_static_call_end(void *data)
{
    ntw16s_static_call *scope=data;if(scope->active){static_call=scope->previous;scope->active=FALSE;}
}
HFONT ntw16s_static_text_font(HDC dc,LPCWSTR text,int count,UINT flags,const void *address)
{
    ntw16s_static_call *s=static_call;
    if(!s || !s->active || !s->cancelled || *s->cancelled || !static_paint || static_paint->window!=s->window)return NULL;
    return win16_control_static_text_font(dc,text,count,flags,address,s->window,s->native_module);
}
#define class_is style_control_class_is
static BOOL vcl_frame_owned(HWND w,void *owner){return lookup(w)==owner;}
static BOOL vcl_frame_current(win16_control_vcl_frame_scope *scope){return static_paint && &static_paint->frame==scope;}
static void group_frame_drawn(void *context)
{win16_control_vcl_frame_result *r=context;attachment *a=r->owner;if(!a->detached){a->group_frame_parent=r->parent;a->group_frame_verified=TRUE;}}
static void panel_frame_drawn(void *context)
{win16_control_vcl_frame_result *r=context;attachment *a=r->owner;if(!a->detached){a->panel_frame_parent=r->parent;a->panel_frame_verified=TRUE;}}
void ntw16s_group_frame(HDC dc,int part,int state,const RECT *rect,const RECT *clip,LPCWSTR text,int count,DWORD flags,const void *caller,BOOL is_text)
{
    static_paint_scope *scope=static_paint;if(!scope || !scope->owner)return;attachment *a=scope->owner;
    const win16_control_vcl_frame_view v={&scope->frame,a,scope->window,&a->detached,&a->group_frame_candidate,&a->group_frame_retired,&a->group_frame_parent,vcl_frame_owned,vcl_frame_current,group_frame_drawn};
    win16_control_vcl_group_frame(&v,dc,part,state,rect,clip,text,count,flags,caller,is_text);
}
void ntw16s_panel_polyline(HDC dc,const POINT *points,int count,const void *caller,BOOL (WINAPI *draw)(HDC,const POINT*,int))
{
    static_paint_scope *scope=static_paint;if(!scope)return;if(!scope->owner){scope->frame.panel_first=FALSE;return;}attachment *a=scope->owner;
    const win16_control_vcl_frame_view v={&scope->frame,a,scope->window,&a->detached,&a->panel_frame_candidate,&a->panel_frame_retired,&a->panel_frame_parent,vcl_frame_owned,vcl_frame_current,panel_frame_drawn};
    win16_control_vcl_panel_polyline(&v,dc,points,count,caller,draw);
}

typedef struct box_witness_scope {struct box_witness_scope *previous;attachment *owner;HWND window;BOOL witnessed,completed,nested;ntw16s_vb_paint *paint;void *status_prior,*spin_prior;} box_witness_scope;
static _Thread_local box_witness_scope *box_current;
void ntw16s_box_witness(HWND w,UINT msg,const void *address)
{box_witness_scope *s=box_current;if(s)win16_attachment_box_witness(w,msg,address,s->owner,s->window,&s->witnessed);}

BOOL ntw16s_controlbar_edge_active(void)
{return box_current && box_current->owner->controlbar_candidate && !box_current->owner->detached;}
static BOOL controlbar_frame_identity(HWND w,void *owner)
{return controlbar_identity(w,(attachment *)owner);}
void ntw16s_controlbar_edge(HDC dc,const RECT *rect,UINT edge,UINT flags,const void *address)
{
    box_witness_scope *s=box_current;if(!s || !s->owner)return;attachment *a=s->owner;
    const win16_control_controlbar_paint_view v={a,s->window,&a->controlbar_candidate,&a->controlbar_verified,&a->controlbar_owner,controlbar_frame_identity};
    win16_control_controlbar_edge(&v,dc,rect,edge,flags,address);
}
BOOL ntw16s_rich_valid(HWND w)
{return win16_attachment_rich_valid(w,lookup(w),rich_identity);}
static _Thread_local BOOL month_query_active;
static void month_query_end(void *unused){(void)unused;month_query_active=FALSE;}
static win16_control_month_view month_view(static_paint_scope *s);
static PROCESS_HOOK_UNWIND_FUNCTION void month_header_query(HWND w,static_paint_scope *scope)
{
 process_hook_unwind_scope unwind;
 if(month_query_active)return;
 scope->month.header_valid=FALSE;month_query_active=TRUE;PROCESS_HOOK_UNWIND_BEGIN(unwind,month_query_end,NULL);
 const win16_control_month_view view=month_view(scope);win16_control_month_query(w,&view);
 PROCESS_HOOK_UNWIND_END(unwind);
}
void ntw16s_rich_witness(WNDPROC proc,HWND w,UINT msg,const void *address,BOOL wide)
{
    static_paint_scope *scope=static_paint;
    if(scope && win16_attachment_rich_witness(proc,w,msg,address,wide,scope->owner,scope->window,rich_identity) && scope->owner->rich_month)
        month_header_query(w,scope);
}
static BOOL month_view_identity(HWND w,void *owner){return rich_identity(w,(attachment *)owner);}
static BOOL month_view_current(void *scope){return static_paint==scope;}
static win16_control_month_view month_view(static_paint_scope *s)
{attachment *a=s->owner;const win16_control_month_view v={s,a,s->window,&s->month,&a->rich_month,&a->rich_witness,&a->detached,&a->month_cached_font,month_view_identity,month_view_current};return v;}
void ntw16s_month_arrow(HTHEME t,HDC dc,int p,int state,const RECT *r,const RECT *clip,const void *addr)
{static_paint_scope *s=static_paint;if(s){const win16_control_month_view v=month_view(s);win16_control_month_arrow(&v,t,dc,p,state,r,clip,addr);}}
HFONT ntw16s_month_text_borrow(HTHEME t,HDC dc,int p,int state,DWORD f,DWORD f2,const RECT *r,const void *addr)
{static_paint_scope *s=static_paint;if(!s)return NULL;const win16_control_month_view v=month_view(s);return win16_control_month_text_borrow(&v,t,dc,p,state,f,f2,r,addr);}
BOOL ntw16s_month_text_room(HTHEME t,HDC dc,int p,int state,DWORD f,DWORD f2,const RECT *r,int width,const void *addr)
{static_paint_scope *s=static_paint;if(!s)return FALSE;const win16_control_month_view v=month_view(s);return win16_control_month_text_room(&v,t,dc,p,state,f,f2,r,width,addr);}
void ntw16s_month_header_text(HTHEME t,HDC dc,int p,int state,const RECT *r,DWORD f,const void *addr)
{static_paint_scope *s=static_paint;if(s){const win16_control_month_view v=month_view(s);win16_control_month_header_text(&v,t,dc,p,state,r,f,addr);}}
void ntw16s_month_selection_background(HTHEME t,HDC dc,int p,int state,const RECT *r,const RECT *clip,const void *addr)
{static_paint_scope *s=static_paint;if(s){const win16_control_month_view v=month_view(s);win16_control_month_selection_background(&v,t,dc,p,state,r,clip,addr);}}
void ntw16s_month_selection_text(HTHEME t,HDC dc,int p,int state,const RECT *r,DWORD f,const void *addr)
{static_paint_scope *s=static_paint;if(s){const win16_control_month_view v=month_view(s);win16_control_month_selection_text(&v,t,dc,p,state,r,f,addr);}}
static void release(attachment *a)
{if(!--a->calls && a->detached){attachment *source=a->lookup_popup_source_attachment,*host=a->mdi_host_attachment;win16_style_system_release(a->month_cached_font);win16_style_system_release(a->status_font.font);ntw16s_vb_fonts_clear(&a->fonts);free(a);if(source)release(source);if(host)release(host);}}
static PROCESS_HOOK_UNWIND_FUNCTION LRESULT static_paint_dispatch(attachment *,HWND,UINT,WPARAM,LPARAM,ntw16s_vb_paint *,void *,void *);
/* One paint scope per CoolBar invocation. It is never shared across threads. */
typedef struct cool_scope {struct cool_scope *previous;attachment *owner;HWND window;BOOL ours,completed,nested;cool_observation observer;ntw16s_vb_paint *paint;void *status_prior,*spin_prior;} cool_scope;
static _Thread_local cool_scope *cool_current;

static BOOL cool_identity(HWND w,attachment *a)
{return win16_attachment_cool_identity(w,a,lookup);}
static BOOL cool_identity_callback(HWND w,void *owner)
{return cool_identity(w,(attachment *)owner);}
static BOOL cool_grip(HWND w,attachment *a,const NMCUSTOMDRAW *n)
{
    WNDPROC native=cool_observing && cool_observing->owner==a && cool_observing->window==w?cool_observing->native:NULL;
    return win16_control_cool_grip_identity(w,a,n,native,cool_identity_callback);
}
static BOOL cool_grip_callback(HWND w,void *owner,const NMCUSTOMDRAW *draw)
{return cool_grip(w,(attachment *)owner,draw) && win16_control_cool_grip(draw->hdc,&draw->rc);}
static LRESULT CALLBACK cool_notify(HWND parent,UINT m,WPARAM p,LPARAM l,UINT_PTR id,DWORD_PTR data)
{
    (void)id;attachment *a=(attachment *)data;cool_scope *s=cool_current;
    return style_control_custom_draw_post(parent,m,p,l,a,s && s->owner==a?s->window:NULL,
        s && s->owner==a?&s->ours:NULL,&a->cool_verified,cool_identity_callback,cool_grip_callback);
}
static void cool_unbind(attachment *a)
{
    DWORD_PTR value;if(a->cool_parent && GetWindowSubclass(a->cool_parent,cool_notify,(UINT_PTR)a->report_window,&value) && value==(DWORD_PTR)a)RemoveWindowSubclass(a->cool_parent,cool_notify,(UINT_PTR)a->report_window);
    a->cool_parent=NULL;
}
static void cool_end(void *data)
{
    cool_scope *s=data;cool_current=s->previous;cool_observing=s->observer.previous;if(!s->completed){ntw16s_vb_paint_end(s->paint);status_current=s->status_prior;spin_current=s->spin_prior;if(!s->nested)release(s->owner);}
}
static PROCESS_HOOK_UNWIND_FUNCTION LRESULT cool_dispatch(attachment *a,HWND w,UINT m,WPARAM p,LPARAM l,ntw16s_vb_paint *paint,void *status_prior,void *spin_prior)
{
    cool_scope scope={.previous=cool_current,.owner=a,.window=w,.paint=paint,.status_prior=status_prior,.spin_prior=spin_prior,.nested=static_paint!=NULL,.observer={.previous=cool_observing,.owner=a,.window=w}};process_hook_unwind_scope unwind;DWORD entry=GetLastError(),error;LRESULT result;
    cool_current=&scope;cool_observing=&scope.observer;PROCESS_HOOK_UNWIND_BEGIN(unwind,cool_end,&scope);SetLastError(entry);
    result=scope.nested?static_paint_dispatch(a,w,m,p,l,NULL,NULL,NULL):DefSubclassProc(w,m,p,l);
    error=GetLastError();scope.completed=TRUE;PROCESS_HOOK_UNWIND_END(unwind);SetLastError(error);return result;
}
static void static_paint_end(void *data)
{static_paint_scope *scope=data;static_paint=scope->previous;if(!scope->completed){if(scope->owned_paint){ntw16s_vb_paint_end(scope->owned_paint);status_current=scope->status_prior;spin_current=scope->spin_prior;}release(scope->owner);}}
static PROCESS_HOOK_UNWIND_FUNCTION LRESULT static_paint_dispatch(attachment *a,HWND w,UINT m,WPARAM p,LPARAM l,ntw16s_vb_paint *owned_paint,void *status_prior,void *spin_prior)
{
    static_paint_scope scope={.previous=static_paint,.owner=a,.window=w,.owned_paint=owned_paint,.status_prior=status_prior,.spin_prior=spin_prior};process_hook_unwind_scope unwind;LRESULT result;DWORD entry=GetLastError(),error;
    static_paint=win16_attachment_static_paint(a,m)?&scope:NULL;
    PROCESS_HOOK_UNWIND_BEGIN(unwind,static_paint_end,&scope);
    SetLastError(entry);result=DefSubclassProc(w,m,p,l);error=GetLastError();scope.completed=TRUE;
    PROCESS_HOOK_UNWIND_END(unwind);SetLastError(error);return result;
}
typedef struct lookup_popup_unwind {HWND previous;attachment *owner;ntw16s_vb_paint *paint;void *status_prior,*spin_prior;BOOL nested,completed;} lookup_popup_unwind;
static void lookup_popup_unwind_end(void *data)
{lookup_popup_unwind *s=data;lookup_popup_creator=s->previous;if(!s->completed){ntw16s_vb_paint_end(s->paint);status_current=s->status_prior;spin_current=s->spin_prior;if(!s->nested)release(s->owner);}}
static PROCESS_HOOK_UNWIND_FUNCTION LRESULT lookup_popup_dispatch(attachment *a,HWND w,UINT m,WPARAM p,LPARAM l,ntw16s_vb_paint *paint,void *status_prior,void *spin_prior)
{lookup_popup_unwind state={.previous=lookup_popup_creator,.owner=a,.paint=paint,.status_prior=status_prior,.spin_prior=spin_prior,.nested=static_paint!=NULL};process_hook_unwind_scope unwind;DWORD entry=GetLastError(),error;lookup_popup_creator=w;PROCESS_HOOK_UNWIND_BEGIN(unwind,lookup_popup_unwind_end,&state);SetLastError(entry);LRESULT result=state.nested?static_paint_dispatch(a,w,m,p,l,NULL,NULL,NULL):DefSubclassProc(w,m,p,l);error=GetLastError();state.completed=TRUE;PROCESS_HOOK_UNWIND_END(unwind);SetLastError(error);return result;}
static void box_witness_end(void *data)
{box_witness_scope *s=data;box_current=s->previous;if(!s->completed){ntw16s_vb_paint_end(s->paint);status_current=s->status_prior;spin_current=s->spin_prior;if(!s->nested)release(s->owner);}}
static PROCESS_HOOK_UNWIND_FUNCTION LRESULT box_dispatch(attachment *a,HWND w,UINT m,WPARAM p,LPARAM l,ntw16s_vb_paint *paint,void *status_prior,void *spin_prior,BOOL *witnessed)
{box_witness_scope scope={.previous=box_current,.owner=a,.window=w,.nested=static_paint!=NULL,.paint=paint,.status_prior=status_prior,.spin_prior=spin_prior};process_hook_unwind_scope unwind;DWORD entry=GetLastError(),error;LRESULT result;box_current=&scope;PROCESS_HOOK_UNWIND_BEGIN(unwind,box_witness_end,&scope);SetLastError(entry);result=scope.nested?static_paint_dispatch(a,w,m,p,l,NULL,NULL,NULL):DefSubclassProc(w,m,p,l);error=GetLastError();*witnessed=scope.witnessed;scope.completed=TRUE;PROCESS_HOOK_UNWIND_END(unwind);SetLastError(error);return result;}
typedef struct modern_spin_unwind {
    spin_draw_scope *scope,*prior;status_draw_scope *status_prior;ntw16s_vb_paint *paint;attachment *owner;BOOL completed,nested;
} modern_spin_unwind;
static void modern_spin_end(void *data)
{
    modern_spin_unwind *s=data;if(!s->completed){spin_current=s->prior;spin_stack=s->scope->previous_stack;
        win16_control_spin_buffers_clear(s->scope);status_current=s->status_prior;ntw16s_vb_paint_end(s->paint);if(!s->nested)release(s->owner);}
}
static PROCESS_HOOK_UNWIND_FUNCTION LRESULT modern_spin_dispatch(attachment *a,HWND w,UINT m,WPARAM p,LPARAM l,spin_draw_scope *scope,spin_draw_scope *prior,status_draw_scope *status_prior,ntw16s_vb_paint *paint)
{
    modern_spin_unwind state={scope,prior,status_prior,paint,a,FALSE,static_paint!=NULL};process_hook_unwind_scope unwind;
    DWORD entry=GetLastError(),error;LRESULT result;PROCESS_HOOK_UNWIND_BEGIN(unwind,modern_spin_end,&state);
    SetLastError(entry);result=state.nested?static_paint_dispatch(a,w,m,p,l,NULL,NULL,NULL):DefSubclassProc(w,m,p,l);
    error=GetLastError();state.completed=TRUE;PROCESS_HOOK_UNWIND_END(unwind);SetLastError(error);return result;
}
typedef struct color_print_scope {attachment *owner;HRGN clip;BOOL completed,nested;ntw16s_vb_paint *paint;status_draw_scope *status_prior;spin_draw_scope *spin_prior;} color_print_scope;
static void color_print_end(void *data)
{
    color_print_scope *scope=data;
    if(!scope->completed){ntw16s_vb_paint_end(scope->paint);status_current=scope->status_prior;spin_current=scope->spin_prior;DeleteObject(scope->clip);if(!scope->nested)release(scope->owner);}
}
static PROCESS_HOOK_UNWIND_FUNCTION LRESULT color_print_dispatch(attachment *a,HWND w,UINT m,WPARAM p,LPARAM l,HRGN clip,ntw16s_vb_paint *paint,status_draw_scope *status_prior,spin_draw_scope *spin_prior)
{
    color_print_scope scope={a,clip,FALSE,static_paint!=NULL,paint,status_prior,spin_prior};process_hook_unwind_scope unwind;
    DWORD entry=GetLastError(),error;LRESULT result;
    PROCESS_HOOK_UNWIND_BEGIN(unwind,color_print_end,&scope);
    SetLastError(entry);result=scope.nested?static_paint_dispatch(a,w,m,p,l,NULL,NULL,NULL):DefSubclassProc(w,m,p,l);
    error=GetLastError();scope.completed=TRUE;PROCESS_HOOK_UNWIND_END(unwind);SetLastError(error);return result;
}
typedef struct status_print_unwind {attachment *owner;ntw16s_vb_paint *paint;status_draw_scope *prior;spin_draw_scope *spin_prior;BOOL completed,nested;} status_print_unwind;
static void status_print_end(void *data)
{status_print_unwind *s=data;if(!s->completed){ntw16s_vb_paint_end(s->paint);status_current=s->prior;spin_current=s->spin_prior;if(!s->nested)release(s->owner);}}
static PROCESS_HOOK_UNWIND_FUNCTION LRESULT status_print_dispatch(attachment *a,HWND w,UINT m,WPARAM p,LPARAM l,ntw16s_vb_paint *paint,status_draw_scope *prior,spin_draw_scope *spin_prior)
{status_print_unwind state={a,paint,prior,spin_prior,FALSE,static_paint!=NULL};process_hook_unwind_scope unwind;DWORD entry=GetLastError(),error;LRESULT result;
 PROCESS_HOOK_UNWIND_BEGIN(unwind,status_print_end,&state);SetLastError(entry);result=state.nested?static_paint_dispatch(a,w,m,p,l,NULL,NULL,NULL):DefSubclassProc(w,m,p,l);error=GetLastError();state.completed=TRUE;PROCESS_HOOK_UNWIND_END(unwind);SetLastError(error);return result;}
static BOOL color_identity(HWND w,attachment *a,COMBOBOXINFO *info)
{const style_control_native_combo_view v={&a->detached,&a->color_parent,&a->color_witness_owner};return style_control_color_identity(w,&v,info);}
static void color_button(HWND w,attachment *a,HDC dc,const RECT *dirty)
{const style_control_native_combo_view v={&a->detached,&a->color_parent,&a->color_witness_owner};win16_control_color_button(w,&v,dc,dirty);}
static BOOL date_identity(HWND w,attachment *a,DATETIMEPICKERINFO *info)
{const style_control_native_combo_view v={&a->detached,&a->date_parent,&a->date_witness_owner};return style_control_date_identity(w,&v,info);}
static void date_button(HWND w,attachment *a,HDC dc,const RECT *dirty)
{const style_control_native_combo_view v={&a->detached,&a->date_parent,&a->date_witness_owner};win16_control_date_button(w,&v,dc,dirty);}
static LRESULT CALLBACK attachment_proc(HWND w,UINT m,WPARAM p,LPARAM l,UINT_PTR id,DWORD_PTR data)
{
    attachment *a=(attachment *)data;LRESULT result;DWORD status_error=GetLastError();BOOL status=a->status||a->spin,color=a->color_candidate||a->date_candidate;(void)id;
    win16_attachment_retire_frames(w,a);
    RECT color_dirty;BOOL color_update;style_clip_snapshot print_clip;
    win16_attachment_native_prepare(w,m,p,a,color,&color_dirty,&color_update,&print_clip);
    ++a->calls;
    const win16_attachment_guard_services guards={lookup,mdi_identity,cool_identity,Ntw16sWindowRemove};
    win16_attachment_guard_cool(w,m,a,&guards);
    if(a->status)style_control_status_invalidate(m,&a->status_font.invalid,&a->status_epoch);
    status_draw_scope scope={0},*prior=status_current;
    spin_draw_scope spin_scope={0},*spin_prior=spin_current;
    BOOL spin_message=win16_attachment_spin_message(a,m);
    if(spin_prior)spin_current=NULL;
    if(spin_message){spin_scope.modern_spin=&a->modern_spin;spin_scope.detached=&a->detached;spin_scope.verified=&a->modern_spin_verified;spin_scope.window=w;spin_scope.parent=GetParent(w);
        spin_scope.dc=(m==WM_PRINT || m==WM_PRINTCLIENT)?(HDC)p:NULL;spin_current=&spin_scope;
        spin_scope.previous_stack=spin_stack;spin_stack=&spin_scope;}
    BOOL painting=a->status && !a->detached && (m==WM_PAINT || m==WM_PRINT || m==WM_PRINTCLIENT);
    if(prior && prior->window!=w)status_current=NULL;
    if(painting){
        scope.previous=status_current;scope.owner=a;scope.window=w;scope.dc=m==WM_PAINT?NULL:(HDC)p;scope.epoch=a->status_epoch;
        const style_control_status_snapshot_view snapshot={a,&a->detached,&a->status_epoch,scope.epoch,
            &scope.native,&scope.simple,&scope.simple_length,scope.simple_text,&scope.text_count,scope.text_panes,
            &scope.count,scope.panes,status_frame_identity};
        style_control_status_snapshot(w,&snapshot);
        status_current=&scope;
    }
    win16_attachment_guard_roles(w,m,a,&guards);
    if(m==WM_NCDESTROY)(void)Ntw16sWindowTerminal(w);
    ntw16s_vb_paint paint;win16_control_vb_paint_begin(&paint,w,m,&a->fonts,ntw16s_vb_user_label_target);
    win16_attachment_prepare_report(w,m,p,a,&paint,report_try_bind);
    BOOL box_witnessed=FALSE;
    win16_attachment_reset_witness(a,m);
    if(status || color)SetLastError(status_error);
    switch(win16_attachment_dispatch_kind(w,a,m,p,l,status_current==&scope,print_clip.region!=NULL,spin_message,static_paint!=NULL)){
    case WIN16_ATTACHMENT_DISPATCH_COOL:result=cool_dispatch(a,w,m,p,l,&paint,prior,spin_prior);break;
    case WIN16_ATTACHMENT_DISPATCH_STATUS:result=status_print_dispatch(a,w,m,p,l,&paint,prior,spin_prior);break;
    case WIN16_ATTACHMENT_DISPATCH_COLOR:result=color_print_dispatch(a,w,m,p,l,print_clip.region,&paint,prior,spin_prior);break;
    case WIN16_ATTACHMENT_DISPATCH_SPIN:result=modern_spin_dispatch(a,w,m,p,l,&spin_scope,spin_prior,prior,&paint);break;
    case WIN16_ATTACHMENT_DISPATCH_BOX:result=box_dispatch(a,w,m,p,l,&paint,prior,spin_prior,&box_witnessed);break;
    case WIN16_ATTACHMENT_DISPATCH_LOOKUP:result=lookup_popup_dispatch(a,w,m,p,l,&paint,prior,spin_prior);break;
    case WIN16_ATTACHMENT_DISPATCH_STATIC:result=static_paint_dispatch(a,w,m,p,l,&paint,prior,spin_prior);break;
    default:result=DefSubclassProc(w,m,p,l);break;
    }
    if(status || color)status_error=GetLastError();
    ntw16s_vb_paint_end(&paint);
    status_current=prior;
    if(spin_message){spin_stack=spin_scope.previous_stack;win16_control_spin_buffers_clear(&spin_scope);}
    spin_current=spin_prior;
    const win16_attachment_post_services post={win16_style_grid_install_created,win16_style_control_remove_owned,rich_identity,ntw16s_native_get_scroll_info};
    win16_attachment_post_box(w,a,box_witnessed,&post);
    const win16_attachment_verified_services verified={win16_style_toolbar_install_created,win16_style_control_install_created,
        win16_style_trackbar_install_created,win16_style_control_remove_owned,win16_attachment_combo_same_role,ntw16s_theme_enable,
        win16_style_edit_scrollbars_enable,ntw16s_theme_remove,vcl_report_try_bind};
    win16_attachment_post_toolbar(w,m,a,&verified);
    win16_attachment_post_controls(w,m,a,&post);
    win16_attachment_post_verified(w,m,a,&verified);
    const win16_attachment_native_services native={color_identity,status_identity,date_identity,color_button,date_button};
    win16_attachment_post_native(w,m,p,a,&print_clip,color_update,&color_dirty,&native);
    release(a);if(status || color)SetLastError(status_error);return result;
}
static void bit_draw_end(void *data)
{
    bit_draw_scope *scope=data;bit_current=scope->previous;
    if(!scope->completed)release(scope->owner);
}
static PROCESS_HOOK_UNWIND_FUNCTION LRESULT bit_draw_dispatch(attachment *a,HWND w,UINT m,WPARAM p,LPARAM l,const DRAWITEMSTRUCT *item)
{
    bit_draw_scope scope={bit_current,a,item->hwndItem,item->hDC,item->rcItem,item->itemState,FALSE};process_hook_unwind_scope unwind;
    DWORD entry=GetLastError(),error;LRESULT result;bit_current=&scope;
    PROCESS_HOOK_UNWIND_BEGIN(unwind,bit_draw_end,&scope);
    SetLastError(entry);result=DefSubclassProc(w,m,p,l);error=GetLastError();scope.completed=TRUE;
    PROCESS_HOOK_UNWIND_END(unwind);SetLastError(error);return result;
}
/* Observe this verified child's native custom/owner-draw DC. Never request extra stages,
 * rewrite the application's result, select its font, or inspect item lParam. */
static LRESULT CALLBACK report_notify(HWND w,UINT m,WPARAM p,LPARAM l,UINT_PTR id,DWORD_PTR data)
{
    const win16_attachment_notify_services services={report_notify,bit_draw_dispatch,release,ntw16s_vb_user_label_target,&checklist_current};
    return win16_attachment_report_notify(w,m,p,l,id,data,&services);
}
/* Some frameworks set the report view after native WM_CREATE. This is still
 * our exact newly created attachment, on its GUI thread, after initialization.
 * A failed public protocol remains native; there is no existing-window scan. */
static const win16_attachment_report_services report_services={report_notify,win16_style_tree_install_created,
    win16_style_install_created,Ntw16sWindowRemove};
static void report_try_bind(HWND w,attachment *a)
{win16_attachment_report_bind(w,a,&report_services);}
static void vcl_report_try_bind(HWND w,attachment *a)
{win16_attachment_vcl_report_bind(w,a,&report_services);}
static void report_unbind(attachment *a)
{win16_attachment_report_unbind(a,&report_services);}
static const win16_attachment_lifecycle_services lifecycle_services={lookup,cool_identity,mdi_identity,controlbar_identity,rich_identity,
    ntw16s_vb_user_label_target,attachment_proc,report_unbind,cool_unbind,win16_style_frame_remove_owned,
    win16_style_control_remove_owned,ntw16s_theme_remove,win16_style_frame_terminal,win16_style_control_terminal,
    style_theme_terminal,release,win16_style_scrollbars_paint};
BOOL __cdecl Ntw16sWindowAttached(HWND w)
{return win16_attachment_attached(w,&lifecycle_services);}
void __cdecl Ntw16sScrollChanged(HWND w)
{win16_attachment_scroll_changed(w,&lifecycle_services);}
static BOOL composed_frame_class(HWND w)
{
    return class_is(w,L"CASCADIA_HOSTING_WINDOW_CLASS") ||
        class_is(w,L"Chrome_WidgetWin_1") || class_is(w,L"MSPaintApp") ||
        class_is(w,L"XamlWindow");
}
BOOL __cdecl Ntw16sWindowRemove(HWND w)
{
    if(composed_frame_class(w) && win16_uwp_frame_surface(w) &&
       !win16_uwp_frame_remove(w))return FALSE;
    return win16_attachment_remove(w,&lifecycle_services);
}
BOOL __cdecl Ntw16sWindowTerminal(HWND w)
{return win16_attachment_terminal(w,&lifecycle_services);}

static BOOL install_created(HWND w,DWORD original_style)
{
    BOOL ready=InterlockedCompareExchange(&status_hooks_ready,0,0)!=0;
    BOOL control,edit,installed,report=FALSE,tree=FALSE,legacy_progress=FALSE,picture=ntw16s_vb_picture(w),checklist=style_control_vcl_checklist_identity(w),status=ready && status_identity(w,NULL,NULL),spin=ready && (style_control_vcl_spin_identity(w)||style_control_vcl_modern_spin_shape(w));HWND parent;HMODULE owner;attachment *a;DWORD error;
    style_control_protocol *protocol=NULL;
    if(lookup(w))return TRUE;
    /* COMBOBOX installs its own internal list when construction finishes.
     * The internal EDIT also belongs to that native composite, not a frame. */
    for(parent=GetParent(w);parent;parent=GetParent(parent))
        if(class_is(parent,L"COMBOBOX"))return TRUE;
    if(class_is(w,L"ComboLBox") || class_is(w,L"#32768"))return TRUE;
    win16_attachment_initial_classification initial;
    if(!win16_attachment_classify_initial(w,original_style,ready,checklist,status,spin,&initial))return FALSE;
    edit=initial.edit;control=initial.control;report=initial.report;tree=initial.tree;
    legacy_progress=initial.legacy_progress;protocol=initial.protocol;owner=initial.owner;
    win16_control_vcl_candidates candidates;
    win16_attachment_classify_vcl(w,original_style,ready,picture,checklist,status,spin,&initial,&candidates);
    HWND popup_source=lookup_popup_creator,popup_source_parent=popup_source?GetParent(popup_source):NULL;
    attachment *popup_source_attachment=popup_source?lookup(popup_source):NULL;
    BOOL lookup_popup_candidate=ready && popup_source_attachment && style_control_vcl_lookup_popup_identity(w,popup_source,popup_source_parent,owner);
    win16_control_vcl_extra_candidates extra;
    win16_control_vcl_classify_extra(w,original_style,ready,owner,&extra);
    BOOL cool_candidate=extra.cool_candidate;
    BOOL vcl_mdi_probe=mdi_identity(w,GetParent(w),GetParent(GetParent(w)),owner);
    int rejected=win16_attachment_eligibility(original_style,picture,checklist,status,spin,vcl_mdi_probe,&initial,&candidates,&extra);
    if(rejected){if(rejected==1)style_control_protocol_release(protocol);SetLastError(ERROR_NOT_SUPPORTED);return FALSE;}
    a=calloc(1,sizeof(*a));
    if(!a){style_control_protocol_release(protocol);SetLastError(ERROR_NOT_ENOUGH_MEMORY);return FALSE;}
    const win16_attachment_seed seed={&initial,&candidates,&extra,picture,checklist,status,spin,
        vcl_mdi_probe,lookup_popup_candidate,popup_source,popup_source_parent,popup_source_attachment,lookup};
    win16_attachment_initialize(w,original_style,a,&seed);
    if(!SetWindowSubclass(w,attachment_proc,(UINT_PTR)attachment_proc,(DWORD_PTR)a)){
        DWORD failure=GetLastError();style_control_protocol_release(protocol);if(a->lookup_popup_source_attachment)release(a->lookup_popup_source_attachment);if(a->mdi_host_attachment)release(a->mdi_host_attachment);free(a);SetLastError(failure);return FALSE;
    }
    if((report || tree || a->file_owner || a->button_owner || a->checklist || a->bit_candidate) && (!a->report_parent || !SetWindowSubclass(a->report_parent,report_notify,(UINT_PTR)w,(DWORD_PTR)a))){
        DWORD error=GetLastError();Ntw16sWindowRemove(w);release(a);SetLastError(error);return FALSE;
    }
    if(cool_candidate && (!cool_identity(w,a) || !SetWindowSubclass(a->cool_parent,cool_notify,(UINT_PTR)w,(DWORD_PTR)a))){
        DWORD failure=GetLastError();Ntw16sWindowRemove(w);release(a);SetLastError(failure?failure:ERROR_NOT_SUPPORTED);return FALSE;
    }
    SetLastError(ERROR_SUCCESS);
    if(protocol && class_is(w,L"COMBOBOX")){
        COMBOBOXINFO info={.cbSize=sizeof(info)};
        if(ntw16s_native_get_combo_info(w,&info) && info.hwndItem!=w && lookup(info.hwndItem))
            Ntw16sWindowRemove(info.hwndItem);
    }
    if(a->detached){release(a);SetLastError(ERROR_CANCELLED);return FALSE;}
    const win16_attachment_install_services install_services={win16_style_legacy_progress_border_install_created,
        win16_style_toolbar_install_created,win16_style_updown_install_created,win16_style_trackbar_install_created,
        win16_style_progress_install_created,win16_style_tree_install_created,win16_style_picture_install_created,
        win16_style_grid_install_created,win16_style_frame_scrollbars_install_owned,win16_style_control_install_created,
        win16_style_install_created,ntw16s_native_get_combo_info,ntw16s_theme_enable,win16_style_list_scrollbar_enable,
        win16_style_edit_scrollbars_enable,ntw16s_theme_remove,Ntw16sWindowRemove};
    installed=win16_attachment_install_style(w,a,protocol,original_style,legacy_progress,tree,picture,control,edit,report,&error,&install_services);
    release(a);
    if(installed && lookup(w) &&
       (class_is(w,L"CASCADIA_HOSTING_WINDOW_CLASS") ||
        (composed_frame_class(w) &&
         (GetWindowLongPtrW(w,GWL_STYLE)&WS_CAPTION)==WS_CAPTION)) &&
       (GetWindowLongPtrW(w,GWL_EXSTYLE)&WS_EX_NOREDIRECTIONBITMAP) &&
       !win16_uwp_frame_apply_nonclient(w)){
        error=GetLastError();Ntw16sWindowRemove(w);installed=FALSE;
    }
    if(!installed)SetLastError(error?error:ERROR_GEN_FAILURE);
    return installed;
}
BOOL __cdecl Ntw16sWindowAttach(HWND w)
{
    if(!w || !win32_style_window_current_thread(w)){
        SetLastError(ERROR_INVALID_THREAD_ID);return FALSE;
    }
    if(lookup(w)){SetLastError(ERROR_ALREADY_EXISTS);return FALSE;}
    if(!install_created(w,(DWORD)GetWindowLongPtrW(w,GWL_STYLE)))return FALSE;
    if(!lookup(w)){SetLastError(ERROR_NOT_SUPPORTED);return FALSE;}
    return TRUE;
}
static DWORD window_thread_perform(const CWPSTRUCT *message)
{
    DWORD error=0;HMODULE pinned=NULL;BOOL ok=FALSE;
    if(!win32_style_window_current_thread(message->hwnd))
        return ERROR_INVALID_THREAD_ID;
    SetLastError(0);
    if(message->wParam==NTW16S_WINDOW_THREAD_REMOVE)ok=Ntw16sWindowRemove(message->hwnd);
    else if(Ntw16sWindowAttached(message->hwnd))ok=TRUE;
    else if(GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_PIN,
              (PCWSTR)(uintptr_t)Ntw16sWindowThreadHook,&pinned))ok=Ntw16sWindowAttach(message->hwnd);
    if(!ok)error=GetLastError()?GetLastError():ERROR_GEN_FAILURE;
    return error;
}
LRESULT CALLBACK Ntw16sWindowThreadHook(int code,WPARAM wp,LPARAM lp)
{
    static _Thread_local BOOL busy;
    DWORD saved=GetLastError();
    const CWPSTRUCT *message=win32_style_window_hook_match(code,lp,busy,NTW16S_WINDOW_THREAD_MESSAGE);
    if(message && (message->wParam==NTW16S_WINDOW_THREAD_ATTACH || message->wParam==NTW16S_WINDOW_THREAD_REMOVE) &&
       (HWND)message->lParam==message->hwnd){
        busy=TRUE;
        DWORD error=window_thread_perform(message);
        win32_style_window_hook_finish(message->hwnd,NTW16S_WINDOW_THREAD_RESULT,error);
        busy=FALSE;
    }
    LRESULT result=CallNextHookEx(NULL,code,wp,lp);SetLastError(saved);return result;
}
static LRESULT CALLBACK creation_before(int code,WPARAM p,LPARAM l)
{
    ntw16s_creation_scope *scope=current;DWORD saved=GetLastError();
    if(code==HCBT_CREATEWND && scope && scope->enabled){
        const CBT_CREATEWNDW *create=(const CBT_CREATEWNDW *)l;
        pending_window *pending=win32_style_creation_add(&scope->pending,(HWND)p,(DWORD)create->lpcs->style);
        if(!pending){scope->result.failure=ERROR_NOT_ENOUGH_MEMORY;return 1;}
#ifdef NTW16S_EXPERIMENTAL_CREATION_SUBCLASS
        if(!creation_return_hook && !SetWindowSubclass((HWND)p,creation_subclass,(UINT_PTR)creation_subclass,0)){
            scope->result.failure=GetLastError()?GetLastError():ERROR_GEN_FAILURE;return 1;
        }
#endif
        if(class_is((HWND)p,L"LISTBOX") || class_is((HWND)p,L"ComboLBox") ||
           (class_is((HWND)p,L"EDIT") && (pending->style&ES_MULTILINE))){
            if(!style_theme_capture((HWND)p)){
                scope->result.failure=GetLastError();return 1;
            }
        }else if((pending->style&WS_CHILD) && !style_theme_stage((HWND)p)){
            scope->result.failure=GetLastError();return 1;
        }
    }
    SetLastError(saved);return CallNextHookEx(NULL,code,p,l);
}
#ifdef NTW16S_EXPERIMENTAL_CREATION_SUBCLASS
static LRESULT creation_observe(int code,WPARAM p,LPARAM l,BOOL synthetic)
{
    LRESULT result=synthetic?0:CallNextHookEx(NULL,code,p,l);DWORD saved=GetLastError();
#else
static LRESULT CALLBACK creation_return(int code,WPARAM p,LPARAM l)
{
    LRESULT result=CallNextHookEx(NULL,code,p,l);DWORD saved=GetLastError();
#endif
    ntw16s_creation_scope *scope=current;
    if(code>=0 && scope && scope->enabled){
        const CWPRETSTRUCT *m=(const CWPRETSTRUCT *)l;
        BOOL created=m->message==WM_CREATE && m->lResult!=-1;
        /* Dialog managers finish layout and format their caption during
         * WM_INITDIALOG. Attach after that native initialization (ShellAbout
         * also formats its caption there); never rewrite application text.
         * Keep its pending style until the real dialog initialization exits. */
        if(created && class_is(m->hwnd,L"#32770")){
            SetLastError(saved);return result;
        }
        if(m->message==WM_NCDESTROY){pending_window *pending=win32_style_creation_take(&scope->pending,m->hwnd);style_theme_cancel_stage(m->hwnd);free(pending);}
        /* Some dialogs show themselves and run a nested loop inside
         * WM_INITDIALOG. Its return may be delayed until the application exits.
         * A completed first-show notification is also a safe boundary, but
         * only for a captioned top-level dialog owned by this creation chain.
         * Mark its pending node before installation can reenter USER. */
        BOOL first_show=(m->message==WM_SHOWWINDOW && m->wParam) ||
            (m->message==WM_WINDOWPOSCHANGED && m->lParam &&
             (((const WINDOWPOS *)m->lParam)->flags&SWP_SHOWWINDOW));
        ntw16s_creation_scope *owner=scope;
        if(first_show){
            DWORD style=(DWORD)GetWindowLongPtrW(m->hwnd,GWL_STYLE);
            first_show=(style&WS_VISIBLE) && !(style&WS_CHILD) &&
                (style&WS_CAPTION)==WS_CAPTION && class_is(m->hwnd,L"#32770");
            if(first_show){
                for(;owner;owner=owner->previous){
                    pending_window *pending=win32_style_creation_find(owner->pending,m->hwnd);
                    if(owner->enabled && pending && !pending->settled)break;
                }
                first_show=owner!=NULL;
            }
        }
        if((created || m->message==WM_INITDIALOG || first_show) && IsWindow(m->hwnd)){
            if(!first_show)owner=scope;
            pending_window *pending;
            if(first_show){
                pending=win32_style_creation_find(owner->pending,m->hwnd);
                if(pending)pending->settled=TRUE;
            }else pending=win32_style_creation_take(&owner->pending,m->hwnd);
            /* A first-show attachment has already completed this creation.
             * In particular, do not undo an explicit removal by attaching
             * again when the delayed initializer finally returns. */
            if(!first_show && pending && pending->settled && pending->installed){free(pending);SetLastError(saved);return result;}
            DWORD style=created?(DWORD)((const CREATESTRUCTW *)m->lParam)->style:
                (DWORD)GetWindowLongPtrW(m->hwnd,GWL_STYLE);
            if(pending){style=pending->style;if(!first_show)free(pending);}
            BOOL already=lookup(m->hwnd)!=NULL;
            BOOL installed=style_theme_commit(m->hwnd) && install_created(m->hwnd,style);
            if(installed){
                if(first_show && win32_style_creation_contains(owner->pending,pending,m->hwnd))pending->installed=TRUE;
                if(!already && lookup(m->hwnd))++owner->result.installed;
            }else{
                DWORD error=GetLastError();
                if(error==ERROR_NOT_SUPPORTED){
                    ++owner->result.unsupported;ntw16s_theme_remove(m->hwnd,NULL);
                    if(class_is(m->hwnd,L"COMBOBOX")){
                        COMBOBOXINFO info={.cbSize=sizeof(info)};
                        if(ntw16s_native_get_combo_info(m->hwnd,&info))ntw16s_theme_remove(info.hwndList,NULL);
                    }
                }
                else if(!owner->result.failure)owner->result.failure=error?error:ERROR_GEN_FAILURE;
            }
            if(owner->result.failure && m->message==WM_INITDIALOG)
                EndDialog(m->hwnd,-1);
        }
    }
    SetLastError(saved);return result;
}
#ifdef NTW16S_EXPERIMENTAL_CREATION_SUBCLASS
static LRESULT CALLBACK creation_return(int code,WPARAM p,LPARAM l)
{
    LRESULT result=creation_observe(code,p,l,FALSE);
    if(code>=0 && l && ((const CWPRETSTRUCT *)l)->message==WM_INITDIALOG && creation_dialog_observer){
        HWND w=((const CWPRETSTRUCT *)l)->hwnd;DWORD error=GetLastError();
        if(IsWindow(w))creation_dialog_observer(w);
        SetLastError(error);
    }
    return result;
}
static ntw16s_creation_scope *creation_begin(BOOL enabled,BOOL dialog)
#else
ntw16s_creation_scope *__cdecl Ntw16sCreationBegin(BOOL enabled)
#endif
{
    ntw16s_creation_scope *scope=calloc(1,sizeof(*scope));
    if(!scope){SetLastError(ERROR_NOT_ENOUGH_MEMORY);return NULL;}
    if(!current){
        creation_before_hook=SetWindowsHookExW(WH_CBT,creation_before,NULL,GetCurrentThreadId());
        if(!creation_before_hook){free(scope);return NULL;}
#ifndef NTW16S_EXPERIMENTAL_CREATION_SUBCLASS
        creation_return_hook=style_message_observer_begin(creation_return);
        if(!creation_return_hook){DWORD error=GetLastError();UnhookWindowsHookEx(creation_before_hook);creation_before_hook=NULL;free(scope);SetLastError(error);return NULL;}
#endif
    }
#ifdef NTW16S_EXPERIMENTAL_CREATION_SUBCLASS
    if(dialog && creation_return_hook){
        if(!UnhookWindowsHookEx(creation_return_hook)){DWORD error=GetLastError();free(scope);SetLastError(error);return NULL;}
        creation_return_hook=NULL;scope->suspended_return_hook=TRUE;
    }
    if(!dialog && !creation_return_hook){
        creation_return_hook=style_message_observer_begin(creation_return);
        if(!creation_return_hook){DWORD error=GetLastError();if(!current){UnhookWindowsHookEx(creation_before_hook);creation_before_hook=NULL;}free(scope);SetLastError(error);return NULL;}
        scope->owns_return_hook=TRUE;
    }
#endif
    scope->thread=GetCurrentThreadId();scope->enabled=enabled;
    scope->previous=current;current=scope;return scope;
}
#ifdef NTW16S_EXPERIMENTAL_CREATION_SUBCLASS
ntw16s_creation_scope *__cdecl Ntw16sCreationBegin(BOOL enabled)
{return creation_begin(enabled,FALSE);}
ntw16s_creation_scope *ntw16s_creation_begin_dialog(BOOL enabled)
{return creation_begin(enabled,TRUE);}
#endif
BOOL __cdecl Ntw16sCreationEnd(ntw16s_creation_scope *scope,ntw16s_creation_result *result)
{
    if(!scope || scope!=current){SetLastError(ERROR_INVALID_STATE);return FALSE;}
    if(scope->thread!=GetCurrentThreadId()){SetLastError(ERROR_INVALID_THREAD_ID);return FALSE;}
    current=scope->previous;if(result)*result=scope->result;
    while(scope->pending){pending_window *pending=scope->pending;scope->pending=pending->next;
#ifdef NTW16S_EXPERIMENTAL_CREATION_SUBCLASS
        if(IsWindow(pending->window))RemoveWindowSubclass(pending->window,creation_subclass,(UINT_PTR)creation_subclass);
#endif
        style_theme_cancel_stage(pending->window);free(pending);}
#ifdef NTW16S_EXPERIMENTAL_CREATION_SUBCLASS
    if(scope->owns_return_hook){style_message_observer_end(creation_return_hook);creation_return_hook=NULL;}
    if(scope->suspended_return_hook){
        creation_return_hook=style_message_observer_begin(creation_return);
        if(!creation_return_hook){DWORD error=GetLastError()?GetLastError():ERROR_GEN_FAILURE;
            if(result && !result->failure)result->failure=error;
            if(current && !current->result.failure)current->result.failure=error;}
    }
#endif
    if(!current){style_message_observer_end(creation_return_hook);creation_return_hook=NULL;
        UnhookWindowsHookEx(creation_before_hook);creation_before_hook=NULL;}
    free(scope);return TRUE;
}
