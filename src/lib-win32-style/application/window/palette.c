#include "palette.h"

LRESULT style_window_dialog_colors(HDC dc,const style_window_palette *p)
{
    SetBkColor(dc,p->light);
    SetTextColor(dc,p->edge);
    SetDCBrushColor(dc,p->light);
    return (LRESULT)GetStockObject(DC_BRUSH);
}
void style_window_palette_default(style_window_palette *p)
{
    if(!p)return;
    *p=(style_window_palette){
        GetSysColor(COLOR_ACTIVECAPTION),GetSysColor(COLOR_INACTIVECAPTION),
        GetSysColor(COLOR_CAPTIONTEXT),GetSysColor(COLOR_INACTIVECAPTIONTEXT),
        GetSysColor(COLOR_BTNFACE),GetSysColor(COLOR_BTNHIGHLIGHT),
        GetSysColor(COLOR_BTNSHADOW),GetSysColor(COLOR_WINDOWTEXT)};
}


BOOL style_window_high_contrast(void)
{
    HIGHCONTRASTW hc={.cbSize=sizeof(hc)};
    return SystemParametersInfoW(SPI_GETHIGHCONTRAST,sizeof(hc),&hc,0) &&
        (hc.dwFlags&HCF_HIGHCONTRASTON);
}

void style_window_palette_resolve(const style_window_palette *source,style_window_palette *p)
{
    if(style_window_high_contrast())style_window_palette_default(p);
    else *p=*source;
}
