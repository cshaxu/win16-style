#include <windows.h>
#include "lib-style/common/menu_items.h"
#include "lib-style/common/classic_menu_layout.h"

static BOOL nested_restore(void)
{
    HMENU first=CreatePopupMenu(),second=CreatePopupMenu();style_menu_item *items=NULL;
    MENUITEMINFOW info={.cbSize=sizeof(info),.fMask=MIIM_DATA,.dwItemData=99};LRESULT result;
    BOOL ok=first && second && AppendMenuW(first,MF_STRING,10,L"&First") &&
        AppendMenuW(second,MF_STRING,20,L"&Second") && SetMenuItemInfoW(first,0,TRUE,&info) &&
        style_menu_prepare(first,&items,1) && style_menu_prepare(second,&items,2) &&
        CheckMenuItem(first,10,MF_BYCOMMAND|MF_CHECKED)!=(UINT)-1;
    style_menu_restore_depth(2,&items);
    ok=ok && items && !style_menu_char(items,second,L's',&result) &&
        style_menu_char(items,first,L'f',&result) && result==MAKELRESULT(0,MNC_EXECUTE);
    style_menu_restore_popup(first,&items);
    info.fMask=MIIM_DATA|MIIM_FTYPE|MIIM_STATE;
    ok=ok && !items && GetMenuItemInfoW(first,0,TRUE,&info) && info.dwItemData==99 &&
        !(info.fType&MFT_OWNERDRAW) && (info.fState&MFS_CHECKED);
    style_menu_restore(&items);
    if(first)DestroyMenu(first);
    if(second)DestroyMenu(second);
    return ok;
}

int main(void)
{
    HMENU menu=CreatePopupMenu();style_menu_item *items=NULL;LRESULT result;
    if(!menu)return 1;
    MENUITEMINFOW info={.cbSize=sizeof(info),.fMask=MIIM_DATA,.dwItemData=1234};
    BOOL ok=AppendMenuW(menu,MF_STRING,1,L"&File") &&
        AppendMenuW(menu,MF_STRING,2,L"&Find") &&
        AppendMenuW(menu,MF_STRING,3,L"Save && Exit\t&X") &&
        AppendMenuW(menu,MF_STRING|MF_DISABLED,4,L"&Disabled") &&
        SetMenuItemInfoW(menu,0,TRUE,&info) && style_menu_prepare(menu,&items,0) &&
        style_menu_char(items,menu,L'f',&result) && result==MAKELRESULT(0,MNC_SELECT) &&
        style_menu_char(items,menu,L'x',&result) && result==MAKELRESULT(0,MNC_IGNORE) &&
        style_menu_char(items,menu,L'd',&result) && result==MAKELRESULT(0,MNC_IGNORE);
    style_menu_restore(&items);
    info.fMask=MIIM_DATA|MIIM_FTYPE;
    ok=ok && !items && GetMenuItemInfoW(menu,0,TRUE,&info) && info.dwItemData==1234 && !(info.fType&MFT_OWNERDRAW);
    SIZE sizes[]={{30,20},{40,25},{20,20}};UINT flags[]={0,0,MFT_RIGHTJUSTIFY};RECT rects[3];int height;
    ok=ok && style_classic_menu_arrange((POINT){10,20},120,sizes,flags,3,rects,&height) &&
        height==25 && rects[0].left==10 && rects[1].left==40 && rects[2].left==110 &&
        style_classic_menu_hit(rects,3,(POINT){110,20})==2 &&
        style_classic_menu_hit(rects,3,(POINT){100,20})==-1 &&
        style_classic_menu_arrange((POINT){10,20},60,sizes,flags,3,rects,&height) && height==45 &&
        !style_classic_menu_arrange((POINT){10,20},0,sizes,flags,3,rects,&height);
    DestroyMenu(menu);return ok && nested_restore()?0:2;
}
