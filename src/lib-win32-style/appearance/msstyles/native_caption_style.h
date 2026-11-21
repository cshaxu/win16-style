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
    return style_native_caption_measure_height(dpi,tool,theme?&theme->caption_metrics:NULL,
        theme?theme->border_presentation.geometry.caption_floor:0);
}

static inline void classic_theme_frame_border_paint(HDC dc,int width,int height,int thickness,const classic_theme *theme,BOOL active,BOOL dialog)
{
    if(theme)style_native_frame_border_paint(dc,(RECT){0,0,width,height},thickness,&theme->border_presentation,
        &theme->frame,classic_theme_caption_palette(active,theme).left,dialog);
}
#endif
