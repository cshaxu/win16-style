#include "palette.h"

LRESULT win16_control_dialog_colors(HDC dc,const win16_style_palette *p)
{
    SetBkColor(dc,p->light);
    SetTextColor(dc,p->edge);
    SetDCBrushColor(dc,p->light);
    return (LRESULT)GetStockObject(DC_BRUSH);
}
void win16_style_default_palette(win16_style_palette *p)
{
    if(!p)return;
    /* Owner reference preset, not a claim about universal Win3.1 defaults. */
    p->active=RGB(164,200,240);p->inactive=RGB(255,255,255);
    p->active_text=p->inactive_text=RGB(0,0,0);
    p->face=RGB(192,192,192);p->light=RGB(255,255,255);
    p->shadow=RGB(128,128,128);p->edge=RGB(0,0,0);
}


void win16_control_resolve_palette(const win16_style_palette *source,win16_style_palette *p)
{
    HIGHCONTRASTW hc={.cbSize=sizeof(hc)};
    *p=*source;
    if(SystemParametersInfoW(SPI_GETHIGHCONTRAST,sizeof(hc),&hc,0) &&
        (hc.dwFlags&HCF_HIGHCONTRASTON)){
        p->active=GetSysColor(COLOR_ACTIVECAPTION);
        p->inactive=GetSysColor(COLOR_INACTIVECAPTION);
        p->active_text=GetSysColor(COLOR_CAPTIONTEXT);
        p->inactive_text=GetSysColor(COLOR_INACTIVECAPTIONTEXT);
        p->face=GetSysColor(COLOR_BTNFACE);p->light=GetSysColor(COLOR_BTNHIGHLIGHT);
        p->shadow=GetSysColor(COLOR_BTNSHADOW);p->edge=GetSysColor(COLOR_WINDOWTEXT);
    }
}
