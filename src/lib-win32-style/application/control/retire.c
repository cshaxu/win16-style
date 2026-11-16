#include "retire.h"
#include "control_class.h"

BOOL style_control_retire(HWND window,style_instance *instance,style_control_state *state,BOOL terminal,
    void *context,const style_control_retire_ops *ops)
{
    if(!terminal && state->removing)return TRUE;
    style_instance_retain(instance);
    if(!terminal){
        state->removing=TRUE;
        if(ops->finish_input)ops->finish_input(window,context);
        if(instance->detached){style_instance_release(instance);return TRUE;}
    }
    if(!style_instance_detach(instance)){
        if(!terminal)state->removing=FALSE;
        style_instance_release(instance);if(terminal)SetLastError(ERROR_INVALID_STATE);return FALSE;
    }
    style_control_retired retired=ops->release(window,context,terminal);
    style_instance_release(instance);
    if(!terminal){
        if(retired.colors_changed && IsWindow(window))SendMessageW(window,WM_SYSCOLORCHANGE,0,0);
        RedrawWindow(window,NULL,NULL,RDW_INVALIDATE|RDW_FRAME);
        if(style_control_is_scroll(window) && GetFocus()==window)SendMessageW(window,WM_SETFOCUS,0,0);
    }
    return retired.result;
}
