#ifndef STYLE_SCROLL_TRACK_H
#define STYLE_SCROLL_TRACK_H
#include "scroll_geometry.h"
#include <limits.h>
#define STYLE_SCROLL_KEEP_TIMER UINT_MAX

/* No HWND/callback ownership here. The adapter applies capture/timer/paint and
 * dispatches notices, revalidating its attachment after EVERY notification. */
typedef struct style_scroll_notice { UINT command; LONG position; } style_scroll_notice;
typedef struct style_scroll_actions {
    style_scroll_notice notices[2];
    UINT count,timer_ms; /* KEEP_TIMER: unchanged; 0: stop; otherwise rearm. */
    BOOL repaint;
} style_scroll_actions;
typedef struct style_scroll_track {
    style_scroll_part part;
    RECT drag_limit;
    int grab_offset,initial_pixel,pixel;
    LONG position;
    UINT repeat_ms;
    BOOL active,inside;
} style_scroll_track;

typedef struct style_scroll_policy {
    UINT initial_repeat_ms,repeat_ms;
    int tolerance_x,tolerance_y; /* logical pixels; scaled once by controller */
    style_scroll_part (*hit)(const style_scroll_layout *,POINT,UINT);
    RECT (*drag_bounds)(HWND,const style_scroll_layout *);
} style_scroll_policy;

BOOL style_scroll_begin(style_scroll_track *track,const style_scroll_layout *layout,
    UINT disabled,POINT point,const RECT *drag_limit,style_scroll_actions *actions,const style_scroll_policy *policy);
void style_scroll_motion(style_scroll_track *track,const style_scroll_layout *layout,
    UINT disabled,POINT point,BOOL timer,style_scroll_actions *actions,const style_scroll_policy *policy);
void style_scroll_end(style_scroll_track *track,BOOL cancel,style_scroll_actions *actions);
#endif
