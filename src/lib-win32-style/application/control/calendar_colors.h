#ifndef STYLE_CONTROL_CALENDAR_COLORS_H
#define STYLE_CONTROL_CALENDAR_COLORS_H
#include "../lifecycle/instance.h"

#define STYLE_CALENDAR_COLOR_COUNT 6

typedef struct style_calendar_colors {
    COLORREF old[STYLE_CALENDAR_COLOR_COUNT],applied[STYLE_CALENDAR_COLOR_COUNT];
    unsigned owned;
    BOOL setting;
} style_calendar_colors;

/* Native date and calendar controls keep their time, font, and date layout.
 * Only a color still at its system default acquires a style owner. External
 * color setters reclaim ownership, even if they explicitly set the same RGB. */
void style_calendar_colors_apply(HWND,const style_instance *,style_calendar_colors *,BOOL picker,const COLORREF *);
void style_calendar_colors_observe(style_calendar_colors *,BOOL picker,UINT,WPARAM);
void style_calendar_colors_restore(HWND,style_calendar_colors *,BOOL picker);
#endif
