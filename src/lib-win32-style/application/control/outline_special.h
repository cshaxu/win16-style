#ifndef STYLE_CONTROL_OUTLINE_SPECIAL_H
#define STYLE_CONTROL_OUTLINE_SPECIAL_H
#include "lib-win32-style/application/window/palette.h"
#include "control_chrome.h"
#include <uxtheme.h>
#include <commctrl.h>
void style_outline_control_draw_sizebox(HWND window,HDC dc,const COLORREF *face);
void style_outline_control_legacy_progress_border(HWND window,const style_window_palette *palette,HDC dc);
void style_outline_control_vb_data_arrow_candidate(HDC dc,int x,int y,int width,int height,HDC source,int sx,int sy,int sw,int sh,DWORD rop,const void *caller,BOOL (*attached_palette)(HWND,style_window_palette *,style_scroll_arrow_art *));
BOOL style_outline_control_graphical_frame_prepare(HDC dc,const RECT *rect,UINT type,UINT flags,const void *address,RECT *copy,SUBCLASSPROC proc);
BOOL style_outline_control_graphical_frame(HDC dc,const RECT *rect,UINT type,UINT flags,const void *address,SUBCLASSPROC proc);
BOOL style_outline_control_cool_grip_identity(HWND,void *,const NMCUSTOMDRAW *,WNDPROC,BOOL (*)(HWND,void *));
BOOL style_outline_control_cool_grip(HDC dc,const RECT *bounds,const classic_bar_colors *);
BOOL style_outline_control_color_frame(HDC dc,const RECT *bounds,const RECT *clip,COLORREF color);
BOOL style_outline_control_color_swatch_frame(HDC dc,RECT *bounds,const RECT *original,COLORREF color);
void style_outline_control_group_frame(HDC,const RECT *,const RECT *,const RECT *,COLORREF,void (*)(void *),void *);
void style_outline_control_panel_shadow(HDC,const POINT *,BOOL (WINAPI *)(HDC,const POINT *,int),COLORREF,
    void (*)(void *),void *);
#endif
