#ifndef STYLE_MSSTYLES_NATIVE_SCROLL_STYLE_H
#define STYLE_MSSTYLES_NATIVE_SCROLL_STYLE_H
#include "classic_theme.h"
#include "lib-win32-style/application/control/native_scroll.h"

static inline style_relief_palette classic_theme_native_scroll_palette(const classic_theme *theme,style_relief_palette arrows)
{
    arrows.face=theme->scroll_arrow.face;
    arrows.dark=theme->scroll_arrow.edge;
    return arrows;
}

static inline void classic_theme_native_scroll(style_native_scroll *scroll,const classic_theme *theme)
{
    style_relief_palette palette=classic_theme_native_scroll_palette(theme,classic_theme_relief_palette(theme));
    style_native_scroll_configure(scroll,palette,&theme->scroll_art,&theme->scroll,theme->scroll_metrics);
}
#endif
