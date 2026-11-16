#ifndef WIN16_CONTROL_FRAME_FONT_H
#define WIN16_CONTROL_FRAME_FONT_H
#include "lib-win32-style/application/graphics/font_pair.h"
BOOL win16_control_refresh_frame_fonts(HWND,style_font_pair *);
void win16_control_release_frame_fonts(const style_font_pair *);
#endif
