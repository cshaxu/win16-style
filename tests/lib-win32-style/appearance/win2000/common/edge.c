/* Original-image/source contracts, not a second implementation of DrawEdge. */
#include "lib-win32-style/appearance/win2000/control/button_paint.h"
#include "lib-win32-style/appearance/win2000/common/edge.h"
#include <stdio.h>
#define CHECK(x) do {if(!(x)){fprintf(stderr,"WIN2K_EDGE_FAIL line=%d error=%lu: %s\n",__LINE__,GetLastError(),#x);return 1;}}while(0)
int main(void)
{
    HDC dc=CreateCompatibleDC(NULL);void *pixels;
    BITMAPINFO info={.bmiHeader={sizeof(BITMAPINFOHEADER),40,-30,1,32,BI_RGB}};
    HBITMAP bitmap=CreateDIBSection(dc,&info,DIB_RGB_COLORS,&pixels,NULL,0);
    CHECK(dc && bitmap);HGDIOBJ previous=SelectObject(dc,bitmap);CHECK(previous && previous!=HGDI_ERROR);
    const win2k_control_palette *p=&win2k_control_standard_palette;RECT r={0,0,40,30};
    SetDCBrushColor(dc,RGB(1,2,3));SetTextColor(dc,RGB(4,5,6));SetBkColor(dc,RGB(7,8,9));
    CHECK(win2k_push_button_frame(dc,&r,DFCS_ADJUSTRECT,TRUE,p));
    CHECK(r.left==3 && r.top==3 && r.right==37 && r.bottom==27);
    /* Accessibility Options original: default OK at121,410, frame pixel
     * roles are black outer / white soft highlight / standard face. */
    CHECK(GetPixel(dc,0,10)==RGB(0,0,0) && GetPixel(dc,1,10)==RGB(255,255,255));
    CHECK(GetPixel(dc,2,10)==RGB(212,208,200) && GetPixel(dc,3,10)==RGB(212,208,200));
    CHECK(GetPixel(dc,39,10)==RGB(0,0,0) && GetPixel(dc,38,10)==RGB(0,0,0));
    CHECK(GetPixel(dc,37,10)==RGB(128,128,128));
    CHECK(GetDCBrushColor(dc)==RGB(1,2,3) && GetTextColor(dc)==RGB(4,5,6) && GetBkColor(dc)==RGB(7,8,9));
    r=(RECT){0,0,40,30};CHECK(win2k_push_button_frame(dc,&r,DFCS_ADJUSTRECT,FALSE,p));
    CHECK(r.left==2 && GetPixel(dc,0,0)==RGB(255,255,255) && GetPixel(dc,39,0)==RGB(0,0,0));
    CHECK(GetPixel(dc,1,8)==RGB(212,208,200) && GetPixel(dc,38,8)==RGB(128,128,128));
    r=(RECT){0,0,40,30};CHECK(win2k_push_button_frame(dc,&r,DFCS_PUSHED,FALSE,p));
    CHECK(GetPixel(dc,0,8)==RGB(0,0,0) && GetPixel(dc,1,8)==RGB(128,128,128));
    CHECK(GetPixel(dc,39,8)==RGB(255,255,255));
    r=(RECT){0,0,40,30};CHECK(win2k_push_button_frame(dc,&r,DFCS_PUSHED,TRUE,p));
    CHECK(GetPixel(dc,0,8)==RGB(0,0,0) && GetPixel(dc,1,8)==RGB(128,128,128) && GetPixel(dc,2,8)==RGB(212,208,200));
    win2k_control_palette different=*p;different.window=RGB(20,40,60);different.highlight=RGB(70,80,90);
    r=(RECT){0,0,40,30};CHECK(win2k_control_edge(dc,&r,EDGE_RAISED,BF_RECT|BF_MONO|BF_MIDDLE,&different));
    CHECK(GetPixel(dc,1,8)==different.window && GetPixel(dc,5,8)==different.window);
    CHECK(!win2k_control_edge(dc,&r,EDGE_RAISED,BF_TOP,p));
    CHECK(SelectObject(dc,previous) && DeleteObject(bitmap) && DeleteDC(dc));
    puts("WIN2K_EDGE_OK original_default_samples=1 normal=1 pressed=1 default_pressed=1 mono_window_slot=1 dc_restored=1");return 0;
}
