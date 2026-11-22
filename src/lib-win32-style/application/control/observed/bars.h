#ifndef STYLE_OBSERVED_CONTROL_BARS_H
#define STYLE_OBSERVED_CONTROL_BARS_H
#include "lib-win32-style/application/control/notify.h"
#include "lib-win32-style/application/control/control_status.h"
#include "lib-win32-style/application/graphics/paint_frame.h"
#include "lib-win32-style/application/control/control_colors.h"

typedef struct style_observed_bar_draw {
    const style_instance *instance;
    style_relief_palette palette;
    COLORREF pressed_face;
} style_observed_bar_draw;

extern const style_control_notify_ops style_observed_toolbar_notify;
BOOL style_observed_rebar_supported(HWND);
void style_observed_rebar_grip(HWND,const NMCUSTOMDRAW *,const void *);
void style_observed_status_panes(HWND,HDC,const style_instance *,const style_control_status_state *,const style_relief_palette *,
    const classic_bar_colors *);

#endif
