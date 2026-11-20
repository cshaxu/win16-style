#ifndef WIN16_CONTROL_MONTH_PAINT_H
#define WIN16_CONTROL_MONTH_PAINT_H
#include "lib-win32-style/application/control/month.h"
#include "../common/palette.h"
typedef struct win16_control_month_view {
    style_control_month_scope native;
    HFONT *cached_font;
    const win16_style_palette *palette;
    const style_scroll_arrow_art *arrows;
    COLORREF selection;
} win16_control_month_view;
COLORREF win16_control_month_selected_color(const classic_theme *,COLORREF);
void win16_control_month_arrow(const win16_control_month_view *,HTHEME,HDC,int,int,const RECT *,const RECT *,const void *);
HFONT win16_control_month_text_borrow(const win16_control_month_view *,HTHEME,HDC,int,int,DWORD,DWORD,const RECT *,const void *);
BOOL win16_control_month_text_room(const win16_control_month_view *,HTHEME,HDC,int,int,DWORD,DWORD,const RECT *,int,const void *);
void win16_control_month_header_text(const win16_control_month_view *,HTHEME,HDC,int,int,const RECT *,DWORD,const void *);
void win16_control_month_selection_text(const win16_control_month_view *,HTHEME,HDC,int,int,const RECT *,DWORD,const void *);
#endif
