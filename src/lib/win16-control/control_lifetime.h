#ifndef WIN16_CONTROL_LIFETIME_H
#define WIN16_CONTROL_LIFETIME_H
#include "control_state.h"
#include <commctrl.h>
HWND win16_control_spin_before(HWND,UINT,RECT *);
void win16_control_finish_message(HWND,control_style *,UINT,LPARAM,HWND,const RECT *,BOOL,HFONT,HFONT,HFONT,SUBCLASSPROC);
#endif
