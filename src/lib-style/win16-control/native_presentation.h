#ifndef WIN16_CONTROL_NATIVE_PRESENTATION_H
#define WIN16_CONTROL_NATIVE_PRESENTATION_H
#include "control_state.h"
typedef void (*win16_control_scroll_painter)(HWND,control_style *,HDC,LONG,BOOL);
void win16_control_nonclient_scrollbars(HWND,control_style *,HDC,win16_control_scroll_painter);
void win16_control_draw_tab(HWND,control_style *,HDC);
void win16_control_draw_header(HWND,control_style *,HDC);
void win16_control_draw_progress(HWND,control_style *,HDC);
#endif
