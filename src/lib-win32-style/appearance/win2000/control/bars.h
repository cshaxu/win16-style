#ifndef WIN2K_CONTROL_BARS_H
#define WIN2K_CONTROL_BARS_H
#include "lib-win32-style/application/control/notify.h"
#include "lib-win32-style/application/control/control_status.h"

extern const style_control_notify_ops win2k_toolbar_notify;
BOOL win2k_rebar_supported(HWND);
void win2k_rebar_grip(HWND,const NMCUSTOMDRAW *,const void *);
void win2k_status_panes(HWND,HDC,const style_instance *,const style_control_status_state *);

#endif
