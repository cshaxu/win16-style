#define UNICODE
#include <windows.h>
#include <stdio.h>
#include "ntw16s-dll/font.h"
/* 64 sizes are measured, not claimed to be visually designed. */
int main(void)
{
    HDC dc=CreateCompatibleDC(NULL);BITMAPINFO bi={0};DWORD *pixels=NULL;
    bi.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);bi.bmiHeader.biWidth=128;bi.bmiHeader.biHeight=-64;
    bi.bmiHeader.biPlanes=1;bi.bmiHeader.biBitCount=32;HBITMAP bitmap=CreateDIBSection(dc,&bi,DIB_RGB_COLORS,(void **)&pixels,NULL,0);
    if(!dc||!bitmap||!pixels){return 1;}
    HGDIOBJ oldbmp=SelectObject(dc,bitmap),original=GetCurrentObject(dc,OBJ_FONT);
    SetTextColor(dc,RGB(0,0,0));SetBkColor(dc,RGB(255,255,255));unsigned success=0,failed=0,native=0,painted=0;
    for(int h=1;h<=64;h++){
        HFONT source=CreateFontW(h,0,0,0,FW_NORMAL,0,0,0,ANSI_CHARSET,OUT_DEFAULT_PRECIS,0,DEFAULT_QUALITY,0,L"Tahoma");
        if(!source){return 2;}
        SelectObject(dc,source);TEXTMETRICW before,after;if(!GetTextMetricsW(dc,&before))return 3;
        HFONT f=win16_style_vb_font_acquire(source);
        if(!f){printf("FONT_SIZE_FALLBACK requested=%d cell=%ld\n",h,before.tmHeight);++failed;SelectObject(dc,original);DeleteObject(source);continue;}
        if(f==source)++native;
        SelectObject(dc,f);if(!GetTextMetricsW(dc,&after)||after.tmHeight!=before.tmHeight||after.tmAscent!=before.tmAscent){printf("FONT_SIZE_METRIC_FAIL requested=%d\n",h);return 4;}
        for(WCHAR ch=32;ch<127;++ch){SIZE a,b;SelectObject(dc,source);if(!GetTextExtentPoint32W(dc,&ch,1,&a))return 5;
            SelectObject(dc,f);if(!GetTextExtentPoint32W(dc,&ch,1,&b)||a.cx!=b.cx){printf("FONT_SIZE_ADVANCE_FAIL requested=%d cp=%u %ld/%ld\n",h,ch,a.cx,b.cx);return 6;}
            if(before.tmHeight>=8 && ch!=32){RECT clear={0,0,128,64};FillRect(dc,&clear,GetStockObject(WHITE_BRUSH));
                if(!TextOutW(dc,0,0,&ch,1)){return 7;}
                GdiFlush();BOOL ink=FALSE;
                for(int y=0;y<before.tmHeight && !ink;++y)for(int x=0;x<b.cx && x<128;++x)if((pixels[y*128+x]&0xffffff)!=0xffffff){ink=TRUE;break;}
                if(!ink){printf("FONT_SIZE_EMPTY_GLYPH requested=%d cell=%ld cp=%u\n",h,before.tmHeight,ch);return 8;}++painted;
            }
        }
        const WCHAR extra[]={0x00e9,0x00c9,0x00e0,0x00f1,0x00f6,0x00e7,0x20ac};
        for(unsigned i=0;i<sizeof(extra)/sizeof(extra[0]);++i){SIZE a,b;SelectObject(dc,source);
            if(!GetTextExtentPoint32W(dc,extra+i,1,&a)){return 9;}
            SelectObject(dc,f);
            if(!GetTextExtentPoint32W(dc,extra+i,1,&b)||a.cx!=b.cx){printf("FONT_SIZE_EXTENT_FAIL requested=%d cp=%u\n",h,extra[i]);return 10;}
            if(before.tmHeight>=12){RECT clear={0,0,128,64};FillRect(dc,&clear,GetStockObject(WHITE_BRUSH));
                if(!TextOutW(dc,0,0,extra+i,1)){return 11;}
                GdiFlush();BOOL ink=FALSE;
                for(int y=0;y<before.tmHeight && !ink;++y)for(int x=0;x<b.cx && x<128;++x)if((pixels[y*128+x]&0xffffff)!=0xffffff){ink=TRUE;break;}
                if(!ink){printf("FONT_SIZE_EMPTY_EXTENDED requested=%d cp=%u\n",h,extra[i]);return 12;}
            }
        }
        SelectObject(dc,original);win16_style_system_release(f);DeleteObject(source);if(f!=source)++success;
    }
    SelectObject(dc,oldbmp);DeleteObject(bitmap);DeleteDC(dc);
    printf("FONT_SIZE_MATRIX success=%u native=%u fallback=%u total=64 nonempty_ascii=%u\n",success,native,failed,painted);return 0;
}
