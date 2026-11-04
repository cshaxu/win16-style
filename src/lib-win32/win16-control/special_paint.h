#ifndef WIN16_CONTROL_SPECIAL_PAINT_H
#define WIN16_CONTROL_SPECIAL_PAINT_H
#include "../win16-common/palette.h"
#include <uxtheme.h>
#include <commctrl.h>
void win16_style_month_arrow(HDC dc,const RECT *bounds,const RECT *clip,int part,int state);
void win16_control_draw_sizebox(HWND window,HDC dc);
void win16_control_legacy_progress_border(HWND window,const win16_style_palette *palette,HDC dc);
void win16_control_vb_data_arrow(HWND window,HDC dc,int x,int y,int width,HDC source,const RECT *client_bounds);
void win16_control_vb_data_arrow_candidate(HDC dc,int x,int y,int width,int height,HDC source,int sx,int sy,int sw,int sh,DWORD rop,const void *caller,BOOL (*attached)(HWND));
BOOL win16_style_bitbtn_face(HWND window,HDC dc,const RECT *bounds,const RECT *clip,UINT state);
BOOL win16_control_checklist_frame(HDC dc,const RECT *rect,UINT type,UINT flags,const void *address,BOOL (*target)(HDC,const RECT *,const void *));
BOOL win16_control_graphical_frame_prepare(HDC dc,const RECT *rect,UINT type,UINT flags,const void *address,RECT *copy,SUBCLASSPROC proc);
BOOL win16_control_graphical_frame(HDC dc,const RECT *rect,UINT type,UINT flags,const void *address,SUBCLASSPROC proc);
BOOL win16_control_graphical_face(HWND window,HDC dc,const RECT *bounds,UINT flags,const win16_style_palette *palette);
BOOL win16_control_cool_grip(HDC dc,const RECT *bounds);
BOOL win16_control_black_frame(HDC dc,const RECT *bounds,const RECT *clip);
BOOL win16_control_color_swatch_frame(HDC dc,RECT *bounds,const RECT *original);
void win16_control_month_fill(HDC dc,const RECT *bounds,const RECT *clip);
BOOL win16_control_month_text(HTHEME theme,HDC dc,int part,int state,HFONT font,LPCWSTR text,int count,DWORD flags,const RECT *bounds,
    int (WINAPI *draw)(HDC,LPCWSTR,int,LPRECT,UINT),BOOL (*room)(void *,int),void *context);
void win16_control_group_frame(HDC dc,const RECT *bounds,const RECT *clip,const RECT *exclude,void (*drawn)(void *),void *context);
void win16_control_panel_shadow(HDC dc,const POINT points[3],BOOL (WINAPI *draw)(HDC,const POINT *,int),void (*drawn)(void *),void *context);
BOOL win16_control_spin_snapshot(HDC dc,int width,int height,const POINT pt[2],DWORD *background,BYTE *valid);
BOOL win16_control_spin_compose(HWND window,HDC dest,int x,int y,int width,int height,HDC source,
    int source_width,int source_height,const RECT halves[2],const int states[2],const DWORD *background,DWORD *original);
#endif
