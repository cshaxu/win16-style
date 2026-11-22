#include "raster_font.h"
#include "lib-win32-style/application/graphics/bitmap.h"
#include "lib-win32-style/application/graphics/bitmap_font.h"
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <wchar.h>

const style_font_ops style_raster_font_isolated_operations={
    .acquire=style_raster_font_acquire_isolated,
    .retain=style_raster_font_try_retain,
    .release=style_raster_font_release
};

const style_font_ops style_raster_font_stroke_operations={
    .acquire=style_raster_font_acquire_stroke_fit,
    .release=style_raster_font_release
};

static _Thread_local style_font_pool variants;
static LONG variant_serial;
enum { menu_dpi_bits=9, menu_dpi_mask=(1u<<menu_dpi_bits)-1 };

HFONT style_raster_font_source(HFONT font)
{return style_font_pool_source(&variants,font);}

static HFONT system_create(HFONT source,UINT key,void *context,style_font_pool_entry **owned)
{
    UINT menu_dpi=(key>>1)&menu_dpi_mask;
    unsigned points=key>>(1+menu_dpi_bits);
    BOOL preserve_strokes=(key&1)!=0;(void)context;
    HDC original=NULL,src=NULL,dst=NULL;HFONT base=NULL,font=NULL;
    HBITMAP a=NULL;HGDIOBJ oa=NULL,of=NULL,oo=NULL;style_bitmap rendered={0};
    BYTE *data=NULL;DWORD *pixels=NULL;HANDLE resource=NULL;DWORD count=0;
    TEXTMETRICW tm,system;char face[32],actual[32];BOOL ok=FALSE;
    unsigned cursor=118+225*4,sum=0,maxwidth=0;style_font_pool_entry *v=NULL;
    original=CreateCompatibleDC(NULL);if(!original)goto done;
    oo=SelectObject(original,source?source:GetStockObject(SYSTEM_FONT));
    if(!oo || !GetTextMetricsW(original,&tm) || !GetTextFaceA(original,32,actual))goto done;
    /* Borrow the stock face or a face already registered by a live lease. */
    if(!menu_dpi && (!strcmp(actual,"System") || style_memory_font_has_face(actual))){
        font=source;ok=TRUE;goto done;
    }
    if(tm.tmHeight<1 || tm.tmHeight>64)goto done;
    data=calloc(65536,1);v=calloc(1,sizeof(*v));if(!data || !v)goto done;
    wsprintfA(face,"Style Raster %08lX",(DWORD)InterlockedIncrement(&variant_serial));
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
    if(ok && resource){
        v->lease.font=font;
        v->lease.resource=resource;
        style_memory_font_register_face(&v->lease,face);
        *owned=v;
        return font;
    }
    free(v);
    if(ok)return font;
    if(font)DeleteObject(font);
    if(resource)RemoveFontMemResourceEx(resource);
    return NULL;
}

static HFONT system_acquire(HFONT source,unsigned points,UINT menu_dpi,BOOL preserve_strokes,BOOL isolated)
{
    if(!points || points>48)return NULL;
    UINT key=(points<<(1+menu_dpi_bits))|(menu_dpi<<1)|!!preserve_strokes;
    return style_font_pool_get(&variants,source,key,isolated,system_create,NULL);
}

HFONT style_raster_font_acquire(HFONT source,unsigned points)
{return system_acquire(source,points,0,FALSE,FALSE);}

HFONT style_raster_font_acquire_isolated(HFONT source,unsigned points)
{return system_acquire(source,points,0,FALSE,TRUE);}

HFONT style_raster_font_acquire_stroke_fit(HFONT source,unsigned points)
{return system_acquire(source,points,0,TRUE,FALSE);}

static HFONT logical_menu_create(HFONT source,UINT dpi,void *context,style_font_pool_entry **owned)
{
    (void)source;
    style_font_pool_entry *entry=calloc(1,sizeof(*entry));
    if(!entry)return NULL;
    entry->lease.font=msstyles_font_create(context,dpi,MSSTYLES_FONT_MENU);
    if(!entry->lease.font){free(entry);return NULL;}
    *owned=entry;
    return entry->lease.font;
}

HFONT style_menu_font_acquire(UINT dpi,const msstyles_font *defined,bool legacy_bitmap)
{
    BOOL bitmap=defined && defined->menu_mode==STYLE_MENU_FONT_SYSTEM_BITMAP;
#ifdef STYLE_THEME_EXTERNAL_ONLY
    (void)legacy_bitmap;
    if(!dpi || (bitmap && (dpi<48 || dpi>384)))return NULL;
#else
    bool legacy=legacy_bitmap && (!defined || defined->menu_mode==STYLE_MENU_FONT_DEFAULT);
    if(legacy && defined)
        bitmap=!_wcsicmp(defined->logical.lfFaceName,L"System") && defined->logical.lfWeight==FW_BOLD &&
            !defined->logical.lfItalic && !defined->logical.lfUnderline && !defined->logical.lfStrikeOut;
    if(!dpi || ((bitmap || legacy) && (dpi<48 || dpi>384)))return NULL;
#endif
    if(!bitmap || !defined->points || defined->points>48){
        msstyles_font request=defined?*defined:(msstyles_font){0};
        return style_font_pool_get(&variants,NULL,dpi,TRUE,logical_menu_create,&request);
    }
    UINT key=(dpi<<1)|(defined->points<<(1+menu_dpi_bits));
    return style_font_pool_get(&variants,NULL,key,FALSE,system_create,NULL);
}

#ifndef STYLE_THEME_EXTERNAL_ONLY
HFONT style_menu_font_acquire_with_bitmap_default(UINT dpi,const msstyles_font *defined)
{return style_menu_font_acquire(dpi,defined,true);}
#endif

void style_menu_font_release(HFONT font)
{style_font_pool_release(&variants,font);}

/* Scale the stock bitmap at the declared sampling density. The calendar
 * caller independently limits the native path to the witnessed window DPI. */
HFONT style_raster_font_acquire_at_density(unsigned raster_dpi,unsigned points)
{return raster_dpi>=48 && raster_dpi<=96?system_acquire(NULL,points,raster_dpi,FALSE,FALSE):NULL;}

void style_raster_font_release(HFONT font)
{style_font_pool_release(&variants,font);}
BOOL style_raster_font_try_retain(HFONT font)
{return style_font_pool_try_retain(&variants,font);}
void style_raster_font_retain(HFONT font)
{style_font_pool_retain(&variants,font);}
