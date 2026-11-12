#include "control_scope.h"
#include "message_observer.h"
#include <stdlib.h>
#include <stdint.h>

struct style_control_scope {
    struct style_control_scope *next;
    HWND root;
    unsigned calls;
    BOOL detached;
    max_align_t data[];
};

style_control_scope *style_control_scope_begin(HWND root,size_t size,
    style_control_scope **scopes,HHOOK *hook,HOOKPROC observer)
{
    if(GetWindowThreadProcessId(root,NULL)!=GetCurrentThreadId()){SetLastError(ERROR_INVALID_THREAD_ID);return NULL;}
    if(size>SIZE_MAX-sizeof(style_control_scope)){SetLastError(ERROR_NOT_ENOUGH_MEMORY);return NULL;}
    style_control_scope *scope=calloc(1,sizeof(*scope)+size);
    if(!scope){SetLastError(ERROR_NOT_ENOUGH_MEMORY);return NULL;}
    if(!*scopes){*hook=style_message_observer_begin(observer);
        if(!*hook){free(scope);return NULL;}}
    scope->root=root;scope->next=*scopes;*scopes=scope;return scope;
}
BOOL style_control_scope_end(style_control_scope *scope,style_control_scope **scopes,HHOOK *hook)
{
    style_control_scope **link=scopes;
    while(*link && *link!=scope)link=&(*link)->next;
    if(!scope || !*link){SetLastError(ERROR_INVALID_PARAMETER);return FALSE;}
    *link=scope->next;scope->detached=TRUE;
    if(!*scopes){style_message_observer_end(*hook);*hook=NULL;}
    if(!scope->calls)free(scope);
    return TRUE;
}
style_control_scope *style_control_scope_find(style_control_scope *scope,HWND child)
{for(;scope;scope=scope->next)if(scope->root && IsChild(scope->root,child))return scope;return NULL;}
BOOL style_control_scope_contains(style_control_scope *scope,const style_control_scope *target)
{for(;scope;scope=scope->next)if(scope==target)return TRUE;return FALSE;}
void style_control_scope_root_destroyed(style_control_scope *scope,HWND root)
{for(;scope;scope=scope->next)if(scope->root==root)scope->root=NULL;}
void *style_control_scope_data(style_control_scope *scope){return scope->data;}
void style_control_scope_retain(style_control_scope *scope){++scope->calls;}
void style_control_scope_release(style_control_scope *scope)
{if(!--scope->calls && scope->detached)free(scope);}
void style_control_scope_observe(style_control_scope *scopes,const CWPRETSTRUCT *message,
    void (*created)(style_control_scope *,const CWPRETSTRUCT *,void *),void *context)
{
    if(message->message==WM_NCDESTROY)style_control_scope_root_destroyed(scopes,message->hwnd);
    else if(message->message==WM_CREATE && message->lResult!=-1){
        style_control_scope *scope=style_control_scope_find(scopes,message->hwnd);
        if(scope){style_control_scope_retain(scope);created(scope,message,context);style_control_scope_release(scope);}
    }
}
