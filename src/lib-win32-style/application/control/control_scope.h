#ifndef STYLE_CONTROL_SCOPE_H
#define STYLE_CONTROL_SCOPE_H
#include <windows.h>
#include <commctrl.h>
#include <stddef.h>

static inline BOOL style_control_subclass_owned(HWND window,SUBCLASSPROC proc,UINT_PTR id,const void *owner)
{
    DWORD_PTR value;
    return window && GetWindowSubclass(window,proc,id,&value) && value==(DWORD_PTR)owner;
}
static inline void style_control_subclass_remove_owned(HWND window,SUBCLASSPROC proc,UINT_PTR id,const void *owner)
{
    if(style_control_subclass_owned(window,proc,id,owner))RemoveWindowSubclass(window,proc,id);
}

typedef struct style_control_scope style_control_scope;
/* All operations, including callbacks, belong to the root's GUI thread.
 * The caller supplies a hook procedure and owns the list and hook variables.
 * Data is zeroed and remains valid until the last retained callback returns. */
style_control_scope *style_control_scope_begin(HWND root,size_t data_size,
    style_control_scope **scopes,HHOOK *hook,HOOKPROC observer);
BOOL style_control_scope_end(style_control_scope *scope,style_control_scope **scopes,HHOOK *hook);
BOOL style_control_scope_contains(style_control_scope *scopes,const style_control_scope *scope);
style_control_scope *style_control_scope_find(style_control_scope *scopes,HWND child);
void style_control_scope_root_destroyed(style_control_scope *scopes,HWND root);
void *style_control_scope_data(style_control_scope *scope);
void style_control_scope_retain(style_control_scope *scope);
void style_control_scope_release(style_control_scope *scope);
/* Dispatch a successful child creation while retaining the matched scope. */
void style_control_scope_observe(style_control_scope *scopes,const CWPRETSTRUCT *message,
    void (*created)(style_control_scope *,const CWPRETSTRUCT *,void *),void *context);
#endif
