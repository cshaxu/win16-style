#ifndef WIN2K_LIST_COMBO_H
#define WIN2K_LIST_COMBO_H
#include "lib-win32-style/application/lifecycle/instance.h"
#include "../common/palette.h"
#include "lib-win32-style/appearance/msstyles/classic_theme.h"

void win2k_list_paint(HWND,style_instance *,HWND color_parent,HDC,const win2k_palette *);
BOOL win2k_combo_info(HWND,PCOMBOBOXINFO);
BOOL win2k_combo_paint(HWND,style_instance *,HDC,const win2k_palette *,const classic_combo_colors *);
#endif
