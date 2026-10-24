#ifndef WIN16_STYLE_MENU_PAINT_H
#define WIN16_STYLE_MENU_PAINT_H
#include "api.h"
#include "win16_style.h"
typedef enum win16_style_menu_kind {
    WIN16_STYLE_MENU_ROW,
    WIN16_STYLE_MENU_SEPARATOR,
    WIN16_STYLE_MENU_BAR
} win16_style_menu_kind;
/* Presentation only. The caller retains HMENU, command and tracking ownership.
 * No item data, font or DC is retained. Bitmap/owner-drawn application items
 * require separate support before general menu integration. */
NTW16S_API BOOL win16_style_menu_text(HDC dc,const RECT *bounds,HFONT font,UINT dpi,
    const win16_style_palette *palette,const WCHAR *text,UINT state,win16_style_menu_kind kind,BOOL submenu);
NTW16S_API BOOL win16_style_menu_measure(HDC dc,HFONT font,UINT dpi,const WCHAR *text,
    win16_style_menu_kind kind,SIZE *size);
/* Private shared popup column geometry; measurement and painting use the
 * same maxima across all adapted string rows of a native popup. */
typedef struct win16_style_menu_columns {
    int label,shortcut,height;
} win16_style_menu_columns;
BOOL win16_style_menu_columns_read(HDC dc,HFONT font,UINT dpi,const WCHAR *text,
    win16_style_menu_columns *columns);
BOOL win16_style_menu_text_columns(HDC dc,const RECT *bounds,HFONT font,UINT dpi,
    const win16_style_palette *palette,const WCHAR *text,UINT state,
    win16_style_menu_kind kind,BOOL submenu,const win16_style_menu_columns *columns);
int win16_style_menu_columns_width(const win16_style_menu_columns *columns,UINT dpi,BOOL arrows);
#endif
