#include "surface.h"
#include <stdint.h>

BOOL style_uwp_frame_bounds(HWND window,HWND title,RECT *outer,RECT *content,BOOL *zoomed)
{
    RECT title_rect;
    if(!GetWindowRect(window,outer) || !GetClientRect(window,content))return FALSE;
    if(title && (!IsWindow(title) || GetParent(title)!=window ||
       !IsWindowVisible(title) || !GetWindowRect(title,&title_rect)))return FALSE;
    *zoomed=IsZoomed(window);
    if(*zoomed){
        MONITORINFO monitor={.cbSize=sizeof(monitor)};RECT visible;
        if(!GetMonitorInfoW(MonitorFromWindow(window,MONITOR_DEFAULTTONEAREST),&monitor) ||
           !IntersectRect(&visible,outer,&monitor.rcWork))return FALSE;
        *outer=visible;
    }
    POINT origin={0,0};ClientToScreen(window,&origin);
    OffsetRect(content,origin.x-outer->left,origin.y-outer->top);
    if(title){
        if(title_rect.left<origin.x || title_rect.right!=outer->left+content->right || title_rect.bottom<=origin.y)return FALSE;
        content->top=title_rect.bottom-outer->top;
    }
    if(*zoomed){
        RECT bounds={0,0,outer->right-outer->left,outer->bottom-outer->top};
        if(!IntersectRect(content,content,&bounds))return FALSE;
    }
    return TRUE;
}

void style_uwp_bitmap_dispose(style_uwp_bitmap *s)
{
    if(s->old_bitmap)SelectObject(s->dc,s->old_bitmap);
    if(s->bitmap)DeleteObject(s->bitmap);
    if(s->dc)DeleteDC(s->dc);
    s->dc=NULL;s->bitmap=NULL;s->old_bitmap=NULL;s->pixels=NULL;
}
BOOL style_uwp_bitmap_ensure(style_uwp_bitmap *s,SIZE size)
{
    if(s->dc && size.cx==s->size.cx && size.cy==s->size.cy)return TRUE;
    if(size.cx<=0 || size.cy<=0 || (SIZE_T)size.cx>SIZE_MAX/4/(SIZE_T)size.cy){SetLastError(ERROR_INVALID_PARAMETER);return FALSE;}
    style_uwp_bitmap_dispose(s);
    BITMAPINFO info={0};info.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
    info.bmiHeader.biWidth=size.cx;info.bmiHeader.biHeight=-size.cy;
    info.bmiHeader.biPlanes=1;info.bmiHeader.biBitCount=32;
    s->dc=CreateCompatibleDC(NULL);
    if(s->dc)s->bitmap=CreateDIBSection(s->dc,&info,DIB_RGB_COLORS,(void **)&s->pixels,NULL,0);
    if(!s->bitmap){style_uwp_bitmap_dispose(s);return FALSE;}
    s->old_bitmap=SelectObject(s->dc,s->bitmap);s->size=size;
    if(!s->old_bitmap || s->old_bitmap==HGDI_ERROR){s->old_bitmap=NULL;style_uwp_bitmap_dispose(s);return FALSE;}
    return TRUE;
}
