#include "lib-win32-style/application/control/guard.h"
#include "../lib-win32-style/appearance/win3x/control/control_class.h"
#include "lib-win32-style/appearance/win3x/control/scroll_native.h"
#include "attachment.h"
/* Product policies and shared runtime-role check. */
BOOL ntw16s_control_role_valid(HWND);
BOOL ntw16s_vcl_combo_candidate(HWND);
BOOL ntw16s_group_push_button(HWND);
BOOL ntw16s_group_choice_button(HWND);
BOOL ntw16s_vb_data_attached(HWND);
BOOL ntw16s_vb_user_edge_dormant(HWND);
#include "controls.h"
#include "../lib-win32-style/appearance/win3x/control/basic_paint.h"
#include "../lib-win32-style/appearance/win3x/control/other_paint.h"
#include "../lib-win32-style/appearance/win3x/control/list_paint.h"
#include "../lib-win32-style/appearance/win3x/control/combo_paint.h"
#include "../lib-win32-style/appearance/win3x/control/scroll_paint.h"
#include "../lib-win32-style/appearance/win3x/control/special_paint.h"
#include "lib-win32-style/application/graphics/present.h"
#include "lib-win32-style/application/control/scroll_state.h"
#include "lib-win32-style/application/control/control_scope.h"
#include "../lib-win32-style/appearance/win3x/control/choice_view.h"
#include "lib-win32-style/application/control/control_vcl_spin.h"
#include "../lib-win32-style/appearance/win3x/control/control_state.h"
#include "lib-win32-style/application/control/notify.h"
#include "lib-win32-style/application/control/combo.h"
#include "lib-win32-style/application/control/dialog_combo.h"
#include "../lib-win32-style/appearance/win3x/control/control_dispatch.h"
#include "../lib-win32-style/appearance/win3x/control/control_guard.h"
#include "../lib-win32-style/appearance/win3x/control/control_install.h"
#include "../lib-win32-style/appearance/win3x/control/special_install.h"

#include "../lib-win32-style/appearance/win3x/resources/scroll_assets.h"
#include "lib-win32-style/application/control/scroll_track.h"
#include "../lib-win32-style/appearance/win3x/common/font.h"
#include "native_api.h"
#include "theme.h"
#include "lib-win32-style/appearance/win3x/control/tab_paint.h"
#include "lib-win32-style/appearance/win3x/control/header_paint.h"
#include "lib-win32-style/appearance/win3x/control/progress_paint.h"
#include "lib-win32-style/appearance/win3x/control/trackbar_paint.h"
#include "lib-win32-style/appearance/win3x/control/toolbar_paint.h"
#include "../lib-win32-style/appearance/win3x/control/vb_text.h"
#include "../lib-win32/process-hook/unwind.h"
#include <commctrl.h>
#include <uxtheme.h>
#include <vssym32.h>
#include <oleacc.h>
#include <stdlib.h>
#include <limits.h>

static _Thread_local style_control_track_calls track_calls;
static LRESULT CALLBACK control_proc(HWND,UINT,WPARAM,LPARAM,UINT_PTR,DWORD_PTR);
static LRESULT CALLBACK trackbar_notify(HWND,UINT,WPARAM,LPARAM,UINT_PTR,DWORD_PTR);
static LRESULT CALLBACK toolbar_notify(HWND,UINT,WPARAM,LPARAM,UINT_PTR,DWORD_PTR);

static style_control_state *lookup_control(HWND window)
{
    style_instance *instance=style_instance_lookup(window,control_proc);
    return instance?&((control_style *)instance)->state:NULL;
}
static const style_control_combo_ops combo_operations={win16_style_vb_font_acquire,lookup_control};
static BOOL track_border(HWND w,const NMCUSTOMDRAW *draw,const void *context)
{return win16_style_trackbar_runtime_border(w,draw,context);}
static LRESULT CALLBACK trackbar_notify(HWND parent,UINT m,WPARAM p,LPARAM l,UINT_PTR id,DWORD_PTR data)
{
    control_style *s=(control_style *)data;
    const style_control_notify_ops ops={&s->palette,win16_style_trackbar_supported,win16_style_trackbar_part,track_border};
    return style_control_notify_message(parent,m,p,l,id,&s->instance,&s->bindings.track,trackbar_notify,&track_calls,&ops);
}
static LRESULT CALLBACK toolbar_notify(HWND parent,UINT m,WPARAM p,LPARAM l,UINT_PTR id,DWORD_PTR data)
{
    control_style *s=(control_style *)data;
    const style_control_notify_ops ops={&s->palette,win16_style_toolbar_supported,win16_style_toolbar_part,NULL};
    return style_control_notify_message(parent,m,p,l,id,&s->instance,&s->bindings.toolbar,toolbar_notify,NULL,&ops);
}
static BOOL is_exact_native_button(HWND w)
{WCHAR name[32];return GetClassNameW(w,name,ARRAYSIZE(name)) && !lstrcmpiW(name,L"BUTTON");}


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
    const win16_control_choice_view view={&s->instance,&s->state,&s->palette,
        style_control_button_type,style_control_is_static,style_control_vb_frame,style_control_vb_button,
        ntw16s_group_choice_button,ntw16s_group_push_button};
    win16_control_draw_choice(w,&view,dc);
}

/* The native list owns strings, sorting, selection and scrolling. Query only
 * visible rows; never replace item data or convert application owner-draw. */
static void draw_list(HWND w,control_style *s,HDC dc)
{win16_control_draw_list(w,&s->instance,&s->state,&s->palette,dc);}
static void edit_border(HWND w,control_style *s,HDC target)
{win16_control_edit_border(w,s->edit_border,&s->palette,target);}
static void list_border(HWND w,control_style *s,HDC target)
{win16_control_list_border(w,&s->palette,target);}

static void draw_combo(HWND w,control_style *s,HDC dc)
{win16_control_draw_combo(w,&s->instance,&s->state,s->common_dialog_combo,&s->palette,dc,
    ntw16s_native_get_combo_info,style_control_vb_combo,style_control_vb_drive);}


/* The runtime owns navigation and paints its native bitmap first. This
 * bounded overlay accepts only observed 96/144-DPI public-DC geometry. */
void win16_style_vb_data_arrow(HDC dc,int x,int y,int width,int height,HDC source,int sx,int sy,int sw,int sh,DWORD rop,const void *caller)
{
    win16_control_vb_data_arrow_candidate(dc,x,y,width,height,source,sx,sy,sw,sh,rop,caller,ntw16s_vb_data_attached);
}

static BOOL scroll_layout(HWND w,style_scroll_layout *layout)
{return win16_scroll_control_layout(w,ntw16s_native_get_scroll_info,layout);}
static BOOL embedded_scroll_layout(HWND w,LONG object,style_scroll_layout *layout,POINT *screen_origin)
{return win16_scroll_embedded_layout(w,object,ntw16s_native_get_scroll_info,layout,screen_origin);}

static BOOL list_axis_owned(HWND w,LONG object)
{return style_scroll_list_axis_owned(w,object,ntw16s_native_get_scroll_info);}
static BOOL edit_thumb_messages(HWND w,LONG object)
{return style_scroll_edit_thumb_messages(w,object,ntw16s_native_get_scroll_info);}

static void draw_scroll(HWND w,control_style *s,HDC dc,LONG object,BOOL vertical)
{
    const win16_control_scroll_view view={&s->palette,&s->bindings.scroll,scroll_layout,
        embedded_scroll_layout,edit_thumb_messages,list_axis_owned};
    win16_control_draw_scroll(w,&view,dc,object,vertical);
}

static void scroll_draw_callback(HWND w,void *context,HDC dc,LONG object,BOOL vertical)
{draw_scroll(w,(control_style *)context,dc,object,vertical);}
static void nonclient_scrollbars(HWND w,control_style *s,HDC target)
{style_scroll_nonclient_paint(w,s,target,scroll_draw_callback);}

void win16_style_scrollbars_paint(HWND w,HDC target,const win16_style_palette *palette)
{win16_control_scrollbars_paint(w,target,palette,control_proc,nonclient_scrollbars);}

static void scroll_caret_sync(HWND w,control_style *s)
{
    style_scroll_layout layout;RECT rectangle;
    if(s->instance.detached || !style_control_is_scroll(w))return;
    if(GetFocus()!=w || !IsWindowEnabled(w) || !scroll_layout(w,&layout) || !layout.thumb_fits){
        style_scroll_caret_destroy(w,&s->bindings.scroll.resources);return;}
    rectangle=win16_scroll_caret_rectangle(w,&layout,&s->bindings.scroll.track);
    style_scroll_caret_sync(w,&s->bindings.scroll.resources,&rectangle,s->bindings.scroll.track.active);
}
static void scroll_nonclient_callback(HWND w,void *context,HDC dc)
{nonclient_scrollbars(w,(control_style *)context,dc);}
static void scroll_caret_callback(HWND w,void *context)
{scroll_caret_sync(w,(control_style *)context);}
static BOOL scroll_measure_callback(HWND w,void *context,LONG object,style_scroll_layout *layout,POINT *origin)
{
    (void)context;
    return object==OBJID_CLIENT?scroll_layout(w,layout):embedded_scroll_layout(w,object,layout,origin);
}
static style_scroll_ops scroll_operations(control_style *s)
{
    return (style_scroll_ops){s,&win16_scroll_policy,ntw16s_native_get_scroll_info,scroll_measure_callback,
        scroll_draw_callback,scroll_nonclient_callback,scroll_caret_callback};
}
static void scroll_finish(HWND w,control_style *s,BOOL cancel)
{const style_scroll_ops ops=scroll_operations(s);style_scroll_finish(w,&s->instance,&s->bindings.scroll,&ops,cancel);}
static BOOL embedded_scroll_input(HWND w,control_style *s,UINT m,WPARAM a,LPARAM b)
{const style_scroll_ops ops=scroll_operations(s);return style_scroll_embedded_input(w,&s->instance,&s->state,&s->bindings.scroll,&ops,m,a,b);}

static void draw_sizebox(HWND w,control_style *s,HDC dc)
{(void)s;win16_control_draw_sizebox(w,dc);}
/* The v5 progress color and state getters are unavailable. Only paint the
 * verified static edge, after the native procedure; leave the client intact. */
static LRESULT control_dispatch(HWND w,void *context,style_control_message *call)
{
    control_style *s=context;const style_scroll_ops scroll=scroll_operations(s);
    const win16_control_fileowner_services fileowner={embedded_scroll_input,draw_combo,list_border,nonclient_scrollbars};
    const win16_control_list_services list={embedded_scroll_input,draw_list,list_border,nonclient_scrollbars};
    const win16_control_edit_services edit={embedded_scroll_input,nonclient_scrollbars,list_border,edit_border};
    const win16_control_dispatch_services services={win16_style_control_terminal,&track_calls,trackbar_notify,
        draw_combo,draw,list_border,draw_sizebox,
        &scroll,&fileowner,&list,&edit};
    return win16_control_dispatch_message(w,s,&call->message,&call->parameter,&call->value,call->tab,call->header,call->progress,&services);
}
static LRESULT CALLBACK control_proc(HWND w,UINT m,WPARAM a,LPARAM b,UINT_PTR id,DWORD_PTR data)
{
    (void)id;control_style *s=(control_style *)data;
    const style_control_message_ops ops={s,
        {ntw16s_control_role_valid,ntw16s_vb_user_edge_dormant,is_exact_native_button,win16_control_choice_unsupported,Ntw16sWindowRemove,win16_style_control_remove},
        win16_control_state_prepare_font,control_dispatch,lookup_control};
    return style_control_message_dispatch(w,&s->instance,&s->state,&s->bindings,m,a,b,&ops);
}

static BOOL install(HWND w,const win16_style_palette *palette,BOOL edit,BOOL original_border,BOOL frame_bars,
    int tab_count,const int *tabs,const void *creation_owner,const BOOL *cancelled)
{
    const win16_control_install_services services={control_proc,&combo_operations,ntw16s_vcl_combo_candidate,
        ntw16s_native_get_combo_info,win16_style_control_install,win16_style_control_remove};
    return win16_control_install(w,palette,edit,original_border,frame_bars,tab_count,tabs,creation_owner,cancelled,
        style_control_dialog_combo_pending(),&services);
}
/* Called only for the known owner-drawn drive combo in a system old-style
 * file dialog. Native item painting and item data stay with COMDLG32. */
static BOOL dialog_combo_apply(HWND window,style_dialog_combo_part part,BOOL border,const void *owner)
{
    switch(part){
    case STYLE_DIALOG_COMBO:
        return install(window,NULL,FALSE,FALSE,FALSE,-1,NULL,owner,NULL);
    case STYLE_DIALOG_COMBO_LIST:
        if(!win16_style_grid_install_created(window,NULL,owner,NULL))return FALSE;
        return win16_style_list_scrollbar_enable(window);
    case STYLE_DIALOG_COMBO_EDIT:
        return win16_style_control_install_created(window,TRUE,border,FALSE,NULL,owner,NULL);
    }
    return FALSE;
}
BOOL win16_style_common_dialog_combo_install(HWND window,const void *owner)
{
    const style_dialog_combo_ops ops={dialog_combo_apply,ntw16s_native_get_combo_info};
    return style_control_dialog_combo_install(window,owner,&ops);
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
static style_scroll_binding lookup_scroll(HWND window)
{
    style_instance *instance=style_instance_lookup(window,control_proc);
    if(!instance)return (style_scroll_binding){0};
    return (style_scroll_binding){instance,&((control_style *)instance->context)->bindings.scroll};
}
static BOOL scroll_eligible(HWND window,style_scroll_role role)
{return role==STYLE_SCROLL_LIST?style_control_list_class(window):style_control_is_edit(window);}
static const style_scroll_managed_ops managed_services={lookup_scroll,win16_style_control_remove,
    ntw16s_native_get_scroll_info,ntw16s_native_set_scroll_info,scroll_draw_callback,scroll_eligible};
void win16_style_frame_scrollbars_palette_owned(HWND w,const win16_style_palette *palette,const void *owner)
{win16_control_managed_frame_scrollbars_palette_owned(w,palette,owner,&managed_services);}
static const win16_control_special_services special_services={control_proc,trackbar_notify,toolbar_notify,
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
{style_scroll_managed_frame_remove(w,&managed_services);}
void win16_style_frame_scrollbars_palette(HWND w,const win16_style_palette *palette)
{win16_control_managed_frame_scrollbars_palette(w,palette,&managed_services);}
BOOL ntw16s_managed_scrollbars_active(HWND w)
{return style_scroll_managed_active(w,&managed_services);}
BOOL win16_style_get_scroll_info(HWND w,int bar,LPSCROLLINFO info)
{return style_scroll_managed_get_info(w,bar,info,&managed_services);}
int win16_style_set_scroll_info(HWND w,int bar,LPCSCROLLINFO info,BOOL redraw)
{return style_scroll_managed_set_info(w,bar,info,redraw,&managed_services);}
BOOL win16_style_list_scrollbar_enable(HWND w)
{return style_scroll_managed_enable(w,STYLE_SCROLL_LIST,&managed_services);}
BOOL win16_style_edit_install(HWND w,BOOL original_border,const win16_style_palette *palette)
{return install(w,palette,TRUE,original_border,FALSE,-1,NULL,NULL,NULL);}
BOOL win16_style_edit_scrollbars_enable(HWND w)
{return style_scroll_managed_enable(w,STYLE_SCROLL_EDIT,&managed_services);}
static const win16_control_remove_services control_remove_services={control_proc,scroll_finish,trackbar_notify,toolbar_notify,
    lookup_control,win16_style_control_remove,ntw16s_theme_remove,style_theme_terminal,win16_style_control_terminal};
BOOL win16_style_control_remove(HWND w)
{return win16_control_remove(w,&control_remove_services);}
BOOL win16_style_control_remove_owned(HWND w,const void *owner)
{
    if(!owner || GetWindowThreadProcessId(w,NULL)!=GetCurrentThreadId())return FALSE;
    style_instance *instance=style_instance_lookup(w,control_proc);
    return instance && instance->owner==owner && win16_style_control_remove(w);
}
BOOL win16_style_control_terminal(HWND w)
{return win16_control_terminal(w,&control_remove_services);}
