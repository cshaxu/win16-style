#include "v4_font.h"

HFONT msstyles_font_create(const msstyles_font *defined,UINT dpi,enum msstyles_font_role role)
{
    if(defined && defined->points){
        LOGFONTW logical=msstyles_font_at_dpi(defined,dpi);
        return CreateFontIndirectW(&logical);
    }
    NONCLIENTMETRICSW metrics={.cbSize=sizeof(metrics)};
    if(!SystemParametersInfoForDpi(SPI_GETNONCLIENTMETRICS,sizeof(metrics),&metrics,0,dpi))return NULL;
    switch(role){
    case MSSTYLES_FONT_CAPTION:return CreateFontIndirectW(&metrics.lfCaptionFont);
    case MSSTYLES_FONT_SMALL_CAPTION:return CreateFontIndirectW(&metrics.lfSmCaptionFont);
    case MSSTYLES_FONT_MENU:return CreateFontIndirectW(&metrics.lfMenuFont);
    }
    return NULL;
}
