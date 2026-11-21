#ifndef STYLE_SCROLL_GEOMETRY_H
#define STYLE_SCROLL_GEOMETRY_H
#include <windows.h>
#include <stdint.h>
/* Caller-relative measured geometry. The provider owns thumb size, overlap and
 * hit priority; the controller owns tracking, resources and native notices. */
typedef struct style_scroll_layout {
    RECT bar,first,last,thumb;
    int minimum_pixel,travel;
    LONG minimum,maximum,position;
    BOOL vertical,thumb_fits,thumb_draggable,short_track_pages;
} style_scroll_layout;
typedef enum style_scroll_part {
    STYLE_SCROLL_NONE,STYLE_SCROLL_LINE_BEFORE,STYLE_SCROLL_LINE_AFTER,
    STYLE_SCROLL_PAGE_BEFORE,STYLE_SCROLL_PAGE_AFTER,STYLE_SCROLL_THUMB
} style_scroll_part;

typedef enum style_scroll_thumb {
    STYLE_SCROLL_THUMB_DEFAULT,STYLE_SCROLL_THUMB_FIXED,STYLE_SCROLL_THUMB_PAGE
} style_scroll_thumb;
enum style_scroll_drag_scale { STYLE_SCROLL_DRAG_SYSTEM, STYLE_SCROLL_DRAG_LOGICAL, STYLE_SCROLL_DRAG_BAR };
typedef struct style_scroll_drag {
    uint8_t scale, first, second; /* X/Y units or cross/along multipliers. */
} style_scroll_drag;

typedef struct style_scroll_fixed_metrics { int arrow,thumb,border; } style_scroll_fixed_metrics;
BOOL style_scroll_measure_fixed(const RECT *,BOOL,const style_scroll_fixed_metrics *,LONG,LONG,LONG,style_scroll_layout *);
BOOL style_scroll_measure_page(const RECT *,BOOL,int,const SCROLLINFO *,style_scroll_layout *);
BOOL style_scroll_measure(const RECT *,BOOL,int,int,const SCROLLINFO *,style_scroll_thumb,style_scroll_layout *);
style_scroll_part style_scroll_hit(const style_scroll_layout *,POINT,UINT);
#endif
