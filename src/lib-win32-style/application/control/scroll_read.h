#ifndef STYLE_SCROLL_READ_H
#define STYLE_SCROLL_READ_H
#include "scroll_state.h"
#include "scroll_track.h"

BOOL style_scroll_read_control_layout(HWND,style_scroll_info_fn,style_scroll_metrics,style_scroll_defaults,style_scroll_layout *);
BOOL style_scroll_read_embedded_layout(HWND,LONG,style_scroll_info_fn,style_scroll_metrics,style_scroll_defaults,style_scroll_layout *,POINT *);
RECT style_scroll_caret_rectangle(const style_scroll_layout *,const style_scroll_track *,int inset);
#endif
