#ifndef WIN16_CONTROL_SCROLL_PAINT_H
#define WIN16_CONTROL_SCROLL_PAINT_H
#include "palette.h"
#include "scroll_track.h"
typedef struct win16_control_scroll_view {
    const win16_style_palette *palette;
    const win16_scroll_track *track;
    const BOOL *frame_scrollbars,*edit_scrollbars,*list_scrollbar;
    const LONG *embedded_object;
    BOOL (*scroll_layout)(HWND,win16_scroll_layout *);
    BOOL (*embedded_scroll_layout)(HWND,LONG,win16_scroll_layout *,POINT *);
    BOOL (*edit_thumb_messages)(HWND,LONG);
    BOOL (*list_axis_owned)(HWND,LONG);
} win16_control_scroll_view;
void win16_control_draw_scroll(HWND window,const win16_control_scroll_view *view,HDC dc,LONG object,BOOL vertical);
#endif
