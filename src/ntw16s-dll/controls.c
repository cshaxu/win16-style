#include "control_class.h"
#include "lib/win16-control/scroll_native.h"
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
#include "../lib/win16-control/basic_paint.h"
#include "../lib/win16-control/other_paint.h"
#include "../lib/win16-control/list_paint.h"
#include "../lib/win16-control/combo_paint.h"
#include "../lib/win16-control/scroll_paint.h"
#include "../lib/win16-control/special_paint.h"
#include "../lib/win16-control/present.h"
#include "../lib/win16-control/choice_paint.h"
#include "../lib/win16-control/choice_view.h"
#include "../lib/win16-control/spin_identity.h"
#include "../lib/win16-control/control_state.h"
#include "../lib/win16-control/list_message.h"
#include "../lib/win16-control/edit_message.h"
#include "../lib/win16-control/basic_message.h"
#include "../lib/win16-control/scroll_message.h"
#include "../lib/win16-control/fileowner_message.h"
#include "../lib/win16-control/notify_message.h"
#include "../lib/win16-control/combo_text.h"
#include "../lib/win16-control/native_presentation.h"
#include "../lib/win16-control/control_dispatch.h"
#include "../lib/win16-control/control_lifetime.h"

#include "../lib/win16-control/scroll_assets.h"
#include "scroll_track.h"
#include "font.h"
#include "native_api.h"
#include "theme.h"
#include "tab_paint.h"
#include "header_paint.h"
#include "progress_paint.h"
#include "trackbar_paint.h"
#include "updown_paint.h"
#include "vb_text.h"
#include "injection/unwind.h"
#include <commctrl.h>
#include <uxtheme.h>
#include <vssym32.h>
#include <oleacc.h>
#include <stdlib.h>
#include <limits.h>

static _Thread_local track_paint_scope *track_paint;
static void track_paint_end(void *data)
{track_paint_scope *scope=data;track_paint=scope->previous;}
static NTW16S_UNWIND_FUNCTION LRESULT track_paint_dispatch(control_style *s,HWND w,UINT m,WPARAM a,LPARAM b)
{
    track_paint_scope scope={.previous=track_paint,.owner=s};ntw16s_unwind_scope unwind;
    DWORD entry=GetLastError(),error;LRESULT result;
    track_paint=&scope;NTW16S_UNWIND_BEGIN(unwind,track_paint_end,&scope);
    SetLastError(entry);result=DefSubclassProc(w,m,a,b);error=GetLastError();
    NTW16S_UNWIND_END(unwind);SetLastError(error);return result;
}
static LRESULT CALLBACK control_proc(HWND,UINT,WPARAM,LPARAM,UINT_PTR,DWORD_PTR);
static LRESULT CALLBACK trackbar_notify(HWND,UINT,WPARAM,LPARAM,UINT_PTR,DWORD_PTR);
static LRESULT CALLBACK toolbar_notify(HWND,UINT,WPARAM,LPARAM,UINT_PTR,DWORD_PTR);
static BOOL updown_acquire(void *owner)
{
    control_style *s=owner;DWORD_PTR value;
    if(s->detached || !GetWindowSubclass(s->updown.window,control_proc,(UINT_PTR)control_proc,&value) || value!=(DWORD_PTR)s)return FALSE;
    ++s->calls;return TRUE;
}
static void updown_release(void *owner)
{control_style *s=owner;if(!--s->calls && s->detached)win16_control_state_dispose(s);}

static LRESULT CALLBACK combo_text_proc(HWND w,UINT m,WPARAM a,LPARAM b,UINT_PTR id,DWORD_PTR data)
{return win16_control_combo_text_message(w,m,a,b,id,(control_style *)data,combo_text_proc,control_proc);}
static void combo_text_unbind(control_style *s,BOOL terminal)
{win16_control_combo_text_unbind(s,terminal,combo_text_proc);}
static void trackbar_unbind(control_style *s)
{
    DWORD_PTR value;
    if(s->track_parent && GetWindowSubclass(s->track_parent,trackbar_notify,(UINT_PTR)s->track_window,&value) &&
        value==(DWORD_PTR)s)RemoveWindowSubclass(s->track_parent,trackbar_notify,(UINT_PTR)s->track_window);
    s->track_parent=NULL;s->track_internal=FALSE;
}
static void toolbar_unbind(control_style *s)
{
    DWORD_PTR value;
    if(s->toolbar_parent && GetWindowSubclass(s->toolbar_parent,toolbar_notify,(UINT_PTR)s->toolbar_window,&value) &&
        value==(DWORD_PTR)s)RemoveWindowSubclass(s->toolbar_parent,toolbar_notify,(UINT_PTR)s->toolbar_window);
    s->toolbar_parent=NULL;s->toolbar_internal=FALSE;
}
static LRESULT CALLBACK trackbar_notify(HWND parent,UINT m,WPARAM p,LPARAM l,UINT_PTR id,DWORD_PTR data)
{return win16_control_trackbar_notify(parent,m,p,l,id,(control_style *)data,trackbar_notify,&track_paint);}
static LRESULT CALLBACK toolbar_notify(HWND parent,UINT m,WPARAM p,LPARAM l,UINT_PTR id,DWORD_PTR data)
{return win16_control_toolbar_notify(parent,m,p,l,id,(control_style *)data,toolbar_notify);}
static BOOL is_exact_native_button(HWND w)
{WCHAR name[32];return GetClassNameW(w,name,ARRAYSIZE(name)) && !lstrcmpiW(name,L"BUTTON");}
static BOOL is_static(HWND w)
{return ntw16s_class_is(w,L"STATIC");}
static BOOL is_edit(HWND w)
{return ntw16s_class_is(w,L"EDIT");}
static BOOL is_list(HWND w)
{WCHAR name[16];return ntw16s_class_is(w,L"LISTBOX") || (GetClassNameW(w,name,16) && !lstrcmpiW(name,L"ComboLBox"));}
static BOOL is_combo(HWND w)
{return ntw16s_class_is(w,L"COMBOBOX");}
static BOOL is_scroll(HWND w)
{return ntw16s_class_is(w,L"SCROLLBAR");}
static BOOL is_link(HWND w)
{return ntw16s_class_is(w,L"SysLink") || ntw16s_control_protocol_is(w,WC_LINK);}
static BOOL is_tab(HWND w)
{return ntw16s_class_is(w,WC_TABCONTROLW) || ntw16s_control_protocol_is(w,WC_TABCONTROLW);}
static BOOL is_header(HWND w)
{return ntw16s_class_is(w,WC_HEADERW);}
static BOOL is_separator(HWND w)
{DWORD type=(DWORD)GetWindowLongPtrW(w,GWL_STYLE)&SS_TYPEMASK;
 return is_static(w) && type>=SS_ETCHEDHORZ && type<=SS_ETCHEDFRAME;}

static _Thread_local HWND common_dialog_combo_pending;
static BOOL supported(HWND w)
{
    DWORD style=(DWORD)GetWindowLongPtrW(w,GWL_STYLE),type=ntw16s_button_type(w,style);
    if(ntw16s_vb_owner_file(w) || (w==common_dialog_combo_pending && win16_control_common_dialog_combo_valid(w,GetParent(w))))return TRUE;
    if((ntw16s_vb_checkbox(w) || ntw16s_vb_option(w)) &&
        !ntw16s_vb_plain_choice(w) && !ntw16s_vb_graphical_choice(w))return FALSE;
    if(is_scroll(w))return !(style&SBS_SIZEGRIP);
    if(is_link(w))return TRUE;
    if(is_tab(w))return win16_style_tab_supported(w);
    if(is_header(w))return win16_style_header_supported(w);
    if(ntw16s_class_is(w,PROGRESS_CLASSW) || ntw16s_control_protocol_is(w,PROGRESS_CLASSW))return win16_style_progress_supported(w);
    if(is_combo(w))return ((style&3)==CBS_DROPDOWNLIST || (style&3)==CBS_DROPDOWN || (style&3)==CBS_SIMPLE) &&
        !(style&(CBS_OWNERDRAWFIXED|CBS_OWNERDRAWVARIABLE));
    if(is_list(w))return !(style&(LBS_OWNERDRAWFIXED|LBS_OWNERDRAWVARIABLE|LBS_NODATA|LBS_USETABSTOPS));
    if(is_edit(w))return TRUE;
    if(is_static(w)){type=style&SS_TYPEMASK;return type==SS_LEFT || type==SS_CENTER || type==SS_RIGHT ||
        type==SS_ICON || type==SS_BITMAP || type==SS_SIMPLE || type==SS_LEFTNOWORDWRAP ||
        (type>=SS_BLACKRECT && type<=SS_WHITEFRAME) || (type>=SS_ETCHEDHORZ && type<=SS_ETCHEDFRAME);}
    if(!ntw16s_class_is(w,L"BUTTON"))return FALSE;
    if((style&(BS_ICON|BS_BITMAP)) && type!=BS_PUSHBUTTON && type!=BS_DEFPUSHBUTTON && !(style&BS_PUSHLIKE))return FALSE;
    if(type==BS_PUSHBUTTON || type==BS_DEFPUSHBUTTON || type==BS_GROUPBOX)return TRUE;
    return type==BS_CHECKBOX || type==BS_AUTOCHECKBOX || type==BS_RADIOBUTTON ||
        type==BS_AUTORADIOBUTTON || type==BS_3STATE || type==BS_AUTO3STATE;
}

BOOL win16_style_checklist_frame(HDC dc,const RECT *rect,UINT type,UINT flags,const void *address)
{
    const UINT allowed=DFCS_CHECKED|DFCS_PUSHED;
    if(type!=DFC_BUTTON || (flags&15u)!=DFCS_BUTTONCHECK || (flags&~allowed) ||
        !ntw16s_checklist_draw_target(dc,rect,address))return FALSE;
    return win16_control_checklist_glyph(dc,rect,flags);
}
BOOL win16_style_vb_checklist(HDC dc,int x,int y,int width,int height,HDC source,int sx,int sy,DWORD rop,const void *address)
{return win16_control_vb_checklist(dc,x,y,width,height,source,sx,sy,rop,address);}
static control_style *graphical_target(HDC dc,UINT type,UINT flags,const void *address,HWND *w)
{
    DWORD_PTR value;HMODULE caller=NULL;
    const UINT allowed=DFCS_BUTTONPUSH|DFCS_PUSHED|DFCS_INACTIVE|DFCS_ADJUSTRECT|DFCS_CHECKED;
    if(type!=DFC_BUTTON || !(flags&DFCS_BUTTONPUSH) || (flags&~allowed) ||
            !ntw16s_report_paint_target(dc,w) || !ntw16s_vb_graphical_button(*w) ||
            ((flags&DFCS_CHECKED) && !ntw16s_vb_class(*w,L"ThunderRT6OptionButton") &&
             !ntw16s_vb_class(*w,L"ThunderRT6CheckBox") && !ntw16s_vb5_class(*w,L"ThunderRT5CheckBox") &&
             !ntw16s_vb5_class(*w,L"ThunderRT5OptionButton")) ||
            !GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,(const WCHAR *)address,&caller) ||
            caller!=(HMODULE)GetClassLongPtrW(*w,GCLP_HMODULE) ||
            !GetWindowSubclass(*w,control_proc,(UINT_PTR)control_proc,&value))return NULL;
    control_style *s=(void *)value;return !s->detached && s->frame_scrollbars?s:NULL;
}
BOOL win16_style_graphical_frame_prepare(HDC dc,const RECT *rect,UINT type,UINT flags,const void *address,RECT *copy)
{
    HWND w;
    /* Ordinary API calls remain untouched, including parameter validation.
     * Copy only a trusted owned-runtime primitive inside its exact scope. */
    if(!rect || !copy || !graphical_target(dc,type,flags,address,&w))return FALSE;
    *copy=*rect;return TRUE;
}
BOOL win16_style_graphical_frame(HDC dc,const RECT *rect,UINT type,UINT flags,const void *address)
{
    HWND w;RECT client;POINT origin;control_style *s=graphical_target(dc,type,flags,address,&w);
    if(!s || !rect || !GetClientRect(w,&client) || !EqualRect(rect,&client) ||
            GetMapMode(dc)!=MM_TEXT || (GetLayout(dc)&LAYOUT_RTL) || !GetViewportOrgEx(dc,&origin) || origin.x || origin.y)return FALSE;
    return win16_control_graphical_face(w,dc,rect,flags,&s->palette);
}
/* Keep the bitmap glyph's proportions when a VCL button is shorter than its
 * source font plus the classic border. The application retains its font and
 * layout; only the text image is fitted inside the button's existing face. */
static void draw(HWND w,control_style *s,HDC dc)
{
    const win16_control_choice_view view={&s->detached,&s->palette,&s->font,
        ntw16s_button_type,is_static,ntw16s_vb_frame,ntw16s_vb_button,
        ntw16s_group_choice_button,ntw16s_group_push_button};
    win16_control_draw_choice(w,&view,dc);
}

/* The native list owns strings, sorting, selection and scrolling. Query only
 * visible rows; never replace item data or convert application owner-draw. */
static void draw_list(HWND w,control_style *s,HDC dc)
{win16_control_draw_list(w,&s->detached,&s->palette,&s->font,&s->tab_count,&s->tab_positions,dc,ntw16s_vb_list);}
static void edit_border(HWND w,control_style *s,HDC target)
{win16_control_edit_border(w,s->edit_border,&s->palette,target);}
static void list_border(HWND w,control_style *s,HDC target)
{win16_control_list_border(w,&s->palette,target);}

static void draw_combo(HWND w,control_style *s,HDC dc)
{win16_control_draw_combo(w,&s->detached,&s->common_dialog_combo,&s->palette,&s->font,dc,
    ntw16s_native_get_combo_info,ntw16s_vb_combo,ntw16s_vb_drive);}

static void detach_combo_list(control_style *s)
{
    DWORD_PTR data;HWND list=s->combo_list;s->combo_list=NULL;
    /* An independently removed/reinstalled child is no longer ours. */
    if(list && GetWindowSubclass(list,control_proc,(UINT_PTR)control_proc,&data) &&
        ((control_style *)data)->owner==s)win16_style_control_remove(list);
}
static BOOL terminal_combo_list(control_style *s)
{
    DWORD_PTR data;HWND list=s->combo_list;s->combo_list=NULL;
    /* An independently removed/reinstalled child is no longer ours. */
    if(list && GetWindowSubclass(list,control_proc,(UINT_PTR)control_proc,&data) &&
        ((control_style *)data)->owner==s)return win16_style_control_terminal(list);
    return TRUE;
}

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
{return win16_scroll_list_axis_owned(w,object,ntw16s_native_get_scroll_info);}
static BOOL edit_thumb_messages(HWND w,LONG object)
{return win16_scroll_edit_thumb_messages(w,object,ntw16s_native_get_scroll_info);}

static void draw_scroll(HWND w,control_style *s,HDC dc,LONG object,BOOL vertical)
{
    const win16_control_scroll_view view={&s->palette,&s->scroll,&s->frame_scrollbars,
        &s->edit_scrollbars,&s->list_scrollbar,&s->embedded_object,scroll_layout,
        embedded_scroll_layout,edit_thumb_messages,list_axis_owned};
    win16_control_draw_scroll(w,&view,dc,object,vertical);
}

static void nonclient_scrollbars(HWND w,control_style *s,HDC target)
{win16_control_nonclient_scrollbars(w,s,target,draw_scroll);}

void win16_style_scrollbars_paint(HWND w,HDC target,const win16_style_palette *palette)
{
    control_style s={0};DWORD_PTR attached;
    if(palette)s.palette=*palette;else win16_style_default_palette(&s.palette);
    if(GetWindowSubclass(w,control_proc,(UINT_PTR)control_proc,&attached)){
        control_style *owner=(control_style *)attached;
        if(!palette)s.palette=owner->palette;
        s.list_scrollbar=owner->list_scrollbar;s.edit_scrollbars=owner->edit_scrollbars;s.scroll=owner->scroll;
        s.frame_scrollbars=owner->frame_scrollbars;
        s.embedded_object=owner->embedded_object;
    }
    nonclient_scrollbars(w,&s,target);
}

static win16_scroll_caret_view scroll_caret_view(control_style *s)
{
    const win16_scroll_caret_view view={&s->detached,&s->scroll_caret,&s->scroll_caret_shown,
        &s->scroll_caret_size,&s->scroll,is_scroll,scroll_layout};return view;
}
static void scroll_caret_destroy(HWND w,control_style *s)
{const win16_scroll_caret_view view=scroll_caret_view(s);win16_scroll_caret_destroy(w,&view);}
static void scroll_caret_sync(HWND w,control_style *s)
{const win16_scroll_caret_view view=scroll_caret_view(s);win16_scroll_caret_sync(w,&view);}
static void scroll_stop_timer(HWND w,control_style *s)
{win16_scroll_stop_timer(w,&s->scroll_timer);}
static void scroll_draw_callback(HWND w,void *context,HDC dc,LONG object,BOOL vertical)
{draw_scroll(w,(control_style *)context,dc,object,vertical);}
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
{win16_scroll_release_capture(w,&s->list_capture_before,s->list_scrollbar,s->edit_scrollbars,s->frame_scrollbars);}
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
static void present_control(HWND w,control_style *s,HDC target,void (*painter)(HWND,control_style *,HDC))
{
    present_context context={s,painter};
    const win16_control_present_view view={&s->detached,&s->palette,&context,present_painter};
    win16_control_present(w,&view,target);
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
static void draw_tab(HWND w,control_style *s,HDC dc)
{win16_control_draw_tab(w,s,dc);}
static void draw_header(HWND w,control_style *s,HDC dc)
{win16_control_draw_header(w,s,dc);}
static void draw_progress(HWND w,control_style *s,HDC dc)
{win16_control_draw_progress(w,s,dc);}
/* The v5 progress color and state getters are unavailable. Only paint the
 * verified static edge, after the native procedure; leave the client intact. */
static LRESULT CALLBACK control_proc(HWND w,UINT m,WPARAM a,LPARAM b,UINT_PTR id,DWORD_PTR data)
{
    control_style *s=(control_style *)data;LRESULT result=0;HFONT previous=NULL,pinned=s->font,outbound=NULL;(void)id;++s->calls;
    RECT spin_before={0};HWND spin_button=win16_control_spin_before(w,m,&spin_before);
    BOOL tab=is_tab(w),header=is_header(w),progress=ntw16s_class_is(w,PROGRESS_CLASSW) || ntw16s_control_protocol_is(w,PROGRESS_CLASSW),native_font=tab || header;
    BOOL combo_paint=ntw16s_vb_combo(w) && (m==WM_PAINT || m==WM_PRINT || m==WM_PRINTCLIENT);if(combo_paint)++s->combo_paints;
    if(progress && win16_style_progress_changes(m))++s->progress_epoch;
    if(tab && win16_style_tab_changes(m))++s->tab_epoch;
    if(header && win16_style_header_changes(m))++s->header_epoch;
    win16_style_system_retain(pinned);
    /* A verified VCL control belongs only to its original parent and role.
     * Let the native transition finish, then retire this exact attachment. */
    if(m!=WM_NCDESTROY && (!ntw16s_rich_valid(w) || !ntw16s_vcl_combo_valid(w) || !ntw16s_group_button_valid(w) || !ntw16s_grid_valid(w) || !ntw16s_vb_data_valid(w) || !ntw16s_vb_user_edge_valid(w) || !ntw16s_vcl_toolbar_valid(w) || !ntw16s_vcl_tab_valid(w))){
        if(m==WM_SETFONT)s->original_font=win16_style_system_source((HFONT)a);
        result=DefSubclassProc(w,m,a,b);
        if(!s->detached && IsWindow(w) && !ntw16s_vb_user_edge_dormant(w) && Ntw16sWindowRemove(w) && IsWindow(w))InvalidateRect(w,NULL,TRUE);
        goto finished;
    }
    /* A native BUTTON can become owner-drawn after creation. Its parent must
     * receive WM_DRAWITEM rather than the old ordinary-button painter. */
    if(m!=WM_NCDESTROY && is_exact_native_button(w) &&
       (((DWORD)GetWindowLongPtrW(w,GWL_STYLE)&BS_TYPEMASK)==BS_OWNERDRAW)){
        result=DefSubclassProc(w,m,a,b);
        if(!s->detached && IsWindow(w)){
            if(!Ntw16sWindowRemove(w))win16_style_control_remove(w);
            if(IsWindow(w))InvalidateRect(w,NULL,TRUE);
        }
        goto finished;
    }
    /* Unverified graphical/owner-drawn VB variants stay native. A live
     * transition retires only this painter; do not keep drawing a plain box. */
    if(m!=WM_NCDESTROY && (ntw16s_vb_checkbox(w) || ntw16s_vb_option(w)) &&
            !ntw16s_vb_plain_choice(w) && !ntw16s_vb_graphical_choice(w) && !ntw16s_vb_graphical_button(w)){
        if(m==WM_SETFONT)s->original_font=win16_style_system_source((HFONT)a);
        result=DefSubclassProc(w,m,a,b);
        if(!s->detached)win16_style_control_remove(w);
        goto finished;
    }
    if(!win16_control_state_prepare_font(w,s,m,&a,native_font,&previous,&outbound))goto finished;
    {
        const win16_control_scroll_services scroll={embedded_scroll_input,scroll_input,scroll_finish,scroll_caret_sync,scroll_caret_destroy,draw_scroll,list_border,nonclient_scrollbars,present_control,draw_sizebox};
        const win16_control_fileowner_services fileowner={embedded_scroll_input,draw_combo,list_border,nonclient_scrollbars};
        const win16_control_list_services list={embedded_scroll_input,set_list_tabs,present_control,draw_list,list_border,nonclient_scrollbars};
        const win16_control_edit_services edit={embedded_scroll_input,nonclient_scrollbars,list_border,edit_border};
        const win16_control_dispatch_services services={win16_style_control_terminal,ntw16s_class_is,track_paint_dispatch,trackbar_unbind,
            is_link,is_scroll,is_combo,is_list,is_edit,present_control,draw_tab,draw_header,draw_progress,draw_combo,draw,
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
    DWORD_PTR existing;control_style *s;BOOL result=TRUE,tabbed=tab_count>=0;LONG_PTR style=GetWindowLongPtrW(w,GWL_STYLE);
    if(GetWindowThreadProcessId(w,NULL)!=GetCurrentThreadId()){SetLastError(ERROR_INVALID_PARAMETER);return FALSE;}
    if(GetWindowSubclass(w,control_proc,(UINT_PTR)control_proc,&existing)){SetLastError(ERROR_ALREADY_EXISTS);return FALSE;}
    if(tabbed){
        if(!is_list(w) || !(style&LBS_USETABSTOPS) || (style&(LBS_OWNERDRAWFIXED|LBS_OWNERDRAWVARIABLE|LBS_NODATA))){
            SetLastError(ERROR_NOT_SUPPORTED);return FALSE;}
        if((tab_count && !tabs) || (size_t)tab_count>((size_t)-1)/sizeof(int)){
            SetLastError(ERROR_INVALID_PARAMETER);return FALSE;}
    }else if(!frame_bars && (!supported(w) || is_edit(w)!=edit)){SetLastError(ERROR_NOT_SUPPORTED);return FALSE;}
    s=win16_control_state_create();if(!s)return FALSE;
    s->creation_owner=creation_owner;
    s->header_pressed=-1;
    SetLastError(ERROR_SUCCESS);
    s->protocol=ntw16s_control_protocol_acquire(w);
    if(!s->protocol && GetLastError()==ERROR_ARITHMETIC_OVERFLOW){win16_control_state_dispose(s);SetLastError(ERROR_ARITHMETIC_OVERFLOW);return FALSE;}
    if(tabbed && tab_count){s->tab_positions=malloc((size_t)tab_count*sizeof(int));
        if(!s->tab_positions){win16_control_state_dispose(s);return FALSE;}
        CopyMemory(s->tab_positions,tabs,(size_t)tab_count*sizeof(int));s->tab_count=tab_count;}
    s->edit_border=edit && original_border;
    s->frame_scrollbars=frame_bars;
    s->common_dialog_combo=!frame_bars && w==common_dialog_combo_pending && win16_control_common_dialog_combo_valid(w,GetParent(w));
    s->common_dialog_parent=s->common_dialog_combo?GetParent(w):NULL;
    s->file_owner=!frame_bars && (ntw16s_vb_owner_file(w) || s->common_dialog_combo);s->combo_passthrough=ntw16s_vcl_combo_candidate(w);
    if(!frame_bars){s->original_font=(HFONT)SendMessageW(w,WM_GETFONT,0,0);
        if(cancelled && *cancelled){win16_control_state_dispose(s);SetLastError(ERROR_CANCELLED);return FALSE;}
        /* The font query may reenter installation. Do not replace that
         * successful attachment's subclass data with this outer attempt. */
        if(GetWindowSubclass(w,control_proc,(UINT_PTR)control_proc,&existing)){
            win16_control_state_dispose(s);SetLastError(ERROR_ALREADY_EXISTS);return FALSE;}
        s->original_font=win16_style_system_source(s->original_font);}
    if(palette)s->palette=*palette;else win16_style_default_palette(&s->palette);
    if(!SetWindowSubclass(w,control_proc,(UINT_PTR)control_proc,(DWORD_PTR)s)){win16_control_state_dispose(s);return FALSE;}
    /* Child attachment and WM_SETFONT can synchronously remove/destroy us.
     * Retain this attempt until its result is known, just like a callback. */
    ++s->calls;
    if(!frame_bars && is_combo(w) && !s->file_owner){
        COMBOBOXINFO info={.cbSize=sizeof(info)};
        if(!ntw16s_native_get_combo_info(w,&info) || !win16_style_control_install(info.hwndList,palette)){
            if(!s->detached)win16_style_control_remove(w);
            result=FALSE;
            goto installed;
            }
        if(s->detached){win16_style_control_remove(info.hwndList);goto installed;}
        s->combo_list=info.hwndList;
        if(GetWindowSubclass(info.hwndList,control_proc,(UINT_PTR)control_proc,&existing))
            ((control_style *)existing)->owner=s;
        BOOL native_edit=ntw16s_vb_combo(w) && info.hwndItem!=w && ntw16s_vb_combo_edit(info.hwndItem);
        if(s->detached)goto installed;
        if(native_edit){
            HFONT source=(HFONT)SendMessageW(info.hwndItem,WM_GETFONT,0,0);
            if(s->detached)goto installed;
            s->combo_parent=w;s->combo_edit_source=win16_style_system_source(source);
            DWORD_PTR child_owner;
            if(GetWindowSubclass(info.hwndItem,combo_text_proc,(UINT_PTR)combo_text_proc,&child_owner)){
                win16_style_control_remove(w);SetLastError(ERROR_ALREADY_EXISTS);result=FALSE;goto installed;
            }
            if(!SetWindowSubclass(info.hwndItem,combo_text_proc,(UINT_PTR)combo_text_proc,(DWORD_PTR)s)){
                DWORD error=GetLastError();win16_style_control_remove(w);SetLastError(error);result=FALSE;goto installed;
            }
            s->combo_edit=info.hwndItem;s->combo_bound=TRUE;++s->calls;
            SendMessageW(info.hwndItem,WM_SETFONT,(WPARAM)s->combo_edit_source,FALSE);
            if(s->detached)goto installed;
        }
    }
    if(!frame_bars){if(s->combo_passthrough && ((DWORD)GetWindowLongPtrW(w,GWL_STYLE)&3)==CBS_DROPDOWNLIST){HFONT replacement=win16_style_system_acquire(s->original_font);s->font=replacement?replacement:s->original_font;}
        if(!s->combo_passthrough)SendMessageW(w,WM_SETFONT,(WPARAM)s->original_font,FALSE);
        if(!s->detached && is_separator(w))SetWindowPos(w,NULL,0,0,0,0,
            SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE|SWP_FRAMECHANGED);
        if(!s->detached)RedrawWindow(w,NULL,NULL,RDW_INVALIDATE|RDW_FRAME);}
installed:
    if(s->detached){result=FALSE;SetLastError(ERROR_CANCELLED);}
    if(!--s->calls && s->detached)win16_control_state_dispose(s);
    return result;
}
/* Called only for the known owner-drawn drive combo in a system old-style
 * file dialog. Native item painting and item data stay with COMDLG32. */
BOOL win16_style_common_dialog_combo_install(HWND w,const void *owner)
{
    HWND previous=common_dialog_combo_pending;
    if(!win16_control_common_dialog_combo_valid(w,GetParent(w))){SetLastError(ERROR_NOT_SUPPORTED);return FALSE;}
    common_dialog_combo_pending=w;
    BOOL result=install(w,NULL,FALSE,FALSE,FALSE,-1,NULL,owner,NULL);
    common_dialog_combo_pending=previous;
    if(result && IsWindow(w)){
        COMBOBOXINFO info={.cbSize=sizeof(info)};
        if(ntw16s_native_get_combo_info(w,&info)){
            if(info.hwndList && win16_style_grid_install_created(info.hwndList,NULL,owner,NULL))
                (void)win16_style_list_scrollbar_enable(info.hwndList);
            if(info.hwndItem && info.hwndItem!=w && is_edit(info.hwndItem))
                (void)win16_style_control_install_created(info.hwndItem,TRUE,
                    (GetWindowLongPtrW(info.hwndItem,GWL_STYLE)&WS_BORDER)!=0,FALSE,NULL,owner,NULL);
        }
    }
    return result;
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
void win16_style_frame_scrollbars_palette_owned(HWND w,const win16_style_palette *palette,const void *owner)
{
    DWORD_PTR data;
    if(GetWindowSubclass(w,control_proc,(UINT_PTR)control_proc,&data) &&
        ((control_style *)data)->frame_scrollbars && ((control_style *)data)->creation_owner==owner)
        ((control_style *)data)->palette=*palette;
}
BOOL win16_style_updown_install_created(HWND w,const win16_style_palette *palette,const void *owner,const BOOL *cancelled)
{
    DWORD_PTR value;HMODULE native=ntw16s_control_protocol_updown_module(w);
    if(!native || !ntw16s_updown_api_enabled()){SetLastError(ERROR_NOT_SUPPORTED);return FALSE;}
    if(!win16_style_frame_scrollbars_install_owned(w,palette,owner,cancelled))return FALSE;
    if(!GetWindowSubclass(w,control_proc,(UINT_PTR)control_proc,&value) ||
        ((control_style *)value)->creation_owner!=owner){SetLastError(ERROR_CANCELLED);return FALSE;}
    control_style *s=(control_style *)value;
    ntw16s_updown_register(&s->updown,w,native,&s->palette,win16_control_scroll_glyphs(),s,updown_acquire,updown_release);
    return TRUE;
}
BOOL win16_style_trackbar_install_created(HWND w,const win16_style_palette *palette,const void *owner,const BOOL *cancelled)
{
    DWORD_PTR value;
    if(!win16_style_trackbar_supported(w)){SetLastError(ERROR_NOT_SUPPORTED);return FALSE;}
    if(!win16_style_frame_scrollbars_install_owned(w,palette,owner,cancelled))return FALSE;
    if(!GetWindowSubclass(w,control_proc,(UINT_PTR)control_proc,&value) ||
        ((control_style *)value)->creation_owner!=owner){SetLastError(ERROR_CANCELLED);return FALSE;}
    control_style *s=(control_style *)value;++s->calls;s->track_window=w;s->track_parent=GetParent(w);
    BOOL okay=s->track_parent && SetWindowSubclass(s->track_parent,trackbar_notify,(UINT_PTR)w,(DWORD_PTR)s);
    /* Native trackbars cache part drawing. Invalidate only this new control's
     * native cache after binding, without changing theme/global properties. */
    if(okay)SendMessageW(w,WM_SYSCOLORCHANGE,0,0);
    if(s->detached)okay=FALSE;
    if(!okay && !s->detached)win16_style_control_remove_owned(w,owner);
    if(!--s->calls && s->detached)win16_control_state_dispose(s);
    return okay;
}
BOOL win16_style_toolbar_install_created(HWND w,const win16_style_palette *palette,const void *owner,const BOOL *cancelled)
{
    DWORD_PTR value;
    if(!ntw16s_control_protocol_toolbar(w)){SetLastError(ERROR_NOT_SUPPORTED);return FALSE;}
    if(!win16_style_frame_scrollbars_install_owned(w,palette,owner,cancelled))return FALSE;
    if(!GetWindowSubclass(w,control_proc,(UINT_PTR)control_proc,&value) ||
        ((control_style *)value)->creation_owner!=owner){SetLastError(ERROR_CANCELLED);return FALSE;}
    control_style *s=(control_style *)value;++s->calls;s->toolbar_window=w;s->toolbar_parent=GetParent(w);
    BOOL okay=s->toolbar_parent && SetWindowSubclass(s->toolbar_parent,toolbar_notify,(UINT_PTR)w,(DWORD_PTR)s);
    if(s->detached)okay=FALSE;
    if(!okay && !s->detached)win16_style_control_remove_owned(w,owner);
    if(!--s->calls && s->detached)win16_control_state_dispose(s);
    return okay;
}
BOOL win16_style_progress_install_created(HWND w,const win16_style_palette *palette,const void *owner,const BOOL *cancelled)
{
    if(!win16_style_progress_supported(w)){SetLastError(ERROR_NOT_SUPPORTED);return FALSE;}
    /* No text or font property belongs to this painter. Reuse the owned
     * passive-font leaf; the progress branch owns its paint, not bar input. */
    return win16_style_frame_scrollbars_install_owned(w,palette,owner,cancelled);
}
BOOL win16_style_legacy_progress_border_install_created(HWND w,const win16_style_palette *palette,const void *owner,const BOOL *cancelled)
{
    DWORD_PTR data;
    if(!ntw16s_control_protocol_legacy_progress(w)){SetLastError(ERROR_NOT_SUPPORTED);return FALSE;}
    if(!win16_style_frame_scrollbars_install_owned(w,palette,owner,cancelled))return FALSE;
    if(!GetWindowSubclass(w,control_proc,(UINT_PTR)control_proc,&data) ||
        ((control_style *)data)->creation_owner!=owner){SetLastError(ERROR_CANCELLED);return FALSE;}
    ((control_style *)data)->legacy_progress_border=TRUE;
    RedrawWindow(w,NULL,NULL,RDW_INVALIDATE|RDW_FRAME);
    return TRUE;
}
BOOL win16_style_tree_install_created(HWND w,const win16_style_palette *palette,const void *owner,const BOOL *cancelled)
{
    DWORD_PTR data;
    if(!ntw16s_control_protocol_is(w,WC_TREEVIEWW)){SetLastError(ERROR_NOT_SUPPORTED);return FALSE;}
    if(!win16_style_frame_scrollbars_install_owned(w,palette,owner,cancelled))return FALSE;
    if(!GetWindowSubclass(w,control_proc,(UINT_PTR)control_proc,&data) ||
        ((control_style *)data)->creation_owner!=owner){SetLastError(ERROR_CANCELLED);return FALSE;}
    ((control_style *)data)->native_border=TRUE;
    return TRUE;
}
BOOL win16_style_grid_install_created(HWND w,const win16_style_palette *palette,const void *owner,const BOOL *cancelled)
{
    DWORD_PTR data;control_style *s;BOOL result=TRUE;
    if(!win16_style_frame_scrollbars_install_owned(w,palette,owner,cancelled))return FALSE;
    if(!GetWindowSubclass(w,control_proc,(UINT_PTR)control_proc,&data) ||
        ((control_style *)data)->creation_owner!=owner){SetLastError(ERROR_CANCELLED);return FALSE;}
    s=(control_style *)data;++s->calls;s->native_border=TRUE;
    /* Owner-drawn lists use the shared frame scrollbar painter but may not
     * have the ordinary control attachment's theme lease. Common-controls
     * theme animations can otherwise repaint over that scrollbar later. */
    if(is_list(w) && !ntw16s_theme_retained(w)){
        if(!ntw16s_theme_capture(w))result=FALSE;
        else {s->list_theme_owned=TRUE;if(!ntw16s_theme_enable(w,s))result=FALSE;}
    }
    if(s->detached || (cancelled && *cancelled)){result=FALSE;SetLastError(ERROR_CANCELLED);}
    if(!result && !s->detached)win16_style_control_remove_owned(w,owner);
    if(!--s->calls && s->detached)win16_control_state_dispose(s);
    return result;
}
BOOL win16_style_picture_install_created(HWND w,const win16_style_palette *palette,const void *owner,const BOOL *cancelled)
{
    DWORD_PTR data;
    if(!ntw16s_vb_picture(w)){SetLastError(ERROR_NOT_SUPPORTED);return FALSE;}
    if(!win16_style_frame_scrollbars_install_owned(w,palette,owner,cancelled))return FALSE;
    if(!GetWindowSubclass(w,control_proc,(UINT_PTR)control_proc,&data) ||
        ((control_style *)data)->creation_owner!=owner){SetLastError(ERROR_CANCELLED);return FALSE;}
    ((control_style *)data)->native_border=TRUE;
    return TRUE;
}
void win16_style_frame_scrollbars_remove(HWND w)
{
    DWORD_PTR data;
    if(GetWindowSubclass(w,control_proc,(UINT_PTR)control_proc,&data) && ((control_style *)data)->frame_scrollbars)
        win16_style_control_remove(w);
}
void win16_style_frame_scrollbars_palette(HWND w,const win16_style_palette *palette)
{
    DWORD_PTR data;
    if(GetWindowSubclass(w,control_proc,(UINT_PTR)control_proc,&data) && ((control_style *)data)->frame_scrollbars)
        ((control_style *)data)->palette=*palette;
}
BOOL ntw16s_managed_scrollbars_active(HWND w)
{
    DWORD_PTR data;
    return w && GetWindowThreadProcessId(w,NULL)==GetCurrentThreadId() &&
        GetWindowSubclass(w,control_proc,(UINT_PTR)control_proc,&data) &&
        !((control_style *)data)->detached &&
        (((control_style *)data)->frame_scrollbars || ((control_style *)data)->list_scrollbar || ((control_style *)data)->edit_scrollbars);
}
BOOL win16_style_get_scroll_info(HWND w,int bar,LPSCROLLINFO info)
{
    DWORD_PTR data;BOOL result=ntw16s_native_get_scroll_info(w,bar,info);DWORD error=GetLastError();
    if(result && (bar==SB_HORZ || bar==SB_VERT) && info && info->cbSize==sizeof(*info) &&
        (info->fMask&SIF_TRACKPOS) && GetWindowSubclass(w,control_proc,(UINT_PTR)control_proc,&data)){
        control_style *s=(control_style *)data;
        if((s->frame_scrollbars || s->list_scrollbar || s->edit_scrollbars) &&
            s->embedded_object==(bar==SB_HORZ?OBJID_HSCROLL:OBJID_VSCROLL)){
            if(s->scroll_reporting)info->nTrackPos=s->scroll_report;
            else if(s->scroll.active)info->nTrackPos=s->scroll.position;
        }
    }
    SetLastError(error);return result;
}
int win16_style_set_scroll_info(HWND w,int bar,LPCSCROLLINFO info,BOOL redraw)
{
    DWORD_PTR data;int result;DWORD error;
    BOOL styled=(bar==SB_HORZ || bar==SB_VERT) &&
        GetWindowSubclass(w,control_proc,(UINT_PTR)control_proc,&data) &&
        (((control_style *)data)->frame_scrollbars || ((control_style *)data)->list_scrollbar || ((control_style *)data)->edit_scrollbars);
    result=ntw16s_native_set_scroll_info(w,bar,info,styled?FALSE:redraw);
    error=GetLastError();
    if(styled && redraw){
        if(GetWindowSubclass(w,control_proc,(UINT_PTR)control_proc,&data)){
            HDC dc=GetWindowDC(w);
            if(dc){draw_scroll(w,(control_style *)data,dc,bar==SB_HORZ?OBJID_HSCROLL:OBJID_VSCROLL,bar==SB_VERT);ReleaseDC(w,dc);}
        }else RedrawWindow(w,NULL,NULL,RDW_FRAME|RDW_INVALIDATE);
    }
    SetLastError(error);return result;
}
BOOL win16_style_list_scrollbar_enable(HWND w)
{
    DWORD_PTR data;
    if(GetWindowThreadProcessId(w,NULL)!=GetCurrentThreadId() || !is_list(w) ||
        !GetWindowSubclass(w,control_proc,(UINT_PTR)control_proc,&data)){
        SetLastError(ERROR_NOT_SUPPORTED);return FALSE;}
    ((control_style *)data)->list_scrollbar=TRUE;
    RedrawWindow(w,NULL,NULL,RDW_FRAME|RDW_INVALIDATE);return TRUE;
}
BOOL win16_style_edit_install(HWND w,BOOL original_border,const win16_style_palette *palette)
{return install(w,palette,TRUE,original_border,FALSE,-1,NULL,NULL,NULL);}
BOOL win16_style_edit_scrollbars_enable(HWND w)
{
    DWORD_PTR data;LONG_PTR style=GetWindowLongPtrW(w,GWL_STYLE);
    if(GetWindowThreadProcessId(w,NULL)!=GetCurrentThreadId() || !is_edit(w) ||
        !(style&ES_MULTILINE) ||
        !GetWindowSubclass(w,control_proc,(UINT_PTR)control_proc,&data)){
        SetLastError(ERROR_NOT_SUPPORTED);return FALSE;}
    ((control_style *)data)->edit_scrollbars=TRUE;
    RedrawWindow(w,NULL,NULL,RDW_FRAME|RDW_INVALIDATE);return TRUE;
}
BOOL win16_style_control_remove(HWND w)
{
    DWORD_PTR data;control_style *s;
    if(GetWindowThreadProcessId(w,NULL)!=GetCurrentThreadId() ||
        !GetWindowSubclass(w,control_proc,(UINT_PTR)control_proc,&data))return FALSE;
    s=(control_style *)data;
    if(s->removing)return TRUE;
    ++s->calls;s->removing=TRUE;
    /* End the live gesture while queries still see its full-width report.
     * The application may remove again or destroy the HWND in this notice. */
    scroll_finish(w,s,TRUE);
    if(s->detached){if(!--s->calls)win16_control_state_dispose(s);return TRUE;}
    if(!RemoveWindowSubclass(w,control_proc,(UINT_PTR)control_proc)){
        s->removing=FALSE;--s->calls;return FALSE;}
    s->detached=TRUE;ntw16s_updown_retire(&s->updown);BOOL track=s->track_window!=NULL;trackbar_unbind(s);toolbar_unbind(s);combo_text_unbind(s,FALSE);detach_combo_list(s);
    if(s->list_theme_owned){s->list_theme_owned=FALSE;ntw16s_theme_remove(w,s);}
    if(s->font && IsWindow(w) && !s->combo_passthrough)SendMessageW(w,WM_SETFONT,(WPARAM)s->original_font,FALSE);
    if(IsWindow(w) && is_separator(w))SetWindowPos(w,NULL,0,0,0,0,
        SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE|SWP_FRAMECHANGED);
    scroll_caret_destroy(w,s);
    if(s->list_suppression && !s->list_redraw_off)SendMessageW(w,WM_SETREDRAW,TRUE,0);
    scroll_stop_timer(w,s);
    if(s->scroll.active){s->scroll.active=FALSE;scroll_release_capture(w,s);}
    if(!--s->calls)win16_control_state_dispose(s);
    if(track && IsWindow(w))SendMessageW(w,WM_SYSCOLORCHANGE,0,0);
    RedrawWindow(w,NULL,NULL,RDW_INVALIDATE|RDW_FRAME);
    if(is_scroll(w) && GetFocus()==w)SendMessageW(w,WM_SETFOCUS,0,0);
    return TRUE;
}
BOOL win16_style_control_remove_owned(HWND w,const void *owner)
{
    DWORD_PTR data;
    if(!owner || GetWindowThreadProcessId(w,NULL)!=GetCurrentThreadId() ||
        !GetWindowSubclass(w,control_proc,(UINT_PTR)control_proc,&data) ||
        ((control_style *)data)->creation_owner!=owner)return FALSE;
    return win16_style_control_remove(w);
}
BOOL win16_style_control_terminal(HWND w)
{
    DWORD_PTR data;control_style *s;
    BOOL result;
    if(!GetWindowSubclass(w,control_proc,(UINT_PTR)control_proc,&data))return TRUE;
    s=(control_style *)data;
    if(!RemoveWindowSubclass(w,control_proc,(UINT_PTR)control_proc)){
        SetLastError(ERROR_INVALID_STATE);return FALSE;
    }
    ++s->calls;s->detached=TRUE;ntw16s_updown_retire(&s->updown);trackbar_unbind(s);toolbar_unbind(s);
    if(s->list_theme_owned){s->list_theme_owned=FALSE;(void)ntw16s_theme_terminal(w,s);}
    scroll_caret_destroy(w,s);
    scroll_stop_timer(w,s);
    combo_text_unbind(s,TRUE);result=terminal_combo_list(s);
    if(!--s->calls)win16_control_state_dispose(s);
    return result;
}
