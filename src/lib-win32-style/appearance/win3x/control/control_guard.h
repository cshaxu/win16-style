#ifndef WIN16_CONTROL_GUARD_H
#define WIN16_CONTROL_GUARD_H
#include "control_state.h"
#include "lib-win32-style/application/control/basic.h"
BOOL win16_control_supported(HWND,HWND);
style_control_basic_kind win16_control_basic_kind(HWND);
BOOL win16_control_choice_unsupported(HWND);
#endif
