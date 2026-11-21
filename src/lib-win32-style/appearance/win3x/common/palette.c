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
    *p=(win16_style_palette){
        GetSysColor(COLOR_ACTIVECAPTION),GetSysColor(COLOR_INACTIVECAPTION),
        GetSysColor(COLOR_CAPTIONTEXT),GetSysColor(COLOR_INACTIVECAPTIONTEXT),
        GetSysColor(COLOR_BTNFACE),GetSysColor(COLOR_BTNHIGHLIGHT),
        GetSysColor(COLOR_BTNSHADOW),GetSysColor(COLOR_WINDOWTEXT)};
}


void win16_style_theme_palette(win16_style_palette *p,const classic_theme *theme)
{
    p->active=theme->active.start;
    p->inactive=theme->inactive.start;
    p->active_text=theme->active.text;
    p->inactive_text=theme->inactive.text;
    if(theme->frame.present){
        p->face=theme->frame.face;
        p->light=theme->frame.light;
        p->shadow=theme->frame.shadow;
        p->edge=theme->frame.edge;
    }
}

BOOL win16_control_high_contrast(void)
{
    HIGHCONTRASTW hc={.cbSize=sizeof(hc)};
    return SystemParametersInfoW(SPI_GETHIGHCONTRAST,sizeof(hc),&hc,0) &&
        (hc.dwFlags&HCF_HIGHCONTRASTON);
}

void win16_control_resolve_palette(const win16_style_palette *source,win16_style_palette *p)
{
    if(win16_control_high_contrast())win16_style_default_palette(p);
    else *p=*source;
}
