#ifndef STYLE_RUNTIME_CHECKLIST_H
#define STYLE_RUNTIME_CHECKLIST_H
#include "runtime_observer.h"
/* Synchronous runtime observations only. All HWND/HDC/module/bitmap handles
 * and item rectangles are borrowed. The renderer supplies its accepted maximum
 * side in current DC pixels; no profile geometry or raster is owned here. */
BOOL style_runtime_checklist_target(HDC dc,HDC expected,HWND window,HWND parent,
    const RECT *item,const RECT *bounds,const void *caller,BOOL (*identity)(HWND),int maximum_side);
/* Synchronous owner-draw association. Caller pins observation.instance and its
 * runtime until end. Nested unrelated messages do not publish a scope. The
 * preexisting notify path has normal-return cleanup only, not unwind cleanup. */
typedef struct style_runtime_checklist_scope {
    struct style_runtime_checklist_scope *previous;
    style_runtime_observation observation;HDC dc;RECT item;
} style_runtime_checklist_scope;
typedef struct style_runtime_checklists {style_runtime_checklist_scope *current;} style_runtime_checklists;
BOOL style_runtime_checklist_begin(style_runtime_checklists *,style_runtime_checklist_scope *,const style_runtime_observation *,HWND,UINT,LPARAM,BOOL (*)(HWND));
void style_runtime_checklist_end(style_runtime_checklists *,const style_runtime_checklist_scope *);
BOOL style_runtime_checklist_draw_target(const style_runtime_checklists *,HDC,const RECT *,const void *,BOOL (*)(HWND),int);

typedef BOOL (*style_runtime_checklist_painter)(HDC,const RECT *,UINT);
/* Existing VB5/VB6 bitmap protocol, including native-state revalidation and
 * thread-local nested-query exclusion. Paint is borrowed for this call only.
 * Rejected observations return FALSE and leave native pixels authoritative. */
BOOL style_runtime_vb_checklist(HDC,int,int,int,int,HDC,int,int,DWORD,const void *,style_runtime_checklist_painter);
#endif
