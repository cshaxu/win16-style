#ifndef WIN16_CONTROL_LIST_PAINT_H
#define WIN16_CONTROL_LIST_PAINT_H
#include "../common/palette.h"
#include "lib-win32-style/application/control/control_state.h"
void win16_control_draw_list(HWND window,style_instance *instance,const style_control_state *state,const win16_style_palette *palette,HDC dc);
void win16_control_edit_border(HWND window,BOOL enabled,const win16_style_palette *palette,HDC dc);
void win16_control_list_border(HWND window,const win16_style_palette *palette,HDC dc);
#endif
