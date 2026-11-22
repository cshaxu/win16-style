#include "guard.h"
#include "control_class.h"
#include "tab_snapshot.h"
#include "header_snapshot.h"
#include "progress_snapshot.h"
BOOL style_control_guard_message(HWND w,style_instance *instance,style_control_state *state,UINT m,WPARAM a,LPARAM b,const style_control_guard_ops *ops,LRESULT *result)
{
    if(m!=WM_NCDESTROY && !ops->valid(w)){
        if(m==WM_SETFONT)state->original_font=state->fonts->source((HFONT)a);
        *result=DefSubclassProc(w,m,a,b);
        if(!instance->detached && IsWindow(w) && !ops->dormant(w) && ops->remove_window(w) && IsWindow(w))InvalidateRect(w,NULL,TRUE);
        return TRUE;
    }
    if(m!=WM_NCDESTROY && ops->native_button(w) && (((DWORD)GetWindowLongPtrW(w,GWL_STYLE)&BS_TYPEMASK)==BS_OWNERDRAW)){
        *result=DefSubclassProc(w,m,a,b);
        if(!instance->detached && IsWindow(w)){
            if(!ops->remove_window(w))ops->remove_control(w);
            if(IsWindow(w))InvalidateRect(w,NULL,TRUE);
        }
        return TRUE;
    }
    if(m!=WM_NCDESTROY && ops->unsupported_choice(w)){
        if(m==WM_SETFONT)state->original_font=state->fonts->source((HFONT)a);
        *result=DefSubclassProc(w,m,a,b);
        if(!instance->detached)ops->remove_control(w);
        return TRUE;
    }
    return FALSE;
}

BOOL style_control_vb_checklist_owner(HWND w)
{
    DWORD style=(DWORD)GetWindowLongPtrW(w,GWL_STYLE),ex=(DWORD)GetWindowLongPtrW(w,GWL_EXSTYLE);
    UINT dpi=GetDpiForWindow(w);
    return (style&WS_CHILD) && !(ex&(WS_EX_LAYOUTRTL|WS_EX_RTLREADING)) &&
        (dpi==96 || dpi==144) && style_control_vb_checklist_class(w) &&
        (style&(LBS_OWNERDRAWFIXED|LBS_OWNERDRAWVARIABLE|LBS_HASSTRINGS|LBS_MULTIPLESEL|LBS_EXTENDEDSEL|LBS_NODATA))==
            (LBS_OWNERDRAWFIXED|LBS_HASSTRINGS|LBS_MULTIPLESEL);
}
/* Only the observed intrinsic owner-draw modes; native runtime keeps images,
 * item data, measurement and input. Unsupported live styles stay native. */
BOOL style_control_vb_owner_file(HWND w)
{
    DWORD style=(DWORD)GetWindowLongPtrW(w,GWL_STYLE),ex=(DWORD)GetWindowLongPtrW(w,GWL_EXSTYLE);
    if(ex&(WS_EX_LAYOUTRTL|WS_EX_RTLREADING))return FALSE;
    if(style_control_vb_checklist_owner(w))return TRUE;
    if(style_control_vb_directory(w))return (style&(LBS_OWNERDRAWFIXED|LBS_OWNERDRAWVARIABLE|LBS_NODATA|LBS_HASSTRINGS))==(LBS_OWNERDRAWVARIABLE|LBS_HASSTRINGS);
    return style_control_vb_drive(w) && (style&(3|CBS_OWNERDRAWFIXED|CBS_OWNERDRAWVARIABLE|CBS_HASSTRINGS))==(CBS_DROPDOWNLIST|CBS_OWNERDRAWFIXED|CBS_HASSTRINGS);
}

BOOL style_control_outline_choice_unsupported(HWND w)
{return (style_control_vb_checkbox(w) || style_control_vb_option(w)) &&
    !style_control_vb_plain_choice(w) && !style_control_vb_graphical_choice(w) && !style_control_vb_graphical_button(w);}

BOOL style_control_outline_supported(HWND w,HWND common_dialog_combo_pending)
{
    DWORD style=(DWORD)GetWindowLongPtrW(w,GWL_STYLE),type=style_control_button_type(w,style);
    if(style_control_vb_owner_file(w) || (w==common_dialog_combo_pending && style_control_dialog_combo_valid(w,GetParent(w),style_control_is_combo)))return TRUE;
    if((style_control_vb_checkbox(w) || style_control_vb_option(w)) &&
        !style_control_vb_plain_choice(w) && !style_control_vb_graphical_choice(w))return FALSE;
    if(style_control_is_scroll(w))return !(style&SBS_SIZEGRIP);
    if(style_control_is_link(w))return TRUE;
    if(style_control_is_tab(w))return style_control_tab_supported(w);
    if(style_control_is_header(w))return style_header_client_supported(w);
    if(style_control_class_is(w,PROGRESS_CLASSW))return style_progress_bound_supported(w);
    if(style_control_is_combo(w))return ((style&3)==CBS_DROPDOWNLIST || (style&3)==CBS_DROPDOWN || (style&3)==CBS_SIMPLE) &&
        !(style&(CBS_OWNERDRAWFIXED|CBS_OWNERDRAWVARIABLE));
    if(style_control_list_class(w))return !(style&(LBS_OWNERDRAWFIXED|LBS_OWNERDRAWVARIABLE|LBS_NODATA|LBS_USETABSTOPS));
    if(style_control_is_edit(w))return TRUE;
    if(style_control_is_static(w)){type=style&SS_TYPEMASK;return type==SS_LEFT || type==SS_CENTER || type==SS_RIGHT ||
        type==SS_ICON || type==SS_BITMAP || type==SS_SIMPLE || type==SS_LEFTNOWORDWRAP ||
        (type>=SS_BLACKRECT && type<=SS_WHITEFRAME) || (type>=SS_ETCHEDHORZ && type<=SS_ETCHEDFRAME);}
    if(!style_control_class_is(w,L"BUTTON"))return FALSE;
    if((style&(BS_ICON|BS_BITMAP)) && type!=BS_PUSHBUTTON && type!=BS_DEFPUSHBUTTON && !(style&BS_PUSHLIKE))return FALSE;
    if(type==BS_PUSHBUTTON || type==BS_DEFPUSHBUTTON || type==BS_GROUPBOX)return TRUE;
    return type==BS_CHECKBOX || type==BS_AUTOCHECKBOX || type==BS_RADIOBUTTON ||
        type==BS_AUTORADIOBUTTON || type==BS_3STATE || type==BS_AUTO3STATE;
}

style_control_basic_kind style_control_outline_basic_kind(HWND window)
{
    if(style_control_is_static(window) && (GetWindowLongPtrW(window,GWL_STYLE)&SS_TYPEMASK)==SS_BITMAP)return STYLE_CONTROL_BITMAP;
    if(style_control_is_separator(window))return STYLE_CONTROL_SEPARATOR;
    if(style_control_vb_frame(window))return STYLE_CONTROL_NATIVE_OVERLAY;
    return style_control_is_static(window)?STYLE_CONTROL_STATIC:STYLE_CONTROL_BUTTON;
}
