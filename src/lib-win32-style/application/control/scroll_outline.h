#ifndef STYLE_SCROLL_OUTLINE_H
#define STYLE_SCROLL_OUTLINE_H
#include "../window/palette.h"
#include "../graphics/paint_frame.h"
#include "lib-win32-style/application/control/scroll_controller.h"
#include "scroll_palette.h"
typedef struct style_scroll_outline_view {
    const style_window_palette *palette;
    const style_scroll_controller *scroll;
    BOOL (*scroll_layout)(HWND,style_scroll_layout *);
    BOOL (*embedded_scroll_layout)(HWND,LONG,style_scroll_layout *,POINT *);
    BOOL (*edit_thumb_messages)(HWND,LONG);
    BOOL (*list_axis_owned)(HWND,LONG);
    const style_window_palette *arrow_palette;
    const style_native_scroll_colors *colors;
    const style_scroll_arrow_art *art;
} style_scroll_outline_view;
void style_scroll_outline_paint(HWND window,const style_scroll_outline_view *view,HDC dc,LONG object,BOOL vertical);
#endif
