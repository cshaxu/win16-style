#ifndef STYLE_COMMON_MENU_METRICS_H
#define STYLE_COMMON_MENU_METRICS_H
#include "text_metrics.h"

/* USER adds two average native-menu font cells to owner-draw widths. */
static inline int style_menu_native_text_margin(HDC dc,UINT dpi)
{
    NONCLIENTMETRICSW ncm={.cbSize=sizeof(ncm)};HFONT font;HGDIOBJ old;int average,result=0;
    if(!SystemParametersInfoForDpi(SPI_GETNONCLIENTMETRICS,sizeof(ncm),&ncm,0,dpi))return 0;
    font=CreateFontIndirectW(&ncm.lfMenuFont);if(!font)return 0;
    old=SelectObject(dc,font);
    if(old && style_text_alphabet_average(dc,&average))result=2*average;
    if(old)SelectObject(dc,old);
    DeleteObject(font);return result;
}
#endif
