#ifndef WIN16_UWP_FRAME_LAYOUT_H
#define WIN16_UWP_FRAME_LAYOUT_H
#include "lib-win32-style/application/window/full_client_layout.h"

/* All rectangles use physical pixels relative to the adapter surface (the
 * outer window, clipped to the monitor work area when maximized).
 * The application's content rectangle is the transparent, non-intercepting hole.
 * Native title height is retained: the adapter must not move XAML content. */
BOOL win16_uwp_frame_layout(SIZE outer,UINT dpi,DWORD style,DWORD ex_style,BOOL maximized,
    RECT content,const classic_button_group *buttons,const classic_caption_metrics *metrics,
    const style_frame_geometry *frame,style_full_client_layout *layout);
LRESULT win16_uwp_frame_hit(const style_full_client_layout *layout,POINT point);
#endif
