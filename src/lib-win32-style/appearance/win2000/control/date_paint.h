#ifndef WIN2K_DATE_PAINT_H
#define WIN2K_DATE_PAINT_H
#include "lib-win32-style/application/control/calendar_colors.h"
#include "lib-win32-style/application/control/control_native_combo.h"

BOOL win2k_date_supported(HWND);
BOOL win2k_month_supported(HWND);
void win2k_date_colors(HWND,const style_instance *,style_calendar_colors *,BOOL);
void win2k_date_button(HWND,const style_instance *,HDC,const RECT *,const style_control_native_button *);
#endif
