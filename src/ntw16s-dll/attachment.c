#include "lib-win32-style/application/adapters/win32/message.h"
#include "lib-win32-style/application/adapters/win32/paint_dispatch.h"
#include "lib-win32-style/application/control/control_vcl_identity.h"
#include "../lib-win32-style/appearance/win3x/control/vb_text.h"
#include "../lib-win32-style/appearance/win3x/control/control_class.h"
#include "ntw16s.h"
#include "native_api.h"
#include "theme.h"
#include "lib-win32-style/application/adapters/win32/runtime_native.h"
#include "../lib-win32-style/appearance/win3x/control/basic_paint.h"
#include "lib-win32-style/application/control/native_overlay.h"
#include "lib-win32-style/application/control/report.h"
#include "lib-win32-style/appearance/win3x/control/combo_paint.h"
#include "lib-win32-style/application/control/control_custom_draw.h"
#include "../lib-win32-style/appearance/win3x/control/vcl_native_paint.h"
#include "../lib-win32-style/appearance/win3x/control/text_policy.h"
#include "lib-win32-style/application/adapters/win32/frame_paint.h"
#include "lib-win32-style/application/adapters/win32/runtime_observer.h"
#include "../lib-win32-style/appearance/win3x/control/special_paint.h"
#include "lib-win32-style/application/control/control_vcl_spin.h"
#include "lib-win32-style/application/control/spin.h"
#include "lib-win32-style/application/control/status_paint.h"
#include "../lib-win32-style/appearance/win3x/control/status_font.h"
#include "lib-win32-style/application/control/control_status.h"
#include "../lib-win32-style/appearance/win3x/control/vcl_identity.h"
#include "../lib-win32-style/appearance/win3x/control/attachment_state.h"
#include "../lib-win32-style/appearance/win3x/control/month_paint.h"
#include "../lib-win32-style/appearance/win3x/window/modern_frame.h"
#include "lib-win32-style/application/dialog/message_observer.h"
#include "lib-win32-style/access/runtime/window_hook.h"
#include "lib-win32-style/application/window/window_identity.h"
#include "lib-win32-style/application/lifecycle/creation_pending.h"
#include "../lib-win32/process-hook/unwind.h"
#include "status_theme.h"
#include "../lib-win32-style/appearance/win3x/control/month_text.h"
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
static _Thread_local style_runtime_paint_manager paint_runtime;
static const style_runtime_observation *current_observation(void){return paint_runtime.paint?&paint_runtime.paint->observation:NULL;}
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


/* Status frames are drawn by the native common control before text, icons
 * and WM_DRAWITEM. A per-message snapshot makes nested unrelated drawing a
 * barrier and allows a changing or destroyed control to fall back safely. */
static LONG status_hooks_ready;
void ntw16s_status_hooks_enable(void){InterlockedExchange(&status_hooks_ready,1);}

/* Scope is active only while dispatching the observed control's own paint or
 * pointer/enable messages. Unrelated nested attachment messages are barriers. */
void ntw16s_modern_spin_dc_created(HDC a,HDC b){style_control_spin_dc_created(paint_runtime.spin,a,b);}
void ntw16s_modern_spin_theme_begin(HDC dc,const RECT *r){style_control_spin_theme_begin(paint_runtime.spin,dc,r);}
void ntw16s_modern_spin_dc_deleted(HDC dc){style_control_spin_dc_deleted(paint_runtime.spin_stack,dc);}
void ntw16s_modern_spin_selected(HDC dc){style_control_spin_selected(paint_runtime.spin_stack,dc);}
void ntw16s_modern_spin_object_deleted(HGDIOBJ o){style_control_spin_object_deleted(paint_runtime.spin_stack,o);}
void ntw16s_modern_spin_theme(HDC dc,int part,int state,const RECT *r,const RECT *clip,const void *caller)
{style_control_spin_theme(paint_runtime.spin,dc,part,state,r,clip,caller);}
void ntw16s_modern_spin_blit(HDC dst,int x,int y,int w,int h,HDC src,int sx,int sy,DWORD rop)
{style_control_spin_blit(paint_runtime.spin,dst,x,y,w,h,src,sx,sy,rop,win16_style_bitbtn_face);}
void ntw16s_spin_frame(HDC dc,const RECT *r,UINT type,UINT flags,const void *caller)
{style_control_spin_frame(paint_runtime.spin,dc,r,type,flags,caller,win16_control_black_frame);}

void ntw16s_bit_theme_frame(HDC dc,int part,const RECT *rect,const RECT *clip,const void *address)
{
    style_runtime_bit_scope *s=paint_runtime.bit;if(!s || !s->observation.owner)return;attachment *a=s->observation.owner;
    const style_runtime_bit_frame_paint v={s->observation.window,s->dc,s->item,s->state,&a->base.instance,&a->base.runtime.roles.bit,&a->base.runtime.report};
    style_runtime_bit_frame(&v,dc,part,rect,clip,address,win16_style_bitbtn_face);
}
static void inplace_draw(HDC dc,int part,int state,const RECT *r,const RECT *clip,const void *address,BOOL drop)
{
    style_runtime_paint_scope *s=paint_runtime.paint;if(!s)return;
    const win16_control_vcl_paint_view v={s->observation.window,s->observation.instance,&s->observation.runtime->roles.inplace};
    win16_control_vcl_inplace_draw(&v,dc,part,state,r,clip,address,drop);
}
void ntw16s_nav_theme(HDC dc,int part,int state,const RECT *r,const RECT *clip,const void *address)
{
    style_runtime_paint_scope *s=paint_runtime.paint;if(!s)return;
    const win16_control_vcl_paint_view v={s->observation.window,s->observation.instance,&s->observation.runtime->roles.nav};
    win16_control_vcl_nav_draw(&v,dc,part,state,r,clip,address);
}
BOOL ntw16s_inplace_paint(HDC dc)
{style_runtime_paint_scope *s=paint_runtime.paint;return win16_attachment_inplace_paint(s?s->observation.owner:NULL,s?s->observation.window:NULL,dc);}
void ntw16s_inplace_button(HDC dc,int part,int state,const RECT *r,const RECT *clip,const void *address){inplace_draw(dc,part,state,r,clip,address,FALSE);}
void ntw16s_inplace_dropdown(HDC dc,int part,int state,const RECT *r,const RECT *clip,const void *address){inplace_draw(dc,part,state,r,clip,address,TRUE);}
static BOOL status_identity(HWND w,attachment *a,HMODULE *native)
{
    return style_control_status_identity(w,a?&a->base.instance:NULL,a?&a->base.runtime.status:NULL,L"TStatusBar",native);
}

BOOL ntw16s_status_theme_begin(style_control_status_theme *t,HDC dc,int part,int state,LPCWSTR text,int count,DWORD flags,const RECT *rect,const void *address)
{
    return style_control_status_theme_begin(&paint_runtime.status_themes,t,paint_runtime.status,dc,part,state,text,count,flags,rect,address,win16_control_status_text_supported);
}
void ntw16s_status_theme_end(void *data)
{style_control_status_theme_end(&paint_runtime.status_themes,data);}

HFONT ntw16s_status_simple_font(HDC dc,LPCWSTR text,int count,UINT flags,const void *address,void **token)
{
    style_control_status_scope *s=paint_runtime.status;if(!s){if(token)*token=NULL;return NULL;}
    if(token)*token=NULL;
    if(!token || !style_control_status_text_matches(s,paint_runtime.status_themes.current,dc,text,count,flags,address,win16_control_status_text_supported))return NULL;
    attachment *a=s->context;return win16_control_status_font_borrow(&a->status_font,dc,a,token);
}
void ntw16s_status_simple_return(void *token,HFONT font)
{attachment *a=token;if(a)win16_control_status_font_return(&a->status_font,font);}


void ntw16s_status_text_frame(HDC dc,const RECT *text,UINT flags,const void *address)
{RECT pane;if(style_control_status_text_frame(paint_runtime.status,dc,text,flags,address,&pane))win16_control_black_frame(dc,&pane,NULL);}
void ntw16s_status_icon_frame(HDC dc,int x,int y,int cx,int cy,HICON icon,const void *address)
{RECT pane;if(style_control_status_icon_frame(paint_runtime.status,dc,x,y,cx,cy,icon,address,&pane))win16_control_black_frame(dc,&pane,NULL);}
void ntw16s_status_frame(HDC dc,const RECT *rect,const RECT *clip,const void *caller)
{HWND scoped=NULL;BOOL observed=style_runtime_text_native_target(dc,&scoped);if(style_control_status_frame(paint_runtime.status,dc,rect,caller,observed,scoped))win16_control_black_frame(dc,rect,clip);}

BOOL ntw16s_checklist_draw_target(HDC dc,const RECT *r,const void *address)
{return style_runtime_checklist_draw_target(&paint_runtime.checklists,dc,r,address,style_control_vcl_checklist_identity,16);}
static LRESULT CALLBACK attachment_proc(HWND,UINT,WPARAM,LPARAM,UINT_PTR,DWORD_PTR);

static BOOL report_install(HWND,BOOL,void *,const BOOL *);
static LRESULT CALLBACK report_notify(HWND,UINT,WPARAM,LPARAM,UINT_PTR,DWORD_PTR);

static attachment *lookup(HWND w)
{
    return (attachment *)style_instance_lookup(w,attachment_proc);
}
static BOOL mdi_host_attached(HWND host){return lookup(host)!=NULL;}
BOOL ntw16s_vcl_combo_candidate(HWND w)
{return win16_attachment_vcl_combo_candidate(w,lookup(w));}
BOOL ntw16s_group_push_button(HWND w)
{return win16_attachment_group_push_button(w,lookup(w));}
BOOL ntw16s_group_choice_button(HWND w)
{return win16_attachment_group_choice_button(w,lookup(w));}

static BOOL data_current(HWND w)
{return win16_attachment_vb_data_valid(w,lookup(w));}
BOOL ntw16s_vb_data_attached(HWND w)
{return win16_attachment_vb_data_attached(w,lookup(w));}


BOOL ntw16s_vb_user_label_target(HWND w)
{return win16_attachment_vb_user_label_target(w,lookup(w));}
BOOL ntw16s_vb_user_edge_dormant(HWND w)
{return win16_attachment_vb_user_edge_dormant(w,lookup(w));}
static BOOL user_current(HWND w)
{return win16_attachment_vb_user_edge_valid(w,lookup(w));}
BOOL ntw16s_control_role_valid(HWND w)
{
    const style_runtime_attachment_policy policy={data_current,user_current,MCS_MULTISELECT};
    return style_runtime_attachment_current(w,attachment_proc,&policy);
}
/* Observe only the exact data edit forwarding its own paint to ANSI EDIT. */
void ntw16s_dbedit_ansi_witness(WNDPROC proc,HWND w,UINT message,const void *address)
{style_runtime_observe_ansi(current_observation(),proc,w,message,address,FALSE);}
/* Data-aware ANSI combos must forward their own paint to the native class. */
void ntw16s_dbcombo_ansi_witness(WNDPROC proc,HWND w,UINT message,const void *address)
{style_runtime_observe_ansi(current_observation(),proc,w,message,address,TRUE);}
BOOL ntw16s_static_call_begin(style_runtime_native_call *scope,WNDPROC proc,HWND w,UINT message,const void *address)
{return style_runtime_native_call_begin(&paint_runtime.native_calls,scope,proc,w,message,address,current_observation(),paint_runtime.cool?&paint_runtime.cool->observation:NULL);}
HFONT ntw16s_static_text_font(HDC dc,LPCWSTR text,int count,UINT flags,const void *address)
{
    const style_runtime_native_call *s=style_runtime_native_current(&paint_runtime.native_calls,current_observation());
    return s?win16_control_static_text_font(dc,text,count,flags,address,s->window,s->native_module):NULL;
}
#define class_is style_control_class_is
static BOOL vcl_frame_owned(HWND w,void *owner){return lookup(w)==owner;}
static BOOL vcl_frame_current(style_runtime_frame_scope *scope){return paint_runtime.paint && &paint_runtime.paint->frame==scope;}
static const style_runtime_frame_renderer frame_renderer={win16_control_group_frame,win16_control_panel_shadow};
void ntw16s_group_frame(HDC dc,int part,int state,const RECT *rect,const RECT *clip,LPCWSTR text,int count,DWORD flags,const void *caller,BOOL is_text)
{
    style_runtime_paint_scope *scope=paint_runtime.paint;if(!scope || !scope->observation.owner)return;attachment *a=scope->observation.owner;
    const style_runtime_frame_paint v={&scope->frame,&a->base.instance,&a->base.runtime.group_frame,a,scope->observation.window,vcl_frame_owned,vcl_frame_current};
    style_runtime_group_frame(&v,&frame_renderer,dc,part,state,rect,clip,text,count,flags,caller,is_text);
}
void ntw16s_panel_polyline(HDC dc,const POINT *points,int count,const void *caller,BOOL (WINAPI *draw)(HDC,const POINT*,int))
{
    style_runtime_paint_scope *scope=paint_runtime.paint;if(!scope)return;if(!scope->observation.owner){scope->frame.panel_first=FALSE;return;}attachment *a=scope->observation.owner;
    const style_runtime_frame_paint v={&scope->frame,&a->base.instance,&a->base.runtime.panel_frame,a,scope->observation.window,vcl_frame_owned,vcl_frame_current};
    style_runtime_panel_polyline(&v,&frame_renderer,dc,points,count,caller,draw);
}

void ntw16s_box_witness(HWND w,UINT msg,const void *address)
{style_runtime_box_scope *s=paint_runtime.box;if(s && style_runtime_observe_box(w,msg,address,&s->observation))s->witnessed=TRUE;}

BOOL ntw16s_controlbar_edge_active(void)
{return paint_runtime.box && paint_runtime.box->observation.runtime->controlbar.candidate && !paint_runtime.box->observation.instance->detached;}
void ntw16s_controlbar_edge(HDC dc,const RECT *rect,UINT edge,UINT flags,const void *address)
{
    style_runtime_box_scope *s=paint_runtime.box;if(!s || !s->observation.owner)return;attachment *a=s->observation.owner;
    style_runtime_controlbar_edge(s->observation.window,&a->base.instance,&a->base.runtime.controlbar,dc,rect,edge,flags,address,win16_control_black_frame);
}
static win16_control_month_view month_view(style_runtime_paint_scope *s);
void ntw16s_rich_witness(WNDPROC proc,HWND w,UINT msg,const void *address,BOOL wide)
{
    style_runtime_paint_scope *scope=paint_runtime.paint;
    if(scope && style_runtime_observe_rich(proc,w,msg,address,wide,current_observation(),MCS_MULTISELECT) && scope->observation.runtime->rich.month)
        {const win16_control_month_view view=month_view(scope);style_control_month_query(&view.native,&paint_runtime.month_queries);}
}
static BOOL month_view_current(void *scope){return paint_runtime.paint==scope;}
static win16_control_month_view month_view(style_runtime_paint_scope *s)
{attachment *a=s->observation.owner;const win16_control_month_view v={.native={.context=s,.window=s->observation.window,.instance=&a->base.instance,.rich=&a->base.runtime.rich,.state=&s->month,.rejected_styles=MCS_MULTISELECT,.current=month_view_current},.cached_font=&a->month_cached_font};return v;}

void ntw16s_month_arrow(HTHEME t,HDC dc,int p,int state,const RECT *r,const RECT *clip,const void *addr)
{style_runtime_paint_scope *s=paint_runtime.paint;if(s){const win16_control_month_view v=month_view(s);win16_control_month_arrow(&v,t,dc,p,state,r,clip,addr);}}
HFONT ntw16s_month_text_borrow(HTHEME t,HDC dc,int p,int state,DWORD f,DWORD f2,const RECT *r,const void *addr)
{style_runtime_paint_scope *s=paint_runtime.paint;if(!s)return NULL;const win16_control_month_view v=month_view(s);return win16_control_month_text_borrow(&v,t,dc,p,state,f,f2,r,addr);}
BOOL ntw16s_month_text_room(HTHEME t,HDC dc,int p,int state,DWORD f,DWORD f2,const RECT *r,int width,const void *addr)
{style_runtime_paint_scope *s=paint_runtime.paint;if(!s)return FALSE;const win16_control_month_view v=month_view(s);return win16_control_month_text_room(&v,t,dc,p,state,f,f2,r,width,addr);}
void ntw16s_month_header_text(HTHEME t,HDC dc,int p,int state,const RECT *r,DWORD f,const void *addr)
{style_runtime_paint_scope *s=paint_runtime.paint;if(s){const win16_control_month_view v=month_view(s);win16_control_month_header_text(&v,t,dc,p,state,r,f,addr);}}
void ntw16s_month_selection_background(HTHEME t,HDC dc,int p,int state,const RECT *r,const RECT *clip,const void *addr)
{style_runtime_paint_scope *s=paint_runtime.paint;if(s){const win16_control_month_view v=month_view(s);style_control_month_selection_begin(&v.native,t,dc,p,state,r,clip,addr);}}
void ntw16s_month_selection_text(HTHEME t,HDC dc,int p,int state,const RECT *r,DWORD f,const void *addr)
{style_runtime_paint_scope *s=paint_runtime.paint;if(s){const win16_control_month_view v=month_view(s);win16_control_month_selection_text(&v,t,dc,p,state,r,f,addr);}}
static void release(attachment *a);
static void attachment_dispose(void *context)
{
    attachment *a=context;style_runtime_references references=a->base.runtime.roles.references;
    win16_style_system_release(a->month_cached_font);win16_style_system_release(a->status_font.font);
    ntw16s_vb_fonts_clear(&a->fonts);free(a);style_runtime_references_release(&references);
}
static void release(attachment *a){style_instance_release(&a->base.instance);}
static BOOL cool_identity(HWND w,attachment *a)
{return !a->base.instance.detached && style_control_vcl_cool_identity(w,a->base.runtime.cool.notify.parent,a->base.runtime.cool.owner,vcl_frame_owned,a);}
static BOOL cool_identity_callback(HWND w,void *owner)
{return cool_identity(w,(attachment *)owner);}
static BOOL cool_grip(HWND w,attachment *a,const NMCUSTOMDRAW *n)
{
    const style_runtime_observation *o=paint_runtime.cool?&paint_runtime.cool->observation:NULL;
    WNDPROC native=o && o->owner==a && o->window==w?o->native:NULL;
    return win16_control_cool_grip_identity(w,a,n,native,cool_identity_callback);
}
static BOOL cool_grip_callback(HWND w,void *owner,const NMCUSTOMDRAW *draw)
{return cool_grip(w,(attachment *)owner,draw) && win16_control_cool_grip(draw->hdc,&draw->rc);}
static LRESULT CALLBACK cool_notify(HWND parent,UINT m,WPARAM p,LPARAM l,UINT_PTR id,DWORD_PTR data)
{
    (void)id;attachment *a=(attachment *)data;style_runtime_cool_scope *s=paint_runtime.cool;
    return style_control_custom_draw_post(parent,m,p,l,a,s && s->observation.owner==a?s->observation.window:NULL,
        s && s->observation.owner==a?&s->ours:NULL,&a->base.runtime.cool.verified,cool_identity_callback,cool_grip_callback);
}
static BOOL runtime_control_render(HWND w,style_control_runtime_kind kind,void *context,const BOOL *cancelled)
{
    attachment *a=context;
    if(kind==STYLE_RUNTIME_TOOLBAR)return win16_style_toolbar_install_created(w,NULL,a,cancelled);
    if(kind==STYLE_RUNTIME_TRACK)return win16_style_trackbar_install_created(w,NULL,a,cancelled);
    if(kind==STYLE_RUNTIME_EDIT)return win16_style_control_install_created(w,!a->base.runtime.link && !a->base.runtime.combo,
        (a->base.runtime.edit_style&WS_BORDER)!=0,FALSE,NULL,a,cancelled);
    return win16_style_control_install_created(w,FALSE,FALSE,FALSE,NULL,a,cancelled);
}
static BOOL runtime_edge_eligible(HWND w,HWND parent,style_control_edge_kind kind,void *context)
{
    (void)context;
    return kind==STYLE_EDGE_DATA?win16_control_vb_data_role(w,parent):win16_control_vb_user_edge_identity(w,parent);
}
static BOOL runtime_edge_install(HWND w,void *context,const BOOL *cancelled)
{return win16_style_grid_install_created(w,NULL,context,cancelled);}
static void message_guard(const style_runtime_observation *observation,UINT m)
{
    attachment *a=observation->owner;const win16_attachment_guard_services guards={mdi_host_attached,cool_identity,Ntw16sWindowRemove};
    win16_attachment_guard_cool(observation->window,m,a,&guards);
    if(a->base.runtime.status.present)style_control_status_invalidate(m,&a->status_font.invalid,&a->base.runtime.status);
}
static void message_prepare(const style_runtime_observation *observation,UINT m,WPARAM p,style_runtime_text_scope *paint)
{
    attachment *a=observation->owner;HWND w=observation->window;
    const win16_attachment_guard_services guards={mdi_host_attached,cool_identity,Ntw16sWindowRemove};
    win16_attachment_guard_roles(w,m,a,&guards);
    if(m==WM_NCDESTROY)(void)Ntw16sWindowTerminal(w);
    win16_control_vb_paint_begin(paint,w,m,&a->fonts,ntw16s_vb_user_label_target);
    if(a->file_owner || a->picture || a->button_owner || a->base.runtime.user.candidate)paint->cancelled=&a->base.instance.detached;
    style_runtime_text_prepare_report(paint,w,p,&a->base.instance,&a->base.runtime,a->file_owner,report_notify,a,ntw16s_vb_graphical_button);
}
static LRESULT CALLBACK attachment_proc(HWND w,UINT m,WPARAM p,LPARAM l,UINT_PTR id,DWORD_PTR data)
{
    (void)id;attachment *a=(attachment *)data;
    const style_runtime_observation observation={.instance=&a->base.instance,.runtime=&a->base.runtime,.owner=a,.window=w,.current=vcl_frame_owned};
    style_runtime_attachment_controls controls={&a->base,runtime_control_render,win16_style_control_remove_owned,
        ntw16s_theme_enable,win16_style_edit_scrollbars_enable,ntw16s_theme_remove};
    const style_runtime_message_ops ops={message_guard,message_prepare,
        {a,runtime_edge_install,win16_style_control_remove_owned},
        {&controls,style_runtime_attachment_control_install,style_runtime_attachment_control_remove},
        {a,runtime_edge_eligible,runtime_edge_install,win16_style_control_remove_owned,ntw16s_native_get_scroll_info},
        {a,report_notify,report_install,Ntw16sWindowRemove},win16_control_native_combo_button,L"TStatusBar",MCS_MULTISELECT};
    return style_runtime_message_dispatch(&paint_runtime,&observation,m,p,l,&ops);
}
static void report_text_prepare(style_runtime_text_scope *paint,HWND window,void *owner)
{
    attachment *a=owner;win16_control_vb_paint_begin(paint,window,WM_PRINTCLIENT,&a->fonts,ntw16s_vb_user_label_target);
}
/* Application owner/custom-draw result and payload stay authoritative. */
static LRESULT CALLBACK report_notify(HWND w,UINT m,WPARAM p,LPARAM l,UINT_PTR id,DWORD_PTR data)
{
    attachment *a=(attachment *)data;
    const style_runtime_observation observation={.instance=&a->base.instance,.runtime=&a->base.runtime,.owner=a,.window=a->base.runtime.report.window};
    const style_runtime_report_notify_ops ops={report_notify,a->file_owner,a->button_owner,a->checklist,
        ntw16s_vb_owner_file,ntw16s_vb_graphical_button,report_text_prepare};
    return style_runtime_report_notify(&paint_runtime,&observation,w,m,p,l,id,&ops);
}
/* Some frameworks set the report view after native WM_CREATE. This is still
 * our exact newly created attachment, on its GUI thread, after initialization.
 * A failed public protocol remains native; there is no existing-window scan. */
static BOOL report_install(HWND w,BOOL tree,void *context,const BOOL *cancelled)
{
    DWORD style=(DWORD)GetWindowLongPtrW(w,GWL_STYLE);
    return tree?win16_style_tree_install_created(w,NULL,context,cancelled):
        !(style&(WS_BORDER|WS_THICKFRAME|WS_CAPTION))?TRUE:win16_style_install_created(w,NULL,context,cancelled);
}
static void runtime_scrollbars_paint(HWND w){win16_style_scrollbars_paint(w,NULL,NULL);}
static const style_runtime_attachment_ops lifecycle_ops={cool_identity_callback,mdi_host_attached,
    ntw16s_vb_user_label_target,report_notify,cool_notify,win16_style_frame_remove_owned,
    win16_style_control_remove_owned,ntw16s_theme_remove,win16_style_frame_terminal,
    win16_style_control_terminal,style_theme_terminal,runtime_scrollbars_paint,MCS_MULTISELECT};
BOOL __cdecl Ntw16sWindowAttached(HWND w)
{return style_runtime_attachment_attached(w,attachment_proc,&lifecycle_ops);}
void __cdecl Ntw16sScrollChanged(HWND w)
{style_runtime_attachment_scroll_changed(w,attachment_proc,&lifecycle_ops);}
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
    return style_runtime_attachment_remove(w,attachment_proc,&lifecycle_ops);
}
BOOL __cdecl Ntw16sWindowTerminal(HWND w)
{return style_runtime_attachment_terminal(w,attachment_proc,&lifecycle_ops);}

typedef struct owner_selection {attachment *owner;BOOL picture,checklist;} owner_selection;
static style_runtime_owner_roles initialize_owner_roles(HWND window,void *context)
{
    const owner_selection *selection=context;attachment *a=selection->owner;
    a->file_owner=ntw16s_vb_owner_file(window);a->picture=selection->picture;
    a->button_owner=ntw16s_vb_graphical_button(window);a->checklist=selection->checklist;
    return (style_runtime_owner_roles){a->file_owner,a->button_owner,a->checklist};
}
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
    style_control_runtime_candidates candidates;
    win16_attachment_classify_vcl(w,original_style,ready,picture,checklist,status,spin,&initial,&candidates);
    HWND popup_source=paint_runtime.lookup_popup,popup_source_parent=popup_source?GetParent(popup_source):NULL;
    attachment *popup_source_attachment=popup_source?lookup(popup_source):NULL;
    BOOL lookup_popup_candidate=ready && popup_source_attachment && style_control_vcl_lookup_popup_identity(w,popup_source,popup_source_parent,owner);
    style_control_runtime_extras extra;
    win16_control_vcl_classify_extra(w,original_style,ready,owner,&extra);
    BOOL cool_candidate=extra.cool_candidate;
    BOOL vcl_mdi_probe=style_control_vcl_mdi_identity(w,GetParent(w),GetParent(GetParent(w)),owner,mdi_host_attached);
    int rejected=win16_attachment_eligibility(original_style,picture,checklist,status,spin,vcl_mdi_probe,&initial,&candidates,&extra);
    if(rejected){if(rejected==1)style_control_protocol_release(protocol);SetLastError(ERROR_NOT_SUPPORTED);return FALSE;}
    a=calloc(1,sizeof(*a));
    if(!a){style_control_protocol_release(protocol);SetLastError(ERROR_NOT_ENOUGH_MEMORY);return FALSE;}
    style_instance_init(&a->base.instance,a,attachment_dispose);
    owner_selection selection={a,picture,checklist};
    const style_runtime_attachment_seed seed={&candidates,&extra,owner,protocol,report,tree,status,spin,
        vcl_mdi_probe,lookup_popup_candidate,popup_source,popup_source_parent,
        popup_source_attachment?&popup_source_attachment->base.instance:NULL,attachment_proc,initialize_owner_roles,&selection};
    style_runtime_attachment_initialize(w,original_style,&a->base,&seed);
    if(!style_runtime_attachment_publish(w,&a->base,attachment_proc,free))return FALSE;
    if((report || tree || a->file_owner || a->button_owner || a->checklist || a->base.runtime.roles.bit.candidate) && (!a->base.runtime.report.parent || !SetWindowSubclass(a->base.runtime.report.parent,report_notify,(UINT_PTR)w,(DWORD_PTR)a))){
        DWORD error=GetLastError();Ntw16sWindowRemove(w);release(a);SetLastError(error);return FALSE;
    }
    if(cool_candidate && (!cool_identity(w,a) || !style_control_notify_bind(w,&a->base.runtime.cool.notify,cool_notify,a))){
        DWORD failure=GetLastError();Ntw16sWindowRemove(w);release(a);SetLastError(failure?failure:ERROR_NOT_SUPPORTED);return FALSE;
    }
    SetLastError(ERROR_SUCCESS);
    if(protocol && class_is(w,L"COMBOBOX")){
        COMBOBOXINFO info={.cbSize=sizeof(info)};
        if(ntw16s_native_get_combo_info(w,&info) && info.hwndItem!=w && lookup(info.hwndItem))
            Ntw16sWindowRemove(info.hwndItem);
    }
    if(a->base.instance.detached){release(a);SetLastError(ERROR_CANCELLED);return FALSE;}
    const win16_attachment_install_services install_services={win16_style_legacy_progress_border_install_created,
        win16_style_toolbar_install_created,win16_style_updown_install_created,win16_style_trackbar_install_created,
        win16_style_progress_install_created,win16_style_tree_install_created,win16_style_picture_install_created,
        win16_style_grid_install_created,win16_style_frame_scrollbars_install_owned,win16_style_control_install_created,
        win16_style_install_created};
    installed=win16_attachment_install_render(w,a,protocol,original_style,legacy_progress,tree,picture,control,edit,report,&install_services);
    const style_runtime_attachment_install_ops settlement={ntw16s_native_get_combo_info,ntw16s_theme_enable,
        win16_style_list_scrollbar_enable,win16_style_edit_scrollbars_enable,ntw16s_theme_remove,Ntw16sWindowRemove};
    installed=style_runtime_attachment_install_finish(w,&a->base,original_style,control,edit,a->file_owner,installed,&error,&settlement);
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
static BOOL window_thread_accept(const CWPSTRUCT *message,void *context)
{
    (void)context;return (message->wParam==NTW16S_WINDOW_THREAD_ATTACH || message->wParam==NTW16S_WINDOW_THREAD_REMOVE) &&
        (HWND)message->lParam==message->hwnd;
}
static DWORD window_thread_perform(const CWPSTRUCT *message,void *context)
{
    (void)context;
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
    const style_window_hook_spec spec={NTW16S_WINDOW_THREAD_MESSAGE,NTW16S_WINDOW_THREAD_RESULT,
        window_thread_accept,window_thread_perform,NULL};
    style_window_hook_handle(code,lp,&busy,&spec);
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
