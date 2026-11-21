#include "control_guard.h"
#include "control_class.h"
#include "../../../application/control/control_protocol.h"
#include "lib-win32-style/application/control/tab_snapshot.h"
#include "header_paint.h"
#include "lib-win32-style/application/control/progress_snapshot.h"
#include <commctrl.h>

BOOL win16_control_choice_unsupported(HWND w)
{return (style_control_vb_checkbox(w) || style_control_vb_option(w)) &&
    !ntw16s_vb_plain_choice(w) && !ntw16s_vb_graphical_choice(w) && !ntw16s_vb_graphical_button(w);}

BOOL win16_control_supported(HWND w,HWND common_dialog_combo_pending)
{
    DWORD style=(DWORD)GetWindowLongPtrW(w,GWL_STYLE),type=style_control_button_type(w,style);
    if(ntw16s_vb_owner_file(w) || (w==common_dialog_combo_pending && style_control_dialog_combo_valid(w,GetParent(w),style_control_is_combo)))return TRUE;
    if((style_control_vb_checkbox(w) || style_control_vb_option(w)) &&
        !ntw16s_vb_plain_choice(w) && !ntw16s_vb_graphical_choice(w))return FALSE;
    if(style_control_is_scroll(w))return !(style&SBS_SIZEGRIP);
    if(style_control_is_link(w))return TRUE;
    if(style_control_is_tab(w))return style_control_tab_supported(w);
    if(style_control_is_header(w))return win16_style_header_supported(w);
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

style_control_basic_kind win16_control_basic_kind(HWND window)
{
    if(style_control_is_static(window) && (GetWindowLongPtrW(window,GWL_STYLE)&SS_TYPEMASK)==SS_BITMAP)return STYLE_CONTROL_BITMAP;
    if(style_control_is_separator(window))return STYLE_CONTROL_SEPARATOR;
    if(style_control_vb_frame(window))return STYLE_CONTROL_NATIVE_OVERLAY;
    return style_control_is_static(window)?STYLE_CONTROL_STATIC:STYLE_CONTROL_BUTTON;
}
