#include "paint_dispatch.h"
#include "../../lifecycle/dispatch.h"
#include "../../control/control_vcl_identity.h"
BOOL style_runtime_paint_eligible(const style_instance *instance,const style_control_runtime *r,UINT m)
{
    return ((r->roles.text.candidate || r->scroll.candidate || r->edit.candidate || r->group.candidate || r->grid.candidate || r->roles.inplace.candidate || r->roles.nav.candidate || r->toolbar.candidate || r->track.candidate || r->tab.candidate || r->report.binding.candidate || r->color.candidate || r->date.candidate || r->status.candidate || r->rich.candidate || r->group_frame.candidate || r->panel_frame.candidate) && !instance->detached && m==WM_PAINT);
}
style_runtime_dispatch style_runtime_dispatch_kind(HWND w,const style_instance *instance,const style_control_runtime *r,UINT m,WPARAM p,LPARAM l,BOOL status_scope,BOOL print_clip,BOOL spin_message,BOOL nested_static)
{
    if(r->cool.candidate && !instance->detached && m==WM_PAINT)return STYLE_RUNTIME_DISPATCH_COOL;
    if(m==WM_PRINTCLIENT && status_scope)return STYLE_RUNTIME_DISPATCH_STATUS;
    if(print_clip)return STYLE_RUNTIME_DISPATCH_COLOR;
    if(r->spin.modern && spin_message)return STYLE_RUNTIME_DISPATCH_SPIN;
    if(((r->box.candidate && !r->box.verified) || r->controlbar.candidate) && !instance->detached && m==WM_NCPAINT)return STYLE_RUNTIME_DISPATCH_BOX;
    if(r->roles.inplace.candidate && !instance->detached && (m==WM_LBUTTONDOWN || (m==WM_SYSKEYDOWN && p==VK_DOWN && ((ULONG_PTR)l&((ULONG_PTR)1<<29)))) && style_control_vcl_lookup_identity(w,r->roles.inplace.owner,r->roles.inplace.parent))return STYLE_RUNTIME_DISPATCH_LOOKUP;
    if(nested_static || style_runtime_paint_eligible(instance,r,m))return STYLE_RUNTIME_DISPATCH_STATIC;
    return STYLE_RUNTIME_DISPATCH_NATIVE;
}
typedef struct paint_call {
    style_runtime_paint_manager *manager;style_runtime_observation observation;style_runtime_dispatch kind;
    const style_runtime_paint_cleanup *cleanup;BOOL nested;
    union {style_runtime_paint_scope paint;style_runtime_cool_scope cool;style_runtime_box_scope box;style_runtime_bit_scope bit;HWND lookup_prior;} scope;
} paint_call;
static LRESULT nested_paint(void *context,HWND w,UINT m,WPARAM a,LPARAM b)
{
    paint_call *call=context;const style_runtime_paint_cleanup cleanup={0};
    return style_runtime_paint_dispatch(call->manager,&call->observation,STYLE_RUNTIME_DISPATCH_STATIC,w,m,a,b,&cleanup);
}
static void finish(void *context,BOOL completed)
{
    paint_call *call=context;style_runtime_paint_manager *manager=call->manager;const style_runtime_paint_cleanup *c=call->cleanup;
    switch(call->kind){
    case STYLE_RUNTIME_DISPATCH_STATIC:manager->paint=call->scope.paint.previous;break;
    case STYLE_RUNTIME_DISPATCH_COOL:manager->cool=call->scope.cool.previous;break;
    case STYLE_RUNTIME_DISPATCH_BOX:manager->box=call->scope.box.previous;break;
    case STYLE_RUNTIME_DISPATCH_LOOKUP:manager->lookup_popup=call->scope.lookup_prior;break;
    case STYLE_RUNTIME_DISPATCH_BIT:manager->bit=call->scope.bit.previous;break;
    default:break;
    }
    if(completed)return;
    if(call->kind==STYLE_RUNTIME_DISPATCH_SPIN){
        manager->spin=c->spin_prior;manager->spin_stack=c->spin->previous_stack;style_control_spin_buffers_clear(c->spin);
        manager->status=c->status_prior;style_runtime_text_end(c->text);
    }else if(call->kind!=STYLE_RUNTIME_DISPATCH_BIT && (call->kind!=STYLE_RUNTIME_DISPATCH_STATIC || c->text)){
        style_runtime_text_end(c->text);manager->status=c->status_prior;manager->spin=c->spin_prior;
        if(call->kind==STYLE_RUNTIME_DISPATCH_COLOR)style_control_native_overlay_dispose(c->overlay);
    }
    if(call->kind==STYLE_RUNTIME_DISPATCH_STATIC || call->kind==STYLE_RUNTIME_DISPATCH_BIT || !call->nested)style_instance_release(call->observation.instance);
}
LRESULT style_runtime_paint_dispatch(style_runtime_paint_manager *manager,const style_runtime_observation *observation,style_runtime_dispatch kind,HWND w,UINT m,WPARAM a,LPARAM b,const style_runtime_paint_cleanup *cleanup)
{
    if(kind==STYLE_RUNTIME_DISPATCH_NATIVE)return DefSubclassProc(w,m,a,b);
    paint_call call={.manager=manager,.observation=*observation,.kind=kind,.cleanup=cleanup,.nested=manager->paint!=NULL};DWORD entry=GetLastError();
    switch(kind){
    case STYLE_RUNTIME_DISPATCH_STATIC:
        call.scope.paint.previous=manager->paint;call.scope.paint.observation=*observation;
        manager->paint=style_runtime_paint_eligible(observation->instance,observation->runtime,m)?&call.scope.paint:NULL;break;
    case STYLE_RUNTIME_DISPATCH_COOL:
        call.scope.cool.previous=manager->cool;call.scope.cool.observation=*observation;call.scope.cool.observation.previous=manager->cool?&manager->cool->observation:NULL;manager->cool=&call.scope.cool;break;
    case STYLE_RUNTIME_DISPATCH_BOX:
        call.scope.box.previous=manager->box;call.scope.box.observation=*observation;manager->box=&call.scope.box;break;
    case STYLE_RUNTIME_DISPATCH_LOOKUP:call.scope.lookup_prior=manager->lookup_popup;manager->lookup_popup=w;break;
    case STYLE_RUNTIME_DISPATCH_BIT:
        call.scope.bit.previous=manager->bit;call.scope.bit.observation=*observation;call.scope.bit.observation.window=cleanup->item->hwndItem;
        call.scope.bit.dc=cleanup->item->hDC;call.scope.bit.item=cleanup->item->rcItem;call.scope.bit.state=cleanup->item->itemState;manager->bit=&call.scope.bit;break;
    default:break;
    }
    const style_dispatch_ops ops={&call,&call,call.nested && kind!=STYLE_RUNTIME_DISPATCH_STATIC && kind!=STYLE_RUNTIME_DISPATCH_BIT?nested_paint:NULL,finish};
    if(kind==STYLE_RUNTIME_DISPATCH_STATIC)SetLastError(entry);
    LRESULT result=style_dispatch_message(w,m,a,b,&ops);
    if(kind==STYLE_RUNTIME_DISPATCH_BOX)*cleanup->box_witnessed=call.scope.box.witnessed;
    return result;
}
