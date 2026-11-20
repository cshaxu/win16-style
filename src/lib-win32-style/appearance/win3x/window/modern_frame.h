#ifndef WIN16_UWP_FRAME_H
#define WIN16_UWP_FRAME_H
#include "lib-win32-style/application/adapters/frame.h"
#include "lib-win32-style/appearance/msstyles/classic_theme.h"
BOOL win16_uwp_frame_uses(HWND);
/* Selects the Win3.x renderer; lifetime, action borrowing and local GUI-thread
 * removal/surface lookup use the common frame contract. */
BOOL win16_uwp_frame_apply_actions(HWND,HWND native_title,const style_uwp_frame_action *,unsigned,
    const classic_theme_mapping *);
/* The caller owns allocation and restoration of the native frame. */
BOOL win16_uwp_frame_apply_nonclient(HWND);
#endif
