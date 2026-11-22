#include "control_scope.h"
#include "control_class.h"
#include "../dialog/message_observer.h"
#include <stdlib.h>
#include <stdint.h>

struct style_control_scope {
    struct style_control_scope *next;
    HWND root;
    unsigned calls;
    BOOL detached;
    style_control_scope_counts counts;
    max_align_t data[];
};

style_control_scope *style_control_scope_begin(HWND root,size_t size,
    style_control_scope_manager *manager,HOOKPROC observer)
{
    if(GetWindowThreadProcessId(root,NULL)!=GetCurrentThreadId()){SetLastError(ERROR_INVALID_THREAD_ID);return NULL;}
    if(size>SIZE_MAX-sizeof(style_control_scope)){SetLastError(ERROR_NOT_ENOUGH_MEMORY);return NULL;}
    style_control_scope *scope=calloc(1,sizeof(*scope)+size);
    if(!scope){SetLastError(ERROR_NOT_ENOUGH_MEMORY);return NULL;}
    if(!manager->scopes){manager->hook=style_message_observer_begin(observer);
        if(!manager->hook){free(scope);return NULL;}}
    scope->root=root;scope->next=manager->scopes;manager->scopes=scope;return scope;
}
BOOL style_control_scope_end(style_control_scope *scope,style_control_scope_counts *counts,style_control_scope_manager *manager)
{
    style_control_scope **link=&manager->scopes;
    while(*link && *link!=scope)link=&(*link)->next;
    if(!scope || !*link){SetLastError(ERROR_INVALID_PARAMETER);return FALSE;}
    style_control_scope_counts value=scope->counts;
    *link=scope->next;scope->detached=TRUE;
    if(!manager->scopes){style_message_observer_end(manager->hook);manager->hook=NULL;}
    if(!scope->calls)free(scope);
    if(counts)*counts=value;
    return TRUE;
}
style_control_scope *style_control_scope_find(const style_control_scope_manager *manager,HWND child)
{for(style_control_scope *s=manager->scopes;s;s=s->next)if(s->root && IsChild(s->root,child))return s;return NULL;}
BOOL style_control_scope_contains(const style_control_scope_manager *manager,const style_control_scope *target)
{for(style_control_scope *s=manager->scopes;s;s=s->next)if(s==target)return TRUE;return FALSE;}
void *style_control_scope_data(style_control_scope *scope){return scope->data;}
void style_control_scope_observe(style_control_scope_manager *manager,const CWPRETSTRUCT *message,
    BOOL (*install)(style_control_scope *,const CWPRETSTRUCT *,BOOL,void *),void *context)
{
    if(message->message==WM_NCDESTROY){
        for(style_control_scope *s=manager->scopes;s;s=s->next)if(s->root==message->hwnd)s->root=NULL;
    }else if(message->message==WM_CREATE && message->lResult!=-1){
        style_control_scope *scope=style_control_scope_find(manager,message->hwnd);
        if(!scope)return;
        ++scope->calls;
        HWND window=message->hwnd;BOOL edit=style_control_class_is(window,L"EDIT");
        if(!edit && !style_control_class_is(window,L"BUTTON") && !style_control_class_is(window,L"STATIC") &&
           !style_control_class_is(window,L"LISTBOX") && !style_control_class_is(window,L"COMBOBOX") &&
           !style_control_class_is(window,L"SCROLLBAR") && !style_control_class_is(window,L"SysLink"))goto done;
        /* A combo installs its internal list/edit after native construction. */
        for(HWND ancestor=GetParent(window);ancestor;ancestor=GetParent(ancestor))
            if(style_control_class_is(ancestor,L"COMBOBOX"))goto done;
        BOOL installed=install(scope,message,edit,context);DWORD failure=GetLastError();
        if(installed)++scope->counts.installed;
        else if(failure==ERROR_NOT_SUPPORTED)++scope->counts.unsupported;
        else ++scope->counts.failed;
done:
        if(!--scope->calls && scope->detached)free(scope);
    }
}

BOOL style_control_palette_scope_install(style_control_scope *scope,const CWPRETSTRUCT *message,BOOL edit,void *context)
{
    const style_control_palette_scope_services *services=context;
    const CREATESTRUCTW *create=(const CREATESTRUCTW *)message->lParam;
    const style_window_palette *palette=style_control_scope_data(scope);
    return edit?services->edit_install(message->hwnd,(create->style&WS_BORDER)!=0,palette):
        services->control_install(message->hwnd,palette);
}
style_control_scope *style_control_palette_scope_begin(HWND root,const style_window_palette *palette,
    style_control_scope_manager *manager,HOOKPROC observer)
{
    style_control_scope *scope=style_control_scope_begin(root,sizeof(style_window_palette),manager,observer);
    if(!scope)return NULL;
    style_window_palette *data=style_control_scope_data(scope);
    if(palette)*data=*palette;else style_window_palette_default(data);
    return scope;
}
