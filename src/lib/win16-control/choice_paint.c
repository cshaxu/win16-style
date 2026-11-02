#include "choice_paint.h"
#include "control_glyphs.h"
#include <stdlib.h>

BOOL win16_control_checklist_glyph(HDC dc,const RECT *rect,UINT flags)
{
    int width=rect->right-rect->left,height=rect->bottom-rect->top;
    if(width<=0 || height<=0 || width>256 || height>256)return FALSE;
    DWORD *pixels=calloc((size_t)width*(size_t)height,sizeof(*pixels));if(!pixels)return FALSE;
    int column=((flags&DFCS_CHECKED)?1:0)+((flags&DFCS_PUSHED)?2:0);
    for(int y=0;y<13;y++)for(int x=0;x<14;x++){
        int left=MulDiv(x,width,14),right=MulDiv(x+1,width,14);
        int top=MulDiv(y,height,13),bottom=MulDiv(y+1,height,13);
        DWORD color=control_checks[y][column*14+x]=='0'?0:0x00ffffff;
        for(int py=top;py<bottom;py++)for(int px=left;px<right;px++)pixels[py*width+px]=color;
    }
    BITMAPINFO info={0};info.bmiHeader.biSize=sizeof(info.bmiHeader);
    info.bmiHeader.biWidth=width;info.bmiHeader.biHeight=-height;info.bmiHeader.biPlanes=1;info.bmiHeader.biBitCount=32;
    int drawn=SetDIBitsToDevice(dc,rect->left,rect->top,(DWORD)width,(DWORD)height,0,0,0,(UINT)height,pixels,&info,DIB_RGB_COLORS);
    free(pixels);return drawn!=0;
}
void win16_control_choice_glyph(HDC dc,RECT glyph,UINT dpi,int row,int column,const win16_style_palette *palette)
{
    RECT pixel;
    for(int y=0;y<13;++y)for(int x=0;x<14;++x){
        SetDCBrushColor(dc,control_checks[row*13+y][column*14+x]=='0'?palette->edge:palette->light);
        SetRect(&pixel,glyph.left+MulDiv(x,(int)dpi,96),glyph.top+MulDiv(y,(int)dpi,96),
            glyph.left+MulDiv(x+1,(int)dpi,96),glyph.top+MulDiv(y+1,(int)dpi,96));
        FillRect(dc,&pixel,(HBRUSH)GetStockObject(DC_BRUSH));
    }
}
