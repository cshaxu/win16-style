#ifndef WIN2K_REPORT_PAINT_H
#define WIN2K_REPORT_PAINT_H
#include <windows.h>
#include <commctrl.h>
#include "../common/palette.h"
#include "lib-win32-style/appearance/msstyles/classic_theme.h"
#include "lib-win32-style/application/lifecycle/instance.h"

typedef struct win2k_report_draw {
    const style_instance *instance;
    win2k_palette palette;
    const classic_report_item_colors *colors;
} win2k_report_draw;

BOOL win2k_listview_supported(HWND);
BOOL win2k_listview_plain(HWND);
BOOL win2k_treeview_supported(HWND);
void win2k_listview_colors(HWND,NMCUSTOMDRAW *,const void *);
void win2k_treeview_colors(HWND,NMCUSTOMDRAW *,const void *);
#endif
