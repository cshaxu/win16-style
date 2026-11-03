#ifndef WIN16_CONTROL_COMBO_TEXT_H
#define WIN16_CONTROL_COMBO_TEXT_H
#include "control_state.h"
#include <commctrl.h>
LRESULT win16_control_combo_text_message(HWND,UINT,WPARAM,LPARAM,UINT_PTR,control_style *,SUBCLASSPROC,SUBCLASSPROC);
void win16_control_combo_text_unbind(control_style *,BOOL,SUBCLASSPROC);
#endif
