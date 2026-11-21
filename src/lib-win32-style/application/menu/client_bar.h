#ifndef STYLE_MENU_CLIENT_BAR_H
#define STYLE_MENU_CLIENT_BAR_H
#include "row_paint.h"
#include "lib-win32-style/application/menu/classic_menu_layout.h"
#include "lib-win32-style/application/window/full_client_layout.h"
#include "lib-win32-style/application/menu/owner.h"
#include "lib-win32-style/application/menu/bar_snapshot.h"
#include "../window/caption_paint.h"
/* Bind the measured client-area menu and MDI cells to one snapshot. */
BOOL style_menu_client_bar_read(HWND,HMENU,HFONT,UINT,const style_full_client_layout *,POINT,int,style_menu_bar_snapshot *);
void style_menu_client_bar_paint(const style_menu_bar_snapshot *bar,HDC dc,HFONT font,UINT dpi,
    const style_window_palette *palette,const classic_menu_colors *colors,const style_menu_metrics *metrics,
    const classic_caption_art *art,const style_caption_presentation *presentation);
/* The native command bitmap identifiers also encode disabled variants. */
unsigned style_menu_mdi_role(HBITMAP bitmap);
/* Internal shared original-resource painter for MDI command cells. */
void style_menu_mdi_paint(HDC dc,RECT bounds,unsigned role,UINT flags,int unit,
    const style_window_palette *palette,const classic_caption_art *art,const style_caption_presentation *presentation);
/* Whether native menu navigation has reached the edge of the active popup. */
int style_menu_client_hit_screen(HWND window,const style_full_client_layout *layout,
    HFONT font,UINT dpi,POINT point);
int style_menu_client_item_screen(HWND window,const style_full_client_layout *layout,
    HFONT font,UINT dpi,LPARAM point,BOOL *system_close);
#endif
