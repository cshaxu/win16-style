#include "lib-style/uwp/menu.h"
#include <stdio.h>

#define CHECK(x) do{if(!(x)){fprintf(stderr,"UWP menu line %d error %lu\n",__LINE__,(unsigned long)GetLastError());return 1;}}while(0)
int main(void)
{
    HWND w=CreateWindowW(L"STATIC",L"",WS_OVERLAPPEDWINDOW,0,0,100,100,NULL,NULL,NULL,NULL);CHECK(w);
    HMENU system=GetSystemMenu(w,FALSE),menu=CreatePopupMenu();CHECK(system && menu);
    UINT before=GetMenuState(system,SC_RESTORE,MF_BYCOMMAND);style_uwp_menu_state state;
    style_uwp_menu_restrict(w,system,&state);
    CHECK((GetMenuState(system,SC_RESTORE,MF_BYCOMMAND)&MF_GRAYED)!=0);
    style_uwp_menu_restore(system,&state);
    CHECK((GetMenuState(system,SC_RESTORE,MF_BYCOMMAND)&(MF_DISABLED|MF_GRAYED))==(before&(MF_DISABLED|MF_GRAYED)));
    CHECK(AppendMenuW(menu,MF_STRING,0xeff0,L"Existing"));int owner=1,other=2;
    UINT id=style_uwp_menu_add(menu,L"Action",&owner);CHECK(id==0xefe0);
    style_uwp_menu_remove(menu,id,&other);CHECK(GetMenuState(menu,id,MF_BYCOMMAND)!=(UINT)-1);
    style_uwp_menu_remove(menu,id,&owner);CHECK(GetMenuState(menu,id,MF_BYCOMMAND)==(UINT)-1);
    for(UINT i=0xe000;i<0xeff0;i+=16)CHECK(AppendMenuW(menu,MF_STRING,i,L"Occupied"));
    CHECK(!style_uwp_menu_add(menu,L"No space",&owner));
    DestroyMenu(menu);DestroyWindow(w);puts("UWP_MENU_OK restore=1 collision=1 ownership=1 full=1");return 0;
}
