#ifndef WIN16_CONTROL_UPDOWN_PAINT_H
#define WIN16_CONTROL_UPDOWN_PAINT_H
#include "../common/palette.h"
#include "lib-win32-style/application/control/updown.h"
typedef const char ntw16s_updown_glyph[17][18];
BOOL win16_control_updown_paint(HWND window,const win16_style_palette *palette,
    ntw16s_updown_glyph *const *glyphs,HDC dc,const RECT *bounds,const RECT *clip,
    unsigned direction,BOOL pressed,BOOL disabled);
typedef struct win16_control_updown {
    const win16_style_palette *palette;ntw16s_updown_glyph *const *glyphs;
} win16_control_updown;
void win16_control_updown_register(win16_control_updown *,style_control_updown *,HWND,HMODULE,const win16_style_palette *,ntw16s_updown_glyph *const *,void *,BOOL (*)(void *),void (*)(void *));
#endif
