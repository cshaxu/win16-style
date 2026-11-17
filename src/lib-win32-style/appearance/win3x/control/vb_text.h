#ifndef NTW16S_VB_TEXT_H
#define NTW16S_VB_TEXT_H
#include <windows.h>
#include "lib-win32-style/application/adapters/win32/runtime_text.h"
typedef struct ntw16s_vb_fonts {
    HFONT source[32],font[32];
    struct ntw16s_vb_glyphmap *glyphs[32];
    unsigned count;
} ntw16s_vb_fonts;
/* Native/report scope belongs to the public runtime observer. This provider
 * only selects Win3x font eligibility and owns its font/glyph leases. */
void win16_control_vb_paint_begin(style_runtime_text_scope *,HWND,UINT,ntw16s_vb_fonts *,BOOL (*)(HWND));
void ntw16s_vb_fonts_clear(ntw16s_vb_fonts *);
HFONT ntw16s_vb_paint_font(HDC dc);
BOOL ntw16s_vb_decode_glyphs(HDC dc,const WCHAR *input,UINT count,WCHAR *decoded);
#endif
