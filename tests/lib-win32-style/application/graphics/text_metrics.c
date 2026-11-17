#include "lib-win32-style/application/graphics/text_metrics.h"
#include <stdio.h>

enum fault { FIT_NONE, FIT_MEMORY, FIT_BITMAP, FIT_SELECT_BITMAP, FIT_SELECT_FONT, FIT_SAVE, FIT_DRAW, FIT_PUBLISH };
static enum fault failure;
static HDC WINAPI create_dc(HDC dc){return failure==FIT_MEMORY?NULL:CreateCompatibleDC(dc);}
static HBITMAP WINAPI create_bitmap(HDC dc,int w,int h){return failure==FIT_BITMAP?NULL:CreateCompatibleBitmap(dc,w,h);}
static HGDIOBJ WINAPI select_object(HDC dc,HGDIOBJ object)
{
    DWORD type=GetObjectType(object);
    if((failure==FIT_SELECT_BITMAP && type==OBJ_BITMAP) || (failure==FIT_SELECT_FONT && type==OBJ_FONT))return HGDI_ERROR;
    return SelectObject(dc,object);
}
static int WINAPI save_dc(HDC dc){return failure==FIT_SAVE?0:SaveDC(dc);}
static int WINAPI draw_text(HDC dc,LPCWSTR text,int count,LPRECT rect,UINT flags)
{return failure==FIT_DRAW && !(flags&DT_CALCRECT)?0:DrawTextW(dc,text,count,rect,flags);}
static BOOL WINAPI stretch(HDC dc,int x,int y,int w,int h,HDC source,int sx,int sy,int sw,int sh,DWORD rop)
{return failure==FIT_PUBLISH?FALSE:StretchBlt(dc,x,y,w,h,source,sx,sy,sw,sh,rop);}
#define CreateCompatibleDC create_dc
#define CreateCompatibleBitmap create_bitmap
#define SelectObject select_object
#define SaveDC save_dc
#define DrawTextW draw_text
#define StretchBlt stretch
#include "lib-win32-style/application/graphics/text_metrics.c"
#undef CreateCompatibleDC
#undef CreateCompatibleBitmap
#undef SelectObject
#undef SaveDC
#undef DrawTextW
#undef StretchBlt

static BOOL fit_case(HDC dc,HFONT font,enum fault mode)
{
    const WCHAR text[]=L"Shared raster text fit";
    RECT canvas={0,0,96,64},bounds={17,13,59,37},measure={0};SIZE drawn={113,127};
    COLORREF sentinel=RGB(39,71,103),face=RGB(241,219,197),ink=RGB(0,0,0);
    SetDCBrushColor(dc,sentinel);FillRect(dc,&canvas,(HBRUSH)GetStockObject(DC_BRUSH));
    SetTextColor(dc,RGB(23,47,83));SetBkMode(dc,OPAQUE);SetStretchBltMode(dc,BLACKONWHITE);
    if(!DrawTextW(dc,text,-1,&measure,DT_CALCRECT|DT_SINGLELINE))return FALSE;
    int width=measure.right,height=measure.bottom;
    if(width<=42 || height<=0)return FALSE;
    HGDIOBJ before=GetCurrentObject(dc,OBJ_FONT);DWORD resources=GetGuiResources(GetCurrentProcess(),GR_GDIOBJECTS);
    failure=mode;
    BOOL result=style_text_fit(dc,font,text,&bounds,face,ink,(SIZE){4096,256},&drawn);
    failure=FIT_NONE;
    BOOL state=GetCurrentObject(dc,OBJ_FONT)==before && GetTextColor(dc)==RGB(23,47,83) &&
        GetBkMode(dc)==OPAQUE && GetStretchBltMode(dc)==BLACKONWHITE &&
        GetGuiResources(GetCurrentProcess(),GR_GDIOBJECTS)==resources;
    if(!state)return FALSE;
    if(mode!=FIT_NONE){
        if(result || drawn.cx!=113 || drawn.cy!=127)return FALSE;
        for(int y=0;y<64;++y)for(int x=0;x<96;++x)if(GetPixel(dc,x,y)!=sentinel)return FALSE;
        return TRUE;
    }
    int expected_width=42,expected_height=MulDiv(height,42,width);
    if(expected_height>24){expected_width=MulDiv(expected_width,24,expected_height);expected_height=24;}
    if(!result || drawn.cx!=expected_width || drawn.cy!=expected_height)return FALSE;
    int left=bounds.left+(42-drawn.cx)/2,top=bounds.top+(24-drawn.cy)/2;BOOL painted=FALSE;
    for(int y=0;y<64;++y)for(int x=0;x<96;++x){
        COLORREF color=GetPixel(dc,x,y);
        BOOL inside=x>=left && x<left+drawn.cx && y>=top && y<top+drawn.cy;
        if(!inside && color!=sentinel)return FALSE;
        if(inside && color!=sentinel)painted=TRUE;
    }
    SIZE unchanged={113,127};RECT fits={0,0,width+1,height+1};
    BOOL rejected=!style_text_fit(dc,font,text,&fits,face,ink,(SIZE){4096,256},&unchanged) &&
        !style_text_fit(dc,font,text,&bounds,face,ink,(SIZE){width-1,256},&unchanged) &&
        unchanged.cx==113 && unchanged.cy==127;
    return painted && rejected;
}
int main(void)
{
    HDC screen=GetDC(NULL),dc=screen?CreateCompatibleDC(screen):NULL;
    HBITMAP bitmap=dc?CreateCompatibleBitmap(screen,96,64):NULL;
    if(screen)ReleaseDC(NULL,screen);
    HGDIOBJ old=bitmap?SelectObject(dc,bitmap):NULL;
    HFONT font=(HFONT)GetStockObject(SYSTEM_FIXED_FONT);HGDIOBJ old_font=dc?SelectObject(dc,font):NULL;
    SIZE extent;int average=-1;BOOL ok=dc && bitmap && old && old!=HGDI_ERROR && old_font && old_font!=HGDI_ERROR;
    ok=ok && GetTextExtentPoint32W(dc,L"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ",52,&extent) &&
        style_text_alphabet_average(dc,&average) && average==((extent.cx/26)+1)/2 &&
        !style_text_alphabet_average(NULL,&average);
    if(ok)ok=fit_case(dc,font,FIT_NONE);
    for(enum fault mode=FIT_MEMORY;ok && mode<=FIT_PUBLISH;mode=(enum fault)(mode+1))ok=fit_case(dc,font,mode);
    for(unsigned i=0;ok && i<16;++i)ok=fit_case(dc,font,FIT_NONE);
    if(old_font && old_font!=HGDI_ERROR)SelectObject(dc,old_font);
    if(old && old!=HGDI_ERROR)SelectObject(dc,old);
    if(bitmap)DeleteObject(bitmap);
    if(dc)DeleteDC(dc);
    if(!ok)return 1;
    puts("TEXT_METRICS_OK average=1 invalid_dc=1 fit=1 bounded_pixels=1 failed_publish=1 cleanup=1");return 0;
}
