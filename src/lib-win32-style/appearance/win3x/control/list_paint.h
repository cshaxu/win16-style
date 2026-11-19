#ifndef WIN16_CONTROL_LIST_PAINT_H
#define WIN16_CONTROL_LIST_PAINT_H
#include "../common/palette.h"
#include "lib-win32-style/application/control/control_state.h"
struct control_style;
void win16_control_draw_list(HWND window,struct control_style *style,HDC dc);
void win16_control_edit_border(HWND window,const struct control_style *style,HDC dc);
void win16_control_list_border(HWND window,const struct control_style *style,HDC dc);
#endif
