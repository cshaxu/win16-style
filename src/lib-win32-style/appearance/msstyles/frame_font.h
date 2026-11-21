#ifndef STYLE_FRAME_FONT_H
#define STYLE_FRAME_FONT_H
#include "lib-win32-style/application/graphics/font_pair.h"
#include "lib-win32-style/appearance/msstyles/v4_font.h"
BOOL style_frame_fonts_refresh(HWND,style_font_pair *,const msstyles_font *,const msstyles_font *);
HFONT style_frame_menu_font_acquire(UINT dpi,const msstyles_font *defined);
void style_frame_fonts_release(const style_font_pair *);
#endif
