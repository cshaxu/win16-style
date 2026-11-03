#ifndef WIN16_CONTROL_CHOICE_VIEW_H
#define WIN16_CONTROL_CHOICE_VIEW_H
#include "../win16-common/palette.h"
typedef struct win16_control_choice_view {
    const BOOL *detached;
    const win16_style_palette *palette;
    HFONT *font;
    UINT (*button_type)(HWND,DWORD);
    BOOL (*is_static)(HWND);
    BOOL (*vb_frame)(HWND),(*vb_button)(HWND);
    BOOL (*group_choice)(HWND),(*group_push)(HWND);
} win16_control_choice_view;
void win16_control_draw_choice(HWND window,const win16_control_choice_view *view,HDC dc);
#endif
