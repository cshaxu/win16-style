#include "lib-win32-style/application/graphics/pixel_grid.h"
#include "lib-win32-style/application/graphics/pixel_dib.h"
#include "choice_paint.h"
#include "lib-win32-style/appearance/msstyles/classic_theme.h"
extern IMAGE_DOS_HEADER __ImageBase;

BOOL win16_control_checklist_glyph(HDC dc,const RECT *rect,UINT flags)
{
    int width=rect->right-rect->left,height=rect->bottom-rect->top;
    if(width<=0 || height<=0 || width>256 || height>256)return FALSE;
    classic_theme_mapping source={0};
    if(classic_theme_attachment_layout((HMODULE)&__ImageBase,CLASSIC_WINDOW_PROVIDER_FULL_CLIENT,&source) || !source.resources)
        return FALSE;
    const classic_choice_surface *entry=classic_choice_entry(&source.model.choice,FALSE,
        flags&DFCS_CHECKED?BST_CHECKED:BST_UNCHECKED,FALSE,flags&DFCS_PUSHED?BST_PUSHED:0);
    classic_choice_art art={0};
    const char *image=classic_choice_art_get(source.resources,entry->image,&art);
    const classic_surface_colors *configured=classic_choice_color(entry);
    COLORREF ink=configured?configured->edge:GetSysColor(COLOR_WINDOWTEXT);
    COLORREF paper=configured?configured->face:GetSysColor(COLOR_WINDOW);
    classic_theme_close(&source);
    const DWORD colors[]={RGB(GetBValue(ink),GetGValue(ink),GetRValue(ink)),
        RGB(GetBValue(paper),GetGValue(paper),GetRValue(paper))};
    return image && style_paint_indexed_dib(dc,rect,image,CLASSIC_CHOICE_WIDTH,
        (SIZE){CLASSIC_CHOICE_WIDTH,CLASSIC_CHOICE_HEIGHT},colors);
}
void win16_control_choice_glyph(HDC dc,RECT glyph,UINT dpi,UINT flags,const win16_style_palette *palette,
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
