#include "bindings.h"
#include "control_class.h"
#include "scroll_state.h"
void style_control_bindings_init(style_control_bindings *b)
{
    *b=(style_control_bindings){0};style_control_native_init(&b->tab);style_control_native_init(&b->header);style_control_native_init(&b->progress);
}
void style_control_bindings_dispose(style_control_bindings *b,const style_control_state *state)
{style_control_combo_dispose(&b->combo,state);}
BOOL style_control_bindings_setup(HWND w,style_instance *instance,style_control_state *state,style_control_bindings *b,const style_control_setup_options *options)
{
    BOOL result=TRUE;
    if(!options->frame_bars && style_control_is_combo(w) && !options->owner_draw &&
        !style_control_combo_setup(w,instance,&b->combo,options->combo,options->children)){result=FALSE;goto installed;}
    if(!options->frame_bars){
        if(options->combo_passthrough && ((DWORD)GetWindowLongPtrW(w,GWL_STYLE)&3)==CBS_DROPDOWNLIST){
            HFONT replacement=state->fonts->acquire(w,state->original_font,TRUE);state->font=replacement?replacement:state->original_font;}
        if(!options->combo_passthrough)SendMessageW(w,WM_SETFONT,(WPARAM)state->original_font,FALSE);
        if(!instance->detached && style_control_is_separator(w))SetWindowPos(w,NULL,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE|SWP_FRAMECHANGED);
        if(!instance->detached)RedrawWindow(w,NULL,NULL,RDW_INVALIDATE|RDW_FRAME);
    }
    const style_control_list_theme_ops *theme=options->list_theme;
    if(theme && style_control_list_class(w) && !theme->retained(w)){
        if(!theme->capture(w))result=FALSE;
        else {b->list_theme_owned=TRUE;if(!theme->enable(w,theme->owner))result=FALSE;}
    }
installed:
    if(instance->detached || (options->list_theme && options->list_theme->cancelled && *options->list_theme->cancelled)){
        result=FALSE;SetLastError(ERROR_CANCELLED);
    }
    return result;
}
style_control_retired style_control_bindings_release(HWND w,style_instance *instance,style_control_state *state,style_control_bindings *b,BOOL terminal,BOOL combo_passthrough,const style_control_binding_ops *ops)
{
    style_control_updown_retire(&b->updown);BOOL track=b->track.window!=NULL;
    style_control_notify_unbind(&b->track,ops->track_notify,ops->context);style_control_notify_unbind(&b->toolbar,ops->toolbar_notify,ops->context);
    if(terminal){
        if(b->list_theme_owned){b->list_theme_owned=FALSE;(void)ops->theme_terminal(w,ops->context);}
        style_scroll_caret_destroy(w,&b->scroll.resources);style_scroll_stop_timer(w,&b->scroll.resources.timer);
        style_control_combo_unbind(&b->combo,TRUE);
        return (style_control_retired){style_control_combo_list_retire(&b->combo,instance,ops->lookup,ops->terminal_list),FALSE};
    }
    style_control_combo_unbind(&b->combo,FALSE);
    (void)style_control_combo_list_retire(&b->combo,instance,ops->lookup,ops->remove_list);
    if(b->list_theme_owned){b->list_theme_owned=FALSE;ops->theme_remove(w,ops->context);}
    if(state->font && IsWindow(w) && !combo_passthrough)SendMessageW(w,WM_SETFONT,(WPARAM)state->original_font,FALSE);
    if(IsWindow(w) && style_control_is_separator(w))SetWindowPos(w,NULL,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE|SWP_FRAMECHANGED);
    style_scroll_caret_destroy(w,&b->scroll.resources);
    if(state->suppression && !state->redraw_off)SendMessageW(w,WM_SETREDRAW,TRUE,0);
    style_scroll_stop_timer(w,&b->scroll.resources.timer);
    if(b->scroll.track.active){b->scroll.track.active=FALSE;style_scroll_release_capture(w,&b->scroll.resources.previous_capture,b->scroll.resources.list || b->scroll.resources.edit || b->scroll.resources.frame);}
    return (style_control_retired){TRUE,track};
}
