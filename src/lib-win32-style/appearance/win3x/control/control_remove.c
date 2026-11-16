#include "control_remove.h"
#include "control_class.h"
#include "lib-win32-style/application/control/scroll_state.h"
#include "updown_adapter.h"
#include "lib-win32-style/application/control/retire.h"

typedef struct control_retire {
    control_style *state;
    const win16_control_remove_services *services;
} control_retire;
static void finish_input(HWND window,void *context)
{control_retire *retire=context;retire->services->scroll_finish(window,retire->state,TRUE);}
static style_control_retired release_bindings(HWND w,void *context,BOOL terminal)
{
    control_retire *retire=context;control_style *s=retire->state;
    const win16_control_remove_services *v=retire->services;
    ntw16s_updown_retire(&s->updown);BOOL track=s->track_window!=NULL;
    v->trackbar_unbind(s);v->toolbar_unbind(s);
    if(terminal){
        if(s->list_theme_owned){s->list_theme_owned=FALSE;(void)v->theme_terminal(w,s);}
        style_scroll_caret_destroy(w,&s->scroll.resources);style_scroll_stop_timer(w,&s->scroll.resources.timer);
        style_control_combo_unbind(&s->combo,TRUE);
        return (style_control_retired){style_control_combo_list_retire(&s->combo,&s->instance,v->lookup,v->terminal_list),FALSE};
    }
    style_control_combo_unbind(&s->combo,FALSE);
    (void)style_control_combo_list_retire(&s->combo,&s->instance,v->lookup,v->remove_list);
    if(s->list_theme_owned){s->list_theme_owned=FALSE;v->theme_remove(w,s);}
    if(s->state.font && IsWindow(w) && !s->combo_passthrough)SendMessageW(w,WM_SETFONT,(WPARAM)s->state.original_font,FALSE);
    if(IsWindow(w) && style_control_is_separator(w))SetWindowPos(w,NULL,0,0,0,0,
        SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE|SWP_FRAMECHANGED);
    style_scroll_caret_destroy(w,&s->scroll.resources);
    if(s->state.suppression && !s->state.redraw_off)SendMessageW(w,WM_SETREDRAW,TRUE,0);
    style_scroll_stop_timer(w,&s->scroll.resources.timer);
    if(s->scroll.track.active){s->scroll.track.active=FALSE;style_scroll_release_capture(w,&s->scroll.resources.previous_capture,s->scroll.resources.list || s->scroll.resources.edit || s->scroll.resources.frame);}
    return (style_control_retired){TRUE,track};
}
static const style_control_retire_ops retire_operations={finish_input,release_bindings};
BOOL win16_control_remove(HWND w,const win16_control_remove_services *v)
{
    DWORD_PTR data;
    if(GetWindowThreadProcessId(w,NULL)!=GetCurrentThreadId() ||
        !GetWindowSubclass(w,v->control_proc,(UINT_PTR)v->control_proc,&data))return FALSE;
    control_style *s=(control_style *)data;control_retire retire={s,v};
    return style_control_retire(w,&s->instance,&s->state,FALSE,&retire,&retire_operations);
}
BOOL win16_control_terminal(HWND w,const win16_control_remove_services *v)
{
    DWORD_PTR data;if(!GetWindowSubclass(w,v->control_proc,(UINT_PTR)v->control_proc,&data))return TRUE;
    control_style *s=(control_style *)data;control_retire retire={s,v};
    return style_control_retire(w,&s->instance,&s->state,TRUE,&retire,&retire_operations);
}
