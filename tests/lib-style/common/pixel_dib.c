#include "lib-style/common/pixel_dib.h"
#include <stdio.h>
#define CHECK(x) do { if(!(x)){printf("PIXEL_DIB_FAIL line=%d\n",__LINE__);return 1;} } while(0)
int main(void)
{
    HDC dc=CreateCompatibleDC(NULL);CHECK(dc);
    HBITMAP bitmap=CreateBitmap(10,10,1,32,NULL);CHECK(bitmap);
    HGDIOBJ old=SelectObject(dc,bitmap);CHECK(old && old!=HGDI_ERROR);
    RECT canvas={0,0,10,10},target={2,2,6,6},empty={1,1,1,3};
    const char pixels[][3]={"01","10"};const DWORD colors[]={0x00ff0000,0x000000ff};
    CHECK(FillRect(dc,&canvas,(HBRUSH)GetStockObject(WHITE_BRUSH)));
    SetDCBrushColor(dc,RGB(0,128,0));
    CHECK(style_paint_indexed_dib(dc,&target,&pixels[0][0],3,(SIZE){2,2},colors));
    CHECK(GetPixel(dc,2,2)==RGB(255,0,0) && GetPixel(dc,4,2)==RGB(0,0,255));
    CHECK(GetPixel(dc,2,4)==RGB(0,0,255) && GetPixel(dc,6,2)==RGB(255,255,255));
    CHECK(GetDCBrushColor(dc)==RGB(0,128,0));
    CHECK(!style_paint_indexed_dib(dc,&empty,&pixels[0][0],3,(SIZE){2,2},colors));
    CHECK(FillRect(dc,&canvas,(HBRUSH)GetStockObject(WHITE_BRUSH)));
    CHECK(IntersectClipRect(dc,2,2,4,6)!=ERROR);
    CHECK(style_paint_indexed_dib(dc,&target,&pixels[0][0],3,(SIZE){2,2},colors));
    CHECK(GetPixel(dc,2,2)==RGB(255,0,0));
    CHECK(SelectClipRgn(dc,NULL)!=ERROR && GetPixel(dc,4,2)==RGB(255,255,255));
    CHECK(SelectObject(dc,old)==bitmap && DeleteObject(bitmap) && DeleteDC(dc));
    puts("PIXEL_DIB_OK colors=1 scale=1 clip=1 brush=1 empty=1");return 0;
}
