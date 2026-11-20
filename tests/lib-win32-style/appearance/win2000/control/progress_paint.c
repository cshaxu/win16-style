#include "lib-win32-style/appearance/win2000/control/progress_paint.h"
#include <limits.h>
#include <stdio.h>

#define CHECK(x) do {if(!(x)){fprintf(stderr,"WIN2K_PROGRESS_PAINT_FAIL line=%d: %s\n",__LINE__,#x);return 1;}}while(0)

int main(void)
{
    HDC dc=CreateCompatibleDC(NULL);
    BITMAPINFO info={.bmiHeader={sizeof(BITMAPINFOHEADER),36,-36,1,32,BI_RGB}};
    void *pixels;
    HBITMAP bitmap=CreateDIBSection(dc,&info,DIB_RGB_COLORS,&pixels,NULL,0);
    CHECK(dc && bitmap);
    HGDIOBJ previous=SelectObject(dc,bitmap);
    CHECK(previous && previous!=HGDI_ERROR);
    const win2k_palette *p=&win2k_standard_palette;
    style_progress_snapshot model={.client={0,0,30,14},.range={0,100},
        .position=2,.state=PBST_NORMAL,.bar=CLR_DEFAULT,.background=CLR_DEFAULT};
    COLORREF brush=RGB(7,41,90);
    SetDCBrushColor(dc,brush);
    CHECK(win2k_progress_paint(dc,&model,0,p,96,NULL));
    /* 28px lane, 12px tall: round 0.56 to 1, show one full 8px block. */
    CHECK(GetPixel(dc,0,6)==p->face && GetPixel(dc,1,6)==p->selection);
    CHECK(GetPixel(dc,8,6)==p->selection && GetPixel(dc,9,6)==p->face);
    const style_progress_layout outlined=STYLE_PROGRESS_OUTLINED;
    CHECK(win2k_progress_paint(dc,&model,0,p,192,&outlined));
    CHECK(GetPixel(dc,0,6)==p->dark && GetPixel(dc,1,6)==p->dark && GetPixel(dc,2,6)==p->face);
    model.position=20;
    CHECK(win2k_progress_paint(dc,&model,0,p,192,&outlined));
    CHECK(GetPixel(dc,2,6)==p->selection && GetPixel(dc,6,6)==p->selection && GetPixel(dc,7,6)==p->face);
    model.position=35;
    CHECK(win2k_progress_paint(dc,&model,0,p,96,NULL));
    CHECK(GetPixel(dc,10,6)==p->face && GetPixel(dc,11,6)==p->face);
    model.position=39;
    CHECK(win2k_progress_paint(dc,&model,0,p,96,NULL));
    CHECK(GetPixel(dc,10,6)==p->face && GetPixel(dc,11,6)==p->selection);
    model.position=100;
    CHECK(win2k_progress_paint(dc,&model,0,p,96,NULL));
    CHECK(GetPixel(dc,27,6)==p->selection && GetPixel(dc,28,6)==p->face);
    model.client=(RECT){0,0,14,30};
    CHECK(win2k_progress_paint(dc,&model,PBS_VERTICAL,p,96,NULL));
    CHECK(GetPixel(dc,5,28)==p->selection && GetPixel(dc,5,20)==p->face && GetPixel(dc,5,18)==p->selection);
    CHECK(GetPixel(dc,5,1)==p->face && GetPixel(dc,5,2)==p->selection);
    model.range=(PBRANGE){INT_MIN,INT_MAX};
    model.position=0;
    model.bar=RGB(172,20,81);model.background=RGB(38,54,78);
    CHECK(win2k_progress_paint(dc,&model,PBS_VERTICAL|PBS_SMOOTH,p,96,NULL));
    CHECK(GetPixel(dc,5,15)==model.bar && GetPixel(dc,5,14)==model.background);
    CHECK(GetDCBrushColor(dc)==brush);
    model.range=(PBRANGE){0,0};
    CHECK(win2k_progress_paint(dc,&model,PBS_SMOOTH,p,96,NULL) && GetPixel(dc,5,15)==model.background);
    model.client=(RECT){0,0,2,30};
    CHECK(win2k_progress_paint(dc,&model,PBS_VERTICAL,p,96,NULL));
    model.state=PBST_PAUSED;
    CHECK(!win2k_progress_paint(dc,&model,0,p,96,NULL));
    SelectObject(dc,previous);DeleteObject(bitmap);DeleteDC(dc);
    puts("WIN2K_PROGRESS_PAINT_OK blocks=3 directions=2 signed_range=1 edge_cases=3 borrowed_dc=1");
    return 0;
}
