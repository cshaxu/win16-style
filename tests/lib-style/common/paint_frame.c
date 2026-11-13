#include "lib-style/common/paint_frame.h"
#include <stdio.h>
#define CHECK(x) do { if(!(x)){printf("PAINT_FRAME_FAIL line=%d\n",__LINE__);return 1;} } while(0)
int main(void)
{
    HDC dc=CreateCompatibleDC(NULL);CHECK(dc);
    HBITMAP image=CreateBitmap(12,12,1,32,NULL);CHECK(image);
    HGDIOBJ old=SelectObject(dc,image);CHECK(old && old!=HGDI_ERROR);
    RECT canvas={0,0,12,12},frame={2,2,10,10};
    CHECK(FillRect(dc,&canvas,(HBRUSH)GetStockObject(WHITE_BRUSH)));
    style_paint_solid_frame(dc,frame,RGB(0,0,0),2);
    CHECK(GetPixel(dc,2,2)==RGB(0,0,0) && GetPixel(dc,3,3)==RGB(0,0,0));
    CHECK(GetPixel(dc,4,4)==RGB(255,255,255) && GetPixel(dc,1,2)==RGB(255,255,255));
    style_paint_solid_frame(dc,frame,RGB(255,0,0),0);
    CHECK(GetPixel(dc,2,2)==RGB(0,0,0));
    CHECK(SelectObject(dc,old)==image && DeleteObject(image) && DeleteDC(dc));
    puts("PAINT_FRAME_OK width=2 interior=1 outside=1 zero=1");return 0;
}
