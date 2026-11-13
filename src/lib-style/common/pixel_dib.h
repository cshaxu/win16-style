#ifndef STYLE_PIXEL_DIB_H
#define STYLE_PIXEL_DIB_H
#include <windows.h>
#include <stdint.h>
#include <stdlib.h>

/* ASCII indexed rows; colors are 32-bit BI_RGB pixels (0x00RRGGBB).
 * The caller owns the source and target DC; the temporary buffer is freed. */
static inline BOOL style_paint_indexed_dib(HDC dc,const RECT *target,const char *source,int stride,SIZE size,const DWORD *colors)
{
    int width=target->right-target->left,height=target->bottom-target->top;
    if(width<=0 || height<=0 || (size_t)width>SIZE_MAX/sizeof(DWORD)/(size_t)height)return FALSE;
    DWORD *pixels=calloc((size_t)width*(size_t)height,sizeof(*pixels));if(!pixels)return FALSE;
    for(int y=0;y<size.cy;++y)for(int x=0;x<size.cx;++x){
        int left=MulDiv(x,width,size.cx),right=MulDiv(x+1,width,size.cx);
        int top=MulDiv(y,height,size.cy),bottom=MulDiv(y+1,height,size.cy);
        DWORD color=colors[source[y*stride+x]-'0'];
        for(int py=top;py<bottom;++py)for(int px=left;px<right;++px)pixels[(size_t)py*(size_t)width+(size_t)px]=color;
    }
    BITMAPINFO info={0};info.bmiHeader.biSize=sizeof(info.bmiHeader);
    info.bmiHeader.biWidth=width;info.bmiHeader.biHeight=-height;info.bmiHeader.biPlanes=1;info.bmiHeader.biBitCount=32;
    int drawn=SetDIBitsToDevice(dc,target->left,target->top,(DWORD)width,(DWORD)height,0,0,0,(UINT)height,pixels,&info,DIB_RGB_COLORS);
    free(pixels);return drawn!=0;
}
#endif
