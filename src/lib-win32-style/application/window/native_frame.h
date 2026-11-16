#ifndef STYLE_WINDOW_NATIVE_FRAME_H
#define STYLE_WINDOW_NATIVE_FRAME_H
#include "dwm.h"
#include "frame.h"

/* GUI-thread state borrowed by the owning style_instance. DWM exposes the
 * effective rendering state, never the original set-only policy. */
typedef struct style_native_frame {
    DWORD policy;
    BOOL saved, pending, keep_outer;
    BOOL active;
    WPARAM hover;
    UINT refresh_message;
    BOOL refresh_pending;
} style_native_frame;

/* Defer a frame calculation until the creator has finished initialization.
 * A failed post leaves the pending work for the existing show fallback. */
static inline void style_native_frame_queue(style_native_frame *state,HWND window,UINT message)
{
    if(!message)return;
    state->refresh_message=message;state->refresh_pending=TRUE;
    PostMessageW(window,message,0,0);
}
static inline BOOL style_native_frame_take_refresh(style_native_frame *state)
{
    BOOL pending=state->refresh_pending;state->refresh_pending=FALSE;return pending;
}

/* restore_default preserves callers which restore USEWINDOWSTYLE when the
 * effective state cannot be observed. Other callers leave it untouched. */
static inline void style_native_frame_capture(style_native_frame *state,HWND window,
    style_dwm_get get,BOOL restore_default)
{
    state->policy=DWMNCRP_USEWINDOWSTYLE;
    state->saved=FALSE;
    if(!(GetWindowLongPtrW(window,GWL_STYLE)&WS_CHILD))
        state->saved=style_dwm_capture(window,get,&state->policy) || restore_default;
}

/* The owner holds its instance across synchronous position/message reentry.
 * Rendering-policy failure remains best effort for this activation path. */
static inline void style_native_frame_activate(HWND window,const style_native_frame *state,
    const BOOL *detached,const RECT *client,BOOL normal,style_dwm_set set)
{
    if(!(GetWindowLongPtrW(window,GWL_STYLE)&WS_CHILD))
        style_dwm_policy(window,set,DWMNCRP_DISABLED);
    if(!*detached)style_frame_changed(window);
    if(!*detached && normal && !state->keep_outer)style_restore_client(window,client);
}

/* Callers keep their existing error contract: required restoration bubbles
 * up HRESULT; best-effort callers can continue geometry restoration. */
static inline HRESULT style_native_frame_restore_policy(HWND window,
    const style_native_frame *state,style_dwm_set set)
{
    if(!IsWindow(window) || state->pending || !state->saved)return S_OK;
    return style_dwm_policy(window,set,state->policy);
}

static inline void style_native_frame_restore_geometry(HWND window,
    const style_native_frame *state,const RECT *client,BOOL normal)
{
    if(IsWindow(window) && !state->pending)
        style_frame_changed_restore(window,client,normal && !state->keep_outer);
}
#endif
