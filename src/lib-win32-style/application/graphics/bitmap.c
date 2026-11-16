#include "bitmap.h"
#include <stdint.h>

void style_bitmap_dispose(style_bitmap *s)
{
    if(s->old_bitmap)SelectObject(s->dc,s->old_bitmap);
    if(s->bitmap)DeleteObject(s->bitmap);
    if(s->dc)DeleteDC(s->dc);
    s->dc=NULL;s->bitmap=NULL;s->old_bitmap=NULL;s->pixels=NULL;
}
BOOL style_bitmap_ensure(style_bitmap *s,HDC reference,SIZE size)
{
    if(s->dc && size.cx==s->size.cx && size.cy==s->size.cy)return TRUE;
    if(size.cx<=0 || size.cy<=0 || (SIZE_T)size.cx>SIZE_MAX/4/(SIZE_T)size.cy){SetLastError(ERROR_INVALID_PARAMETER);return FALSE;}
    style_bitmap_dispose(s);
    BITMAPINFO info={0};info.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
    info.bmiHeader.biWidth=size.cx;info.bmiHeader.biHeight=-size.cy;
    info.bmiHeader.biPlanes=1;info.bmiHeader.biBitCount=32;
    s->dc=CreateCompatibleDC(reference);
    if(s->dc)s->bitmap=CreateDIBSection(reference?reference:s->dc,&info,DIB_RGB_COLORS,(void **)&s->pixels,NULL,0);
    if(!s->bitmap){style_bitmap_dispose(s);return FALSE;}
    s->old_bitmap=SelectObject(s->dc,s->bitmap);s->size=size;
    if(!s->old_bitmap || s->old_bitmap==HGDI_ERROR){s->old_bitmap=NULL;style_bitmap_dispose(s);return FALSE;}
    return TRUE;
}
