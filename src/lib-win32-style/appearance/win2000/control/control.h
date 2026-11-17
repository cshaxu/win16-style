#ifndef WIN2K_CONTROL_H
#define WIN2K_CONTROL_H
#include "lib-win32-style/application/lifecycle/theme_state.h"

/* GUI-thread operations; native_theme and this module outlive attachments.
 * Native push/checkbox/radio/three-state and applicable pushlike presentation.
 * Other control families keep their existing product path until implemented. */
BOOL win2k_control_supported(HWND);
BOOL win2k_control_attach(HWND,style_native_theme_fn);
BOOL win2k_control_attached(HWND);
BOOL win2k_control_remove(HWND);
#endif
