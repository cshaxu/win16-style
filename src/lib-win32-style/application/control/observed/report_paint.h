#ifndef STYLE_OBSERVED_REPORT_PAINT_H
#define STYLE_OBSERVED_REPORT_PAINT_H
#include <windows.h>
#include <commctrl.h>
#include "lib-win32-style/application/graphics/paint_frame.h"
#include "lib-win32-style/application/control/control_colors.h"
#include "lib-win32-style/application/lifecycle/instance.h"

typedef struct style_observed_report_draw {
    const style_instance *instance;
    style_relief_palette palette;
    const classic_report_item_colors *colors;
} style_observed_report_draw;

BOOL style_observed_listview_supported(HWND);
BOOL style_observed_listview_plain(HWND);
BOOL style_observed_treeview_supported(HWND);
void style_observed_listview_colors(HWND,NMCUSTOMDRAW *,const void *);
void style_observed_treeview_colors(HWND,NMCUSTOMDRAW *,const void *);
#endif
