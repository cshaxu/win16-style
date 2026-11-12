#ifndef STYLE_UWP_MENU_H
#define STYLE_UWP_MENU_H
#include <windows.h>
#include "lib-style/common/menu_snapshot.h"

typedef struct style_uwp_menu_state {
    UINT saved[5];
    BOOL disabled[5];
} style_uwp_menu_state;

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

/* All calls belong to the menu owner's GUI thread. The caller keeps the menu
 * alive and restores its state only while it is still the owner's system menu. */
static inline void style_uwp_menu_restrict(HWND window,HMENU menu,style_uwp_menu_state *state)
{
    static const UINT ids[]={SC_RESTORE,SC_MOVE,SC_SIZE,SC_MINIMIZE,SC_MAXIMIZE};
    DWORD style=(DWORD)GetWindowLongPtrW(window,GWL_STYLE);
    BOOL zoomed=IsZoomed(window),iconic=IsIconic(window);
    const BOOL disabled[]={ !zoomed&&!iconic,zoomed,zoomed||iconic||!(style&WS_THICKFRAME),
        iconic||!(style&WS_MINIMIZEBOX),zoomed||!(style&WS_MAXIMIZEBOX) };
    for(unsigned i=0;i<ARRAYSIZE(ids);++i){
        state->disabled[i]=disabled[i];state->saved[i]=GetMenuState(menu,ids[i],MF_BYCOMMAND);
        if(state->saved[i]!=(UINT)-1 && disabled[i])EnableMenuItem(menu,ids[i],MF_BYCOMMAND|MF_GRAYED);
    }
}
static inline void style_uwp_menu_restore(HMENU menu,const style_uwp_menu_state *state)
{
    static const UINT ids[]={SC_RESTORE,SC_MOVE,SC_SIZE,SC_MINIMIZE,SC_MAXIMIZE};
    for(unsigned i=0;i<ARRAYSIZE(ids);++i)if(state->saved[i]!=(UINT)-1 && state->disabled[i])
        EnableMenuItem(menu,ids[i],MF_BYCOMMAND|(state->saved[i]&(MF_DISABLED|MF_GRAYED)));
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
