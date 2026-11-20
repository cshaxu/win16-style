#ifndef WIN2K_BUTTON_PAINT_H
#define WIN2K_BUTTON_PAINT_H
#include "../common/palette.h"
#include "lib-win32-style/appearance/msstyles/classic_theme.h"
#include "lib-win32-style/application/lifecycle/instance.h"

/* Synchronous native BUTTON snapshot. Caller pins instance and owns the DC;
 * application font/images are borrowed and its input protocol stays native. */
BOOL win2k_button_paint(HWND,style_instance *,HDC,const classic_push_colors *,const classic_choice_colors *,
    const classic_group_colors *,HMODULE,classic_choice_art *,classic_control_presentation);
#endif
