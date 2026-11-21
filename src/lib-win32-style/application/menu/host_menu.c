#include "host_menu.h"

static const UINT commands[]={SC_RESTORE,SC_MOVE,SC_SIZE,SC_MINIMIZE,SC_MAXIMIZE};

void style_system_menu_begin(style_system_menu_session *state,HWND owner,HMENU menu)
{
    state->owner=owner;state->menu=menu;
    DWORD style=(DWORD)GetWindowLongPtrW(owner,GWL_STYLE);
    BOOL zoomed=IsZoomed(owner),iconic=IsIconic(owner);
    const BOOL disabled[]={ !zoomed&&!iconic,zoomed,zoomed||iconic||!(style&WS_THICKFRAME),
        iconic||!(style&WS_MINIMIZEBOX),zoomed||!(style&WS_MAXIMIZEBOX) };
    for(unsigned i=0;i<ARRAYSIZE(commands);++i){
        state->disabled[i]=disabled[i];
        state->saved[i]=GetMenuState(menu,commands[i],MF_BYCOMMAND);
        if(state->saved[i]!=(UINT)-1 && disabled[i])EnableMenuItem(menu,commands[i],MF_BYCOMMAND|MF_GRAYED);
    }
}

static BOOL owns_menu(const style_system_menu_session *state)
{return state->menu && IsWindow(state->owner) && GetSystemMenu(state->owner,FALSE)==state->menu;}

UINT style_system_menu_track(const style_system_menu_session *state,HWND popup_owner,POINT anchor)
{
    if(!owns_menu(state))return 0;
    return TrackPopupMenuEx(state->menu,TPM_LEFTALIGN|TPM_TOPALIGN|TPM_RETURNCMD|TPM_NOANIMATION,
        anchor.x,anchor.y,popup_owner,NULL);
}

void style_system_menu_end(style_system_menu_session *state)
{
    if(owns_menu(state))
        for(unsigned i=0;i<ARRAYSIZE(commands);++i)
            if(state->saved[i]!=(UINT)-1 && state->disabled[i])
                EnableMenuItem(state->menu,commands[i],MF_BYCOMMAND|(state->saved[i]&(MF_DISABLED|MF_GRAYED)));
    state->menu=NULL;
}
