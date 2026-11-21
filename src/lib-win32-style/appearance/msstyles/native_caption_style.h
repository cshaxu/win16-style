#ifndef STYLE_MSSTYLES_NATIVE_CAPTION_STYLE_H
#define STYLE_MSSTYLES_NATIVE_CAPTION_STYLE_H
#include "classic_theme.h"
#include "lib-win32-style/application/window/native_caption.h"

static inline style_native_caption_colors classic_theme_caption_palette(BOOL active,const classic_theme *theme)
{
    if(!theme)return style_native_caption_palette(active);
    classic_caption_colors colors=active?theme->active:theme->inactive;
    return (style_native_caption_colors){colors.start,colors.end,colors.text,theme->gradient};
}

static inline int classic_theme_caption_height(UINT dpi,BOOL tool,const classic_theme *theme)
{
    int height=theme?(tool?theme->caption_metrics.tool_height:theme->caption_metrics.height):0;
    int pixels=height>0?MulDiv(height,(int)dpi,96):style_native_caption_height(dpi,tool);
    return theme?max(pixels,theme->border_presentation.geometry.caption_floor):pixels;
}

static inline void classic_theme_frame_border_paint(HDC dc,int width,int height,int thickness,const classic_theme *theme,BOOL active,BOOL dialog)
{
    if(!theme || !theme->border_presentation.fill)return;
    const classic_frame_colors frame=style_native_frame_colors(&theme->frame);
    const style_border_palette palette={frame.face,frame.edge,frame.light,frame.shadow,
        classic_theme_caption_palette(active,theme).left};
    style_frame_border_paint(dc,(RECT){0,0,width,height},thickness,1,dialog,NULL,&theme->border_presentation,&palette);
}
#endif
