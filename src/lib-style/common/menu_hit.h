#ifndef STYLE_COMMON_MENU_HIT_H
#define STYLE_COMMON_MENU_HIT_H
#include <windows.h>

/* USER hot-tracks HTMENU before dispatch; custom menu bars own that input. */
static inline LRESULT style_menu_neutral_hit(LRESULT hit)
{
    return hit==HTMENU?HTBORDER:hit;
}

static inline WPARAM style_menu_route_click(UINT message,WPARAM hit,BOOL inside)
{
    return inside && hit==HTBORDER &&
        (message==WM_NCLBUTTONDOWN || message==WM_NCLBUTTONDBLCLK)?HTMENU:hit;
}
#endif
