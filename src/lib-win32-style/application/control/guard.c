#include "guard.h"
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
