#include "runtime_native.h"
BOOL style_runtime_native_call_begin(style_runtime_native_calls *calls,style_runtime_native_call *scope,WNDPROC proc,HWND w,UINT message,const void *address,const style_runtime_observation *paint,style_runtime_observation *cool)
{
    *scope=(style_runtime_native_call){0};
    HMODULE native=style_runtime_observe_native(w,message,proc,address,paint,cool);
    if(!native && !calls->current)return FALSE;
    scope->manager=calls;scope->previous=calls->current;scope->active=TRUE;
    if(native){scope->window=w;scope->native_module=native;scope->instance=paint->instance;paint->runtime->roles.text.verified=TRUE;}
    calls->current=scope;return TRUE;
}
void style_runtime_native_call_end(void *context)
{
    style_runtime_native_call *scope=context;
    if(scope->active){scope->manager->current=scope->previous;scope->active=FALSE;}
}
const style_runtime_native_call *style_runtime_native_current(const style_runtime_native_calls *calls,const style_runtime_observation *paint)
{
    const style_runtime_native_call *scope=calls->current;
    return scope && scope->active && scope->instance && !scope->instance->detached && paint && paint->window==scope->window?scope:NULL;
}
