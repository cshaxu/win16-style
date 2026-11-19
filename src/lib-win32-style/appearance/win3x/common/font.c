#include "font.h"
#include "lib-win32-style/application/graphics/font_cache.h"
#include "lib-win32-style/application/graphics/bitmap.h"
#include "lib-win32-style/application/graphics/bitmap_font.h"
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <wchar.h>

static _Thread_local style_memory_font shared;

static BOOL create_font(void *context,style_memory_font *result)
{
    (void)context;
    /* FNT v2 is the public bitmap-resource form already exercised by the
     * standalone font probe. Only newly rasterized host glyphs go into it.
     * The original USER/font loader is not imported or reconstructed. */
    BYTE *data=(BYTE *)calloc(65536,1);char face[32];
    unsigned cursor=118+225*4,n,maxwidth=0,sum=0;
    DWORD count=0;HANDLE resource=NULL;HFONT base=NULL,font=NULL;
    HDC src=NULL,scaled=NULL;HBITMAP a=NULL,b=NULL;HGDIOBJ olda=NULL,oldb=NULL,oldfont=NULL;
    TEXTMETRICW tm;SIZE extent;RECT clear={0,0,128,64};BOOL ok=FALSE;
    if(!data)return FALSE;
    wsprintfA(face,"Win16 System20 %08lX",GetCurrentThreadId());
    src=CreateCompatibleDC(NULL);scaled=CreateCompatibleDC(NULL);
    a=CreateBitmap(128,64,1,1,NULL);b=CreateBitmap(128,20,1,1,NULL);
    base=CreateFontW(16,0,0,0,FW_BOLD,FALSE,FALSE,FALSE,ANSI_CHARSET,
        OUT_RASTER_PRECIS,CLIP_DEFAULT_PRECIS,NONANTIALIASED_QUALITY,DEFAULT_PITCH,L"System");
    if(!src || !scaled || !a || !b || !base)goto done;
    olda=SelectObject(src,a);oldb=SelectObject(scaled,b);oldfont=SelectObject(src,base);
    if(!olda || !oldb || !oldfont || !GetTextMetricsW(src,&tm) ||
        tm.tmHeight<1 || tm.tmHeight>64 || tm.tmCharSet!=ANSI_CHARSET)goto done;
    SetBkColor(src,RGB(255,255,255));SetTextColor(src,RGB(0,0,0));
    SetStretchBltMode(scaled,COLORONCOLOR);
    for(n=0;n<224;++n){char ch=(char)(n+32);WCHAR wc;unsigned x,y,w,bytes;
        if(MultiByteToWideChar(1252,0,&ch,1,&wc,1)!=1 ||
            !GetTextExtentPoint32W(src,&wc,1,&extent) || extent.cx<0 || extent.cx>128)goto done;
        w=(unsigned)MulDiv(extent.cx,20,tm.tmHeight);if(w>128)goto done;
        bytes=((w+7)/8)*20;if(cursor+bytes+sizeof(face)>65535)goto done;
        style_bitmap_font_word(data,118+4*n,w);style_bitmap_font_word(data,120+4*n,cursor);
        if(!FillRect(src,&clear,(HBRUSH)GetStockObject(WHITE_BRUSH)) || !TextOutW(src,0,0,&wc,1))goto done;
        if(w && !StretchBlt(scaled,0,0,(int)w,20,src,0,0,extent.cx,tm.tmHeight,SRCCOPY))goto done;
        for(x=0;x<w;++x)for(y=0;y<20;++y){COLORREF pixel=GetPixel(scaled,(int)x,(int)y);
            if(pixel==CLR_INVALID)goto done;
            if(pixel==RGB(0,0,0))data[cursor+(x/8)*20+y]|=(BYTE)(0x80>>(x%8));}
        cursor+=bytes;sum+=w;if(w>maxwidth)maxwidth=w;
    }
    style_bitmap_font_metrics metrics={20,(unsigned)MulDiv(tm.tmAscent,20,tm.tmHeight),
        (unsigned)MulDiv(tm.tmInternalLeading,20,tm.tmHeight),0,(unsigned)tm.tmWeight,15,sum,maxwidth};
    cursor=style_bitmap_font_finish(data,cursor,face,&metrics);
    resource=AddFontMemResourceEx(data,cursor,NULL,&count);if(!resource || count!=1)goto done;
    font=CreateFontA(20,0,0,0,tm.tmWeight,FALSE,FALSE,FALSE,ANSI_CHARSET,OUT_RASTER_PRECIS,
        CLIP_DEFAULT_PRECIS,NONANTIALIASED_QUALITY,DEFAULT_PITCH,face);
    if(font){char actual[32];HGDIOBJ old=SelectObject(scaled,font);
        ok=old && GetTextMetricsW(scaled,&tm) && tm.tmHeight==20 &&
            GetTextFaceA(scaled,32,actual)>0 && !strcmp(actual,face);
        if(old)SelectObject(scaled,old);}
done:
    if(oldfont)SelectObject(src,oldfont);
    if(olda)SelectObject(src,olda);
    if(oldb)SelectObject(scaled,oldb);
    if(base)DeleteObject(base);
    if(a)DeleteObject(a);
    if(b)DeleteObject(b);
    if(src)DeleteDC(src);
    if(scaled)DeleteDC(scaled);
    free(data);
    if(ok){result->font=font;result->resource=resource;return TRUE;}
    if(font)DeleteObject(font);
    if(resource)RemoveFontMemResourceEx(resource);
    return FALSE;
}

HFONT win16_style_font_acquire(void)
{return style_memory_font_acquire(&shared,create_font,NULL);}
void win16_style_font_release(HFONT font)
{style_memory_font_release(&shared,font);}

static _Thread_local style_font_pool variants;
static LONG variant_serial;
enum { menu_dpi_bits=9, menu_dpi_mask=(1u<<menu_dpi_bits)-1, default_menu_points=12 };

HFONT win16_style_system_source(HFONT font)
{return style_font_pool_source(&variants,font);}

static HFONT system_create(HFONT source,UINT key,void *context,style_font_pool_entry **owned)
{
    UINT menu_dpi=(key>>1)&menu_dpi_mask;
    unsigned points=key>>(1+menu_dpi_bits);
    if(!points)points=default_menu_points;
    BOOL preserve_strokes=(key&1)!=0;(void)context;
    HDC original=NULL,src=NULL,dst=NULL;HFONT base=NULL,font=NULL;
    HBITMAP a=NULL;HGDIOBJ oa=NULL,of=NULL,oo=NULL;style_bitmap rendered={0};
    BYTE *data=NULL;DWORD *pixels=NULL;HANDLE resource=NULL;DWORD count=0;
    TEXTMETRICW tm,system;char face[32],actual[32];BOOL ok=FALSE;
    unsigned cursor=118+225*4,sum=0,maxwidth=0;style_font_pool_entry *v=NULL;
    original=CreateCompatibleDC(NULL);if(!original)goto done;
    oo=SelectObject(original,source?source:GetStockObject(SYSTEM_FONT));
    if(!oo || !GetTextMetricsW(original,&tm) || !GetTextFaceA(original,32,actual))goto done;
    /* Already-System fonts, including the explicit legacy bitmap API, need
     * no new resource. NULL remains the native default System font. */
    if(!menu_dpi && (!strcmp(actual,"System") || !strncmp(actual,"Win16 System",12))){
        font=source;ok=TRUE;goto done;
    }
    if(tm.tmHeight<1 || tm.tmHeight>64)goto done;
    data=calloc(65536,1);v=calloc(1,sizeof(*v));if(!data || !v)goto done;
    wsprintfA(face,"Win16 System %08lX",(DWORD)InterlockedIncrement(&variant_serial));
    src=CreateCompatibleDC(NULL);
    a=CreateBitmap(128,64,1,1,NULL);
    SIZE dimensions={128,64};
    if(style_bitmap_ensure(&rendered,NULL,dimensions)){dst=rendered.dc;pixels=(DWORD *)rendered.pixels;}
    base=CreateFontW(MulDiv((int)points,96,72),0,0,0,FW_BOLD,FALSE,FALSE,FALSE,ANSI_CHARSET,
        OUT_RASTER_PRECIS,CLIP_DEFAULT_PRECIS,NONANTIALIASED_QUALITY,DEFAULT_PITCH,L"System");
    if(!src || !dst || !a || !base)goto done;
    oa=SelectObject(src,a);of=SelectObject(src,base);
    if(!oa || !of || !GetTextMetricsW(src,&system) ||
        !GetTextFaceA(src,32,actual) || strcmp(actual,"System") || system.tmHeight<1 || system.tmHeight>64)goto done;
    /* Menu typography has its own proportional contract. Control variants
     * continue to use the application font metrics above, without changes. */
    if(menu_dpi){
        tm=system;tm.tmHeight=MulDiv(system.tmHeight,(int)menu_dpi,96);
        tm.tmAscent=MulDiv(system.tmAscent,(int)menu_dpi,96);
        tm.tmInternalLeading=MulDiv(system.tmInternalLeading,(int)menu_dpi,96);
        tm.tmExternalLeading=MulDiv(system.tmExternalLeading,(int)menu_dpi,96);
        if(tm.tmHeight<1 || tm.tmHeight>64)goto done;
    }
    SetBkColor(src,RGB(255,255,255));SetTextColor(src,RGB(0,0,0));SetStretchBltMode(dst,preserve_strokes?BLACKONWHITE:COLORONCOLOR);
    for(unsigned n=0;n<224;++n){char ch=(char)(n+32);WCHAR wc;SIZE oldsize,syssize;RECT clear={0,0,128,64};
        if(MultiByteToWideChar(1252,0,&ch,1,&wc,1)!=1 ||
            !GetTextExtentPoint32W(original,&wc,1,&oldsize) || !GetTextExtentPoint32W(src,&wc,1,&syssize) ||
            oldsize.cx<0 || oldsize.cx>128 || syssize.cx<0 || syssize.cx>128)goto done;
        unsigned width=(unsigned)(menu_dpi?MulDiv(syssize.cx,(int)menu_dpi,96):oldsize.cx),bytes=((width+7)/8)*(unsigned)tm.tmHeight;
        if(width>128 || cursor+bytes+sizeof(face)>65535)goto done;
        style_bitmap_font_word(data,118+4*n,width);style_bitmap_font_word(data,120+4*n,cursor);
        if(!FillRect(src,&clear,(HBRUSH)GetStockObject(WHITE_BRUSH)) || !TextOutW(src,0,0,&wc,1))goto done;
        if(!FillRect(dst,&clear,(HBRUSH)GetStockObject(WHITE_BRUSH)))goto done;
        if(width && syssize.cx && !StretchBlt(dst,0,0,(int)width,tm.tmHeight,src,0,0,syssize.cx,system.tmHeight,SRCCOPY))goto done;
        GdiFlush(); /* Complete GDI writes before reading DIB memory. */
        for(unsigned x=0;x<width;++x)for(unsigned y=0;y<(unsigned)tm.tmHeight;++y)
            if(!(pixels[y*128+x]&0xffffff))data[cursor+(x/8)*(unsigned)tm.tmHeight+y]|=(BYTE)(0x80>>(x%8));
        cursor+=bytes;sum+=width;if(width>maxwidth)maxwidth=width;
    }
    style_bitmap_font_metrics metrics={(unsigned)tm.tmHeight,(unsigned)tm.tmAscent,
        (unsigned)tm.tmInternalLeading,(unsigned)tm.tmExternalLeading,(unsigned)system.tmWeight,
        menu_dpi?points:8u,sum,maxwidth};
    cursor=style_bitmap_font_finish(data,cursor,face,&metrics);
    resource=AddFontMemResourceEx(data,cursor,NULL,&count);if(!resource || count!=1)goto done;
    font=CreateFontA(tm.tmHeight,0,0,0,system.tmWeight,FALSE,FALSE,FALSE,ANSI_CHARSET,OUT_RASTER_PRECIS,
        CLIP_DEFAULT_PRECIS,NONANTIALIASED_QUALITY,DEFAULT_PITCH,face);
    if(font){HGDIOBJ old=SelectObject(dst,font);TEXTMETRICW realized;
        ok=old && GetTextMetricsW(dst,&realized) && realized.tmHeight==tm.tmHeight &&
            GetTextFaceA(dst,32,actual)>0 && !strcmp(actual,face);
        if(old)SelectObject(dst,old);}
done:
    if(oo)SelectObject(original,oo);
    if(of)SelectObject(src,of);
    if(oa)SelectObject(src,oa);
    if(base)DeleteObject(base);
    if(a)DeleteObject(a);
    if(original)DeleteDC(original);
    if(src)DeleteDC(src);
    style_bitmap_dispose(&rendered);
    free(data);
    if(ok && resource){v->lease.font=font;v->lease.resource=resource;*owned=v;return font;}
    free(v);
    if(ok)return font;
    if(font)DeleteObject(font);
    if(resource)RemoveFontMemResourceEx(resource);
    return NULL;
}

static HFONT system_acquire(HFONT source,UINT menu_dpi,BOOL preserve_strokes,BOOL isolated)
{return style_font_pool_get(&variants,source,((UINT)menu_dpi<<1)|!!preserve_strokes,isolated,system_create,NULL);}

HFONT win16_style_system_acquire(HFONT source)
{return system_acquire(source,0,FALSE,FALSE);}

HFONT win16_style_system_acquire_isolated(HFONT source)
{return system_acquire(source,0,FALSE,TRUE);}

HFONT win16_style_vb_font_acquire(HFONT source)
{return system_acquire(source,0,TRUE,FALSE);}

HFONT win16_style_menu_font_acquire(UINT dpi,const msstyles_font *defined)
{
    if(dpi<48 || dpi>384)return NULL;
    unsigned points=default_menu_points;
    /* A LOGFONT descriptor selects the existing host System raster source.
     * Other faces and text styles require a different renderer. */
    if(defined && defined->points && defined->points<=48 &&
       !_wcsicmp(defined->logical.lfFaceName,L"System") && defined->logical.lfWeight==FW_BOLD &&
       !defined->logical.lfItalic && !defined->logical.lfUnderline && !defined->logical.lfStrikeOut)
        points=defined->points;
    UINT key=(dpi<<1)|((points==default_menu_points?0:points)<<(1+menu_dpi_bits));
    return style_font_pool_get(&variants,NULL,key,FALSE,system_create,NULL);
}

/* 13/16 of the stock System bitmap, preserving the glyph aspect ratio.
 * The calendar caller currently admits only its witnessed 96-DPI case. */
HFONT win16_style_calendar_font_acquire(void)
{return system_acquire(NULL,78,FALSE,FALSE);}

void win16_style_system_release(HFONT font)
{style_font_pool_release(&variants,font);}
BOOL win16_style_system_try_retain(HFONT font)
{return style_font_pool_try_retain(&variants,font);}
void win16_style_system_retain(HFONT font)
{style_font_pool_retain(&variants,font);}
