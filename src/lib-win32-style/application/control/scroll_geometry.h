#ifndef STYLE_SCROLL_GEOMETRY_H
#define STYLE_SCROLL_GEOMETRY_H
#include <windows.h>
/* Caller-relative measured geometry. The provider owns thumb size, overlap and
 * hit priority; the controller owns tracking, resources and native notices. */
typedef struct style_scroll_layout {
    RECT bar,first,last,thumb;
    int minimum_pixel,travel;
    LONG minimum,maximum,position;
    BOOL vertical,thumb_fits,thumb_draggable;
} style_scroll_layout;
typedef enum style_scroll_part {
    STYLE_SCROLL_NONE,STYLE_SCROLL_LINE_BEFORE,STYLE_SCROLL_LINE_AFTER,
    STYLE_SCROLL_PAGE_BEFORE,STYLE_SCROLL_PAGE_AFTER,STYLE_SCROLL_THUMB
} style_scroll_part;
#endif
