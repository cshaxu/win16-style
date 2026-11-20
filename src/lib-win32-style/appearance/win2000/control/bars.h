#ifndef WIN2K_CONTROL_BARS_H
#define WIN2K_CONTROL_BARS_H
#include "lib-win32-style/application/control/notify.h"
#include "lib-win32-style/application/control/control_status.h"
#include "../common/palette.h"
#include "lib-win32-style/appearance/msstyles/classic_theme.h"

typedef struct win2k_bar_draw {
    const style_instance *instance;
    win2k_palette palette;
    COLORREF pressed_face;
} win2k_bar_draw;

extern const style_control_notify_ops win2k_toolbar_notify;
BOOL win2k_rebar_supported(HWND);
void win2k_rebar_grip(HWND,const NMCUSTOMDRAW *,const void *);
void win2k_status_panes(HWND,HDC,const style_instance *,const style_control_status_state *,const win2k_palette *,
    const classic_bar_colors *);

#endif
