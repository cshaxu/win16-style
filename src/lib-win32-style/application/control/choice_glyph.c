#include "choice_glyph.h"
#include "lib-win32-style/application/graphics/pixel_grid.h"
#include "lib-win32-style/application/graphics/pixel_dib.h"

BOOL style_control_checklist_glyph(HDC dc,const RECT *rect,UINT flags,style_check_glyph_read read)
{
    int width=rect->right-rect->left,height=rect->bottom-rect->top;
    if(width<=0 || height<=0 || width>256 || height>256)return FALSE;
    style_check_glyph glyph={0};
    if(!read(flags,&glyph))return FALSE;
    COLORREF ink=glyph.ink,paper=glyph.paper;
    const DWORD colors[]={RGB(GetBValue(ink),GetGValue(ink),GetRValue(ink)),
        RGB(GetBValue(paper),GetGValue(paper),GetRValue(paper))};
    return style_paint_indexed_dib(dc,rect,glyph.art.cell,CLASSIC_CHOICE_WIDTH,
        (SIZE){CLASSIC_CHOICE_WIDTH,CLASSIC_CHOICE_HEIGHT},colors);
}
void style_control_choice_glyph(HDC dc,RECT glyph,UINT dpi,UINT flags,const style_window_palette *palette,
    const char *image)
{
    if(!image){
        DrawFrameControl(dc,&glyph,DFC_BUTTON,flags);
        return;
    }
    const COLORREF colors[2]={palette->edge,palette->light};
    const style_pixel_grid grid={image,CLASSIC_CHOICE_WIDTH,CLASSIC_CHOICE_HEIGHT,CLASSIC_CHOICE_WIDTH,
        -1,colors,{glyph.left,glyph.top},dpi?(SIZE){(int)dpi,(int)dpi}:
            (SIZE){glyph.right-glyph.left,glyph.bottom-glyph.top},
        dpi?(SIZE){96,96}:(SIZE){CLASSIC_CHOICE_WIDTH,CLASSIC_CHOICE_HEIGHT}};
    style_paint_pixel_grid(dc,&grid);
}
