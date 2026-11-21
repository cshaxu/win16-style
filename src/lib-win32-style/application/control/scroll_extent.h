#ifndef STYLE_SCROLL_EXTENT_H
#define STYLE_SCROLL_EXTENT_H
#include "scroll_geometry.h"

static inline int style_scroll_nominal(style_scroll_metrics metrics,enum style_scroll_extent_source fallback,
    const RECT *bar,BOOL vertical,UINT dpi)
{
    unsigned source=metrics.source?metrics.source:fallback;
    if(source==STYLE_SCROLL_EXTENT_FIXED)return max(1,MulDiv(metrics.logical_extent,(int)dpi,96));
    if(source==STYLE_SCROLL_EXTENT_CROSS_AXIS)return vertical?bar->right-bar->left:bar->bottom-bar->top;
    return GetSystemMetricsForDpi(vertical?SM_CYVSCROLL:SM_CXHSCROLL,dpi);
}
#endif
