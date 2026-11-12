#ifndef STYLE_CONTROL_SCOPE_H
#define STYLE_CONTROL_SCOPE_H
#include <windows.h>
#include <stddef.h>

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
#endif
