#include "lib-win32/style-common/menu_hit.h"

int main(void)
{
    if(style_menu_neutral_hit(HTMENU)!=HTBORDER || style_menu_neutral_hit(HTCLIENT)!=HTCLIENT ||
       style_menu_neutral_hit(HTTRANSPARENT)!=HTTRANSPARENT)return 1;
    if(style_menu_route_click(WM_NCLBUTTONDOWN,HTBORDER,TRUE)!=HTMENU ||
       style_menu_route_click(WM_NCLBUTTONDBLCLK,HTBORDER,TRUE)!=HTMENU)return 2;
    if(style_menu_route_click(WM_NCLBUTTONDOWN,HTBORDER,FALSE)!=HTBORDER ||
       style_menu_route_click(WM_NCMOUSEMOVE,HTBORDER,TRUE)!=HTBORDER ||
       style_menu_route_click(WM_NCLBUTTONDOWN,HTCAPTION,TRUE)!=HTCAPTION)return 3;
    return 0;
}
