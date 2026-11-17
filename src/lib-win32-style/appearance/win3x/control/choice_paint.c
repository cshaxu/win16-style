#include "lib-win32-style/application/graphics/pixel_grid.h"
#include "lib-win32-style/application/graphics/pixel_dib.h"
#include "choice_paint.h"
#include "../resources/control_glyphs.h"

BOOL win16_control_checklist_glyph(HDC dc,const RECT *rect,UINT flags)
{
    int width=rect->right-rect->left,height=rect->bottom-rect->top;
    if(width<=0 || height<=0 || width>256 || height>256)return FALSE;
    int column=((flags&DFCS_CHECKED)?1:0)+((flags&DFCS_PUSHED)?2:0);
    const DWORD colors[2]={0,0x00ffffff};
    return style_paint_indexed_dib(dc,rect,&control_checks[0][column*14],57,(SIZE){14,13},colors);
}
void win16_control_choice_glyph(HDC dc,RECT glyph,UINT dpi,int row,int column,const win16_style_palette *palette)
{
    const COLORREF colors[2]={palette->edge,palette->light};
    const style_pixel_grid grid={&control_checks[row*13][column*14],14,13,57,-1,colors,{glyph.left,glyph.top},{(int)dpi,(int)dpi},{96,96}};
    style_paint_pixel_grid(dc,&grid);
}
