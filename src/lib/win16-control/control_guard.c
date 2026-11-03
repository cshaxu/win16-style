#include "control_guard.h"
#include "font.h"
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
