#include "runtime_roles.h"
#include "../../control/runtime.h"
#include "../../control/control_vcl_identity.h"
void style_runtime_reference_init(style_runtime_reference *state,BOOL candidate,HWND parent,HWND target,HMODULE owner,style_instance *held)
{
    *state=(style_runtime_reference){.candidate=candidate,.parent=parent,.target=target,.owner=owner,.held=held};
    if(held)style_instance_retain(held);
}
void style_runtime_references_release(style_runtime_references *references)
{
    style_instance *popup=references->popup.held,*mdi=references->mdi.held;
    references->popup.held=references->mdi.held=NULL;
    if(popup)style_instance_release(popup);
    if(mdi)style_instance_release(mdi);
}
BOOL style_runtime_reference_matches(HWND w,const style_instance *instance,const style_runtime_reference *state,BOOL mdi,BOOL (*host_attached)(HWND))
{
    return style_instance_lookup(state->target,instance->procedure)==state->held &&
        (mdi?style_control_vcl_mdi_identity(w,state->parent,state->target,state->owner,host_attached):
            style_control_vcl_lookup_popup_identity(w,state->target,state->parent,state->owner));
}
void style_runtime_roles_guard(HWND w,UINT message,style_instance *instance,style_control_runtime *runtime,BOOL owner_draw,BOOL (*host_attached)(HWND),BOOL (*remove)(HWND))
{
    style_runtime_roles *roles=&runtime->roles;
    if(message!=WM_DESTROY && message!=WM_NCDESTROY){
        if(owner_draw && !instance->detached && runtime->report.parent && GetParent(w)!=runtime->report.parent)(void)remove(w);
        if(roles->inplace.candidate && !instance->detached && !style_control_vcl_inplace_or_lookup(w,roles->inplace.owner,roles->inplace.parent))(void)remove(w);
        if(roles->references.popup.candidate && !instance->detached && !style_runtime_reference_matches(w,instance,&roles->references.popup,FALSE,host_attached))(void)remove(w);
        if(roles->references.mdi.candidate && !instance->detached && !style_runtime_reference_matches(w,instance,&roles->references.mdi,TRUE,host_attached))(void)remove(w);
        if(roles->nav.candidate && !instance->detached && !style_control_vcl_nav_identity(w,roles->nav.owner,roles->nav.parent))(void)remove(w);
    }
    if(runtime->scroll.candidate && !instance->detached && GetParent(w)!=runtime->scroll.parent)(void)remove(w);
}
