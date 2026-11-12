#ifndef STYLE_UWP_SURFACE_H
#define STYLE_UWP_SURFACE_H
#include <windows.h>
#include "lib-style/common/bitmap.h"
/* Coordinates are relative to the returned outer rectangle. Title may be NULL.
 * Handles are borrowed; the caller revalidates before using the result. */
BOOL style_uwp_frame_bounds(HWND window,HWND title,RECT *outer,RECT *content,BOOL *zoomed);
/* Preserve native invisible sizing margins without consuming client pixels. */
LRESULT style_uwp_resize_margin_hit(LRESULT hit,POINT point,const RECT *outer,
    const RECT *client,SIZE corner,BOOL sizeable);
/* The bitmap is already painted; hole is a valid rectangle within its bounds.
 * The caller keeps the detached flag and all HWNDs alive until return. */
BOOL style_uwp_surface_present(HWND owner,HWND surface,const RECT *outer,
    const RECT *hole,style_bitmap *bitmap,const BOOL *detached);

/* The caller owns the state and keeps it alive across callbacks. Geometry and
 * painting are style-specific; visibility, buffering and presentation are not. */
typedef struct style_uwp_surface_view {
    HWND owner,*surface;
    BOOL *detached,*updating;
    style_bitmap *bitmap;
    void *context;
    BOOL (*geometry)(void *,RECT *,RECT *);
    void (*paint)(void *,HDC);
} style_uwp_surface_view;
BOOL style_uwp_surface_refresh(const style_uwp_surface_view *view);
#endif
