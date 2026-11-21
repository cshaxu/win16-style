#ifndef STYLE_COMMON_MENU_METRICS_H
#define STYLE_COMMON_MENU_METRICS_H
#include "../graphics/text_metrics.h"

static inline int style_menu_native_mark_width(UINT dpi)
{return GetSystemMetricsForDpi(SM_CXMENUCHECK,dpi)+GetSystemMetricsForDpi(SM_CXEDGE,dpi);}

/* The observed layout keeps the native rectangles. Use this when measuring owned rows. */
static inline int style_menu_row_height(UINT dpi,int text_height,unsigned minimum,unsigned padding)
{
    if(!minimum)return max(text_height,GetSystemMetricsForDpi(SM_CYMENU,dpi));
    return max(text_height,MulDiv((int)minimum,(int)dpi,96))+2*MulDiv((int)padding,(int)dpi,96);
}

typedef struct style_menu_popup_columns {int check,gap,arrow,plain;} style_menu_popup_columns;
static inline style_menu_popup_columns style_menu_popup_spacing(UINT dpi,unsigned check,
    unsigned gap,unsigned arrow,unsigned plain)
{
    if(!check){
        int mark=style_menu_native_mark_width(dpi);
        return (style_menu_popup_columns){mark,0,mark,mark};
    }
    return (style_menu_popup_columns){MulDiv((int)check,(int)dpi,96),MulDiv((int)gap,(int)dpi,96),
        MulDiv((int)arrow,(int)dpi,96),MulDiv((int)plain,(int)dpi,96)};
}

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
