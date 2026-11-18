#ifndef WIN2K_REPORT_PAINT_H
#define WIN2K_REPORT_PAINT_H
#include <windows.h>
#include <commctrl.h>

BOOL win2k_listview_supported(HWND);
BOOL win2k_listview_plain(HWND);
BOOL win2k_treeview_supported(HWND);
void win2k_listview_colors(HWND,NMCUSTOMDRAW *,const void *);
void win2k_treeview_colors(HWND,NMCUSTOMDRAW *,const void *);
#endif
