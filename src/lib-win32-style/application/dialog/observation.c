#include "observation.h"
#include "message_observer.h"
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
