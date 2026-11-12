#ifndef STYLE_COMMON_DC_GEOMETRY_H
#define STYLE_COMMON_DC_GEOMETRY_H
#include <windows.h>
static inline BOOL style_dc_pixel_viewport(HDC dc)
{
    POINT origin;
    return GetMapMode(dc)==MM_TEXT && !(GetLayout(dc)&LAYOUT_RTL) &&
        GetViewportOrgEx(dc,&origin) && !origin.x && !origin.y;
}
#endif
