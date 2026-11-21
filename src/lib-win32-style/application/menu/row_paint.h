#ifndef STYLE_MENU_ROW_PAINT_H
#define STYLE_MENU_ROW_PAINT_H
#include "menu_art.h"
#include "row_text.h"
#include "../window/palette.h"

typedef enum style_menu_kind {
    STYLE_MENU_ROW, STYLE_MENU_SEPARATOR, STYLE_MENU_BAR
} style_menu_kind;
typedef struct style_menu_columns {
    int label,shortcut,height;
} style_menu_columns;

/* Uses the same column geometry to measure and paint a row. The caller
 * retains the menu and any fonts and DCs. */
BOOL style_menu_columns_read(HDC,HFONT,UINT,const WCHAR *,style_menu_columns *);
int style_menu_columns_width(const style_menu_columns *,UINT,BOOL,const style_menu_metrics *);
BOOL style_menu_bar_measure(HDC,HFONT,UINT,const WCHAR *,const style_menu_metrics *,SIZE *);
BOOL style_menu_paint_row(HDC,const RECT *,HFONT,UINT,const style_window_palette *,const WCHAR *,UINT,
    style_menu_kind,BOOL,const style_menu_columns *,const classic_menu_colors *,const style_menu_metrics *,
    const style_menu_art *,HBITMAP);
#endif
