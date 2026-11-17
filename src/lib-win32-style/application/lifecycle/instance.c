#include "instance.h"

void style_instance_init(style_instance *instance,void *context,void (*dispose)(void *))
{*instance=(style_instance){.context=context,.dispose=dispose};}

void style_instance_retain(style_instance *instance)
{++instance->calls;}

void style_instance_release(style_instance *instance)
{if(!--instance->calls && instance->detached)instance->dispose(instance->context);}

style_instance *style_instance_lookup(HWND window,SUBCLASSPROC procedure)
{
    DWORD_PTR data=0;
    return GetWindowSubclass(window,procedure,(UINT_PTR)procedure,&data)?(style_instance *)data:NULL;
}

BOOL style_instance_preflight(HWND w,SUBCLASSPROC proc)
{
    DWORD_PTR existing;
    if(GetWindowThreadProcessId(w,NULL)!=GetCurrentThreadId()){SetLastError(ERROR_INVALID_PARAMETER);return FALSE;}
    if(GetWindowSubclass(w,proc,(UINT_PTR)proc,&existing)){SetLastError(ERROR_ALREADY_EXISTS);return FALSE;}
    return TRUE;
}

BOOL style_instance_attach(style_instance *instance,HWND window,SUBCLASSPROC procedure,const void *owner)
{
    DWORD error=GetLastError();
    if(style_instance_lookup(window,procedure)){SetLastError(ERROR_ALREADY_EXISTS);return FALSE;}
    SetLastError(error);
    instance->window=window;instance->procedure=procedure;instance->owner=owner;
    return SetWindowSubclass(window,procedure,(UINT_PTR)procedure,(DWORD_PTR)instance);
}

BOOL style_instance_detach(style_instance *instance)
{
    if(instance->detached)return TRUE;
    if(style_instance_lookup(instance->window,instance->procedure)!=instance){SetLastError(ERROR_INVALID_STATE);return FALSE;}
    if(!RemoveWindowSubclass(instance->window,instance->procedure,(UINT_PTR)instance->procedure))return FALSE;
    instance->detached=TRUE;return TRUE;
}

BOOL style_instance_acquire_owned(void *context)
{
    style_instance *instance=context;
    if(instance->detached || style_instance_lookup(instance->window,instance->procedure)!=instance)return FALSE;
    style_instance_retain(instance);return TRUE;
}
void style_instance_return_owned(void *context)
{style_instance_release(context);}
