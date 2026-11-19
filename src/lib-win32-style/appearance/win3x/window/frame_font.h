#ifndef WIN16_CONTROL_FRAME_FONT_H
#define WIN16_CONTROL_FRAME_FONT_H
#include "lib-win32-style/application/graphics/font_pair.h"
#include "lib-win32-style/appearance/msstyles/v4_font.h"
BOOL win16_control_refresh_frame_fonts(HWND,style_font_pair *,const msstyles_font *,const msstyles_font *);
void win16_control_release_frame_fonts(const style_font_pair *);
#endif
