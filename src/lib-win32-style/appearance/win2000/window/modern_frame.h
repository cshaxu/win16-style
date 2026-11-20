#ifndef WIN2K_MODERN_FRAME_H
#define WIN2K_MODERN_FRAME_H
#include "lib-win32-style/application/adapters/frame.h"

#include "lib-win32-style/appearance/msstyles/classic_theme.h"
BOOL win2k_uwp_frame_apply(HWND owner,HWND title,const style_uwp_frame_action *actions,unsigned count,
    const classic_theme_mapping *source);
BOOL win2k_uwp_frame_uses(HWND owner);
#endif
