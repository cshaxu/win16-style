#ifndef STYLE_SCROLL_RESOURCES_H
#define STYLE_SCROLL_RESOURCES_H
#include <windows.h>

/* GUI-thread-owned runtime resources; geometry and drag policy belong to the
 * caller. The previous capture is borrowed, never destroyed. */
typedef struct style_scroll_resources {
    BOOL frame,edit,list; /* active native scroll protocol roles */
    BOOL native_border; /* application owns native nonclient repaint */
    UINT_PTR timer;
    HWND previous_capture;
    BOOL caret,caret_shown;
    SIZE caret_size;
} style_scroll_resources;

void style_scroll_caret_destroy(HWND window,style_scroll_resources *resources);
void style_scroll_caret_sync(HWND window,style_scroll_resources *resources,const RECT *rectangle,BOOL tracking);
#endif
