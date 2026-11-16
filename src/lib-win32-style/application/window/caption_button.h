#ifndef STYLE_WINDOW_CAPTION_BUTTON_H
#define STYLE_WINDOW_CAPTION_BUTTON_H
#include <windows.h>

/* Owned by the GUI-thread frame instance; hit values belong to its renderer. */
typedef struct style_caption_button { int pressed; BOOL hot; } style_caption_button;

static inline BOOL style_caption_button_begin(HWND window,style_caption_button *state,
    int hit,BOOL require_capture)
{
    state->pressed=hit;state->hot=TRUE;
    SetCapture(window);
    if(require_capture && GetCapture()!=window){*state=(style_caption_button){0};return FALSE;}
    return TRUE;
}

/* Clear before ReleaseCapture, which can synchronously reenter cancellation.
 * Some existing callers release unconditionally; retain that explicit policy.
 * Command selection, invocation order and painting remain with the caller. */
static inline BOOL style_caption_button_move(HWND window,style_caption_button *state,
    BOOL inside,BOOL release,BOOL release_unowned)
{
    state->hot=inside;
    if(!release)return FALSE;
    BOOL invoke=state->hot;state->pressed=0;
    if(release_unowned || GetCapture()==window)ReleaseCapture();
    return invoke;
}

static inline int style_caption_button_cancel(HWND window,style_caption_button *state,UINT message)
{
    int hit=state->pressed;*state=(style_caption_button){0};
    if(message==WM_CANCELMODE && GetCapture()==window)ReleaseCapture();
    return hit;
}
#endif
