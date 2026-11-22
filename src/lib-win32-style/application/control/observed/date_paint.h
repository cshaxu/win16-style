#ifndef STYLE_OBSERVED_DATE_PAINT_H
#define STYLE_OBSERVED_DATE_PAINT_H
#include "lib-win32-style/application/control/calendar_colors.h"
#include "lib-win32-style/application/control/control_native_combo.h"
#include "lib-win32-style/application/graphics/paint_frame.h"
#include "lib-win32-style/application/control/control_colors.h"

BOOL style_observed_date_supported(HWND);
BOOL style_observed_month_supported(HWND);
void style_observed_date_colors(HWND,const style_instance *,style_calendar_colors *,BOOL,const style_relief_palette *,
    const classic_calendar_colors *);
void style_observed_date_button(HWND,const style_instance *,HDC,const RECT *,const style_control_native_button *,
    const style_relief_palette *,const style_scroll_arrow_art *);
#endif
