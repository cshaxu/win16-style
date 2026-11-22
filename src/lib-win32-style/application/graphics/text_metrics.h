#ifndef STYLE_COMMON_TEXT_METRICS_H
#define STYLE_COMMON_TEXT_METRICS_H
#include <windows.h>

/* USER's average alphabet-cell width for proportional fonts. */
static inline BOOL style_text_alphabet_average(HDC dc,int *average)
{
    SIZE size;
    if(!GetTextExtentPoint32W(dc,L"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ",52,&size))return FALSE;
    *average=((size.cx/26)+1)/2;return TRUE;
}

/* Synchronous shrink-only rasterization. Caller borrows DC/font/text and
 * chooses the maximum source extent; drawn changes only after successful
 * publication. FALSE leaves fallback to caller, including already-fitting
 * text and native allocation/drawing failure. Normal-return cleanup only. */
BOOL style_text_fit(HDC,HFONT,const WCHAR *,const RECT *,COLORREF,COLORREF,SIZE limit,SIZE *drawn);

BOOL style_text_ascii(LPCWSTR,int);
/* Eligibility for the caller's single-byte native report protocol. */
BOOL style_text_single_byte_wide(HDC,LPCWSTR,int,BOOL active,UINT code_page);
BOOL style_text_single_byte_ansi(HDC,LPCSTR,int,BOOL active,UINT code_page);
BOOL style_text_glyph_run_allowed(HDC,UINT options,UINT count);
/* Borrow the font only for a paint from the observed native STATIC. */
HFONT style_text_static_source(HDC,LPCWSTR,int,UINT,const void *,HWND,HMODULE);
#endif
