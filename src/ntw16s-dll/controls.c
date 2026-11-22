#include "lib-win32-style/application/control/guard.h"
#include "lib-win32-style/application/control/control_class.h"
#include "lib-win32-style/application/control/scroll_read.h"
#include "attachment.h"
/* Product policies and shared runtime-role check. */
BOOL ntw16s_control_role_valid(HWND);
BOOL ntw16s_vcl_combo_candidate(HWND);
BOOL ntw16s_group_push_button(HWND);
BOOL ntw16s_group_choice_button(HWND);
BOOL ntw16s_vb_user_edge_dormant(HWND);
#include "controls.h"
#include "../lib-win32-style/application/control/control_chrome.h"
#include "../lib-win32-style/application/control/outline_basic.h"
#include "../lib-win32-style/application/control/outline_list.h"
#include "../lib-win32-style/application/control/outline_combo.h"
#include "../lib-win32-style/application/control/scroll_outline.h"
#include "lib-win32-style/application/control/outline_special.h"
BOOL ntw16s_vb_data_palette(HWND,win16_style_palette *,style_scroll_arrow_art *);
#include "lib-win32-style/application/graphics/present.h"
#include "lib-win32-style/application/control/scroll_state.h"
#include "lib-win32-style/application/control/list_snapshot.h"
#include "lib-win32-style/application/control/control_scope.h"
#include "../lib-win32-style/application/control/outline_choice.h"
#include "lib-win32-style/application/control/control_vcl_spin.h"
#include "lib-win32-style/application/control/outline_state.h"
#include "lib-win32-style/application/control/notify.h"
#include "lib-win32-style/application/control/combo.h"
#include "lib-win32-style/application/control/dialog_combo.h"
#include "lib-win32-style/application/control/outline_dispatch.h"
#include "lib-win32-style/application/control/outline_install.h"

#include "lib-win32-style/application/control/scroll_track.h"
#include "lib-win32-style/appearance/msstyles/raster_font.h"
#include "lib-win32-style/appearance/msstyles/control_theme_style.h"
#include "native_api.h"
#include "theme.h"
#include "lib-win32-style/application/control/trackbar_paint.h"
#include "lib-win32-style/application/control/toolbar_text.h"
#include "lib-win32-style/application/adapters/win32/runtime_text.h"
#include "../lib-win32/process-hook/unwind.h"
#include <commctrl.h>
#include <uxtheme.h>
#include <vssym32.h>
#include <oleacc.h>
#include <stdlib.h>
#include <limits.h>

extern IMAGE_DOS_HEADER __ImageBase;

static _Thread_local style_control_track_calls track_calls;
static LRESULT CALLBACK control_proc(HWND,UINT,WPARAM,LPARAM,UINT_PTR,DWORD_PTR);
static LRESULT CALLBACK trackbar_notify(HWND,UINT,WPARAM,LPARAM,UINT_PTR,DWORD_PTR);
static LRESULT CALLBACK toolbar_notify(HWND,UINT,WPARAM,LPARAM,UINT_PTR,DWORD_PTR);

static style_control_state *lookup_control(HWND window)
{
    style_instance *instance=style_instance_lookup(window,control_proc);
    return instance?&((style_outline_control *)instance)->state:NULL;
}
static const style_control_combo_ops combo_operations={style_raster_font_acquire_stroke_fit,lookup_control};
static LRESULT CALLBACK trackbar_notify(HWND parent,UINT m,WPARAM p,LPARAM l,UINT_PTR id,DWORD_PTR data)
{
    style_outline_control *s=(style_outline_control *)data;
    const win16_style_palette *palette=&s->palette;
    const style_trackbar_parts parts={.axes=s->trackbar_colors,.profile=s->controls.trackbar,
        .palette={.face=palette->face,.light=palette->light,.highlight=palette->light,.shadow=palette->shadow,.dark=palette->edge},
        .supported=style_trackbar_bound_supported};
    const style_control_notify_ops ops={.context=&parts,.supported=style_trackbar_bound_supported,
        .part=style_trackbar_notify_part,.runtime_border=style_trackbar_notify_runtime_border};
    return style_control_notify_message(parent,m,p,l,id,&s->instance,&s->bindings.track,trackbar_notify,&track_calls,&ops);
}
static LRESULT CALLBACK toolbar_notify(HWND parent,UINT m,WPARAM p,LPARAM l,UINT_PTR id,DWORD_PTR data)
{
    style_outline_control *s=(style_outline_control *)data;
    const style_toolbar_outline_view toolbar={
        .palette=&s->palette,.frame=&s->frame_colors,.colors=&s->toolbar_colors,
        .font_points=s->controls.control_font_points,.acquire_font=style_raster_font_acquire,
        .release_font=style_raster_font_release
    };
    const style_control_notify_ops ops={.context=&toolbar,.supported=style_toolbar_outline_supported,
        .part=style_toolbar_outline_part};
    return style_control_notify_message(parent,m,p,l,id,&s->instance,&s->bindings.toolbar,toolbar_notify,NULL,&ops);
}
static BOOL is_exact_native_button(HWND w)
{WCHAR name[32];return GetClassNameW(w,name,ARRAYSIZE(name)) && !lstrcmpiW(name,L"BUTTON");}


static BOOL read_check_glyph(UINT flags,style_check_glyph *glyph)
{return classic_control_checklist_read((HMODULE)&__ImageBase,flags,glyph);}

BOOL win16_style_checklist_glyph(HDC dc,const RECT *rect,UINT flags)
{return style_control_checklist_glyph(dc,rect,flags,read_check_glyph);}

BOOL win16_style_checklist_frame(HDC dc,const RECT *rect,UINT type,UINT flags,const void *address)
{
    const UINT allowed=DFCS_CHECKED|DFCS_PUSHED;
    return type==DFC_BUTTON && (flags&15u)==DFCS_BUTTONCHECK && !(flags&~allowed) &&
        ntw16s_checklist_draw_target(dc,rect,address) && win16_style_checklist_glyph(dc,rect,flags);
}
BOOL win16_style_graphical_frame_prepare(HDC dc,const RECT *rect,UINT type,UINT flags,const void *address,RECT *copy)
{return style_outline_control_graphical_frame_prepare(dc,rect,type,flags,address,copy,control_proc);}
BOOL win16_style_graphical_frame(HDC dc,const RECT *rect,UINT type,UINT flags,const void *address)
{return style_outline_control_graphical_frame(dc,rect,type,flags,address,control_proc);}
/* Keep the bitmap glyph's proportions when a VCL button is shorter than its
 * source font plus the classic border. The application retains its font and
 * layout; only the text image is fitted inside the button's existing face. */
static void draw(HWND w,style_outline_control *s,HDC dc)
{
    BOOL themed=s->themed && style_control_class_is(w,L"BUTTON");
    BOOL label=s->themed && style_control_class_is(w,L"STATIC");
    const style_outline_choice_view view={&s->instance,&s->state,&s->palette,
        themed?&s->push_colors:NULL,&s->choice_colors,
        style_control_button_type,style_control_is_static,style_control_vb_frame,style_control_vb_button,
        ntw16s_group_choice_button,ntw16s_group_push_button,s->choice_module,&s->choice_art,s->read_choice_art,
        label?&s->label_colors:NULL,themed?&s->group_colors:NULL,label?&s->frame_colors:NULL,s->controls,themed};
    style_outline_draw_choice(w,&view,dc);
}

/* The native list owns strings, sorting, selection and scrolling. Query only
 * visible rows; never replace item data or convert application owner-draw. */
static void draw_list(HWND window,style_outline_control *style,HDC dc)
{
    const classic_list_colors *list=&style->list_colors;
    const classic_text_colors *label=&style->label_colors;
    const win16_style_palette *palette=&style->palette;
    const style_list_appearance appearance={
        .font_override=style->state.font && style_control_vb_list(window)?style->state.font:NULL,
        .background=list->present?list->background:palette->light,
        .selection=list->present?list->selection:palette->active,
        .text=label->present?label->normal:palette->edge,
        .selected_text=list->present?list->selection_text:palette->active_text,
        .disabled_text=label->present?label->disabled:palette->shadow,
        .tabs=style->state.tab_positions,.tab_count=style->state.tab_count,.use_tabs=TRUE
    };
    style_control_list_strings(window,&style->instance,dc,&appearance);
}
static void edit_border(HWND w,style_outline_control *s,HDC target)
{
    if(s->edit_border && s->controls.edit)
        style_outline_edit_border(w,target,&s->palette,&s->frame_colors,s->edit_colors,s->controls.edit);
}
static void list_border(HWND w,style_outline_control *s,HDC target)
{style_outline_list_border(w,target,&s->palette,&s->frame_colors);}

static void draw_combo(HWND w,style_outline_control *s,HDC dc)
{
    const style_outline_combo_view view={&s->instance,&s->state,&s->palette,
        &s->combo_colors,&s->list_colors,&s->label_colors,&s->combo_arrow,s->common_dialog_combo};
    style_outline_draw_combo(w,&view,dc,ntw16s_native_get_combo_info,style_control_vb_combo,style_control_vb_drive);
}

/* The runtime owns navigation and paints its native bitmap first. This
 * bounded overlay accepts only observed 96/144-DPI public-DC geometry. */
void win16_style_vb_data_arrow(HDC dc,int x,int y,int width,int height,HDC source,int sx,int sy,int sw,int sh,DWORD rop,const void *caller)
{
    style_outline_control_vb_data_arrow_candidate(dc,x,y,width,height,source,sx,sy,sw,sh,rop,caller,ntw16s_vb_data_palette);
}

static classic_scroll_metrics scroll_metrics(HWND window)
{
    const style_instance *instance=style_instance_lookup(window,control_proc);
    return instance?((const style_outline_control *)instance->context)->scroll_metrics:(classic_scroll_metrics){0};
}
static const style_scroll_defaults scroll_defaults={STYLE_SCROLL_EXTENT_CROSS_AXIS,STYLE_SCROLL_THUMB_FIXED};
static BOOL scroll_layout(HWND w,style_scroll_layout *layout)
{return style_scroll_read_control_layout(w,ntw16s_native_get_scroll_info,scroll_metrics(w),scroll_defaults,layout);}
static BOOL embedded_scroll_layout(HWND w,LONG object,style_scroll_layout *layout,POINT *screen_origin)
{return style_scroll_read_embedded_layout(w,object,ntw16s_native_get_scroll_info,scroll_metrics(w),scroll_defaults,layout,screen_origin);}

static BOOL list_axis_owned(HWND w,LONG object)
{return style_scroll_list_axis_owned(w,object,ntw16s_native_get_scroll_info);}
static BOOL edit_thumb_messages(HWND w,LONG object)
{return style_scroll_edit_thumb_messages(w,object,ntw16s_native_get_scroll_info);}

static void draw_scroll(HWND w,style_outline_control *s,HDC dc,LONG object,BOOL vertical)
{
    win16_style_palette arrows=s->palette;
    if(s->themed){
        arrows.face=s->scroll_arrow_colors.face;
        arrows.edge=s->scroll_arrow_colors.edge;
    }
    const style_scroll_outline_view view={&s->palette,&s->bindings.scroll,scroll_layout,
        embedded_scroll_layout,edit_thumb_messages,list_axis_owned,&arrows,
        s->themed?&s->scroll_colors:NULL,&s->scroll_art};
    style_scroll_outline_paint(w,&view,dc,object,vertical);
}

static void scroll_draw_callback(HWND w,void *context,HDC dc,LONG object,BOOL vertical)
{draw_scroll(w,(style_outline_control *)context,dc,object,vertical);}
static void nonclient_scrollbars(HWND w,style_outline_control *s,HDC target)
{style_scroll_nonclient_paint(w,s,target,scroll_draw_callback);}

void win16_style_scrollbars_paint(HWND w,HDC target,const win16_style_palette *palette)
{style_outline_control_scrollbars_paint(w,target,palette,control_proc,nonclient_scrollbars);}

static void scroll_caret_sync(HWND w,style_outline_control *s)
{
    style_scroll_layout layout;RECT rectangle;
    if(s->instance.detached || !style_control_is_scroll(w))return;
    if(GetFocus()!=w || !IsWindowEnabled(w) || !scroll_layout(w,&layout) || !layout.thumb_fits){
        style_scroll_caret_destroy(w,&s->bindings.scroll.resources);return;}
    rectangle=style_scroll_caret_rectangle(&layout,&s->bindings.scroll.track,max(1,MulDiv(2,(int)GetDpiForWindow(w),96)));
    style_scroll_caret_sync(w,&s->bindings.scroll.resources,&rectangle,s->bindings.scroll.track.active);
}
static void scroll_nonclient_callback(HWND w,void *context,HDC dc)
{nonclient_scrollbars(w,(style_outline_control *)context,dc);}
static void scroll_caret_callback(HWND w,void *context)
{scroll_caret_sync(w,(style_outline_control *)context);}
static BOOL scroll_measure_callback(HWND w,void *context,LONG object,style_scroll_layout *layout,POINT *origin)
{
    (void)context;
    return object==OBJID_CLIENT?scroll_layout(w,layout):embedded_scroll_layout(w,object,layout,origin);
}
static style_scroll_ops scroll_operations(style_outline_control *s)
{
    return (style_scroll_ops){s,&s->scroll_policy,ntw16s_native_get_scroll_info,scroll_measure_callback,
        scroll_draw_callback,scroll_nonclient_callback,scroll_caret_callback};
}
static void scroll_finish(HWND w,style_outline_control *s,BOOL cancel)
{const style_scroll_ops ops=scroll_operations(s);style_scroll_finish(w,&s->instance,&s->bindings.scroll,&ops,cancel);}
static BOOL embedded_scroll_input(HWND w,style_outline_control *s,UINT m,WPARAM a,LPARAM b)
{const style_scroll_ops ops=scroll_operations(s);return style_scroll_embedded_input(w,&s->instance,&s->state,&s->bindings.scroll,&ops,m,a,b);}

static void draw_sizebox(HWND w,style_outline_control *s,HDC dc)
{style_outline_control_draw_sizebox(w,dc,s->themed && s->scroll_colors.own_sizebox?&s->scroll_colors.sizebox.face:NULL);}
/* The v5 progress color and state getters are unavailable. Only paint the
 * verified static edge, after the native procedure; leave the client intact. */
static LRESULT control_dispatch(HWND w,void *context,style_control_message *call)
{
    style_outline_control *s=context;const style_scroll_ops scroll=scroll_operations(s);
    const style_outline_control_fileowner_services fileowner={embedded_scroll_input,draw_combo,list_border,nonclient_scrollbars};
    const style_outline_control_list_services list={embedded_scroll_input,draw_list,list_border,nonclient_scrollbars};
    const style_outline_control_edit_services edit={embedded_scroll_input,nonclient_scrollbars,list_border,edit_border};
    const style_outline_control_dispatch_services services={win16_style_control_terminal,&track_calls,trackbar_notify,
        draw_combo,draw,list_border,draw_sizebox,
        &scroll,&fileowner,&list,&edit};
    return style_outline_control_dispatch_message(w,s,&call->message,&call->parameter,&call->value,call->tab,call->header,call->progress,&services);
}
static LRESULT CALLBACK control_proc(HWND w,UINT m,WPARAM a,LPARAM b,UINT_PTR id,DWORD_PTR data)
{
    (void)id;style_outline_control *s=(style_outline_control *)data;
    const style_control_message_ops ops={s,
        {ntw16s_control_role_valid,ntw16s_vb_user_edge_dormant,is_exact_native_button,style_control_outline_choice_unsupported,Ntw16sWindowRemove,win16_style_control_remove},
        style_outline_control_state_prepare_font,control_dispatch,lookup_control};
    return style_control_message_dispatch(w,&s->instance,&s->state,&s->bindings,m,a,b,&ops);
}

static DWORD load_control_theme(style_control_theme *theme)
{return classic_control_theme_load((HMODULE)&__ImageBase,CLASSIC_WINDOW_PROVIDER_FULL_CLIENT,theme);}

static BOOL install_plain_control(HWND,const win16_style_palette *);
static BOOL install(HWND w,const win16_style_palette *palette,BOOL edit,BOOL original_border,BOOL frame_bars,
    int tab_count,const int *tabs,const void *creation_owner,const BOOL *cancelled)
{
    const style_outline_control_install_services services={control_proc,&combo_operations,ntw16s_vcl_combo_candidate,
        ntw16s_native_get_combo_info,install_plain_control,win16_style_control_remove,load_control_theme,classic_control_outline_font_operations()};
    return style_outline_control_install(w,palette,edit,original_border,frame_bars,tab_count,tabs,creation_owner,cancelled,
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
static BOOL install_plain_control(HWND w,const win16_style_palette *palette)
{return install(w,palette,FALSE,FALSE,FALSE,-1,NULL,NULL,NULL);}
BOOL win16_style_control_install(HWND w,const win16_style_palette *palette)
{
#ifdef NTWSHK_UNIFIED
    if(!ntw16s_explicit_install_allowed(w,palette))return FALSE;
#endif
    return install_plain_control(w,palette);
}
BOOL win16_style_control_install_created(HWND w,BOOL edit,BOOL original_border,BOOL tabbed,
    const win16_style_palette *palette,const void *owner,const BOOL *cancelled)
{return install(w,palette,edit,original_border,FALSE,tabbed?0:-1,NULL,owner,cancelled);}
#ifndef NTWSHK_UNIFIED
BOOL win16_style_tabbed_list_install(HWND w,const win16_style_palette *palette,int count,const int *pixels)
{if(count<0){SetLastError(ERROR_INVALID_PARAMETER);return FALSE;}return install(w,palette,FALSE,FALSE,FALSE,count,pixels,NULL,NULL);}
#endif
BOOL win16_style_frame_scrollbars_install(HWND w,const win16_style_palette *palette)
{
#ifdef NTWSHK_UNIFIED
    if(!ntw16s_explicit_install_allowed(w,palette))return FALSE;
#endif
    return install(w,palette,FALSE,FALSE,TRUE,-1,NULL,NULL,NULL);
}
BOOL win16_style_frame_scrollbars_install_owned(HWND w,const win16_style_palette *palette,const void *owner,const BOOL *cancelled)
{
    if(cancelled && *cancelled){SetLastError(ERROR_CANCELLED);return FALSE;}
    return install(w,palette,FALSE,FALSE,TRUE,-1,NULL,owner,cancelled);
}
static style_scroll_binding lookup_scroll(HWND window)
{
    style_instance *instance=style_instance_lookup(window,control_proc);
    if(!instance)return (style_scroll_binding){0};
    return (style_scroll_binding){instance,&((style_outline_control *)instance->context)->bindings.scroll};
}
static BOOL scroll_eligible(HWND window,style_scroll_role role)
{return role==STYLE_SCROLL_LIST?style_control_list_class(window):style_control_is_edit(window);}
static const style_scroll_managed_ops managed_services={lookup_scroll,win16_style_control_remove,
    ntw16s_native_get_scroll_info,ntw16s_native_set_scroll_info,scroll_draw_callback,scroll_eligible};
void win16_style_frame_scrollbars_palette_owned(HWND w,const win16_style_palette *palette,const void *owner)
{style_outline_control_managed_frame_scrollbars_palette_owned(w,palette,owner,&managed_services);}
static const style_outline_control_special_services special_services={control_proc,trackbar_notify,toolbar_notify,
    win16_style_frame_scrollbars_install_owned,win16_style_control_remove_owned,style_theme_retained,style_theme_capture,ntw16s_theme_enable};
BOOL win16_style_updown_install_created(HWND w,const win16_style_palette *p,const void *owner,const BOOL *cancelled)
{return style_outline_control_special_updown_install(w,p,owner,cancelled,&special_services);}
BOOL win16_style_trackbar_install_created(HWND w,const win16_style_palette *p,const void *owner,const BOOL *cancelled)
{return style_outline_control_special_trackbar_install(w,p,owner,cancelled,&special_services);}
BOOL win16_style_toolbar_install_created(HWND w,const win16_style_palette *p,const void *owner,const BOOL *cancelled)
{return style_outline_control_special_toolbar_install(w,p,owner,cancelled,&special_services);}
BOOL win16_style_progress_install_created(HWND w,const win16_style_palette *p,const void *owner,const BOOL *cancelled)
{return style_outline_control_special_progress_install(w,p,owner,cancelled,&special_services);}
BOOL win16_style_legacy_progress_border_install_created(HWND w,const win16_style_palette *p,const void *owner,const BOOL *cancelled)
{return style_outline_control_special_legacy_progress_border_install(w,p,owner,cancelled,&special_services);}
BOOL win16_style_tree_install_created(HWND w,const win16_style_palette *p,const void *owner,const BOOL *cancelled)
{return style_outline_control_special_tree_install(w,p,owner,cancelled,&special_services);}
BOOL win16_style_grid_install_created(HWND w,const win16_style_palette *p,const void *owner,const BOOL *cancelled)
{return style_outline_control_special_grid_install(w,p,owner,cancelled,&special_services);}
BOOL win16_style_picture_install_created(HWND w,const win16_style_palette *p,const void *owner,const BOOL *cancelled)
{return style_outline_control_special_picture_install(w,p,owner,cancelled,&special_services);}

void win16_style_frame_scrollbars_remove(HWND w)
{style_scroll_managed_frame_remove(w,&managed_services);}
#ifndef NTWSHK_UNIFIED
void win16_style_frame_scrollbars_palette(HWND w,const win16_style_palette *palette)
{style_outline_control_managed_frame_scrollbars_palette(w,palette,&managed_services);}
#endif
BOOL ntw16s_managed_scrollbars_active(HWND w)
{return style_scroll_managed_active(w,&managed_services);}
BOOL win16_style_get_scroll_info(HWND w,int bar,LPSCROLLINFO info)
{return style_scroll_managed_get_info(w,bar,info,&managed_services);}
int win16_style_set_scroll_info(HWND w,int bar,LPCSCROLLINFO info,BOOL redraw)
{return style_scroll_managed_set_info(w,bar,info,redraw,&managed_services);}
BOOL win16_style_list_scrollbar_enable(HWND w)
{return style_scroll_managed_enable(w,STYLE_SCROLL_LIST,&managed_services);}
BOOL win16_style_edit_install(HWND w,BOOL original_border,const win16_style_palette *palette)
{
#ifdef NTWSHK_UNIFIED
    if(!ntw16s_explicit_install_allowed(w,palette))return FALSE;
#endif
    return install(w,palette,TRUE,original_border,FALSE,-1,NULL,NULL,NULL);
}
BOOL win16_style_edit_scrollbars_enable(HWND w)
{return style_scroll_managed_enable(w,STYLE_SCROLL_EDIT,&managed_services);}
static const style_outline_control_remove_services control_remove_services={control_proc,scroll_finish,trackbar_notify,toolbar_notify,
    lookup_control,win16_style_control_remove,ntw16s_theme_remove,style_theme_terminal,win16_style_control_terminal};
BOOL win16_style_control_remove(HWND w)
{return style_outline_control_remove(w,&control_remove_services);}
BOOL win16_style_control_remove_owned(HWND w,const void *owner)
{
    if(!owner || GetWindowThreadProcessId(w,NULL)!=GetCurrentThreadId())return FALSE;
    style_instance *instance=style_instance_lookup(w,control_proc);
    return instance && instance->owner==owner && win16_style_control_remove(w);
}
BOOL win16_style_control_terminal(HWND w)
{return style_outline_control_terminal(w,&control_remove_services);}
