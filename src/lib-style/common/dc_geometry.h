#ifndef STYLE_COMMON_DC_GEOMETRY_H
#define STYLE_COMMON_DC_GEOMETRY_H
#include <windows.h>
static inline BOOL style_dc_pixel_viewport(HDC dc)
{
    POINT origin;
    return GetMapMode(dc)==MM_TEXT && !(GetLayout(dc)&LAYOUT_RTL) &&
        GetViewportOrgEx(dc,&origin) && !origin.x && !origin.y;
}

/* Exact, unmirrored pixel coordinates, including both DC origins. */
static inline BOOL style_dc_origin_pixels(HDC dc)
{
    POINT origin,viewport;
    return GetMapMode(dc)==MM_TEXT && GetLayout(dc)==0 &&
        GetWindowOrgEx(dc,&origin) && GetViewportOrgEx(dc,&viewport) &&
        !origin.x && !origin.y && !viewport.x && !viewport.y;
}
#endif
