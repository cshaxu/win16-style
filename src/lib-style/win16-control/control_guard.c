#include "control_guard.h"
#include "../win16-common/font.h"
#include "control_class.h"
#include "control_protocol.h"
#include "fileowner_message.h"
#include "tab_paint.h"
#include "header_paint.h"
#include "progress_paint.h"
#include <commctrl.h>

BOOL win16_control_guard_message(HWND w,control_style *s,UINT m,WPARAM a,LPARAM b,const win16_control_guard_services *v,LRESULT *result)
{
    /* A verified VCL control belongs only to its original parent and role.
     * Let the native transition finish, then retire this exact attachment. */
    if(m!=WM_NCDESTROY && (!v->ntw16s_rich_valid(w) || !v->ntw16s_vcl_combo_valid(w) || !v->ntw16s_group_button_valid(w) || !v->ntw16s_grid_valid(w) || !v->ntw16s_vb_data_valid(w) || !v->ntw16s_vb_user_edge_valid(w) || !v->ntw16s_vcl_toolbar_valid(w) || !v->ntw16s_vcl_tab_valid(w))){
        if(m==WM_SETFONT)s->original_font=win16_style_system_source((HFONT)a);
        *result=DefSubclassProc(w,m,a,b);
        if(!s->detached && IsWindow(w) && !v->ntw16s_vb_user_edge_dormant(w) && v->remove_window(w) && IsWindow(w))InvalidateRect(w,NULL,TRUE);
        return TRUE;
    }
    /* A native BUTTON can become owner-drawn after creation. Its parent must
     * receive WM_DRAWITEM rather than the old ordinary-button painter. */
    if(m!=WM_NCDESTROY && v->is_exact_native_button(w) &&
       (((DWORD)GetWindowLongPtrW(w,GWL_STYLE)&BS_TYPEMASK)==BS_OWNERDRAW)){
        *result=DefSubclassProc(w,m,a,b);
        if(!s->detached && IsWindow(w)){
            if(!v->remove_window(w))v->remove_control(w);
            if(IsWindow(w))InvalidateRect(w,NULL,TRUE);
        }
        return TRUE;
    }
    /* Unverified graphical/owner-drawn VB variants stay native. A live
     * transition retires only this painter; do not keep drawing a plain box. */
    if(m!=WM_NCDESTROY && (v->ntw16s_vb_checkbox(w) || v->ntw16s_vb_option(w)) &&
            !v->ntw16s_vb_plain_choice(w) && !v->ntw16s_vb_graphical_choice(w) && !v->ntw16s_vb_graphical_button(w)){
        if(m==WM_SETFONT)s->original_font=win16_style_system_source((HFONT)a);
        *result=DefSubclassProc(w,m,a,b);
        if(!s->detached)v->remove_control(w);
        return TRUE;
    }
    return FALSE;
}


BOOL win16_control_supported(HWND w,HWND common_dialog_combo_pending)
{
    DWORD style=(DWORD)GetWindowLongPtrW(w,GWL_STYLE),type=ntw16s_button_type(w,style);
    if(ntw16s_vb_owner_file(w) || (w==common_dialog_combo_pending && win16_control_common_dialog_combo_valid(w,GetParent(w))))return TRUE;
    if((ntw16s_vb_checkbox(w) || ntw16s_vb_option(w)) &&
        !ntw16s_vb_plain_choice(w) && !ntw16s_vb_graphical_choice(w))return FALSE;
    if(ntw16s_is_scroll(w))return !(style&SBS_SIZEGRIP);
    if(ntw16s_is_link(w))return TRUE;
    if(ntw16s_is_tab(w))return win16_style_tab_supported(w);
    if(ntw16s_is_header(w))return win16_style_header_supported(w);
    if(ntw16s_class_is(w,PROGRESS_CLASSW))return win16_style_progress_supported(w);
    if(ntw16s_is_combo(w))return ((style&3)==CBS_DROPDOWNLIST || (style&3)==CBS_DROPDOWN || (style&3)==CBS_SIMPLE) &&
        !(style&(CBS_OWNERDRAWFIXED|CBS_OWNERDRAWVARIABLE));
    if(ntw16s_list_class(w))return !(style&(LBS_OWNERDRAWFIXED|LBS_OWNERDRAWVARIABLE|LBS_NODATA|LBS_USETABSTOPS));
    if(ntw16s_is_edit(w))return TRUE;
    if(ntw16s_is_static(w)){type=style&SS_TYPEMASK;return type==SS_LEFT || type==SS_CENTER || type==SS_RIGHT ||
        type==SS_ICON || type==SS_BITMAP || type==SS_SIMPLE || type==SS_LEFTNOWORDWRAP ||
        (type>=SS_BLACKRECT && type<=SS_WHITEFRAME) || (type>=SS_ETCHEDHORZ && type<=SS_ETCHEDFRAME);}
    if(!ntw16s_class_is(w,L"BUTTON"))return FALSE;
    if((style&(BS_ICON|BS_BITMAP)) && type!=BS_PUSHBUTTON && type!=BS_DEFPUSHBUTTON && !(style&BS_PUSHLIKE))return FALSE;
    if(type==BS_PUSHBUTTON || type==BS_DEFPUSHBUTTON || type==BS_GROUPBOX)return TRUE;
    return type==BS_CHECKBOX || type==BS_AUTOCHECKBOX || type==BS_RADIOBUTTON ||
        type==BS_AUTORADIOBUTTON || type==BS_3STATE || type==BS_AUTO3STATE;
}
