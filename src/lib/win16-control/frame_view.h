#ifndef WIN16_CONTROL_FRAME_VIEW_H
#define WIN16_CONTROL_FRAME_VIEW_H
#include "palette.h"
#include "layout.h"
typedef struct win16_control_frame_view {
    void *context;
    const BOOL *detached,*active;
    HFONT *menu_font,*title_font;
    const UINT *dpi;
    const int *bar_selected;
    const BOOL *system_menu,*bar_keyboard;
    const int *pressed;
    const BOOL *hot;
    BOOL (*layout)(HWND,void *,win16_style_layout *);
    void (*colors)(void *,win16_style_palette *);
    void (*scrollbars)(HWND,HDC,const win16_style_palette *);
} win16_control_frame_view;
void win16_control_paint_frame(HWND window,const win16_control_frame_view *view,HDC target);
void win16_control_frame_buffered(HWND window,const win16_control_frame_view *view,HDC target);
void win16_control_frame_menu_state(HWND window,const win16_control_frame_view *view);
void win16_control_frame_button_state(HWND window,const win16_control_frame_view *view,int hit);
#endif
