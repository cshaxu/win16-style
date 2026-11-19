#ifndef WIN16_CONTROL_SCROLL_PAINT_H
#define WIN16_CONTROL_SCROLL_PAINT_H
#include "../common/palette.h"
#include "lib-win32-style/application/control/scroll_controller.h"
#include "lib-win32-style/appearance/msstyles/classic_theme.h"
typedef struct win16_control_scroll_view {
    const win16_style_palette *palette;
    const style_scroll_controller *scroll;
    BOOL (*scroll_layout)(HWND,style_scroll_layout *);
    BOOL (*embedded_scroll_layout)(HWND,LONG,style_scroll_layout *,POINT *);
    BOOL (*edit_thumb_messages)(HWND,LONG);
    BOOL (*list_axis_owned)(HWND,LONG);
    const win16_style_palette *arrow_palette;
    const classic_scroll_colors *colors;
} win16_control_scroll_view;
void win16_control_draw_scroll(HWND window,const win16_control_scroll_view *view,HDC dc,LONG object,BOOL vertical);
#endif
