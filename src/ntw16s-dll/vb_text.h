#ifndef NTW16S_VB_TEXT_H
#define NTW16S_VB_TEXT_H
#include <windows.h>
typedef struct ntw16s_vb_fonts {
    HFONT source[32],font[32];
    struct ntw16s_vb_glyphmap *glyphs[32];
    unsigned count;
} ntw16s_vb_fonts;
typedef struct ntw16s_vb_paint {
    struct ntw16s_vb_paint *previous;
    HWND window;
    ntw16s_vb_fonts *fonts;
    BOOL active,report;
    HDC report_dc;
    HMODULE report_module;
    const BOOL *cancelled;
} ntw16s_vb_paint;
void ntw16s_vb_paint_begin(ntw16s_vb_paint *scope,HWND window,UINT message,ntw16s_vb_fonts *fonts);
void ntw16s_report_paint_enable(ntw16s_vb_paint *,HWND,HMODULE,const BOOL *);
void ntw16s_report_paint_bind(HWND,HDC);
BOOL ntw16s_report_paint_active(void);
BOOL ntw16s_report_paint_caller(HMODULE);
void ntw16s_vb_fonts_clear(ntw16s_vb_fonts *fonts);
void ntw16s_vb_paint_end(ntw16s_vb_paint *scope);
/* Borrowed until the attachment retires after its last active call; NULL means leave the native DC unchanged. */
BOOL ntw16s_vb_paint_active(void);
HFONT ntw16s_vb_paint_font(HDC dc);
BOOL ntw16s_vb_decode_glyphs(HDC dc,const WCHAR *input,UINT count,WCHAR *decoded);
#endif
