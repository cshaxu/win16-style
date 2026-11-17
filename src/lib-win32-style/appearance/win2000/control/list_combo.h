#ifndef WIN2K_LIST_COMBO_H
#define WIN2K_LIST_COMBO_H
#include "lib-win32-style/application/lifecycle/instance.h"

void win2k_list_paint(HWND,style_instance *,HWND color_parent,HDC);
BOOL win2k_combo_info(HWND,PCOMBOBOXINFO);
BOOL win2k_combo_paint(HWND,style_instance *,HDC);
#endif
