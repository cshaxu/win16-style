#ifndef STYLE_CONTROL_STATUS_H
#define STYLE_CONTROL_STATUS_H
#include <windows.h>
/* Alias and witnesses are caller-owned; NULL view permits only native class. */
typedef struct style_control_status_view {
    const BOOL *candidate,*detached;const HMODULE *witness_owner;const HWND *parent;
    LPCWSTR class_name;
} style_control_status_view;
BOOL style_control_status_identity(HWND,const style_control_status_view *,HMODULE *);
/* Invalidate the caller's cached font or content snapshot after native messages. */
void style_control_status_invalidate(UINT,BOOL *,unsigned *);
/* Buffers hold 513 WCHARs and 64 rectangles each; counts start at zero.
 * The owner and borrowed buffers remain alive through synchronous callbacks. */
typedef struct style_control_status_snapshot_view {
    void *owner; const BOOL *detached; const unsigned *owner_epoch; unsigned epoch;
    HMODULE *native; BOOL *simple; int *simple_length; WCHAR *simple_text;
    unsigned *text_count; RECT *text_panes; unsigned *count; RECT *panes;
    BOOL (*identity)(HWND,void *,HMODULE *);
} style_control_status_snapshot_view;
void style_control_status_snapshot(HWND,const style_control_status_snapshot_view *);
#endif
