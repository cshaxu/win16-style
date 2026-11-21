#ifndef STYLE_MSSTYLES_NATIVE_SCROLL_STYLE_H
#define STYLE_MSSTYLES_NATIVE_SCROLL_STYLE_H
#include "classic_theme.h"
#include "lib-win32-style/application/control/native_scroll.h"

static inline void classic_theme_native_scroll(style_native_scroll *scroll,const classic_theme *theme)
{
    style_relief_palette arrows=classic_theme_relief_palette(theme);
    arrows.face=theme->scroll_arrow.face;
    arrows.dark=theme->scroll_arrow.edge;
    style_native_scroll_configure(scroll,arrows,&theme->scroll_art,&theme->scroll,theme->scroll_metrics);
}
#endif
