#ifndef STYLE_OBSERVED_BUTTON_PAINT_H
#define STYLE_OBSERVED_BUTTON_PAINT_H
#include "lib-win32-style/application/graphics/paint_frame.h"
#include "lib-win32-style/application/control/control_colors.h"
#include "lib-win32-style/application/control/presentation.h"
#include "lib-win32-style/application/lifecycle/instance.h"

/* Synchronous native BUTTON snapshot. Caller pins instance and owns the DC;
 * application font/images are borrowed and its input protocol stays native. */
BOOL style_observed_button_paint(HWND,style_instance *,HDC,const classic_push_colors *,const classic_choice_colors *,
    const classic_group_colors *,style_choice_art_read_fn,HMODULE,classic_choice_art *,classic_control_presentation,const style_relief_palette *);
#endif
