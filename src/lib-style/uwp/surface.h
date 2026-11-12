#ifndef STYLE_UWP_SURFACE_H
#define STYLE_UWP_SURFACE_H
#include <windows.h>
/* Coordinates are relative to the returned outer rectangle. Title may be NULL.
 * Handles are borrowed; the caller revalidates before using the result. */
BOOL style_uwp_frame_bounds(HWND window,HWND title,RECT *outer,RECT *content,BOOL *zoomed);
/* Zero-initialize before first use; dispose before abandoning the object. */
typedef struct style_uwp_bitmap {
    HDC dc;HBITMAP bitmap;HGDIOBJ old_bitmap;BYTE *pixels;SIZE size;
} style_uwp_bitmap;
BOOL style_uwp_bitmap_ensure(style_uwp_bitmap *bitmap,SIZE size);
void style_uwp_bitmap_dispose(style_uwp_bitmap *bitmap);
#endif
