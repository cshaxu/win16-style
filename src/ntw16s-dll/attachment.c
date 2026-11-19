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
#include "lib-win32-style/application/lifecycle/creation.h"
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
#include "lib-win32-style/appearance/msstyles/classic_theme.h"

extern IMAGE_DOS_HEADER __ImageBase;

static _Thread_local style_runtime_paint_manager paint_runtime;
static const style_runtime_observation *current_observation(void){return paint_runtime.paint?&paint_runtime.paint->observation:NULL;}

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
static BOOL spin_face(HWND w,HDC dc,const RECT *r,const RECT *clip,UINT state)
{
    const attachment *a=paint_runtime.spin->instance->context;
    return win16_style_bitbtn_face(w,dc,r,clip,state,&a->palette);
}
void ntw16s_modern_spin_blit(HDC dst,int x,int y,int w,int h,HDC src,int sx,int sy,DWORD rop)
{style_control_spin_blit(paint_runtime.spin,dst,x,y,w,h,src,sx,sy,rop,spin_face);}
static BOOL spin_frame(HDC dc,const RECT *bounds,const RECT *clip)
{
    const attachment *a=paint_runtime.spin->instance->context;
    return win16_control_color_frame(dc,bounds,clip,a->palette.edge);
}
void ntw16s_spin_frame(HDC dc,const RECT *r,UINT type,UINT flags,const void *caller)
{style_control_spin_frame(paint_runtime.spin,dc,r,type,flags,caller,spin_frame);}

static BOOL bit_face(HWND w,HDC dc,const RECT *r,const RECT *clip,UINT state)
{
    const attachment *a=paint_runtime.bit->observation.owner;
    return win16_style_bitbtn_face(w,dc,r,clip,state,&a->palette);
}
void ntw16s_bit_theme_frame(HDC dc,int part,const RECT *rect,const RECT *clip,const void *address)
{
    style_runtime_bit_scope *s=paint_runtime.bit;if(!s || !s->observation.owner)return;attachment *a=s->observation.owner;
    const style_runtime_bit_frame_paint v={s->observation.window,s->dc,s->item,s->state,&a->base.instance,&a->base.runtime.roles.bit,&a->base.runtime.report};
    style_runtime_bit_frame(&v,dc,part,rect,clip,address,bit_face);
}
static void inplace_draw(HDC dc,int part,int state,const RECT *r,const RECT *clip,const void *address,BOOL drop)
{
    style_runtime_paint_scope *s=paint_runtime.paint;if(!s)return;
    const attachment *a=s->observation.owner;
    const win16_control_vcl_paint_view v={s->observation.window,s->observation.instance,&s->observation.runtime->roles.inplace,&a->palette};
    win16_control_vcl_inplace_draw(&v,dc,part,state,r,clip,address,drop);
}
void ntw16s_nav_theme(HDC dc,int part,int state,const RECT *r,const RECT *clip,const void *address)
{
    style_runtime_paint_scope *s=paint_runtime.paint;if(!s)return;
    const attachment *a=s->observation.owner;
    const win16_control_vcl_paint_view v={s->observation.window,s->observation.instance,&s->observation.runtime->roles.nav,&a->palette};
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
    attachment *a=s->context;return style_font_snapshot_borrow(&a->status_font,dc,a,token,&win16_status_font_operations);
}
void ntw16s_status_simple_return(void *token,HFONT font)
{attachment *a=token;if(a)style_font_snapshot_return(&a->status_font,font);}


static void draw_status_edge(const style_control_status_scope *scope,HDC dc,const RECT *pane,const RECT *clip)
{
    const attachment *a=scope->context;
    win16_control_color_frame(dc,pane,clip,a->palette.edge);
}
void ntw16s_status_text_frame(HDC dc,const RECT *text,UINT flags,const void *address)
{RECT pane;const style_control_status_scope *scope=paint_runtime.status;if(style_control_status_text_frame(scope,dc,text,flags,address,&pane))draw_status_edge(scope,dc,&pane,NULL);}
void ntw16s_status_icon_frame(HDC dc,int x,int y,int cx,int cy,HICON icon,const void *address)
{RECT pane;const style_control_status_scope *scope=paint_runtime.status;if(style_control_status_icon_frame(scope,dc,x,y,cx,cy,icon,address,&pane))draw_status_edge(scope,dc,&pane,NULL);}
void ntw16s_status_frame(HDC dc,const RECT *rect,const RECT *clip,const void *caller)
{HWND scoped=NULL;const style_control_status_scope *scope=paint_runtime.status;BOOL observed=style_runtime_text_native_target(dc,&scoped);
 if(style_control_status_frame(scope,dc,rect,caller,observed,scoped))draw_status_edge(scope,dc,rect,clip);}

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
BOOL ntw16s_vb_data_palette(HWND w,win16_style_palette *palette)
{
    attachment *a=lookup(w);
    if(!win16_attachment_vb_data_attached(w,a))return FALSE;
    *palette=a->palette;
    return TRUE;
}


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
static BOOL controlbar_frame(HDC dc,const RECT *bounds,const RECT *clip)
{
    const attachment *a=paint_runtime.box->observation.owner;
    return win16_control_color_frame(dc,bounds,clip,a->palette.edge);
}
void ntw16s_controlbar_edge(HDC dc,const RECT *rect,UINT edge,UINT flags,const void *address)
{
    style_runtime_box_scope *s=paint_runtime.box;if(!s || !s->observation.owner)return;attachment *a=s->observation.owner;
    style_runtime_controlbar_edge(s->observation.window,&a->base.instance,&a->base.runtime.controlbar,dc,rect,edge,flags,address,controlbar_frame);
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
{
    attachment *a=s->observation.owner;
    return (win16_control_month_view){
        .native={.context=s,.window=s->observation.window,.instance=&a->base.instance,.rich=&a->base.runtime.rich,
            .state=&s->month,.rejected_styles=MCS_MULTISELECT,.current=month_view_current},
        .cached_font=&a->month_cached_font,.palette=&a->palette,.selection=a->calendar_selection};
}

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
    win16_style_system_release(a->month_cached_font);style_font_snapshot_dispose(&a->status_font);
    style_font_cache_dispose(&a->fonts);free(a);style_runtime_references_release(&references);
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
static void date_button(HWND w,const style_instance *instance,HDC dc,const RECT *dirty,
    const style_control_native_button *button)
{
    const attachment *a=instance->context;
    const win16_style_palette *palette=a->base.runtime.date.installed?&a->palette:NULL;
    win16_control_native_combo_button(w,instance,dc,dirty,button,palette);
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
        {a,report_notify,report_install,Ntw16sWindowRemove},date_button,L"TStatusBar",MCS_MULTISELECT};
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
    if(composed_frame_class(w) && style_uwp_frame_surface(w) &&
       !style_uwp_frame_remove(w))return FALSE;
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
    win16_style_default_palette(&a->palette);
    a->calendar_selection=a->palette.active;
    if(status || candidates.status_candidate || candidates.date_candidate || extra.rich_month || extra.vb_data_edge || spin ||
       candidates.bit_candidate || candidates.inplace_candidate || candidates.nav_candidate || extra.controlbar_candidate){
        classic_theme theme;
        if(classic_theme_default((HMODULE)&__ImageBase,&theme)){
            if(theme.frame.present){
                a->palette.face=theme.frame.face;
                a->palette.light=theme.frame.light;
                a->palette.shadow=theme.frame.shadow;
                a->palette.edge=theme.frame.edge;
            }
            a->palette.active=theme.active.start;
            if(theme.list.present)a->calendar_selection=theme.list.selection;
        }
    }
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
/* Creation ownership belongs to the public transaction; these operations
 * select this renderer's existing theme and attachment behavior. */
static BOOL creation_prepare(HWND window,DWORD style)
{
    if(class_is(window,L"LISTBOX") || class_is(window,L"ComboLBox") ||
       (class_is(window,L"EDIT") && (style&ES_MULTILINE)))return style_theme_capture(window);
    return !(style&WS_CHILD) || style_theme_stage(window);
}
static BOOL creation_install(HWND window,DWORD style)
{return style_theme_commit(window) && install_created(window,style);}
static BOOL creation_attached(HWND window){return lookup(window)!=NULL;}
static void creation_unsupported(HWND window)
{
    ntw16s_theme_remove(window,NULL);
    if(class_is(window,L"COMBOBOX")){
        COMBOBOXINFO info={.cbSize=sizeof(info)};
        if(ntw16s_native_get_combo_info(window,&info))ntw16s_theme_remove(info.hwndList,NULL);
    }
}
const style_creation_ops ntw16s_creation_ops={creation_prepare,creation_install,creation_attached,
    style_theme_cancel_stage,creation_unsupported,
#ifdef NTW16S_EXPERIMENTAL_CREATION_SUBCLASS
    TRUE
#else
    FALSE
#endif
};
ntw16s_creation_scope *__cdecl Ntw16sCreationBegin(BOOL enabled)
{return style_creation_begin(enabled,FALSE,&ntw16s_creation_ops);}
BOOL __cdecl Ntw16sCreationEnd(ntw16s_creation_scope *scope,ntw16s_creation_result *result)
{return style_creation_end(scope,result);}
