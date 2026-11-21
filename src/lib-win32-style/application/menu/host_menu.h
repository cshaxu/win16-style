#ifndef STYLE_UWP_MENU_H
#define STYLE_UWP_MENU_H
#include <windows.h>
#include "lib-win32-style/application/menu/menu_snapshot.h"

typedef struct style_system_menu_session {
    HWND owner;
    HMENU menu;
    UINT saved[5];
    BOOL disabled[5];
} style_system_menu_session;

/* GUI-thread-owned, borrowed system menu. Cleanup restores state only if the
 * original window still owns this HMENU, even after the frame is removed. */
void style_system_menu_begin(style_system_menu_session *,HWND owner,HMENU menu);
UINT style_system_menu_track(const style_system_menu_session *,HWND popup_owner,POINT anchor);
void style_system_menu_end(style_system_menu_session *);

/* USER may temporarily own menu animation; do not modify an already layered popup. */
static inline void style_uwp_menu_present(HWND window,HMENU menu,HWND *presented)
{
    if(GetWindowThreadProcessId(window,NULL)!=GetCurrentThreadId() ||
       !style_menu_popup_is(window,menu))return;
    LONG_PTR ex=GetWindowLongPtrW(window,GWL_EXSTYLE);
    if(ex&WS_EX_LAYERED)return;
    *presented=window;
    SetLastError(0);SetWindowLongPtrW(window,GWL_EXSTYLE,ex|WS_EX_LAYERED);
    if(GetLastError() || !SetLayeredWindowAttributes(window,0,255,LWA_ALPHA)){
        SetWindowLongPtrW(window,GWL_EXSTYLE,ex);*presented=NULL;return;
    }
    RedrawWindow(window,NULL,NULL,RDW_INVALIDATE|RDW_FRAME|RDW_UPDATENOW);
}
static inline void style_uwp_menu_unpresent(HWND window,HMENU menu)
{
    MENUBARINFO info={.cbSize=sizeof(info)};
    if(GetMenuBarInfo(window,OBJID_CLIENT,0,&info) && info.hMenu==menu)
        SetWindowLongPtrW(window,GWL_EXSTYLE,GetWindowLongPtrW(window,GWL_EXSTYLE)&~WS_EX_LAYERED);
}

static inline UINT style_uwp_menu_add(HMENU menu,PCWSTR label,void *owner)
{
    for(UINT id=0xeff0;id>=0xe000;id-=16)if(GetMenuState(menu,id,MF_BYCOMMAND)==(UINT)-1){
        MENUITEMINFOW item={.cbSize=sizeof(item),.fMask=MIIM_ID|MIIM_STRING|MIIM_DATA,
            .wID=id,.dwTypeData=(PWSTR)label,.dwItemData=(ULONG_PTR)owner};
        return InsertMenuItemW(menu,(UINT)-1,TRUE,&item)?id:0;
    }
    return 0;
}
static inline void style_uwp_menu_remove(HMENU menu,UINT id,void *owner)
{
    MENUITEMINFOW item={.cbSize=sizeof(item),.fMask=MIIM_DATA};
    if(GetMenuItemInfoW(menu,id,FALSE,&item) && item.dwItemData==(ULONG_PTR)owner)
        DeleteMenu(menu,id,MF_BYCOMMAND);
}
#endif
