#ifndef STYLE_MENU_ART_H
#define STYLE_MENU_ART_H

#include <windows.h>

typedef struct style_menu_art {
    CHAR check[14][15], arrow[11][8];
    BOOL available;
} style_menu_art;

static inline int style_menu_arrow_extent(UINT dpi)
{
    return MulDiv(11,(int)dpi,96);
}

void style_menu_draw_mark(HDC dc,const RECT *row,BOOL submenu,UINT dpi,COLORREF color,const CHAR *mask);
void style_menu_draw_command(HDC dc,const RECT *row,HBITMAP bitmap,int unit,COLORREF color);
void style_menu_popup_marks(HDC,const RECT *,UINT,COLORREF,BOOL checked,BOOL submenu,const style_menu_art *);

#endif
