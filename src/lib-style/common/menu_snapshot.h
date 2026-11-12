#ifndef STYLE_MENU_SNAPSHOT_H
#define STYLE_MENU_SNAPSHOT_H
#include <windows.h>

/* Caller holds the surrounding popup state during synchronous USER calls. */
typedef struct style_menu_row {
    RECT rect;
    WCHAR text[512];
    UINT type,state;
    HBITMAP bitmap;
    BOOL submenu;
} style_menu_row;

static inline BOOL style_menu_read_row(HMENU menu,UINT index,style_menu_row *row,const BOOL *cancelled)
{
    MENUITEMINFOW item={.cbSize=sizeof(item),.fMask=MIIM_FTYPE|MIIM_STRING|MIIM_STATE|MIIM_BITMAP|MIIM_SUBMENU,
        .dwTypeData=row->text,.cch=ARRAYSIZE(row->text)-1};
    if(!GetMenuItemInfoW(menu,index,TRUE,&item) || (cancelled && *cancelled) ||
       (item.fType&MFT_OWNERDRAW) || item.cch>=ARRAYSIZE(row->text)-1 ||
       (item.hbmpItem && item.hbmpItem!=HBMMENU_POPUP_CLOSE && item.hbmpItem!=HBMMENU_POPUP_RESTORE &&
        item.hbmpItem!=HBMMENU_POPUP_MINIMIZE && item.hbmpItem!=HBMMENU_POPUP_MAXIMIZE) ||
       !GetMenuItemRect(NULL,menu,index,&row->rect) || (cancelled && *cancelled))return FALSE;
    row->type=item.fType;row->state=item.fState;row->bitmap=item.hbmpItem;row->submenu=item.hSubMenu!=NULL;
    return TRUE;
}
#endif
