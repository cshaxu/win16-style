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
#include "../lib/win16-control/custom_draw.h"
#include "../lib/win16-control/spin_identity.h"
#include "../lib/win16-control/control_state.h"

#include "../lib/win16-control/scroll_assets.h"
#include "scroll_track.h"
#include "font.h"
#include "native_api.h"
#include "theme.h"
#include "tab_paint.h"
#include "header_paint.h"
#include "progress_paint.h"
#include "toolbar_paint.h"
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

typedef struct track_paint_scope {
    struct track_paint_scope *previous;
    control_style *owner;
    const NMCUSTOMDRAW *draw;
    HDC dc;
    UINT_PTR item;
    RECT rect;
    BOOL active,pending;
} track_paint_scope;
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

/* Keep native-painter borrowed font returns alive through reentrant setters.
 * Finite cache follows the existing 32-lease VB text scope limit. */
static HFONT combo_edit_font(control_style *s,HFONT source)
{
    HFONT font=win16_style_vb_font_acquire(source);if(!font)return source;
    for(unsigned i=0;i<s->combo_font_count;++i)if(s->combo_fonts[i]==font){win16_style_system_release(font);return font;}
    if(s->combo_font_count==ARRAYSIZE(s->combo_fonts)){win16_style_system_release(font);return source;}
    s->combo_fonts[s->combo_font_count++]=font;return font;
}
static void combo_text_unbind(control_style *,BOOL);
static LRESULT CALLBACK combo_text_proc(HWND w,UINT m,WPARAM a,LPARAM b,UINT_PTR id,DWORD_PTR data)
{
    control_style *s=(control_style *)data;++s->calls;LRESULT result;
    HFONT pinned=s->combo_edit_font,outbound=NULL;win16_style_system_retain(pinned);
    BOOL related=!s->detached && s->combo_edit==w && GetParent(w)==s->combo_parent;
    BOOL painting=related && (m==WM_PAINT || m==WM_PRINT || m==WM_PRINTCLIENT);if(painting)++s->combo_paints;
    if(m==WM_NCDESTROY){RemoveWindowSubclass(w,combo_text_proc,id);if(s->combo_edit==w){s->combo_edit=NULL;if(s->combo_bound){s->combo_bound=FALSE;--s->calls;}}}
    if(related && m==WM_GETFONT)result=(LRESULT)(s->combo_paints && s->combo_edit_font?s->combo_edit_font:s->combo_edit_source);
    else {
        if(related && m==WM_SETFONT){
            HFONT source=win16_style_system_source((HFONT)a),font=combo_edit_font(s,source);
            s->combo_edit_source=source;s->combo_edit_font=font;
            outbound=s->combo_edit_font;win16_style_system_retain(outbound);a=(WPARAM)outbound;
        }
        result=DefSubclassProc(w,m,a,b);
    }
    /* A native setter can reenter removal; restore only after it returns. */
    if(!related && m!=WM_NCDESTROY && !s->detached && s->combo_edit==w)combo_text_unbind(s,FALSE);
    if(m==WM_SETFONT && s->detached && IsWindow(w)){
        DWORD_PTR successor;HFONT source=s->combo_edit_source;
        if(GetWindowSubclass(w,combo_text_proc,(UINT_PTR)combo_text_proc,&successor))source=((control_style *)successor)->combo_edit_source;
        else if(GetWindowSubclass(w,control_proc,(UINT_PTR)control_proc,&successor))source=((control_style *)successor)->original_font;
        SendMessageW(w,WM_SETFONT,(WPARAM)source,b);
    }
    if(painting)--s->combo_paints;
    win16_style_system_release(outbound);win16_style_system_release(pinned);
    if(!--s->calls && s->detached)win16_control_state_dispose(s);
    return result;
}
static void combo_text_unbind(control_style *s,BOOL terminal)
{
    HWND edit=s->combo_edit;DWORD_PTR data;
    if(!s->combo_bound)return;
    BOOL owned=edit && GetWindowSubclass(edit,combo_text_proc,(UINT_PTR)combo_text_proc,&data) && data==(DWORD_PTR)s;
    if(owned && !RemoveWindowSubclass(edit,combo_text_proc,(UINT_PTR)combo_text_proc))return;
    s->combo_edit=NULL;s->combo_bound=FALSE;--s->calls;
    if(owned && !terminal && IsWindow(edit)){
        SendMessageW(edit,WM_SETFONT,(WPARAM)s->combo_edit_source,FALSE);
        if(IsWindow(edit))InvalidateRect(edit,NULL,TRUE);
    }
}
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
/* Extra part notifications are private to this renderer, and are requested
 * only after the application returned exactly CDRF_DODEFAULT. Its own requested
 * stages and all non-drawing notifications keep their original route/result. */
static LRESULT CALLBACK trackbar_notify(HWND parent,UINT m,WPARAM p,LPARAM l,UINT_PTR id,DWORD_PTR data)
{
    control_style *s=(control_style *)data;DWORD entry=GetLastError();LRESULT result;BOOL internal=FALSE;
    ++s->calls;NMCUSTOMDRAW *draw=NULL;
    if(m==WM_NOTIFY && l && !s->detached && GetParent(s->track_window)==parent){
        NMHDR *header=(NMHDR *)l;
        if(header->hwndFrom==s->track_window && header->code==NM_CUSTOMDRAW)draw=(NMCUSTOMDRAW *)l;
    }
    if(draw && ntw16s_control_protocol_runtime_trackbar(s->track_window)){
        track_paint_scope *scope=track_paint;
        BOOL match=scope && scope->owner==s && scope->active && scope->draw==draw && scope->dc==draw->hdc;
        if(match && draw->dwDrawStage==CDDS_ITEMPOSTPAINT && scope->pending && scope->item==draw->dwItemSpec){
            scope->pending=FALSE;
            if(EqualRect(&scope->rect,&draw->rc))win16_style_trackbar_runtime_border(s->track_window,draw,&s->palette);
            result=CDRF_DODEFAULT;
        }else if(match && draw->dwDrawStage==CDDS_POSTPAINT){scope->active=FALSE;result=CDRF_DODEFAULT;}
        else {
            SetLastError(entry);result=DefSubclassProc(parent,m,p,l);DWORD error=GetLastError();
            if(!s->detached && scope && track_paint==scope && scope->owner==s){
                if(draw->dwDrawStage==CDDS_PREPAINT && result==CDRF_NOTIFYITEMDRAW &&
                   win16_style_trackbar_supported(s->track_window)){
                    scope->draw=draw;scope->dc=draw->hdc;scope->active=TRUE;
                    result=CDRF_NOTIFYITEMDRAW|CDRF_NOTIFYPOSTPAINT;
                }else if(match && draw->dwDrawStage==CDDS_ITEMPREPAINT && result==CDRF_DODEFAULT && !scope->pending &&
                    (draw->dwItemSpec==TBCD_CHANNEL || draw->dwItemSpec==TBCD_THUMB)){
                    scope->item=draw->dwItemSpec;scope->rect=draw->rc;scope->pending=TRUE;result=CDRF_NOTIFYPOSTPAINT;
                }
            }
            if(!--s->calls && s->detached)win16_control_state_dispose(s);
            SetLastError(error);return result;
        }
        if(!--s->calls && s->detached)win16_control_state_dispose(s);
        SetLastError(entry);return result;
    }
    if(draw && draw->dwDrawStage!=CDDS_PREPAINT && s->track_internal)internal=TRUE;
    if(m==WM_NCDESTROY){RemoveWindowSubclass(parent,trackbar_notify,id);s->track_parent=NULL;s->track_internal=FALSE;}
    SetLastError(entry);
    result=internal?CDRF_DODEFAULT:DefSubclassProc(parent,m,p,l);DWORD error=GetLastError();
    if(draw && !s->detached)result=win16_control_custom_draw_after(s->track_window,draw,&s->track_internal,internal,result,
        &s->palette,win16_style_trackbar_supported,win16_style_trackbar_part);
    if(!--s->calls && s->detached)win16_control_state_dispose(s);
    SetLastError(error);return result;
}
/* The OCX wrapper retains command routing and its return values.  Item drawing
 * is requested only after it selects the default custom-draw route. */
static LRESULT CALLBACK toolbar_notify(HWND parent,UINT m,WPARAM p,LPARAM l,UINT_PTR id,DWORD_PTR data)
{
    control_style *s=(control_style *)data;DWORD entry=GetLastError();LRESULT result;BOOL internal=FALSE;
    ++s->calls;NMCUSTOMDRAW *draw=NULL;
    if(m==WM_NOTIFY && l && !s->detached && GetParent(s->toolbar_window)==parent){
        NMHDR *header=(NMHDR *)l;
        if(header->hwndFrom==s->toolbar_window && header->code==NM_CUSTOMDRAW)draw=(NMCUSTOMDRAW *)l;
    }
    if(draw && draw->dwDrawStage!=CDDS_PREPAINT && s->toolbar_internal)internal=TRUE;
    if(m==WM_NCDESTROY){RemoveWindowSubclass(parent,toolbar_notify,id);s->toolbar_parent=NULL;s->toolbar_internal=FALSE;}
    SetLastError(entry);result=internal?CDRF_DODEFAULT:DefSubclassProc(parent,m,p,l);DWORD error=GetLastError();
    if(draw && !s->detached)result=win16_control_custom_draw_after(s->toolbar_window,draw,&s->toolbar_internal,internal,result,
        &s->palette,win16_style_toolbar_supported,win16_style_toolbar_part);
    if(!--s->calls && s->detached)win16_control_state_dispose(s);
    SetLastError(error);return result;
}

/* DefWindowProc implements WM_SETREDRAW(TRUE) by setting WS_VISIBLE.
 * Never start an internal bracket on an application-hidden control. Nested
 * native leaves share the already-open bracket, whose FALSE cleared that bit. */
static BOOL can_suppress_redraw(HWND w,const control_style *s)
{return !s->list_redraw_off && (s->list_suppression || (GetWindowLongPtrW(w,GWL_STYLE)&WS_VISIBLE));}
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
static BOOL common_dialog_combo_valid(HWND w,HWND parent)
{
    DWORD style=(DWORD)GetWindowLongPtrW(w,GWL_STYLE);
    int id=GetDlgCtrlID(w);
    return w && parent && GetParent(w)==parent && GetWindowThreadProcessId(w,NULL)==GetCurrentThreadId() &&
        is_combo(w) && id>=1136 && id<=1140 && (style&CBS_OWNERDRAWFIXED) &&
        (style&CBS_HASSTRINGS) && !(style&CBS_OWNERDRAWVARIABLE);
}
static BOOL supported(HWND w)
{
    DWORD style=(DWORD)GetWindowLongPtrW(w,GWL_STYLE),type=ntw16s_button_type(w,style);
    if(ntw16s_vb_owner_file(w) || (w==common_dialog_combo_pending && common_dialog_combo_valid(w,GetParent(w))))return TRUE;
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
{
    DWORD style=(DWORD)GetWindowLongPtrW(w,GWL_STYLE);HDC dc;
    if(!(style&(WS_HSCROLL|WS_VSCROLL)))return;
    dc=target?target:GetWindowDC(w);if(!dc)return;
    if(style&WS_HSCROLL)draw_scroll(w,s,dc,OBJID_HSCROLL,FALSE);
    if(style&WS_VSCROLL)draw_scroll(w,s,dc,OBJID_VSCROLL,TRUE);
    if(!target)ReleaseDC(w,dc);
}

typedef struct list_view_state {
    LRESULT top,selection,caret;
} list_view_state;
static list_view_state list_view(HWND w)
{
    list_view_state v;
    v.top=SendMessageW(w,LB_GETTOPINDEX,0,0);
    v.selection=SendMessageW(w,LB_GETCURSEL,0,0);
    v.caret=SendMessageW(w,LB_GETCARETINDEX,0,0);return v;
}

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
{
    HFONT font=s->font;win16_style_system_retain(font);
    if(!win16_style_tab_paint(w,dc,&s->palette,font,&s->detached,&s->tab_epoch) && !s->detached)
        DefSubclassProc(w,WM_PRINTCLIENT,(WPARAM)dc,PRF_CLIENT);
    win16_style_system_release(font);
}
static void draw_header(HWND w,control_style *s,HDC dc)
{
    HFONT font=s->font;win16_style_system_retain(font);
    if(!win16_style_header_paint(w,dc,&s->palette,font,&s->detached,&s->header_epoch,s->header_pressed) && !s->detached)
        DefSubclassProc(w,WM_PRINTCLIENT,(WPARAM)dc,PRF_CLIENT);
    win16_style_system_release(font);
}
static void draw_progress(HWND w,control_style *s,HDC dc)
{
    if(!win16_style_progress_paint(w,dc,&s->palette,&s->detached,&s->progress_epoch) && !s->detached)
        DefSubclassProc(w,WM_PRINTCLIENT,(WPARAM)dc,PRF_CLIENT);
}
/* The v5 progress color and state getters are unavailable. Only paint the
 * verified static edge, after the native procedure; leave the client intact. */
static LRESULT CALLBACK control_proc(HWND w,UINT m,WPARAM a,LPARAM b,UINT_PTR id,DWORD_PTR data)
{
    control_style *s=(control_style *)data;LRESULT result=0;HFONT previous=NULL,pinned=s->font,outbound=NULL;(void)id;++s->calls;
    RECT spin_before={0};HWND spin_button=NULL;DWORD entry_error=GetLastError();
    if(m==WM_SETFONT && (spin_button=win16_control_spinedit_button(w))!=NULL)
        SendMessageW(w,EM_GETRECT,0,(LPARAM)&spin_before);
    SetLastError(entry_error);
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
    if(m==WM_NCDESTROY){(void)win16_style_control_terminal(w);
        result=DefSubclassProc(w,m,a,b);}
    else if(tab){
        if(m==WM_PAINT && win16_style_tab_supported(w)){
            PAINTSTRUCT ps;HDC dc=BeginPaint(w,&ps);present_control(w,s,dc,draw_tab);EndPaint(w,&ps);
        }else if(m==WM_PRINTCLIENT && win16_style_tab_supported(w))draw_tab(w,s,(HDC)a);
        else result=DefSubclassProc(w,m,a,b);
        if(!s->detached && win16_style_tab_changes(m))
            RedrawWindow(w,NULL,NULL,RDW_INVALIDATE|(s->calls==1?RDW_UPDATENOW:0));
    }
    else if(header){
        if(m==WM_LBUTTONDOWN){
            HDHITTESTINFO hit={.pt={(short)LOWORD(b),(short)HIWORD(b)}};
            LRESULT item=SendMessageW(w,HDM_HITTEST,0,(LPARAM)&hit);
            if(s->detached)goto finished;
            s->header_pressed=(item>=0 && (hit.flags&HHT_ONHEADER))?(int)item:-1;
        }
        if(m==WM_LBUTTONUP || m==WM_CANCELMODE || m==WM_CAPTURECHANGED)s->header_pressed=-1;
        if(m==WM_PAINT && win16_style_header_supported(w)){
            PAINTSTRUCT ps;HDC dc=BeginPaint(w,&ps);present_control(w,s,dc,draw_header);EndPaint(w,&ps);
        }else if(m==WM_PRINTCLIENT && win16_style_header_supported(w))draw_header(w,s,(HDC)a);
        else result=DefSubclassProc(w,m,a,b);
        if(!s->detached && win16_style_header_changes(m))
            RedrawWindow(w,NULL,NULL,RDW_INVALIDATE|(s->calls==1?RDW_UPDATENOW:0));
    }
    else if(progress){
        BOOL classic=win16_style_progress_supported(w);
        if(m==WM_PAINT && classic){PAINTSTRUCT ps;HDC dc=BeginPaint(w,&ps);present_control(w,s,dc,draw_progress);EndPaint(w,&ps);}
        else if(m==WM_PRINTCLIENT && classic)draw_progress(w,s,(HDC)a);
        else if(m==WM_PRINT && classic && (b&PRF_CLIENT)){
            int saved=SaveDC((HDC)a);if(saved){result=DefSubclassProc(w,m,a,b&~PRF_CLIENT);RestoreDC((HDC)a,saved);}
            if(!s->detached)draw_progress(w,s,(HDC)a);
        }else result=DefSubclassProc(w,m,a,b);
        if(!s->detached && win16_style_progress_changes(m))
            RedrawWindow(w,NULL,NULL,RDW_INVALIDATE|(s->calls==1?RDW_UPDATENOW:0));
    }
    else if(ntw16s_class_is(w,UPDOWN_CLASSW)){result=DefSubclassProc(w,m,a,b);}
    else if(ntw16s_class_is(w,TRACKBAR_CLASSW) || ntw16s_control_protocol_is(w,TRACKBAR_CLASSW)){
        if(ntw16s_control_protocol_runtime_trackbar(w) && (m==WM_PAINT || m==WM_PRINT || m==WM_PRINTCLIENT))
            result=track_paint_dispatch(s,w,m,a,b);
        else result=DefSubclassProc(w,m,a,b);
        if(!s->detached && m==WM_STYLECHANGED)SendMessageW(w,WM_SYSCOLORCHANGE,0,0);
        if(!s->detached && s->track_parent && GetParent(w)!=s->track_parent){
            trackbar_unbind(s);SendMessageW(w,WM_SYSCOLORCHANGE,0,0);
        }
    }
    else if(is_link(w)){
        /* Preserve native markup, hit testing, focus and NM_CLICK/NM_RETURN.
         * Only the shared WM_SETFONT path above changes link typography. */
        result=DefSubclassProc(w,m,a,b);
    }
    else if(s->frame_scrollbars){
        if(embedded_scroll_input(w,s,m,a,b))result=0;
        else if(m==WM_NCMOUSEMOVE && (a==HTHSCROLL || a==HTVSCROLL))result=0;
        else {if(m==WM_ENABLE && !a)scroll_finish(w,s,TRUE);
            int saved=(s->native_border && m==WM_PRINT)?SaveDC((HDC)a):0;
            if(!s->detached)result=DefSubclassProc(w,m,a,b);
            if(saved)RestoreDC((HDC)a,saved);}
        if(!s->detached && s->native_border && (m==WM_NCPAINT || (m==WM_PRINT && (b&PRF_NONCLIENT)))){
            list_border(w,s,m==WM_PRINT?(HDC)a:NULL);nonclient_scrollbars(w,s,m==WM_PRINT?(HDC)a:NULL);
        }
        if(!s->detached && s->legacy_progress_border && (m==WM_NCPAINT || (m==WM_PRINT && (b&PRF_NONCLIENT))))
            win16_control_legacy_progress_border(w,&s->palette,m==WM_PRINT?(HDC)a:NULL);
        /* Native controls can repaint their own non-client bars while handling
         * input or position changes without issuing WM_NCPAINT. Restore both
         * axes after the native handler, including programmatic list scrolling. */
        if(!s->detached && (m==WM_VSCROLL || m==WM_HSCROLL || m==WM_MOUSEWHEEL || m==WM_MOUSEHWHEEL ||
            m==WM_KEYDOWN || m==WM_KEYUP || m==WM_LBUTTONUP || m==WM_ENABLE || m==WM_SIZE ||
            m==LB_SETTOPINDEX || m==LB_SETCURSEL || m==LB_SETSEL || m==EM_LINESCROLL))
            nonclient_scrollbars(w,s,NULL);
    }
    else if(is_scroll(w) && (GetWindowLongPtrW(w,GWL_STYLE)&SBS_SIZEBOX)){
        if(m==WM_PAINT){PAINTSTRUCT ps;HDC dc=BeginPaint(w,&ps);present_control(w,s,dc,draw_sizebox);EndPaint(w,&ps);}
        else if(m==WM_PRINTCLIENT)win16_control_draw_sizebox(w,(HDC)a);
        else if(m==WM_ERASEBKGND)result=1;
        else result=DefSubclassProc(w,m,a,b);
    }
    else if(is_scroll(w)){
        if(ntw16s_control_protocol_is(w,L"SCROLLBAR") && !IsWindowEnabled(w) && (m==WM_KEYDOWN || m==WM_KEYUP || m==WM_LBUTTONDOWN || m==WM_LBUTTONDBLCLK || m==WM_LBUTTONUP))result=DefSubclassProc(w,m,a,b);
        else if(scroll_input(w,s,m,a,b))result=0;
        else if(m==WM_SETFOCUS || m==WM_SIZE)scroll_caret_sync(w,s);
        else if(m==WM_KILLFOCUS)scroll_caret_destroy(w,s);
        else if(m==SBM_GETSCROLLINFO){SCROLLINFO *info=(SCROLLINFO *)b;
            result=DefSubclassProc(w,m,a,b);
            if(result && info && info->cbSize==sizeof(*info) && (info->fMask&SIF_TRACKPOS)){
                if(s->scroll_reporting)info->nTrackPos=s->scroll_report;
                else if(s->scroll.active)info->nTrackPos=s->scroll.position;}}
        else if(m==WM_PAINT){PAINTSTRUCT ps;HDC dc=BeginPaint(w,&ps);draw_scroll(w,s,dc,OBJID_CLIENT,(GetWindowLongPtrW(w,GWL_STYLE)&SBS_VERT)!=0);EndPaint(w,&ps);scroll_caret_sync(w,s);}
        else if(m==WM_PRINTCLIENT)draw_scroll(w,s,(HDC)a,OBJID_CLIENT,(GetWindowLongPtrW(w,GWL_STYLE)&SBS_VERT)!=0);
        else if(m==WM_ERASEBKGND)result=1;
        else {BOOL redraw=m==SBM_SETPOS?b!=0:m==SBM_SETSCROLLINFO?a!=0:m!=SBM_SETRANGE;
            if(m==WM_ENABLE && !a)scroll_finish(w,s,TRUE);
            if(s->detached)goto finished;
            if(m==SBM_SETPOS)b=FALSE;
            if(m==SBM_SETSCROLLINFO)a=FALSE;
            if(m==SBM_SETRANGEREDRAW)m=SBM_SETRANGE;
            result=DefSubclassProc(w,m,a,b);
            if(redraw && !s->detached && (m==SBM_SETPOS || m==SBM_SETSCROLLINFO || m==SBM_SETRANGE || m==SBM_SETRANGEREDRAW ||
                m==SBM_ENABLE_ARROWS || m==WM_ENABLE || m==WM_SETFOCUS || m==WM_KILLFOCUS ||
                m==WM_LBUTTONDOWN || m==WM_LBUTTONUP || m==WM_KEYDOWN))RedrawWindow(w,NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW);
            if(redraw)scroll_caret_sync(w,s);}
    }
    else if(s->file_owner){
        /* Runtime owner-draw remains authoritative. Overlay chrome only after
         * its native paint; never clear or recreate the image/text item. */
        if(ntw16s_vb_owner_file(w) && ntw16s_vb_directory(w) && embedded_scroll_input(w,s,m,a,b))result=0;
        else result=DefSubclassProc(w,m,a,b);
        if(!s->detached && (ntw16s_vb_owner_file(w) ||
                (s->common_dialog_combo && common_dialog_combo_valid(w,s->common_dialog_parent)))){
            if(m==WM_PAINT){
                if(ntw16s_vb_drive(w) || s->common_dialog_combo){HDC dc=GetDC(w);if(dc){draw_combo(w,s,dc);ReleaseDC(w,dc);}}
                else {list_border(w,s,NULL);nonclient_scrollbars(w,s,NULL);}
            }else if(m==WM_PRINTCLIENT && (ntw16s_vb_drive(w) || s->common_dialog_combo))draw_combo(w,s,(HDC)a);
            else if(m==WM_NCPAINT || (m==WM_PRINT && (b&PRF_NONCLIENT))){
                if(ntw16s_vb_directory(w)){list_border(w,s,m==WM_PRINT?(HDC)a:NULL);nonclient_scrollbars(w,s,m==WM_PRINT?(HDC)a:NULL);}
            }
        }
    }
    else if(is_combo(w)){
        if(m==WM_SETREDRAW){s->list_redraw_off=!a;result=DefSubclassProc(w,m,a,b);}
        else if(m==WM_PAINT){PAINTSTRUCT ps;HDC dc=BeginPaint(w,&ps);present_control(w,s,dc,draw_combo);EndPaint(w,&ps);}
        else if(m==WM_PRINTCLIENT)draw_combo(w,s,(HDC)a);
        else if(m==WM_ERASEBKGND)result=1;
        else {BOOL suppress=can_suppress_redraw(w,s) &&
                (m==CB_SETCURSEL || m==CB_SHOWDROPDOWN || m==WM_COMMAND || m==WM_ENABLE ||
                 m==WM_SETFONT || m==WM_SETFOCUS || m==WM_KILLFOCUS || m==WM_KEYDOWN ||
                 m==WM_LBUTTONDOWN || m==WM_LBUTTONUP);
            if(suppress && s->list_suppression++==0)DefSubclassProc(w,WM_SETREDRAW,FALSE,0);
            result=DefSubclassProc(w,m,a,b);
            if(suppress && --s->list_suppression==0 && !s->detached && !s->list_redraw_off)
                DefSubclassProc(w,WM_SETREDRAW,TRUE,0);
            if(!s->detached && !s->list_suppression && !s->list_redraw_off && (m==CB_SETCURSEL || m==CB_SHOWDROPDOWN || m==WM_COMMAND || m==WM_ENABLE ||
                m==WM_SETFONT || m==WM_SETFOCUS || m==WM_KILLFOCUS || m==WM_KEYDOWN ||
                m==WM_LBUTTONDOWN || m==WM_LBUTTONUP))RedrawWindow(w,NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW);}
    }
    else if(is_list(w)){
        if(embedded_scroll_input(w,s,m,a,b))result=0;
        else if(m==LB_SETTABSTOPS && (GetWindowLongPtrW(w,GWL_STYLE)&LBS_USETABSTOPS))result=set_list_tabs(w,s,a,b);
        else if(m==WM_SETREDRAW){s->list_redraw_off=!a;result=DefSubclassProc(w,m,a,b);}
        else if(m==WM_PAINT){PAINTSTRUCT ps;HDC dc=BeginPaint(w,&ps);present_control(w,s,dc,draw_list);EndPaint(w,&ps);}
        else if(m==WM_PRINTCLIENT)draw_list(w,s,(HDC)a);
        else if(m==WM_NCPAINT || m==WM_PRINT){
            /* This subclass owns the complete list nonclient surface. Native
             * painting first exposes a second scrollbar skin before ours. */
            if(m==WM_PRINT && (b&PRF_CLIENT)){
                HDC dc=(HDC)a;int saved=SaveDC(dc);RECT wr;POINT origin={0,0};
                if(saved){GetWindowRect(w,&wr);ClientToScreen(w,&origin);
                    OffsetViewportOrgEx(dc,origin.x-wr.left,origin.y-wr.top,NULL);
                    draw_list(w,s,dc);RestoreDC(dc,saved);}
            }
            if(!s->detached && (m==WM_NCPAINT || (b&PRF_NONCLIENT))){
                list_border(w,s,m==WM_PRINT?(HDC)a:NULL);
                nonclient_scrollbars(w,s,m==WM_PRINT?(HDC)a:NULL);}
        }
        else if(m==WM_NCMOUSEMOVE || m==WM_NCMOUSELEAVE){
            /* Win3.x has no scrollbar hover skin. Do not start the host's
             * themed hover/leave repaint; clicks still use native tracking. */
            result=0;
        }
        else if(m==WM_ERASEBKGND)result=1;
        else {
            BOOL moving=m==WM_MOUSEMOVE,changed=FALSE;list_view_state before={0},after;
            BOOL suppress=can_suppress_redraw(w,s) &&
                (m==WM_MOUSEWHEEL || m==WM_VSCROLL || m==WM_HSCROLL || m==WM_KEYDOWN || m==WM_KEYUP ||
                m==WM_SETFOCUS || m==WM_KILLFOCUS || m==WM_ENABLE || m==WM_SETFONT ||
                m==WM_MOUSEMOVE || m==WM_LBUTTONDOWN || m==WM_LBUTTONUP ||
                m==LB_ADDSTRING || m==LB_INSERTSTRING || m==LB_DELETESTRING || m==LB_RESETCONTENT ||
                m==LB_SETCURSEL || m==LB_SETSEL || m==LB_SELITEMRANGE || m==LB_SELITEMRANGEEX ||
                m==LB_SETCARETINDEX || m==LB_SETTOPINDEX);
            if(moving)before=list_view(w);
            if(suppress && s->list_suppression++==0)DefSubclassProc(w,WM_SETREDRAW,FALSE,0);
            result=DefSubclassProc(w,m,a,b);
            if(suppress && --s->list_suppression==0 && !s->detached && !s->list_redraw_off)
                DefSubclassProc(w,WM_SETREDRAW,TRUE,0);
            if(!s->detached && moving){after=list_view(w);
                changed=before.top!=after.top || before.selection!=after.selection || before.caret!=after.caret;}
            if(!s->detached && !s->list_suppression && !s->list_redraw_off && (m==LB_SETCURSEL || m==LB_SETSEL || m==LB_SELITEMRANGE || m==LB_SELITEMRANGEEX ||
                m==LB_SETCARETINDEX || m==LB_SETTOPINDEX || m==LB_ADDSTRING || m==LB_INSERTSTRING ||
                m==LB_DELETESTRING || m==LB_RESETCONTENT || m==LB_SETCOLUMNWIDTH || m==LB_SETHORIZONTALEXTENT || m==WM_SETFONT || m==WM_ENABLE ||
                m==WM_SETFOCUS || m==WM_KILLFOCUS || m==WM_LBUTTONDOWN || m==WM_LBUTTONUP ||
                changed || m==WM_KEYDOWN || m==WM_KEYUP || m==WM_VSCROLL || m==WM_HSCROLL || m==WM_MOUSEWHEEL)){
                RedrawWindow(w,NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW);
                if(!s->detached)nonclient_scrollbars(w,s,NULL);}
        }
    }
    else if(is_edit(w)){
        /* EDIT retains its native buffer, selection, caret, undo and IME.
         * Original flat EDIT borders are painted inside the client area. */
        if(embedded_scroll_input(w,s,m,a,b))goto finished;
        if(s->edit_scrollbars && (m==WM_NCMOUSEMOVE || m==WM_NCMOUSELEAVE))goto finished;
        if(s->edit_scrollbars && m==WM_NCPAINT){
            nonclient_scrollbars(w,s,NULL);
            if(!s->detached && (GetWindowLongPtrW(w,GWL_EXSTYLE)&WS_EX_CLIENTEDGE)){
                if(a==1)list_border(w,s,NULL);
                else if(a){
                    HRGN region=CreateRectRgn(0,0,0,0);
                    if(region){
                        if(CombineRgn(region,(HRGN)a,NULL,RGN_COPY)!=ERROR){
                            HDC dc=GetDCEx(w,region,DCX_WINDOW|DCX_CACHE|DCX_INTERSECTRGN);
                            if(dc){list_border(w,s,dc);ReleaseDC(w,dc);region=NULL;}
                        }
                        if(region)DeleteObject(region);
                    }
                }
            }
            goto finished;
        }
        if(s->edit_scrollbars && m==WM_SETREDRAW)s->list_redraw_off=!a;
        {BOOL suppress=s->edit_scrollbars && can_suppress_redraw(w,s) &&
            (m==WM_VSCROLL || m==WM_HSCROLL || m==WM_MOUSEWHEEL || m==EM_LINESCROLL || m==WM_SETTEXT ||
            m==EM_REPLACESEL || m==EM_UNDO || m==WM_UNDO || m==WM_KEYDOWN || m==WM_CHAR ||
            (m==WM_MOUSEMOVE && GetCapture()==w && (a&MK_LBUTTON)) || m==WM_LBUTTONDOWN || m==WM_LBUTTONUP);
        if(suppress && s->list_suppression++==0)DefSubclassProc(w,WM_SETREDRAW,FALSE,0);
        int native_dc=(m==WM_PRINT || m==WM_PRINTCLIENT)?SaveDC((HDC)a):0;
        result=DefSubclassProc(w,m,a,b);
        if(native_dc)RestoreDC((HDC)a,native_dc);
        if(suppress && --s->list_suppression==0 && !s->detached && !s->list_redraw_off)
            DefSubclassProc(w,WM_SETREDRAW,TRUE,0);
        if(suppress && !s->detached)RedrawWindow(w,NULL,NULL,RDW_INVALIDATE|RDW_FRAME|RDW_UPDATENOW);
        }
        if(!s->detached && (m==WM_PRINTCLIENT || (m==WM_PRINT && (b&PRF_CLIENT)) || m==WM_PAINT || m==WM_ENABLE))
            edit_border(w,s,(m==WM_PRINT || m==WM_PRINTCLIENT)?(HDC)a:NULL);
        if(!s->detached && (m==WM_NCPAINT || (m==WM_PRINT && (b&PRF_NONCLIENT))))
            {nonclient_scrollbars(w,s,m==WM_PRINT?(HDC)a:NULL);
             if(ntw16s_vb_edit(w) || ntw16s_native_superclass(w,L"EDIT") ||
                (GetWindowLongPtrW(w,GWL_EXSTYLE)&WS_EX_CLIENTEDGE))list_border(w,s,m==WM_PRINT?(HDC)a:NULL);}
        else if(!s->detached && (m==WM_VSCROLL || m==WM_HSCROLL || m==WM_MOUSEWHEEL ||
            m==EM_LINESCROLL || m==WM_SETTEXT || m==WM_KEYDOWN || m==WM_CHAR))
            nonclient_scrollbars(w,s,NULL);
    }
    /* Bitmap statics have no themed chrome or text. Keep the native image
     * painter and image ownership, rather than inventing a second blitter. */
    else if(is_static(w) && (GetWindowLongPtrW(w,GWL_STYLE)&SS_TYPEMASK)==SS_BITMAP){
        if(m==WM_PRINTCLIENT){
            HDC dc=(HDC)a;int saved=SaveDC(dc);RECT r;
            /* The centered native painter excludes its image rectangle while
             * filling the border. Do not leak that clip into the caller DC. */
            if(saved){GetClientRect(w,&r);IntersectClipRect(dc,r.left,r.top,r.right,r.bottom);
                result=DefSubclassProc(w,m,a,b);RestoreDC(dc,saved);}
        }else result=DefSubclassProc(w,m,a,b);
    }
    /* Native etched STATIC adds WS_EX_STATICEDGE and can leave zero client
     * height. Own its complete line rectangle without changing the stored
     * style or its outer geometry; detach reinstates native NC calculation. */
    else if(is_separator(w) && (m==WM_NCCALCSIZE || m==WM_NCPAINT))result=0;
    else if(ntw16s_vb_frame(w)){
        /* Unlike native GROUPBOX, a VB Frame owns children and paints its
         * windowless labels. Always run the original painter first. */
        result=DefSubclassProc(w,m,a,b);
        if(!s->detached && (m==WM_PAINT || m==WM_PRINTCLIENT ||
                (m==WM_PRINT && (b&PRF_CLIENT)))){
            HDC dc=(m==WM_PAINT)?GetDC(w):(HDC)a;
            if(dc){draw(w,s,dc);if(m==WM_PAINT)ReleaseDC(w,dc);}
        }
    }
    else if(m==WM_SETREDRAW){s->list_redraw_off=!a;result=DefSubclassProc(w,m,a,b);}
    else if(m==WM_PAINT){PAINTSTRUCT ps;HDC dc=BeginPaint(w,&ps);present_control(w,s,dc,draw);EndPaint(w,&ps);}
    else if(m==WM_PRINT && is_separator(w)){
        /* STATIC prints etched shapes internally rather than forwarding
         * WM_PRINTCLIENT. Keep printed and on-screen separators identical. */
        if((b&PRF_CLIENT) && (!(b&PRF_CHECKVISIBLE) || IsWindowVisible(w)))draw(w,s,(HDC)a);
    }
    else if(m==WM_PRINTCLIENT)draw(w,s,(HDC)a);
    else if(m==WM_ERASEBKGND)result=1;
    else {
        BOOL moving=m==WM_MOUSEMOVE && !is_static(w),changed=FALSE;
        LRESULT previous=moving?SendMessageW(w,BM_GETSTATE,0,0):0;
        /* Only bracket native drawing leaves, not BM_CLICK or mouse/key
         * dispatch: automatic radio-group traversal must see the original
         * visibility throughout the outer input operation. */
        BOOL suppress=!is_static(w) && can_suppress_redraw(w,s) &&
            (m==BM_SETCHECK || m==BM_SETSTATE || m==BM_SETSTYLE || m==WM_ENABLE ||
             m==WM_SETFOCUS || m==WM_KILLFOCUS || m==WM_SETTEXT || m==WM_SETFONT ||
             m==WM_MOUSEMOVE);
        if(suppress && s->list_suppression++==0)DefSubclassProc(w,WM_SETREDRAW,FALSE,0);
        result=DefSubclassProc(w,m,a,b);
        if(suppress && --s->list_suppression==0 && !s->detached && !s->list_redraw_off)
            DefSubclassProc(w,WM_SETREDRAW,TRUE,0);
        if(!s->detached && moving)changed=previous!=SendMessageW(w,BM_GETSTATE,0,0);
        if(!s->detached && s->calls==1 && !s->list_suppression && !s->list_redraw_off && (m==BM_SETCHECK || m==BM_SETSTATE || m==BM_SETSTYLE || m==BM_CLICK || m==WM_ENABLE ||
            m==WM_SETFOCUS || m==WM_KILLFOCUS || m==WM_SETTEXT || m==WM_SETFONT ||
            m==BM_SETIMAGE || m==BCM_SETIMAGELIST || (is_static(w) && (m==STM_SETICON || m==STM_SETIMAGE)) ||
            m==WM_LBUTTONDOWN || m==WM_LBUTTONUP || changed || m==WM_KEYDOWN || m==WM_KEYUP))
            RedrawWindow(w,NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW);
    }
finished:
    if(spin_button && !s->detached && IsWindow(w) && win16_control_spinedit_button(w)==spin_button){
        DWORD error=GetLastError();RECT button,after;
        if(GetWindowRect(spin_button,&button)){
            MapWindowPoints(NULL,w,(POINT*)&button,2);
            SendMessageW(w,EM_GETRECT,0,(LPARAM)&after);
            if(!s->detached && spin_before.right>spin_before.left && spin_before.right<=button.left &&
               after.right>button.left && spin_before.right>after.left){
                after.right=spin_before.right;SendMessageW(w,EM_SETRECTNP,0,(LPARAM)&after);
            }
        }
        SetLastError(error);
    }
    if(m==WM_SETFONT && s->detached && IsWindow(w)){
        /* A native setter can finish AFTER its callback removed this leaf,
         * overwriting the font restored during removal with our retired
         * variant. Settle only after that native call has returned. A new
         * independent owner, if installed reentrantly, keeps its own source. */
        DWORD_PTR successor;HFONT source=s->original_font;
        if(GetWindowSubclass(w,control_proc,(UINT_PTR)control_proc,&successor))
            source=((control_style *)successor)->original_font;
        SendMessageW(w,WM_SETFONT,(WPARAM)source,b);
    }
    if(combo_paint)--s->combo_paints;
    /* Native processing (including nested paint/remove/setfont) has finished
     * with the previous lease. Never delete the application's borrowed font. */
    win16_style_system_release(previous);
    win16_style_system_release(outbound);win16_style_system_release(pinned);
    if(!--s->calls && s->detached)win16_control_state_dispose(s);
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
    s->common_dialog_combo=!frame_bars && w==common_dialog_combo_pending && common_dialog_combo_valid(w,GetParent(w));
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
    if(!common_dialog_combo_valid(w,GetParent(w))){SetLastError(ERROR_NOT_SUPPORTED);return FALSE;}
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
