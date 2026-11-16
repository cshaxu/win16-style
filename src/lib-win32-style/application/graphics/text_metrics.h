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

#endif
