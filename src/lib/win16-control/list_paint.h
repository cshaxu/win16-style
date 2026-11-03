#ifndef WIN16_CONTROL_LIST_PAINT_H
#define WIN16_CONTROL_LIST_PAINT_H
#include "../win16-common/palette.h"
void win16_control_draw_list(HWND window,const BOOL *detached,const win16_style_palette *palette,
    HFONT *font,const int *tab_count,int *const *tab_positions,HDC dc,BOOL (*is_vb_list)(HWND));
void win16_control_edit_border(HWND window,BOOL enabled,const win16_style_palette *palette,HDC dc);
void win16_control_list_border(HWND window,const win16_style_palette *palette,HDC dc);
typedef struct win16_control_tabstops_view {
    BOOL *detached;
    HFONT *font;
    int *count,**positions;
    LRESULT (*forward)(HWND,WPARAM,LPARAM);
} win16_control_tabstops_view;
LRESULT win16_control_set_list_tabs(HWND window,const win16_control_tabstops_view *view,WPARAM count,LPARAM values);
#endif
