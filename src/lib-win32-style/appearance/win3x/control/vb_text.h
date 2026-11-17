#ifndef WIN16_CONTROL_VB_TEXT_H
#define WIN16_CONTROL_VB_TEXT_H
#include "lib-win32-style/application/adapters/win32/runtime_text.h"
#include "lib-win32-style/application/graphics/font_cache.h"
/* Profile scope/font eligibility and CP1252 rendering policy. The common
 * cache owns replacement leases and glyph maps, not the renderer. */
void win16_control_vb_paint_begin(style_runtime_text_scope *,HWND,UINT,style_font_cache *,BOOL (*)(HWND));
HFONT win16_control_vb_paint_font(HDC);
BOOL win16_control_vb_decode_glyphs(HDC,const WCHAR *,UINT,WCHAR *);
#endif
