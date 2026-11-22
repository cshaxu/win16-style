#ifndef STYLE_OBSERVED_LIST_COMBO_H
#define STYLE_OBSERVED_LIST_COMBO_H
#include "lib-win32-style/application/lifecycle/instance.h"
#include "lib-win32-style/application/graphics/paint_frame.h"
#include "lib-win32-style/application/control/control_colors.h"

void style_observed_list_paint(HWND,style_instance *,HWND color_parent,HDC,const style_relief_palette *);
BOOL style_observed_combo_info(HWND,PCOMBOBOXINFO);
BOOL style_observed_combo_paint(HWND,style_instance *,HDC,const style_relief_palette *,const classic_combo_colors *);
#endif
