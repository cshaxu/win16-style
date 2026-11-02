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
void win16_control_frame_ncpaint(HWND window,WPARAM region,void *context,
    void (*paint)(HWND,void *,HDC),void (*buffered)(HWND,void *,HDC));
typedef struct win16_control_frame_button_view {
    void *context;
    int *pressed;
    BOOL *hot,*detached;
    BOOL (*layout)(HWND,void *,win16_style_layout *);
    void (*paint)(HWND,void *,int);
} win16_control_frame_button_view;
void win16_control_frame_button_press(HWND window,const win16_control_frame_button_view *view,int hit);
BOOL win16_control_frame_button_motion(HWND window,const win16_control_frame_button_view *view,UINT message,LPARAM point);
BOOL win16_control_frame_button_cancel(HWND window,const win16_control_frame_button_view *view,UINT message);
#endif
