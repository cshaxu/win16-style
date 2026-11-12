#include "lib-style/common/menu_input.h"

static BOOL arrow_test(void)
{
    HMENU root=CreatePopupMenu(),child=CreatePopupMenu(),nested=CreatePopupMenu();
    if(!root || !child || !nested){if(root)DestroyMenu(root);if(child)DestroyMenu(child);if(nested)DestroyMenu(nested);return FALSE;}
    BOOL ok=AppendMenuW(root,MF_POPUP,(UINT_PTR)child,L"child") &&
        style_menu_bar_arrow(NULL,root,root,VK_LEFT,FALSE) &&
        style_menu_bar_arrow(NULL,root,root,VK_RIGHT,FALSE) &&
        !style_menu_bar_arrow(NULL,root,nested,VK_LEFT,TRUE) &&
        !style_menu_bar_arrow(NULL,NULL,root,VK_RIGHT,TRUE) &&
        !style_menu_bar_arrow(NULL,root,root,VK_UP,FALSE);
    MENUITEMINFOW item={.cbSize=sizeof(item),.fMask=MIIM_STATE,.fState=MFS_HILITE};
    ok=ok && SetMenuItemInfoW(root,0,TRUE,&item) &&
        !style_menu_bar_arrow(NULL,root,root,VK_RIGHT,FALSE) &&
        !style_menu_bar_arrow(NULL,root,root,VK_RIGHT,TRUE);
    item.fState=MFS_HILITE|MFS_DISABLED;
    ok=ok && SetMenuItemInfoW(root,0,TRUE,&item) &&
        style_menu_bar_arrow(NULL,root,root,VK_RIGHT,FALSE);
    DestroyMenu(nested);DestroyMenu(root); /* root owns child after AppendMenu */
    return ok;
}

int main(void)
{
    if(style_menu_neutral_hit(HTMENU)!=HTBORDER || style_menu_neutral_hit(HTCLIENT)!=HTCLIENT ||
       style_menu_neutral_hit(HTTRANSPARENT)!=HTTRANSPARENT)return 1;
    if(style_menu_route_click(WM_NCLBUTTONDOWN,HTBORDER,TRUE)!=HTMENU ||
       style_menu_route_click(WM_NCLBUTTONDBLCLK,HTBORDER,TRUE)!=HTMENU)return 2;
    if(style_menu_route_click(WM_NCLBUTTONDOWN,HTBORDER,FALSE)!=HTBORDER ||
       style_menu_route_click(WM_NCMOUSEMOVE,HTBORDER,TRUE)!=HTBORDER ||
       style_menu_route_click(WM_NCLBUTTONDOWN,HTCAPTION,TRUE)!=HTCAPTION)return 3;
    if(!style_menu_mnemonic(L"&File",L'f',TRUE) || style_menu_mnemonic(L"Save && Exit",L'e',TRUE) ||
       style_menu_mnemonic(L"File\t&X",L'x',TRUE) || !style_menu_mnemonic(L"File\t&X",L'x',FALSE) ||
       !style_menu_mnemonic(L"&& &Edit",L'e',TRUE))return 4;
    if(style_menu_key(VK_LEFT)!=STYLE_MENU_KEY_PREVIOUS || style_menu_key(VK_RIGHT)!=STYLE_MENU_KEY_NEXT ||
       style_menu_key(VK_ESCAPE)!=STYLE_MENU_KEY_LEAVE || style_menu_key(VK_F10)!=STYLE_MENU_KEY_LEAVE ||
       style_menu_key(VK_DOWN)!=STYLE_MENU_KEY_OPEN || style_menu_key(VK_RETURN)!=STYLE_MENU_KEY_OPEN ||
       style_menu_key(VK_SPACE)!=STYLE_MENU_KEY_NONE)return 5;
    int target=99;
    if(style_menu_pointer(WM_MOUSEMOVE,-3,1,-3,-3,TRUE,&target) || target!=99 ||
       style_menu_pointer(WM_MOUSEMOVE,1,1,-3,-3,TRUE,&target) ||
       !style_menu_pointer(WM_MOUSEMOVE,-2,1,-3,-3,TRUE,&target) || target!=-2 ||
       !style_menu_pointer(WM_LBUTTONDOWN,1,1,-3,-3,TRUE,&target) || target!=-3 ||
       style_menu_pointer(WM_LBUTTONDOWN,1,1,-1,-1,FALSE,&target) ||
       !style_menu_pointer(WM_NCLBUTTONDOWN,1,1,-1,-1,TRUE,&target) || target!=-1 ||
       !style_menu_pointer(WM_NCMOUSEMOVE,2,1,-1,-1,FALSE,&target) || target!=2)return 6;
    if(!arrow_test())return 7;
    return 0;
}
