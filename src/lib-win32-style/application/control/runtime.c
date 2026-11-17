#include "runtime.h"
#include "control_vcl_identity.h"
static style_control_runtime_binding *runtime_binding(style_control_runtime *runtime,style_control_runtime_kind kind)
{
    switch(kind){
    case STYLE_RUNTIME_GROUP:return &runtime->group;
    case STYLE_RUNTIME_SCROLL:return &runtime->scroll;
    case STYLE_RUNTIME_TAB:return &runtime->tab;
    case STYLE_RUNTIME_TRACK:return &runtime->track;
    case STYLE_RUNTIME_TOOLBAR:return &runtime->toolbar;
    case STYLE_RUNTIME_EDIT:return &runtime->edit;
    }
    return NULL;
}
static style_control_protocol *runtime_bind(HWND w,style_control_runtime *runtime,style_control_runtime_binding *state,style_control_runtime_kind kind)
{
    switch(kind){
    case STYLE_RUNTIME_EDIT:
        if(runtime->combo && !style_control_runtime_combo_current(w,runtime))return NULL;
        return runtime->link?style_control_protocol_bind_verified_link(w,state->witness):runtime->combo?
            style_control_protocol_bind_verified_combo(w,state->witness):style_control_protocol_bind_verified_edit(w,state->witness);
    case STYLE_RUNTIME_GROUP:return style_control_vcl_group_button_identity(w)?style_control_protocol_bind_verified_group_button(w,state->witness):NULL;
    case STYLE_RUNTIME_SCROLL:return GetParent(w)==state->parent?style_control_protocol_bind_verified_scroll(w,state->witness):NULL;
    case STYLE_RUNTIME_TAB:return GetParent(w)==state->parent?style_control_protocol_bind_verified_tab(w,state->witness):NULL;
    case STYLE_RUNTIME_TRACK:return style_control_protocol_bind_verified_trackbar(w,state->witness);
    case STYLE_RUNTIME_TOOLBAR:return style_control_vcl_toolbar_identity(w,state->owner,state->parent)?style_control_protocol_bind_verified_toolbar(w,state->witness):NULL;
    }
    return NULL;
}
static BOOL runtime_relation(HWND w,const style_control_runtime *runtime,const style_control_runtime_binding *state,style_control_runtime_kind kind)
{
    return (kind!=STYLE_RUNTIME_SCROLL || GetParent(w)==state->parent) &&
        (kind!=STYLE_RUNTIME_EDIT || !runtime->combo || style_control_runtime_combo_current(w,runtime));
}
void style_control_runtime_dispose(style_control_runtime *runtime)
{style_control_protocol_release(runtime->protocol);runtime->protocol=NULL;}
void style_control_runtime_settle(HWND w,UINT message,style_instance *instance,style_control_runtime *runtime,style_control_runtime_kind kind,const style_control_runtime_ops *ops)
{
    style_control_runtime_binding *state=runtime_binding(runtime,kind);
    if(message!=WM_PAINT || !state || !state->candidate || state->attempted || instance->detached || !state->witness)return;
    DWORD error=GetLastError();state->attempted=TRUE;
    style_control_protocol *protocol=runtime_bind(w,runtime,state,kind);
    if(protocol && !instance->detached && runtime_relation(w,runtime,state,kind)){
        runtime->protocol=protocol;
        BOOL installed=ops->install(w,kind,ops->context,&instance->detached);
        if(installed && !instance->detached && runtime_relation(w,runtime,state,kind)){
            if(kind==STYLE_RUNTIME_GROUP)state->parent=GetParent(w);
            state->installed=TRUE;InvalidateRect(w,NULL,TRUE);
        }else if(!instance->detached){ops->remove(w,kind,ops->context);style_control_runtime_dispose(runtime);}
    }else if(protocol)style_control_protocol_release(protocol);
    /* Preserve the existing SysLink boundary: an empty link can be tried
     * again only after a fresh same-owner native forwarding witness. */
    if(kind==STYLE_RUNTIME_EDIT && runtime->link && !instance->detached && !state->installed){state->attempted=FALSE;state->witness=NULL;}
    SetLastError(error);
}
BOOL style_control_runtime_combo_current(HWND w,const style_control_runtime *runtime)
{return style_control_vcl_plain_combo(w,runtime->combo_parent) && ((DWORD)GetWindowLongPtrW(w,GWL_STYLE)&3)==runtime->combo_type;}
