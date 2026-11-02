#ifndef WIN16_CONTROL_SPECIAL_PAINT_H
#define WIN16_CONTROL_SPECIAL_PAINT_H
#include "palette.h"
void win16_style_month_arrow(HDC dc,const RECT *bounds,const RECT *clip,int part,int state);
void win16_control_draw_sizebox(HWND window,HDC dc);
void win16_control_legacy_progress_border(HWND window,const win16_style_palette *palette,HDC dc);
void win16_control_vb_data_arrow(HWND window,HDC dc,int x,int y,int width,HDC source,const RECT *client_bounds);
BOOL win16_style_bitbtn_face(HWND window,HDC dc,const RECT *bounds,const RECT *clip,UINT state);
BOOL win16_control_graphical_face(HWND window,HDC dc,const RECT *bounds,UINT flags,const win16_style_palette *palette);
BOOL win16_control_cool_grip(HDC dc,const RECT *bounds);
BOOL win16_control_black_frame(HDC dc,const RECT *bounds,const RECT *clip);
void win16_control_month_fill(HDC dc,const RECT *bounds,const RECT *clip);
void win16_control_group_frame(HDC dc,const RECT *bounds,const RECT *clip,const RECT *exclude,void (*drawn)(void *),void *context);
void win16_control_panel_shadow(HDC dc,const POINT points[3],BOOL (WINAPI *draw)(HDC,const POINT *,int),void (*drawn)(void *),void *context);
BOOL win16_control_spin_snapshot(HDC dc,int width,int height,const POINT pt[2],DWORD *background,BYTE *valid);
BOOL win16_control_spin_compose(HWND window,HDC dest,int x,int y,int width,int height,HDC source,
    int source_width,int source_height,const RECT halves[2],const int states[2],const DWORD *background,DWORD *original);
#endif
