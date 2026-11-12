#ifndef STYLE_THEME_TRACK_H
#define STYLE_THEME_TRACK_H
#include <windows.h>
#include <uxtheme.h>

/* A ticket is valid only while no theme open/close has crossed the draw. */
typedef struct style_theme_track_ticket {
    ULONGLONG epoch;
    BOOL eligible;
    HTHEME theme;
} style_theme_track_ticket;

/* Kind values are supplied by the caller; the tracker owns no style policy.
 * Each linked module has one process-wide tracker, not one per window. */
style_theme_track_ticket style_theme_track_open_begin(void);
void style_theme_track_open_end(style_theme_track_ticket, HTHEME, int);
void style_theme_track_open_cancel(void);
void style_theme_track_close_begin(HTHEME);
void style_theme_track_close_end(HTHEME, BOOL);
style_theme_track_ticket style_theme_track_begin(HTHEME, int);
BOOL style_theme_track_end(HTHEME, style_theme_track_ticket, int);
#endif
