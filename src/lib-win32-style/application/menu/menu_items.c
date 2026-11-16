#include "menu_items.h"
#include "menu_input.h"
#include <stdlib.h>

style_menu_item *style_menu_item_find(style_menu_item *items,ULONG_PTR data)
{
    for(;items;items=items->next)if((ULONG_PTR)items==data)return items;
    return NULL;
}

BOOL style_menu_char(style_menu_item *items,HMENU menu,WCHAR key,LRESULT *result)
{
    int i,count=GetMenuItemCount(menu),selected=-1,first=-1,next=-1,matches=0;BOOL owned=FALSE;
    for(i=0;i<count;++i){MENUITEMINFOW info={.cbSize=sizeof(info)};info.fMask=MIIM_STATE;
        if(GetMenuItemInfoW(menu,(UINT)i,TRUE,&info) && (info.fState&MFS_HILITE))selected=i;}
    for(i=0;i<count;++i){
        MENUITEMINFOW info={.cbSize=sizeof(info)};style_menu_item *item;
        info.fMask=MIIM_DATA|MIIM_STATE|MIIM_FTYPE;
        if(!GetMenuItemInfoW(menu,(UINT)i,TRUE,&info) || !(item=style_menu_item_find(items,info.dwItemData)))continue;
        owned=TRUE;
        if((info.fState&MFS_DISABLED) || (info.fType&MFT_SEPARATOR))continue;
        if(style_menu_mnemonic(item->text,key,TRUE)){
            if(first<0)first=i;
            if(next<0 && i>selected)next=i;
            ++matches;
        }
    }
    if(!owned)return FALSE;
    *result=!matches?MAKELRESULT(0,MNC_IGNORE):
        MAKELRESULT(matches==1?first:next<0?first:next,matches==1?MNC_EXECUTE:MNC_SELECT);
    return TRUE;
}

static void restore_items(HMENU menu,unsigned depth,BOOL by_depth,style_menu_item **items)
{
    while(*items){
        style_menu_item *item=*items;int i,count=GetMenuItemCount(item->menu);
        if((menu && item->menu!=menu) || (by_depth && item->depth!=depth)){items=&item->next;continue;}
        for(i=0;i<count;++i){
            MENUITEMINFOW info={.cbSize=sizeof(info)};info.fMask=MIIM_DATA|MIIM_FTYPE|MIIM_BITMAP;
            if(GetMenuItemInfoW(item->menu,(UINT)i,TRUE,&info) && info.dwItemData==(ULONG_PTR)item){
                /* Do not restore state/IDs/text changed by the application.
                 * Remove only our owner-draw bit and our borrowed-data marker. */
                info.dwItemData=item->data;
                info.fType=(info.fType&~MFT_OWNERDRAW)|(item->type&MFT_OWNERDRAW);
                if(!info.hbmpItem)info.hbmpItem=item->bitmap;
                SetMenuItemInfoW(item->menu,(UINT)i,TRUE,&info);break;
            }
        }
        *items=item->next;free(item->text);free(item);
    }
}

void style_menu_restore(style_menu_item **items)
{
    restore_items(NULL,0,FALSE,items);
}
void style_menu_restore_popup(HMENU menu,style_menu_item **items)
{restore_items(menu,0,FALSE,items);}
void style_menu_restore_depth(unsigned depth,style_menu_item **items)
{restore_items(NULL,depth,TRUE,items);}

BOOL style_menu_prepare(HMENU menu,style_menu_item **items,unsigned depth)
{
    int i,count=GetMenuItemCount(menu);
    if(count<0)return FALSE;
    for(i=0;i<count;++i){
        MENUITEMINFOW info={.cbSize=sizeof(info)};style_menu_item *item;
        info.fMask=MIIM_FTYPE|MIIM_DATA|MIIM_STRING|MIIM_SUBMENU|MIIM_BITMAP;
        if(!GetMenuItemInfoW(menu,(UINT)i,TRUE,&info))return FALSE;
        if((info.fType&MFT_OWNERDRAW) || (info.hbmpItem &&
            info.hbmpItem!=HBMMENU_POPUP_CLOSE && info.hbmpItem!=HBMMENU_POPUP_RESTORE &&
            info.hbmpItem!=HBMMENU_POPUP_MINIMIZE && info.hbmpItem!=HBMMENU_POPUP_MAXIMIZE))continue;
        item=calloc(1,sizeof(*item));if(!item)return FALSE;
        item->text=calloc((size_t)info.cch+1,sizeof(WCHAR));
        if(!item->text){free(item);return FALSE;}
        item->menu=menu;item->type=info.fType;item->data=info.dwItemData;item->submenu=info.hSubMenu!=NULL;
        item->bitmap=info.hbmpItem;item->depth=depth;
        info.dwTypeData=item->text;++info.cch;
        if(!GetMenuItemInfoW(menu,(UINT)i,TRUE,&info)){free(item->text);free(item);return FALSE;}
        info.fMask=MIIM_FTYPE|MIIM_DATA|MIIM_BITMAP;info.fType|=MFT_OWNERDRAW;
        info.dwItemData=(ULONG_PTR)item;info.hbmpItem=NULL;
        if(!SetMenuItemInfoW(menu,(UINT)i,TRUE,&info)){free(item->text);free(item);return FALSE;}
        item->next=*items;*items=item;
    }
    return TRUE;
}
