#include <windows.h>
#include <stdio.h>
#include "ntw16s-dll/font.h"
#define CHECK(x) do { if(!(x)){fprintf(stderr,"ASPECT_FAIL line=%d error=%lu\n",__LINE__,GetLastError());return 1;} } while(0)
static int check_font(int width, int height, BOOL vb)
{
    HDC original=CreateCompatibleDC(NULL),src=CreateCompatibleDC(NULL),expected=CreateCompatibleDC(NULL),actual=CreateCompatibleDC(NULL);
    HBITMAP bm1=CreateBitmap(128,64,1,1,NULL),bm2=CreateBitmap(128,64,1,1,NULL),bm3=CreateBitmap(128,64,1,1,NULL);
    CHECK(original&&src&&expected&&actual&&bm1&&bm2&&bm3);
    HGDIOBJ old1=SelectObject(src,bm1),old2=SelectObject(expected,bm2),old3=SelectObject(actual,bm3);
    HFONT source=CreateFontW(-height,width,0,0,FW_NORMAL,FALSE,FALSE,FALSE,ANSI_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH,L"Tahoma");
    HFONT system=CreateFontW(16,0,0,0,FW_BOLD,FALSE,FALSE,FALSE,ANSI_CHARSET,OUT_RASTER_PRECIS,CLIP_DEFAULT_PRECIS,NONANTIALIASED_QUALITY,DEFAULT_PITCH,L"System");
    CHECK(source&&system);HGDIOBJ oldf=SelectObject(original,source),oldsys=SelectObject(src,system);
    HFONT adapted=vb?win16_style_vb_font_acquire(source):win16_style_system_acquire(source);CHECK(adapted&&adapted!=source);HGDIOBJ oldadapt=SelectObject(actual,adapted);
    TEXTMETRICW target,base,result;CHECK(GetTextMetricsW(original,&target)&&GetTextMetricsW(src,&base)&&GetTextMetricsW(actual,&result));
    CHECK(target.tmHeight==result.tmHeight&&target.tmAscent==result.tmAscent);
    const WCHAR glyphs[]=L"0MiW";unsigned limiting=0;
    for(unsigned i=0;i<4;++i){SIZE a,b,c;RECT clear={0,0,128,64};
        CHECK(GetTextExtentPoint32W(original,glyphs+i,1,&a)&&GetTextExtentPoint32W(src,glyphs+i,1,&b)&&GetTextExtentPoint32W(actual,glyphs+i,1,&c));CHECK(a.cx==c.cx&&a.cx>0&&b.cx>0);
        CHECK(FillRect(src,&clear,GetStockObject(WHITE_BRUSH))&&FillRect(expected,&clear,GetStockObject(WHITE_BRUSH))&&FillRect(actual,&clear,GetStockObject(WHITE_BRUSH)));
        SetBkColor(src,RGB(255,255,255));SetTextColor(src,RGB(0,0,0));SetBkColor(actual,RGB(255,255,255));SetTextColor(actual,RGB(0,0,0));
        CHECK(TextOutW(src,0,0,glyphs+i,1));
        int left=b.cx,right=-1;GdiFlush();for(int yy=0;yy<base.tmHeight;++yy)for(int xx=0;xx<b.cx;++xx)if(GetPixel(src,xx,yy)==RGB(0,0,0)){left=min(left,xx);right=max(right,xx);}
        CHECK(right>=left);int ink=right-left+1,w=max(1,MulDiv(ink,target.tmHeight,base.tmHeight));if(w>a.cx){w=a.cx;++limiting;}
        int x=(a.cx-w)/2;
        CHECK(SetStretchBltMode(expected,vb?BLACKONWHITE:COLORONCOLOR));CHECK(StretchBlt(expected,x,0,w,target.tmHeight,src,left,0,ink,base.tmHeight,SRCCOPY));
        CHECK(TextOutW(actual,0,0,glyphs+i,1));GdiFlush();
        for(int yy=0;yy<target.tmHeight;++yy)for(int xx=0;xx<a.cx;++xx)CHECK(GetPixel(expected,xx,yy)==GetPixel(actual,xx,yy));
    }
    (void)limiting;SelectObject(actual,oldadapt);win16_style_system_release(adapted);SelectObject(original,oldf);SelectObject(src,oldsys);DeleteObject(source);DeleteObject(system);SelectObject(src,old1);SelectObject(expected,old2);SelectObject(actual,old3);DeleteObject(bm1);DeleteObject(bm2);DeleteObject(bm3);DeleteDC(original);DeleteDC(src);DeleteDC(expected);DeleteDC(actual);return 0;
}
int main(void){CHECK(!check_font(5,12,FALSE));CHECK(!check_font(5,12,TRUE));CHECK(!check_font(7,16,FALSE));CHECK(!check_font(7,16,TRUE));puts("FONT_ASPECT_PASS native=1 vb=1 source_advance=1 line_height=1 pixel_reference=1 cleanup=1");return 0;}
