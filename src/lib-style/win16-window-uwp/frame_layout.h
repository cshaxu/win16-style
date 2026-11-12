#ifndef WIN16_UWP_FRAME_LAYOUT_H
#define WIN16_UWP_FRAME_LAYOUT_H
#include "lib-style/win16-common/layout.h"

/* All rectangles use physical pixels relative to the adapter surface (the
 * outer window, clipped to the monitor work area when maximized).
 * The application's content rectangle is the transparent, non-intercepting hole.
 * Native title height is retained: the adapter must not move XAML content. */
BOOL win16_uwp_frame_layout(SIZE outer,UINT dpi,DWORD style,BOOL maximized,
    RECT content,win16_style_layout *layout);
LRESULT win16_uwp_frame_hit(const win16_style_layout *layout,POINT point);
#endif
