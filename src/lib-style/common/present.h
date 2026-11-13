#ifndef STYLE_COMMON_PRESENT_H
#define STYLE_COMMON_PRESENT_H
#include <windows.h>
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
typedef struct style_present_view {
    const BOOL *detached;
    COLORREF background;
    void *context;
    void (*paint)(HWND,void *,HDC);
    void *classification;
    BOOL (*is_group)(HWND,void *);
} style_present_view;
void style_present(HWND window,const style_present_view *view,HDC target);
void style_print_client_and_children(HWND window,const BOOL *detached,HDC dc,LPARAM flags);
#endif
