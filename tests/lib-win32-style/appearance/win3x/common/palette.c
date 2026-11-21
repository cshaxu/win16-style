#include "lib-win32-style/appearance/win3x/common/palette.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x) do { if(!(x)){printf("WIN16_PALETTE_FAIL line=%d\n",__LINE__);return 1;} } while(0)
int main(int argc,char **argv)
{
    win16_style_palette source,actual,expected;
    HIGHCONTRASTW hc={.cbSize=sizeof(hc)};
    HDC dc;
    style_window_palette_default(NULL);
    style_window_palette_default(&source);
    const COLORREF native[]={
        GetSysColor(COLOR_ACTIVECAPTION),GetSysColor(COLOR_INACTIVECAPTION),
        GetSysColor(COLOR_CAPTIONTEXT),GetSysColor(COLOR_INACTIVECAPTIONTEXT),
        GetSysColor(COLOR_BTNFACE),GetSysColor(COLOR_BTNHIGHLIGHT),
        GetSysColor(COLOR_BTNSHADOW),GetSysColor(COLOR_WINDOWTEXT)};
    CHECK(sizeof(source)==sizeof(native) && memcmp(&source,native,sizeof(native))==0);
    if(argc==2){
        HMODULE dll=LoadLibraryA(argv[1]);
        typedef void (__cdecl *palette_fn)(style_window_palette *);
        palette_fn read=dll?(void *)GetProcAddress(dll,"win16_style_default_palette"):NULL;
        CHECK(read);
        read(&actual);
        CHECK(memcmp(&actual,&source,sizeof(source))==0 && FreeLibrary(dll));
        puts("WINDOW_PALETTE_ABI_PASS");
    }else CHECK(argc==1);
    const classic_theme configured={
        .active={RGB(40,50,60),RGB(40,50,60),RGB(4,5,6)},
        .inactive={RGB(70,80,90),RGB(70,80,90),RGB(7,8,9)},
        .frame={RGB(100,110,120),RGB(130,140,150),RGB(160,170,180),RGB(190,200,210),TRUE}};
    classic_theme_window_palette(&source,&configured);
    CHECK(source.active==configured.active.start && source.inactive==configured.inactive.start &&
          source.active_text==configured.active.text && source.inactive_text==configured.inactive.text &&
          source.face==configured.frame.face && source.light==configured.frame.light &&
          source.shadow==configured.frame.shadow && source.edge==configured.frame.edge);
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
    style_window_palette_resolve(&source,&actual);
    CHECK(memcmp(&actual,&expected,sizeof(actual))==0);
    dc=CreateCompatibleDC(NULL);CHECK(dc);
    CHECK((HBRUSH)style_window_dialog_colors(dc,&source)==(HBRUSH)GetStockObject(DC_BRUSH));
    CHECK(GetBkColor(dc)==source.light && GetTextColor(dc)==source.edge);
    CHECK(GetDCBrushColor(dc)==source.light);
    DeleteDC(dc);
    printf("WIN16_PALETTE_OK high_contrast=%d\n",high);return 0;
}
