#ifndef STYLE_CONTROL_SCOPE_H
#define STYLE_CONTROL_SCOPE_H
#include <windows.h>
#include <commctrl.h>
#include <stddef.h>
#include "lib-win32-style/application/window/palette.h"

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
typedef struct style_control_scope_counts {unsigned installed,unsupported,failed;} style_control_scope_counts;
/* Zero-initialize once; only these APIs modify the manager. All operations,
 * callbacks and the native observer binding belong to the root's GUI thread.
 * Scope data survives an end during a callback until that callback returns. */
typedef struct style_control_scope_manager {
    style_control_scope *scopes;
    HHOOK hook;
} style_control_scope_manager;
style_control_scope *style_control_scope_begin(HWND root,size_t data_size,
    style_control_scope_manager *,HOOKPROC observer);
BOOL style_control_scope_end(style_control_scope *,style_control_scope_counts *,style_control_scope_manager *);
BOOL style_control_scope_contains(const style_control_scope_manager *,const style_control_scope *);
style_control_scope *style_control_scope_find(const style_control_scope_manager *,HWND child);
void *style_control_scope_data(style_control_scope *);
/* Observe successful native child creation, skipping a combo's internal
 * controls. Installation is counted after the callback, including reentry. */
void style_control_scope_observe(style_control_scope_manager *,const CWPRETSTRUCT *,
    BOOL (*install)(style_control_scope *,const CWPRETSTRUCT *,BOOL edit,void *),void *);

typedef struct style_control_palette_scope_services {
    BOOL (*edit_install)(HWND,BOOL,const style_window_palette *);
    BOOL (*control_install)(HWND,const style_window_palette *);
} style_control_palette_scope_services;
BOOL style_control_palette_scope_install(style_control_scope *,const CWPRETSTRUCT *,BOOL edit,void *services);
style_control_scope *style_control_palette_scope_begin(HWND,const style_window_palette *,style_control_scope_manager *,HOOKPROC);
#endif
