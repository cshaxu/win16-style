#include "control_remove.h"
#include "control_class.h"
#include "updown_adapter.h"

BOOL win16_control_remove(HWND w,const win16_control_remove_services *v)
{
    DWORD_PTR data;control_style *s;
    if(GetWindowThreadProcessId(w,NULL)!=GetCurrentThreadId() ||
        !GetWindowSubclass(w,v->control_proc,(UINT_PTR)v->control_proc,&data))return FALSE;
    s=(control_style *)data;
    if(s->removing)return TRUE;
    ++s->calls;s->removing=TRUE;
    /* End the live gesture while queries still see its full-width report.
     * The application may remove again or destroy the HWND in this notice. */
    v->scroll_finish(w,s,TRUE);
    if(s->detached){if(!--s->calls)win16_control_state_dispose(s);return TRUE;}
    if(!RemoveWindowSubclass(w,v->control_proc,(UINT_PTR)v->control_proc)){
        s->removing=FALSE;--s->calls;return FALSE;}
    s->detached=TRUE;ntw16s_updown_retire(&s->updown);BOOL track=s->track_window!=NULL;v->trackbar_unbind(s);v->toolbar_unbind(s);v->combo_text_unbind(s,FALSE);v->detach_combo_list(s);
    if(s->list_theme_owned){s->list_theme_owned=FALSE;v->theme_remove(w,s);}
    if(s->font && IsWindow(w) && !s->combo_passthrough)SendMessageW(w,WM_SETFONT,(WPARAM)s->original_font,FALSE);
    if(IsWindow(w) && style_control_is_separator(w))SetWindowPos(w,NULL,0,0,0,0,
        SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE|SWP_FRAMECHANGED);
    v->scroll_caret_destroy(w,s);
    if(s->list_suppression && !s->list_redraw_off)SendMessageW(w,WM_SETREDRAW,TRUE,0);
    v->scroll_stop_timer(w,s);
    if(s->scroll.active){s->scroll.active=FALSE;v->scroll_release_capture(w,s);}
    if(!--s->calls)win16_control_state_dispose(s);
    if(track && IsWindow(w))SendMessageW(w,WM_SYSCOLORCHANGE,0,0);
    RedrawWindow(w,NULL,NULL,RDW_INVALIDATE|RDW_FRAME);
    if(style_control_is_scroll(w) && GetFocus()==w)SendMessageW(w,WM_SETFOCUS,0,0);
    return TRUE;
}
BOOL win16_control_terminal(HWND w,const win16_control_remove_services *v)
{
    DWORD_PTR data;control_style *s;
    BOOL result;
    if(!GetWindowSubclass(w,v->control_proc,(UINT_PTR)v->control_proc,&data))return TRUE;
    s=(control_style *)data;
    if(!RemoveWindowSubclass(w,v->control_proc,(UINT_PTR)v->control_proc)){
        SetLastError(ERROR_INVALID_STATE);return FALSE;
    }
    ++s->calls;s->detached=TRUE;ntw16s_updown_retire(&s->updown);v->trackbar_unbind(s);v->toolbar_unbind(s);
    if(s->list_theme_owned){s->list_theme_owned=FALSE;(void)v->theme_terminal(w,s);}
    v->scroll_caret_destroy(w,s);
    v->scroll_stop_timer(w,s);
    v->combo_text_unbind(s,TRUE);result=v->terminal_combo_list(s);
    if(!--s->calls)win16_control_state_dispose(s);
    return result;
}

BOOL win16_control_combo_list_retire(control_style *s,SUBCLASSPROC proc,BOOL (*retire)(HWND))
{
    DWORD_PTR data;HWND list=s->combo_list;s->combo_list=NULL;
    /* An independently removed/reinstalled child is no longer ours. */
    if(list && GetWindowSubclass(list,proc,(UINT_PTR)proc,&data) &&
        ((control_style *)data)->owner==s)return retire(list);
    return TRUE;
}
