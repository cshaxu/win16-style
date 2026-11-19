#include "observation.h"
#include "message_observer.h"
#include "lib-win32/process-hook/unwind.h"
static _Thread_local style_dialog_observation *active;
BOOL style_dialog_observation_begin(style_dialog_observation *call,style_creation_context *context,
    HOOKPROC observer,BOOL tracked)
{
    ZeroMemory(call,sizeof(*call));
    if(context){if(!style_creation_call_begin(&call->creation,TRUE,context))return FALSE;}
    else call->creation.enabled=TRUE;
    call->tracked=tracked;
    if(tracked){call->previous=active;active=call;}
    if(call->creation.enabled)call->hook=style_message_observer_begin(observer);
    return TRUE;
}
void style_dialog_observation_end(void *data)
{
    style_dialog_observation *call=data;
    if(call->tracked)active=call->previous;
    style_message_observer_end(call->hook);
    style_creation_call_end(&call->creation);
}
void style_dialog_observation_found(HWND window){if(active)active->window=window;}
HWND style_dialog_observation_current(void){return active?active->window:NULL;}

typedef struct dialog_call_observation {
    style_dialog_observation dialog;
    void (*initialized)(HWND);
    struct dialog_call_observation *previous;
} dialog_call_observation;
static _Thread_local dialog_call_observation *current_call;
static LRESULT CALLBACK observe_initialized(int code,WPARAM wp,LPARAM lp)
{
    DWORD incoming=GetLastError();
    const CWPRETSTRUCT *event=style_message_dialog_initialized(code,lp);
    if(event && current_call)current_call->initialized(event->hwnd);
    SetLastError(incoming);
    return CallNextHookEx(NULL,code,wp,lp);
}
static void end_dialog_call(void *data)
{
    dialog_call_observation *scope=data;
    style_dialog_observation_end(&scope->dialog);
    current_call=scope->previous;
}
PROCESS_HOOK_UNWIND_FUNCTION HRESULT style_dialog_observe(HRESULT (*call)(void *),void *context,void (*initialized)(HWND))
{
    dialog_call_observation scope={.initialized=initialized,.previous=current_call};
    process_hook_unwind_scope unwind;
    DWORD incoming=GetLastError(),error;
    HRESULT result;
    if(!style_dialog_observation_begin(&scope.dialog,NULL,observe_initialized,FALSE))return E_FAIL;
    current_call=&scope;
    PROCESS_HOOK_UNWIND_BEGIN(unwind,end_dialog_call,&scope);
    SetLastError(incoming);
    result=call(context);error=GetLastError();
    PROCESS_HOOK_UNWIND_END(unwind);
    SetLastError(error);
    return result;
}
