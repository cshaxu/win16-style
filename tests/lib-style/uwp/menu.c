#include "lib-style/uwp/menu.h"
#include <stdio.h>
#include <string.h>

#define CHECK(x) do{if(!(x)){fprintf(stderr,"UWP menu line %d error %lu\n",__LINE__,(unsigned long)GetLastError());return 1;}}while(0)
static HMENU observed_menu;
static BOOL popup_seen,popup_restored,observing;
static LRESULT CALLBACK observe_popup(int code,WPARAM wp,LPARAM lp)
{
    if(code>=0 && lp && !popup_seen && !observing){
        const CWPSTRUCT *message=(const CWPSTRUCT *)lp;HWND presented=NULL;
        observing=TRUE;
        style_uwp_menu_present(message->hwnd,observed_menu,&presented);
        if(presented){
            popup_seen=(GetWindowLongPtrW(presented,GWL_EXSTYLE)&WS_EX_LAYERED)!=0;
            style_uwp_menu_unpresent(presented,observed_menu);
            popup_restored=(GetWindowLongPtrW(presented,GWL_EXSTYLE)&WS_EX_LAYERED)==0;
        }
        observing=FALSE;
    }
    return CallNextHookEx(NULL,code,wp,lp);
}
static void CALLBACK stop_menu(HWND window,UINT message,UINT_PTR id,DWORD time)
{(void)window;(void)message;(void)id;(void)time;EndMenu();}
static BOOL test_popup(HWND owner,HMENU menu)
{
    observed_menu=menu;
    HHOOK hook=SetWindowsHookExW(WH_CALLWNDPROC,observe_popup,NULL,GetCurrentThreadId());
    if(!hook)return FALSE;
    UINT_PTR timer=SetTimer(owner,1,500,stop_menu);
    if(timer){
        ShowWindow(owner,SW_SHOWNOACTIVATE);
        TrackPopupMenuEx(menu,TPM_RETURNCMD|TPM_NOANIMATION,0,0,owner,NULL);
        KillTimer(owner,timer);
    }
    UnhookWindowsHookEx(hook);return timer && popup_seen && popup_restored;
}
int main(int argc,char **argv)
{
    HWND w=CreateWindowW(L"STATIC",L"",WS_OVERLAPPEDWINDOW,0,0,100,100,NULL,NULL,NULL,NULL);CHECK(w);
    HMENU system=GetSystemMenu(w,FALSE),menu=CreatePopupMenu();CHECK(system && menu);
    if(argc>1 && !strcmp(argv[1],"--popup")){
        CHECK(test_popup(w,system));DestroyMenu(menu);DestroyWindow(w);
        puts("UWP_MENU_POPUP_OK attached=1 restored=1");return 0;
    }
    HWND presented=NULL;LONG_PTR ex=GetWindowLongPtrW(w,GWL_EXSTYLE);
    style_uwp_menu_present(w,system,&presented);
    CHECK(!presented && GetWindowLongPtrW(w,GWL_EXSTYLE)==ex);
    style_uwp_menu_unpresent(w,menu);
    CHECK(GetWindowLongPtrW(w,GWL_EXSTYLE)==ex);
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
