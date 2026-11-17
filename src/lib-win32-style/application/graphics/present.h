#ifndef STYLE_COMMON_PRESENT_H
#define STYLE_COMMON_PRESENT_H
#include <windows.h>
#include "../lifecycle/instance.h"
typedef struct style_present_buffer {
    HDC target,memory;
    HBITMAP bitmap,old;
    RECT bounds;
} style_present_buffer;
/* All arguments are required. Preserve copies the target pixels first;
 * without it the painter must fill the entire buffer before committing.
 * Call end after every begin, including failure. */
BOOL style_present_buffer_begin(style_present_buffer *,HDC,const RECT *,BOOL preserve);
BOOL style_present_buffer_end(style_present_buffer *,BOOL);
/* Complete bounded buffer transaction on a borrowed target DC. Existing
 * invocation pins instance/context; retirement cancels commit and fallback.
 * Preserves pixels and the existing direct-paint fallback on buffer failure. */
void style_present_buffered(HWND,style_instance *,void *,void (*paint)(HWND,void *,HDC),HDC,const RECT *);
typedef struct style_present_ops {
    void (*paint)(HWND,void *,HDC);
    BOOL (*is_group)(HWND,void *);
} style_present_ops;
/* Synchronous borrowed renderer and instance. The caller holds its existing
 * invocation reference through all native/painter callbacks and cleanup.
 * Retirement suppresses subsequent painting and buffer commit. No new
 * attachment, reference or exception recovery is introduced here. */
void style_present(HWND,style_instance *,void *,COLORREF,const style_present_ops *,HDC);
void style_print_client_and_children(HWND,style_instance *,HDC,LPARAM);
#endif
