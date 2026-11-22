#ifndef STYLE_CONTROL_OUTLINE_LIST_H
#define STYLE_CONTROL_OUTLINE_LIST_H
#include "../window/palette.h"
#include "../graphics/paint_frame.h"
#include "control_colors.h"

void style_outline_edit_border(HWND,HDC,const style_window_palette *,const style_frame_colors *,const classic_edit_border *,style_line_profile);
void style_outline_list_border(HWND,HDC,const style_window_palette *,const style_frame_colors *);
#endif
