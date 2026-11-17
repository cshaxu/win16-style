#ifndef STYLE_RUNTIME_IDENTITY_H
#define STYLE_RUNTIME_IDENTITY_H
#include "../../lifecycle/instance.h"
typedef struct style_runtime_frame_state {
    BOOL candidate,verified,retired;
    HWND parent;
    HMODULE owner;
} style_runtime_frame_state;
typedef struct style_runtime_rich_state {
    BOOL candidate,installed,witness,unicode,hotkey,month,popup;
    HWND parent;
    HMODULE owner;
} style_runtime_rich_state;
typedef struct style_runtime_frame_ops {
    void *context;
    BOOL (*install)(HWND,void *,const BOOL *);
    BOOL (*remove)(HWND,const void *);
} style_runtime_frame_ops;
/* Complete native runtime identities; no renderer or private field views.
 * Handles are borrowed and the caller pins the instance/state for callbacks.
 * The month rejection mask is the caller's supported presentation policy. */
BOOL style_runtime_scrollbox_identity(HWND,HMODULE,HWND);
BOOL style_runtime_controlbar_identity(HWND,const style_instance *,const style_runtime_frame_state *);
BOOL style_runtime_rich_identity(HWND,const style_instance *,const style_runtime_rich_state *,DWORD);
void style_runtime_frame_retire(HWND,style_runtime_frame_state *);
void style_runtime_box_settle(HWND,style_instance *,style_runtime_frame_state *,BOOL,const style_runtime_frame_ops *);
void style_runtime_rich_settle(HWND,UINT,style_instance *,style_runtime_rich_state *,DWORD,const style_runtime_frame_ops *);
#endif
