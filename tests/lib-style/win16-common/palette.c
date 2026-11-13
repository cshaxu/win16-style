#include "lib-style/win16-common/palette.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x) do { if(!(x)){printf("WIN16_PALETTE_FAIL line=%d\n",__LINE__);return 1;} } while(0)
int main(void)
{
    win16_style_palette source,actual,expected;
    HIGHCONTRASTW hc={.cbSize=sizeof(hc)};
    HDC dc;
    win16_style_default_palette(NULL);
    win16_style_default_palette(&source);
    CHECK(source.active==RGB(164,200,240) && source.inactive==RGB(255,255,255));
    CHECK(source.active_text==RGB(0,0,0) && source.inactive_text==RGB(0,0,0));
    CHECK(source.face==RGB(192,192,192) && source.light==RGB(255,255,255));
    CHECK(source.shadow==RGB(128,128,128) && source.edge==RGB(0,0,0));
    expected=source;
    BOOL high=SystemParametersInfoW(SPI_GETHIGHCONTRAST,sizeof(hc),&hc,0) &&
        (hc.dwFlags&HCF_HIGHCONTRASTON);
    if(high){
        expected.active=GetSysColor(COLOR_ACTIVECAPTION);
        expected.inactive=GetSysColor(COLOR_INACTIVECAPTION);
        expected.active_text=GetSysColor(COLOR_CAPTIONTEXT);
        expected.inactive_text=GetSysColor(COLOR_INACTIVECAPTIONTEXT);
        expected.face=GetSysColor(COLOR_BTNFACE);
        expected.light=GetSysColor(COLOR_BTNHIGHLIGHT);
        expected.shadow=GetSysColor(COLOR_BTNSHADOW);
        expected.edge=GetSysColor(COLOR_WINDOWTEXT);
    }
    win16_control_resolve_palette(&source,&actual);
    CHECK(memcmp(&actual,&expected,sizeof(actual))==0);
    dc=CreateCompatibleDC(NULL);CHECK(dc);
    CHECK((HBRUSH)win16_control_dialog_colors(dc,&source)==(HBRUSH)GetStockObject(DC_BRUSH));
    CHECK(GetBkColor(dc)==source.light && GetTextColor(dc)==source.edge);
    CHECK(GetDCBrushColor(dc)==source.light);
    DeleteDC(dc);
    printf("WIN16_PALETTE_OK high_contrast=%d\n",high);return 0;
}
