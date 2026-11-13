#include "lib-style/common/pixel_grid.h"
#include <stdio.h>
#define CHECK(x) do { if(!(x)){printf("PIXEL_GRID_FAIL line=%d\n",__LINE__);return 1;} } while(0)
int main(void)
{
    HDC dc=CreateCompatibleDC(NULL);CHECK(dc);
    HBITMAP bitmap=CreateBitmap(12,12,1,32,NULL);CHECK(bitmap);
    HGDIOBJ old=SelectObject(dc,bitmap);CHECK(old && old!=HGDI_ERROR);
    RECT canvas={0,0,12,12};CHECK(FillRect(dc,&canvas,(HBRUSH)GetStockObject(WHITE_BRUSH)));
    const char pixels[][4]={"010","102"};
    const COLORREF colors[]={RGB(0,0,0),RGB(255,0,0),RGB(0,0,255)};
    style_pixel_grid grid={&pixels[0][0],3,2,4,-1,colors,{2,2},{3,3},{2,2}};
    style_paint_pixel_grid(dc,&grid);
    CHECK(GetPixel(dc,2,2)==colors[0] && GetPixel(dc,4,2)==colors[1]);
    CHECK(GetPixel(dc,5,4)==colors[2] && GetPixel(dc,1,2)==RGB(255,255,255));
    CHECK(FillRect(dc,&canvas,(HBRUSH)GetStockObject(WHITE_BRUSH)));
    grid.transparent=1;grid.colors=NULL;SetDCBrushColor(dc,RGB(0,128,0));
    style_paint_pixel_grid(dc,&grid);
    CHECK(GetPixel(dc,2,2)==RGB(0,128,0) && GetPixel(dc,4,2)==RGB(255,255,255));
    CHECK(GetDCBrushColor(dc)==RGB(0,128,0));
    CHECK(SelectObject(dc,old)==bitmap && DeleteObject(bitmap) && DeleteDC(dc));
    puts("PIXEL_GRID_OK palette=1 transparent=1 brush=1 fractional_scale=1 stride=1");return 0;
}
