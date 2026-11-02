#include <stdio.h>
#include "lib/win16-control/palette.h"
#include "lib/win16-control/basic_paint.h"
#include "lib/win16-control/scroll_assets.h"

int main(void)
{
    win16_style_palette palette;
    win16_style_default_palette(&palette);
    if(palette.face!=RGB(192,192,192) || palette.edge!=RGB(0,0,0))return 1;
    for(int i=WIN16_SCROLL_UP;i<=WIN16_SCROLL_RIGHT_DISABLED;++i)
        if(!win16_control_scroll_glyph((enum win16_control_scroll_asset)i))return 2;
    if(win16_control_scroll_glyph((enum win16_control_scroll_asset)-1))return 3;
    HDC dc=CreateCompatibleDC(NULL);
    HBITMAP bitmap=dc?CreateCompatibleBitmap(dc,20,20):NULL;
    HGDIOBJ previous=bitmap?SelectObject(dc,bitmap):NULL;
    if(!dc || !bitmap || !previous || previous==HGDI_ERROR)return 4;
    RECT bounds={0,0,17,17};
    win16_control_scroll_arrow(dc,bounds,win16_control_scroll_glyph(WIN16_SCROLL_UP),&palette);
    BOOL valid=GetPixel(dc,0,0)==palette.edge;
    SelectObject(dc,previous);DeleteObject(bitmap);DeleteDC(dc);
    if(!valid)return 5;
    puts("WIN16_CONTROL_STANDALONE_LINK_OK palette=1 glyphs=12 pixel=1");
    return 0;
}
