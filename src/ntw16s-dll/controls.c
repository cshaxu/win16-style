#include "../lib-style/win16-control/control_class.h"
#include "lib-style/win16-control/scroll_native.h"
#include "attachment.h"
/* Internal guard for a witnessed VCL group button. */
BOOL ntw16s_group_button_valid(HWND);
BOOL ntw16s_vcl_combo_valid(HWND);
BOOL ntw16s_vcl_combo_candidate(HWND);
BOOL ntw16s_group_push_button(HWND);
BOOL ntw16s_group_choice_button(HWND);
BOOL ntw16s_grid_valid(HWND);
BOOL ntw16s_rich_valid(HWND);
BOOL ntw16s_vb_user_edge_valid(HWND);
BOOL ntw16s_vb_data_valid(HWND);
BOOL ntw16s_vb_data_attached(HWND);
BOOL ntw16s_vb_user_edge_dormant(HWND);
BOOL ntw16s_vcl_toolbar_valid(HWND);
BOOL ntw16s_vcl_tab_valid(HWND);
#include "controls.h"
#include "../lib-style/win16-control/basic_paint.h"
#include "../lib-style/win16-control/other_paint.h"
#include "../lib-style/win16-control/list_paint.h"
#include "../lib-style/win16-control/combo_paint.h"
#include "../lib-style/win16-control/scroll_paint.h"
#include "../lib-style/win16-control/special_paint.h"
#include "lib-style/common/present.h"
#include "lib-style/common/scroll_state.h"
#include "lib-style/common/control_scope.h"
#include "../lib-style/win16-control/choice_view.h"
#include "lib-style/common/control_vcl_spin.h"
#include "../lib-style/win16-control/control_state.h"
#include "../lib-style/win16-control/list_message.h"
#include "../lib-style/win16-control/edit_message.h"
#include "../lib-style/win16-control/scroll_message.h"
#include "../lib-style/win16-control/fileowner_message.h"
#include "../lib-style/win16-control/notify_message.h"
#include "../lib-style/win16-control/combo_text.h"
#include "../lib-style/win16-control/native_presentation.h"
#include "../lib-style/win16-control/control_dispatch.h"
#include "../lib-style/win16-control/control_lifetime.h"
#include "../lib-style/win16-control/control_guard.h"
#include "../lib-style/win16-control/control_install.h"
#include "../lib-style/win16-control/control_remove.h"
#include "../lib-style/win16-control/special_install.h"

#include "../lib-style/win16-control/scroll_assets.h"
#include "scroll_track.h"
#include "../lib-style/win16-common/font.h"
#include "native_api.h"
#include "theme.h"
#include "tab_paint.h"
#include "header_paint.h"
#include "progress_paint.h"
#include "trackbar_paint.h"
#include "../lib-style/win16-control/updown_adapter.h"
#include "../lib-style/win16-control/vb_text.h"
#include "../lib-win32/process-hook/unwind.h"
#include <commctrl.h>
#include <uxtheme.h>
#include <vssym32.h>
#include <oleacc.h>
#include <stdlib.h>
#include <limits.h>

static _Thread_local track_paint_scope *track_paint;
static void track_paint_end(void *data)
{track_paint_scope *scope=data;track_paint=scope->previous;}
static PROCESS_HOOK_UNWIND_FUNCTION LRESULT track_paint_dispatch(control_style *s,HWND w,UINT m,WPARAM a,LPARAM b)
{
    track_paint_scope scope={.previous=track_paint,.owner=s};process_hook_unwind_scope unwind;
    DWORD entry=GetLastError(),error;LRESULT result;
    track_paint=&scope;PROCESS_HOOK_UNWIND_BEGIN(unwind,track_paint_end,&scope);
    SetLastError(entry);result=DefSubclassProc(w,m,a,b);error=GetLastError();
    PROCESS_HOOK_UNWIND_END(unwind);SetLastError(error);return result;
}
static LRESULT CALLBACK control_proc(HWND,UINT,WPARAM,LPARAM,UINT_PTR,DWORD_PTR);
static LRESULT CALLBACK trackbar_notify(HWND,UINT,WPARAM,LPARAM,UINT_PTR,DWORD_PTR);
static LRESULT CALLBACK toolbar_notify(HWND,UINT,WPARAM,LPARAM,UINT_PTR,DWORD_PTR);
static BOOL updown_acquire(void *owner)
{return win16_control_updown_acquire(owner,control_proc);}

static LRESULT CALLBACK combo_text_proc(HWND w,UINT m,WPARAM a,LPARAM b,UINT_PTR id,DWORD_PTR data)
{return win16_control_combo_text_message(w,m,a,b,id,(control_style *)data,combo_text_proc,control_proc);}
static void combo_text_unbind(control_style *s,BOOL terminal)
{win16_control_combo_text_unbind(s,terminal,combo_text_proc);}
static void trackbar_unbind(control_style *s)
{style_control_subclass_remove_owned(s->track_parent,trackbar_notify,(UINT_PTR)s->track_window,s);s->track_parent=NULL;s->track_internal=FALSE;}
static void toolbar_unbind(control_style *s)
{style_control_subclass_remove_owned(s->toolbar_parent,toolbar_notify,(UINT_PTR)s->toolbar_window,s);s->toolbar_parent=NULL;s->toolbar_internal=FALSE;}
static LRESULT CALLBACK trackbar_notify(HWND parent,UINT m,WPARAM p,LPARAM l,UINT_PTR id,DWORD_PTR data)
{return win16_control_trackbar_notify(parent,m,p,l,id,(control_style *)data,trackbar_notify,&track_paint);}
static LRESULT CALLBACK toolbar_notify(HWND parent,UINT m,WPARAM p,LPARAM l,UINT_PTR id,DWORD_PTR data)
{return win16_control_toolbar_notify(parent,m,p,l,id,(control_style *)data,toolbar_notify);}
static BOOL is_exact_native_button(HWND w)
{WCHAR name[32];return GetClassNameW(w,name,ARRAYSIZE(name)) && !lstrcmpiW(name,L"BUTTON");}

static _Thread_local HWND common_dialog_combo_pending;

BOOL win16_style_checklist_frame(HDC dc,const RECT *rect,UINT type,UINT flags,const void *address)
{return win16_control_checklist_frame(dc,rect,type,flags,address,ntw16s_checklist_draw_target);}
BOOL win16_style_graphical_frame_prepare(HDC dc,const RECT *rect,UINT type,UINT flags,const void *address,RECT *copy)
{return win16_control_graphical_frame_prepare(dc,rect,type,flags,address,copy,control_proc);}
BOOL win16_style_graphical_frame(HDC dc,const RECT *rect,UINT type,UINT flags,const void *address)
{return win16_control_graphical_frame(dc,rect,type,flags,address,control_proc);}
/* Keep the bitmap glyph's proportions when a VCL button is shorter than its
 * source font plus the classic border. The application retains its font and
 * layout; only the text image is fitted inside the button's existing face. */
static void draw(HWND w,control_style *s,HDC dc)
{
    const win16_control_choice_view view={&s->detached,&s->palette,&s->font,
        style_control_button_type,style_control_is_static,style_control_vb_frame,style_control_vb_button,
        ntw16s_group_choice_button,ntw16s_group_push_button};
    win16_control_draw_choice(w,&view,dc);
}

/* The native list owns strings, sorting, selection and scrolling. Query only
 * visible rows; never replace item data or convert application owner-draw. */
static void draw_list(HWND w,control_style *s,HDC dc)
{win16_control_draw_list(w,&s->detached,&s->palette,&s->font,&s->tab_count,&s->tab_positions,dc,style_control_vb_list);}
static void edit_border(HWND w,control_style *s,HDC target)
{win16_control_edit_border(w,s->edit_border,&s->palette,target);}
static void list_border(HWND w,control_style *s,HDC target)
{win16_control_list_border(w,&s->palette,target);}

static void draw_combo(HWND w,control_style *s,HDC dc)
{win16_control_draw_combo(w,&s->detached,&s->common_dialog_combo,&s->palette,&s->font,dc,
    ntw16s_native_get_combo_info,style_control_vb_combo,style_control_vb_drive);}

static void detach_combo_list(control_style *s)
{(void)win16_control_combo_list_retire(s,control_proc,win16_style_control_remove);}
static BOOL terminal_combo_list(control_style *s)
{return win16_control_combo_list_retire(s,control_proc,win16_style_control_terminal);}

/* The runtime owns navigation and paints its native bitmap first. This
 * bounded overlay accepts only observed 96/144-DPI public-DC geometry. */
void win16_style_vb_data_arrow(HDC dc,int x,int y,int width,int height,HDC source,int sx,int sy,int sw,int sh,DWORD rop,const void *caller)
{
    win16_control_vb_data_arrow_candidate(dc,x,y,width,height,source,sx,sy,sw,sh,rop,caller,ntw16s_vb_data_attached);
}

static BOOL scroll_layout(HWND w,win16_scroll_layout *layout)
{return win16_scroll_control_layout(w,ntw16s_native_get_scroll_info,layout);}
static BOOL embedded_scroll_layout(HWND w,LONG object,win16_scroll_layout *layout,POINT *screen_origin)
{return win16_scroll_embedded_layout(w,object,ntw16s_native_get_scroll_info,layout,screen_origin);}

static BOOL list_axis_owned(HWND w,LONG object)
{return style_scroll_list_axis_owned(w,object,ntw16s_native_get_scroll_info);}
static BOOL edit_thumb_messages(HWND w,LONG object)
{return style_scroll_edit_thumb_messages(w,object,ntw16s_native_get_scroll_info);}

static void draw_scroll(HWND w,control_style *s,HDC dc,LONG object,BOOL vertical)
{
    const win16_control_scroll_view view={&s->palette,&s->scroll,&s->frame_scrollbars,
        &s->edit_scrollbars,&s->list_scrollbar,&s->embedded_object,scroll_layout,
        embedded_scroll_layout,edit_thumb_messages,list_axis_owned};
    win16_control_draw_scroll(w,&view,dc,object,vertical);
}

static void scroll_draw_callback(HWND w,void *context,HDC dc,LONG object,BOOL vertical)
{draw_scroll(w,(control_style *)context,dc,object,vertical);}
static void nonclient_scrollbars(HWND w,control_style *s,HDC target)
{style_scroll_nonclient_paint(w,s,target,scroll_draw_callback);}

void win16_style_scrollbars_paint(HWND w,HDC target,const win16_style_palette *palette)
{win16_control_scrollbars_paint(w,target,palette,control_proc,nonclient_scrollbars);}

static win16_scroll_caret_view scroll_caret_view(control_style *s)
{
    const win16_scroll_caret_view view={&s->detached,&s->scroll_caret,&s->scroll_caret_shown,
        &s->scroll_caret_size,&s->scroll,style_control_is_scroll,scroll_layout};return view;
}
static void scroll_caret_destroy(HWND w,control_style *s)
{const win16_scroll_caret_view view=scroll_caret_view(s);win16_scroll_caret_destroy(w,&view);}
static void scroll_caret_sync(HWND w,control_style *s)
{const win16_scroll_caret_view view=scroll_caret_view(s);win16_scroll_caret_sync(w,&view);}
static void scroll_stop_timer(HWND w,control_style *s)
{style_scroll_stop_timer(w,&s->scroll_timer);}
static void scroll_nonclient_callback(HWND w,void *context,HDC dc)
{nonclient_scrollbars(w,(control_style *)context,dc);}
static void scroll_caret_callback(HWND w,void *context)
{scroll_caret_sync(w,(control_style *)context);}
static win16_scroll_actions_view scroll_action_view(control_style *s)
{
    const win16_scroll_actions_view view={s,&s->detached,&s->scroll_reporting,&s->frame_scrollbars,
        &s->edit_scrollbars,&s->list_scrollbar,&s->scroll_epoch,&s->scroll_report,&s->embedded_object,
        &s->scroll_timer,&s->scroll,scroll_draw_callback,scroll_nonclient_callback,scroll_caret_callback};
    return view;
}
static void scroll_release_capture(HWND w,control_style *s)
{style_scroll_release_capture(w,&s->list_capture_before,s->list_scrollbar || s->edit_scrollbars || s->frame_scrollbars);}
static void scroll_finish(HWND w,control_style *s,BOOL cancel)
{const win16_scroll_actions_view view=scroll_action_view(s);win16_scroll_finish(w,&view,&s->list_capture_before,cancel);}
static BOOL scroll_input(HWND w,control_style *s,UINT m,WPARAM a,LPARAM b)
{
    const win16_scroll_input_view view={scroll_action_view(s),&s->list_capture_before,&s->scroll_key,scroll_layout};
    return win16_scroll_control_input(w,&view,m,a,b);
}

static BOOL scroll_has_owner(void *context){return ((control_style *)context)->owner!=NULL;}
static BOOL embedded_scroll_input(HWND w,control_style *s,UINT m,WPARAM a,LPARAM b)
{
    const win16_scroll_embedded_view view={scroll_action_view(s),&s->list_capture_before,
        scroll_has_owner,embedded_scroll_layout,ntw16s_native_get_scroll_info};
    return win16_scroll_embedded_input(w,&view,m,a,b);
}

/* Only screen WM_PAINT uses this transient surface. Keep WM_PRINTCLIENT on
 * its borrowed DC. Seed the dirty rectangle so group interiors and native
 * combo EDIT/list children excluded by the painter remain unchanged. */
typedef struct present_context {control_style *style;void (*painter)(HWND,control_style *,HDC);} present_context;
static void present_painter(HWND w,void *context,HDC dc)
{present_context *p=context;p->painter(w,p->style,dc);}
static BOOL present_group(HWND w,void *context)
{(void)context;return style_control_class_is(w,L"BUTTON") &&
    (GetWindowLongPtrW(w,GWL_STYLE)&BS_TYPEMASK)==BS_GROUPBOX;}
static void present_control(HWND w,control_style *s,HDC target,void (*painter)(HWND,control_style *,HDC))
{
    present_context context={s,painter};
    const style_present_view view={&s->detached,s->palette.light,&context,present_painter,NULL,present_group};
    style_present(w,&view,target);
}

/* Original LBSetTabStops stores set-time pixels. GdiGetCharDimensions uses
 * the alphabet average for proportional fonts, not tmAveCharWidth. Keep
 * that public-API arithmetic here; private PLBIV/CFONT are not imported. */
static LRESULT forward_list_tabs(HWND w,WPARAM count,LPARAM values)
{return DefSubclassProc(w,LB_SETTABSTOPS,count,values);}
static LRESULT set_list_tabs(HWND w,control_style *s,WPARAM count,LPARAM values)
{
    const win16_control_tabstops_view view={&s->detached,&s->font,&s->tab_count,&s->tab_positions,forward_list_tabs};
    return win16_control_set_list_tabs(w,&view,count,values);
}
static void draw_sizebox(HWND w,control_style *s,HDC dc)
{(void)s;win16_control_draw_sizebox(w,dc);}
/* The v5 progress color and state getters are unavailable. Only paint the
 * verified static edge, after the native procedure; leave the client intact. */
static LRESULT CALLBACK control_proc(HWND w,UINT m,WPARAM a,LPARAM b,UINT_PTR id,DWORD_PTR data)
{
    control_style *s=(control_style *)data;LRESULT result=0;HFONT previous=NULL,pinned=s->font,outbound=NULL;(void)id;++s->calls;
    RECT spin_before={0};HWND spin_button=style_control_vcl_spin_before(w,m,&spin_before);
    BOOL tab=style_control_is_tab(w),header=style_control_is_header(w),progress=style_control_class_is(w,PROGRESS_CLASSW),native_font=tab || header;
    BOOL combo_paint=style_control_vb_combo(w) && (m==WM_PAINT || m==WM_PRINT || m==WM_PRINTCLIENT);if(combo_paint)++s->combo_paints;
    if(progress && win16_style_progress_changes(m))++s->progress_epoch;
    if(tab && win16_style_tab_changes(m))++s->tab_epoch;
    if(header && win16_style_header_changes(m))++s->header_epoch;
    win16_style_system_retain(pinned);
    {
        const win16_control_guard_services guard={ntw16s_rich_valid,ntw16s_vcl_combo_valid,ntw16s_group_button_valid,ntw16s_grid_valid,
            ntw16s_vb_data_valid,ntw16s_vb_user_edge_valid,ntw16s_vcl_toolbar_valid,ntw16s_vcl_tab_valid,ntw16s_vb_user_edge_dormant,
            is_exact_native_button,style_control_vb_checkbox,style_control_vb_option,ntw16s_vb_plain_choice,ntw16s_vb_graphical_choice,
            ntw16s_vb_graphical_button,Ntw16sWindowRemove,win16_style_control_remove};
        if(win16_control_guard_message(w,s,m,a,b,&guard,&result))goto finished;
    }
    if(!win16_control_state_prepare_font(w,s,m,&a,native_font,&previous,&outbound))goto finished;
    {
        const win16_control_scroll_services scroll={embedded_scroll_input,scroll_input,scroll_finish,scroll_caret_sync,scroll_caret_destroy,draw_scroll,list_border,nonclient_scrollbars,present_control,draw_sizebox};
        const win16_control_fileowner_services fileowner={embedded_scroll_input,draw_combo,list_border,nonclient_scrollbars};
        const win16_control_list_services list={embedded_scroll_input,set_list_tabs,present_control,draw_list,list_border,nonclient_scrollbars};
        const win16_control_edit_services edit={embedded_scroll_input,nonclient_scrollbars,list_border,edit_border};
        const win16_control_dispatch_services services={win16_style_control_terminal,style_control_class_is,track_paint_dispatch,trackbar_unbind,
            style_control_is_link,style_control_is_scroll,style_control_is_combo,style_control_list_class,style_control_is_edit,present_control,win16_control_draw_tab,win16_control_draw_header,win16_control_draw_progress,draw_combo,draw,
            &scroll,&fileowner,&list,&edit};
        result=win16_control_dispatch_message(w,s,&m,&a,&b,tab,header,progress,&services);
    }

finished:
    win16_control_finish_message(w,s,m,b,spin_button,&spin_before,combo_paint,previous,outbound,pinned,control_proc);
    return result;
}

static BOOL install(HWND w,const win16_style_palette *palette,BOOL edit,BOOL original_border,BOOL frame_bars,
    int tab_count,const int *tabs,const void *creation_owner,const BOOL *cancelled)
{
    const win16_control_install_services services={control_proc,combo_text_proc,ntw16s_vcl_combo_candidate,
        ntw16s_native_get_combo_info,win16_style_control_install,win16_style_control_remove};
    return win16_control_install(w,palette,edit,original_border,frame_bars,tab_count,tabs,creation_owner,cancelled,
        common_dialog_combo_pending,&services);
}
/* Called only for the known owner-drawn drive combo in a system old-style
 * file dialog. Native item painting and item data stay with COMDLG32. */
BOOL win16_style_common_dialog_combo_install(HWND w,const void *owner)
{
    const win16_control_dialog_combo_services services={install,ntw16s_native_get_combo_info,
        win16_style_grid_install_created,win16_style_list_scrollbar_enable,style_control_is_edit,win16_style_control_install_created};
    return win16_control_dialog_combo_install(w,owner,&common_dialog_combo_pending,&services);
}
BOOL win16_style_control_install(HWND w,const win16_style_palette *palette)
{return install(w,palette,FALSE,FALSE,FALSE,-1,NULL,NULL,NULL);}
BOOL win16_style_control_install_created(HWND w,BOOL edit,BOOL original_border,BOOL tabbed,
    const win16_style_palette *palette,const void *owner,const BOOL *cancelled)
{return install(w,palette,edit,original_border,FALSE,tabbed?0:-1,NULL,owner,cancelled);}
BOOL win16_style_tabbed_list_install(HWND w,const win16_style_palette *palette,int count,const int *pixels)
{if(count<0){SetLastError(ERROR_INVALID_PARAMETER);return FALSE;}return install(w,palette,FALSE,FALSE,FALSE,count,pixels,NULL,NULL);}
BOOL win16_style_frame_scrollbars_install(HWND w,const win16_style_palette *palette)
{return install(w,palette,FALSE,FALSE,TRUE,-1,NULL,NULL,NULL);}
BOOL win16_style_frame_scrollbars_install_owned(HWND w,const win16_style_palette *palette,const void *owner,const BOOL *cancelled)
{
    if(cancelled && *cancelled){SetLastError(ERROR_CANCELLED);return FALSE;}
    return install(w,palette,FALSE,FALSE,TRUE,-1,NULL,owner,cancelled);
}
static const win16_control_managed_services managed_services={control_proc,win16_style_control_remove,
    ntw16s_native_get_scroll_info,ntw16s_native_set_scroll_info,draw_scroll,style_control_list_class,style_control_is_edit};
void win16_style_frame_scrollbars_palette_owned(HWND w,const win16_style_palette *palette,const void *owner)
{win16_control_managed_frame_scrollbars_palette_owned(w,palette,owner,&managed_services);}
static const win16_control_special_services special_services={control_proc,trackbar_notify,toolbar_notify,updown_acquire,win16_control_updown_release,
    win16_style_frame_scrollbars_install_owned,win16_style_control_remove_owned,style_theme_retained,style_theme_capture,ntw16s_theme_enable};
BOOL win16_style_updown_install_created(HWND w,const win16_style_palette *p,const void *owner,const BOOL *cancelled)
{return win16_control_special_updown_install(w,p,owner,cancelled,&special_services);}
BOOL win16_style_trackbar_install_created(HWND w,const win16_style_palette *p,const void *owner,const BOOL *cancelled)
{return win16_control_special_trackbar_install(w,p,owner,cancelled,&special_services);}
BOOL win16_style_toolbar_install_created(HWND w,const win16_style_palette *p,const void *owner,const BOOL *cancelled)
{return win16_control_special_toolbar_install(w,p,owner,cancelled,&special_services);}
BOOL win16_style_progress_install_created(HWND w,const win16_style_palette *p,const void *owner,const BOOL *cancelled)
{return win16_control_special_progress_install(w,p,owner,cancelled,&special_services);}
BOOL win16_style_legacy_progress_border_install_created(HWND w,const win16_style_palette *p,const void *owner,const BOOL *cancelled)
{return win16_control_special_legacy_progress_border_install(w,p,owner,cancelled,&special_services);}
BOOL win16_style_tree_install_created(HWND w,const win16_style_palette *p,const void *owner,const BOOL *cancelled)
{return win16_control_special_tree_install(w,p,owner,cancelled,&special_services);}
BOOL win16_style_grid_install_created(HWND w,const win16_style_palette *p,const void *owner,const BOOL *cancelled)
{return win16_control_special_grid_install(w,p,owner,cancelled,&special_services);}
BOOL win16_style_picture_install_created(HWND w,const win16_style_palette *p,const void *owner,const BOOL *cancelled)
{return win16_control_special_picture_install(w,p,owner,cancelled,&special_services);}

void win16_style_frame_scrollbars_remove(HWND w)
{win16_control_managed_frame_scrollbars_remove(w,&managed_services);}
void win16_style_frame_scrollbars_palette(HWND w,const win16_style_palette *palette)
{win16_control_managed_frame_scrollbars_palette(w,palette,&managed_services);}
BOOL ntw16s_managed_scrollbars_active(HWND w)
{return win16_control_managed_active(w,&managed_services);}
BOOL win16_style_get_scroll_info(HWND w,int bar,LPSCROLLINFO info)
{return win16_control_managed_get_scroll_info(w,bar,info,&managed_services);}
int win16_style_set_scroll_info(HWND w,int bar,LPCSCROLLINFO info,BOOL redraw)
{return win16_control_managed_set_scroll_info(w,bar,info,redraw,&managed_services);}
BOOL win16_style_list_scrollbar_enable(HWND w)
{return win16_control_managed_list_scrollbar_enable(w,&managed_services);}
BOOL win16_style_edit_install(HWND w,BOOL original_border,const win16_style_palette *palette)
{return install(w,palette,TRUE,original_border,FALSE,-1,NULL,NULL,NULL);}
BOOL win16_style_edit_scrollbars_enable(HWND w)
{return win16_control_managed_edit_scrollbars_enable(w,&managed_services);}
static const win16_control_remove_services control_remove_services={control_proc,scroll_finish,trackbar_unbind,toolbar_unbind,
    combo_text_unbind,detach_combo_list,ntw16s_theme_remove,style_theme_terminal,scroll_caret_destroy,
    scroll_stop_timer,scroll_release_capture,terminal_combo_list};
BOOL win16_style_control_remove(HWND w)
{return win16_control_remove(w,&control_remove_services);}
BOOL win16_style_control_remove_owned(HWND w,const void *owner)
{
    DWORD_PTR data;
    if(!owner || GetWindowThreadProcessId(w,NULL)!=GetCurrentThreadId() ||
        !GetWindowSubclass(w,control_proc,(UINT_PTR)control_proc,&data) ||
        ((control_style *)data)->creation_owner!=owner)return FALSE;
    return win16_style_control_remove(w);
}
BOOL win16_style_control_terminal(HWND w)
{return win16_control_terminal(w,&control_remove_services);}
