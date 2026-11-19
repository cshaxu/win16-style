#ifndef WIN16_STYLE_MENU_BAR_H
#define WIN16_STYLE_MENU_BAR_H
#include "menu_paint.h"
#include "../window/caption_art.h"
#include "lib-win32-style/application/menu/classic_menu_layout.h"
#include "../common/layout.h"
#include "lib-win32-style/application/menu/owner.h"
#include "lib-win32-style/application/menu/bar_snapshot.h"
/* Bind era measurement and MDI presentation to the common snapshot reader. */
BOOL win16_style_bar_read_for_window(HWND,HMENU,HFONT,UINT,POINT,int,style_menu_bar_snapshot *);
void win16_style_bar_paint(const style_menu_bar_snapshot *bar,HDC dc,HFONT font,UINT dpi,
    const win16_style_palette *palette,const classic_menu_colors *colors,const win16_caption_art *art);
/* Internal shared original-resource painter for MDI's system/restore cells. */
void win16_style_merged_control(HDC dc,RECT bounds,BOOL system,BOOL selected,int unit,
    const win16_style_palette *palette,const win16_caption_art *art);
/* Whether native menu navigation has reached the edge of the active popup. */
int win16_control_menu_bar_hit_screen(HWND window,const win16_style_layout *layout,
    HFONT font,UINT dpi,POINT point);
int win16_control_menu_bar_item_screen(HWND window,const win16_style_layout *layout,
    HFONT font,UINT dpi,LPARAM point,BOOL *system_close);
#endif
