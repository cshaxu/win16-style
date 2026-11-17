#include "notify.h"
#include "control_custom_draw.h"
#include "control_protocol.h"
#include "control_scope.h"
#include "lib-win32/process-hook/unwind.h"
BOOL style_control_notify_bind(HWND w,style_control_notify_state *state,SUBCLASSPROC callback,void *owner)
{
    state->window=w;state->parent=GetParent(w);
    return state->parent && SetWindowSubclass(state->parent,callback,(UINT_PTR)w,(DWORD_PTR)owner);
}
void style_control_notify_unbind(style_control_notify_state *state,SUBCLASSPROC callback,void *owner)
{
    style_control_subclass_remove_owned(state->parent,callback,(UINT_PTR)state->window,owner);
    state->parent=NULL;state->internal=FALSE;
}
LRESULT style_control_notify_message(HWND parent,UINT m,WPARAM p,LPARAM l,UINT_PTR id,style_instance *instance,
    style_control_notify_state *state,SUBCLASSPROC callback,style_control_track_calls *calls,const style_control_notify_ops *ops)
{
    DWORD entry=GetLastError();LRESULT result;BOOL internal=FALSE;
    style_instance_retain(instance);NMCUSTOMDRAW *draw=style_control_custom_draw_message(parent,state->window,m,l,instance->detached);
    if(calls && draw && style_control_protocol_runtime_trackbar(state->window)){
        style_control_track_scope *scope=calls->current;
        BOOL match=scope && scope->owner==instance && scope->active && scope->draw==draw && scope->dc==draw->hdc;
        if(match && draw->dwDrawStage==CDDS_ITEMPOSTPAINT && scope->pending && scope->item==draw->dwItemSpec){
            scope->pending=FALSE;
            if(EqualRect(&scope->rect,&draw->rc))ops->runtime_border(state->window,draw,ops->context);
            result=CDRF_DODEFAULT;
        }else if(match && draw->dwDrawStage==CDDS_POSTPAINT){scope->active=FALSE;result=CDRF_DODEFAULT;}
        else {
            SetLastError(entry);result=DefSubclassProc(parent,m,p,l);DWORD error=GetLastError();
            if(!instance->detached && scope && calls->current==scope && scope->owner==instance){
                if(draw->dwDrawStage==CDDS_PREPAINT && result==CDRF_NOTIFYITEMDRAW &&
                   ops->supported(state->window)){
                    scope->draw=draw;scope->dc=draw->hdc;scope->active=TRUE;
                    result=CDRF_NOTIFYITEMDRAW|CDRF_NOTIFYPOSTPAINT;
                }else if(match && draw->dwDrawStage==CDDS_ITEMPREPAINT && result==CDRF_DODEFAULT && !scope->pending &&
                    (draw->dwItemSpec==TBCD_CHANNEL || draw->dwItemSpec==TBCD_THUMB)){
                    scope->item=draw->dwItemSpec;scope->rect=draw->rc;scope->pending=TRUE;result=CDRF_NOTIFYPOSTPAINT;
                }
            }
            style_instance_release(instance);
            SetLastError(error);return result;
        }
        style_instance_release(instance);
        SetLastError(entry);return result;
    }
    if(draw && draw->dwDrawStage!=CDDS_PREPAINT && state->internal)internal=TRUE;
    if(m==WM_NCDESTROY){RemoveWindowSubclass(parent,callback,id);state->parent=NULL;state->internal=FALSE;}
    SetLastError(entry);
    result=internal?CDRF_DODEFAULT:DefSubclassProc(parent,m,p,l);DWORD error=GetLastError();
    if(draw && !instance->detached)result=style_control_custom_draw_after(state->window,draw,&state->internal,internal,result,
        ops->context,ops->supported,ops->part);
    style_instance_release(instance);
    SetLastError(error);return result;
}

typedef struct track_dispatch {style_control_track_scope scope;style_control_track_calls *calls;} track_dispatch;
static void track_end(void *data)
{track_dispatch *dispatch=data;dispatch->calls->current=dispatch->scope.previous;}
PROCESS_HOOK_UNWIND_FUNCTION LRESULT style_control_track_dispatch(style_control_track_calls *calls,style_instance *owner,HWND w,UINT m,WPARAM a,LPARAM b)
{
    track_dispatch dispatch={.scope={.previous=calls->current,.owner=owner},.calls=calls};process_hook_unwind_scope unwind;
    DWORD entry=GetLastError(),error;LRESULT result;
    calls->current=&dispatch.scope;PROCESS_HOOK_UNWIND_BEGIN(unwind,track_end,&dispatch);
    SetLastError(entry);result=DefSubclassProc(w,m,a,b);error=GetLastError();
    PROCESS_HOOK_UNWIND_END(unwind);SetLastError(error);return result;
}

LRESULT style_control_track_message(HWND w,style_instance *instance,style_control_notify_state *state,UINT m,WPARAM a,LPARAM b,SUBCLASSPROC notify,void *owner,style_control_track_calls *calls)
{
    LRESULT result;
    if(style_control_protocol_runtime_trackbar(w) && (m==WM_PAINT || m==WM_PRINT || m==WM_PRINTCLIENT))
        result=style_control_track_dispatch(calls,instance,w,m,a,b);
    else result=DefSubclassProc(w,m,a,b);
    if(!instance->detached && m==WM_STYLECHANGED)SendMessageW(w,WM_SYSCOLORCHANGE,0,0);
    if(!instance->detached && state->parent && GetParent(w)!=state->parent){
        style_control_notify_unbind(state,notify,owner);SendMessageW(w,WM_SYSCOLORCHANGE,0,0);
    }
    return result;
}
