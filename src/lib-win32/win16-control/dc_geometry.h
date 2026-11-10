#ifndef WIN16_CONTROL_DC_GEOMETRY_H
#define WIN16_CONTROL_DC_GEOMETRY_H
#include <windows.h>
static inline BOOL win16_control_pixel_viewport(HDC dc)
{
    POINT origin;
    return GetMapMode(dc)==MM_TEXT && !(GetLayout(dc)&LAYOUT_RTL) &&
        GetViewportOrgEx(dc,&origin) && !origin.x && !origin.y;
}
#endif
