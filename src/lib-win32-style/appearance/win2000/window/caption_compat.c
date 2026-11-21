/* Legacy caption fixtures and the standalone native fallback. */
#include "caption.h"

BOOL win2k_caption_system_buttons(const RECT *title,UINT dpi,BOOL tool,
    const BOOL present[WIN2K_CAPTION_PARTS],RECT bounds[WIN2K_CAPTION_PARTS])
{
    static const classic_button_group group={4,0,0,{
        {CLASSIC_BUTTON_MINIMIZE,CLASSIC_BUTTON_RIGHT,0},
        {CLASSIC_BUTTON_MAXIMIZE,CLASSIC_BUTTON_RIGHT,0},
        {CLASSIC_BUTTON_HELP,CLASSIC_BUTTON_RIGHT,0},
        {CLASSIC_BUTTON_CLOSE,CLASSIC_BUTTON_RIGHT,0}},0};
    return win2k_caption_buttons_with_group(title,dpi,tool,present,&group,NULL,bounds);
}

BOOL win2k_caption_buttons_with_group(const RECT *title,UINT dpi,BOOL tool,
    const BOOL present[WIN2K_CAPTION_PARTS],const classic_button_group *group,
    const classic_caption_metrics *metrics,RECT bounds[WIN2K_CAPTION_PARTS])
{
    return win2k_caption_buttons_with_system(title,dpi,tool,present,group,metrics,bounds,NULL);
}

BOOL win2k_caption_paint(HDC dc,const RECT *bounds,PCWSTR title,HFONT font,
                         const win2k_caption_colors *colors)
{
    if(!dc || !bounds || !title || !font || !colors ||
       bounds->right<=bounds->left || bounds->bottom<=bounds->top)return FALSE;
    int saved=SaveDC(dc);if(!saved)return FALSE;
    BOOL ok=style_caption_fill(dc,*bounds,colors->left,colors->right,colors->gradient) &&
        style_caption_title(dc,*bounds,title,font,colors->text,STYLE_CAPTION_TEXT_LEFT_ELLIPSIS);
    if(!RestoreDC(dc,saved))ok=FALSE;
    return ok;
}

void win2k_caption_render(HDC dc,const win2k_caption_layout *layout,PCWSTR title,HICON icon,
                         HFONT font,UINT dpi,BOOL active,BOOL maximized,
                         const UINT button_flags[WIN2K_CAPTION_PARTS])
{
    win2k_caption_render_with_colors(dc,layout,title,icon,font,dpi,win2k_caption_palette(active),
        NULL,NULL,maximized,button_flags,NULL);
}
