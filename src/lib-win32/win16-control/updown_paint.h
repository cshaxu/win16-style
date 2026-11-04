#ifndef WIN16_CONTROL_UPDOWN_PAINT_H
#define WIN16_CONTROL_UPDOWN_PAINT_H
#include "../win16-common/palette.h"
typedef const char ntw16s_updown_glyph[17][18];
BOOL win16_control_updown_paint(HWND window,const win16_style_palette *palette,
    ntw16s_updown_glyph *const *glyphs,HDC dc,const RECT *bounds,const RECT *clip,
    unsigned direction,BOOL pressed,BOOL disabled);
#endif
