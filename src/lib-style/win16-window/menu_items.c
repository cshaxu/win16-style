/* Temporary presentation of native string popup items. Native tracking,
 * command IDs, enable/check state and menu ownership are unchanged. */
#include "menu_items.h"
#include <stdlib.h>

struct win16_style_menu_item {
    struct win16_style_menu_item *next;
    HMENU menu;
    UINT type;
    ULONG_PTR data;
    WCHAR *text;
    HBITMAP bitmap;
    BOOL submenu;
    unsigned depth;
};

static win16_style_menu_item *find(win16_style_menu_item *items,ULONG_PTR data)
{
    for(;items;items=items->next)if((ULONG_PTR)items==data)return items;
    return NULL;
}

BOOL win16_style_menu_char(win16_style_menu_item *items,HMENU menu,WCHAR key,LRESULT *result)
{
    int i,count=GetMenuItemCount(menu),selected=-1,first=-1,next=-1,matches=0;BOOL owned=FALSE;
    CharUpperBuffW(&key,1);
    for(i=0;i<count;++i){MENUITEMINFOW info={.cbSize=sizeof(info)};info.fMask=MIIM_STATE;
        if(GetMenuItemInfoW(menu,(UINT)i,TRUE,&info) && (info.fState&MFS_HILITE))selected=i;}
    for(i=0;i<count;++i){
        MENUITEMINFOW info={.cbSize=sizeof(info)};win16_style_menu_item *item;const WCHAR *text;
        info.fMask=MIIM_DATA|MIIM_STATE|MIIM_FTYPE;
        if(!GetMenuItemInfoW(menu,(UINT)i,TRUE,&info) || !(item=find(items,info.dwItemData)))continue;
        owned=TRUE;
        if((info.fState&MFS_DISABLED) || (info.fType&MFT_SEPARATOR))continue;
        for(text=item->text;*text && *text!=L'\t';++text)if(*text==L'&' && text[1]){
            WCHAR letter=*++text;if(letter==L'&')continue;CharUpperBuffW(&letter,1);
            if(letter!=key)continue;
            if(first<0)first=i;
            if(next<0 && i>selected)next=i;
            ++matches;
            break;
        }
    }
    if(!owned)return FALSE;
    *result=!matches?MAKELRESULT(0,MNC_IGNORE):
        MAKELRESULT(matches==1?first:next<0?first:next,matches==1?MNC_EXECUTE:MNC_SELECT);
    return TRUE;
}

static void restore_items(HMENU menu,unsigned depth,BOOL by_depth,win16_style_menu_item **items)
{
    while(*items){
        win16_style_menu_item *item=*items;int i,count=GetMenuItemCount(item->menu);
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

void win16_style_menu_restore(win16_style_menu_item **items)
{
    restore_items(NULL,0,FALSE,items);
}
void win16_style_menu_restore_popup(HMENU menu,win16_style_menu_item **items)
{restore_items(menu,0,FALSE,items);}
void win16_style_menu_restore_depth(unsigned depth,win16_style_menu_item **items)
{restore_items(NULL,depth,TRUE,items);}

BOOL win16_style_menu_prepare(HMENU menu,win16_style_menu_item **items,unsigned depth)
{
    int i,count=GetMenuItemCount(menu);
    if(count<0)return FALSE;
    for(i=0;i<count;++i){
        MENUITEMINFOW info={.cbSize=sizeof(info)};win16_style_menu_item *item;
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

static BOOL columns_read(win16_style_menu_item *items,HMENU menu,HDC dc,HFONT font,UINT dpi,
    win16_style_menu_columns *columns,BOOL *arrows)
{
    *columns=(win16_style_menu_columns){0};*arrows=FALSE;
    for(win16_style_menu_item *item=items;item;item=item->next)if(item->menu==menu){
        *arrows|=item->submenu;
        if(!(item->type&MFT_SEPARATOR) && !win16_style_menu_columns_read(dc,font,dpi,item->text,columns))return FALSE;
    }
    return TRUE;
}

BOOL win16_style_menu_draw(win16_style_menu_item *items,const DRAWITEMSTRUCT *draw,
    HFONT font,UINT dpi,const win16_style_palette *palette)
{
    win16_style_menu_item *item;win16_style_menu_columns columns;BOOL arrows;
    if(draw->CtlType!=ODT_MENU || !(item=find(items,draw->itemData)))return FALSE;
    if(!columns_read(items,item->menu,draw->hDC,font,dpi,&columns,&arrows))return FALSE;
    return win16_style_menu_text_columns(draw->hDC,&draw->rcItem,font,dpi,palette,item->text,draw->itemState,
        (item->type&MFT_SEPARATOR)?WIN16_STYLE_MENU_SEPARATOR:WIN16_STYLE_MENU_ROW,item->submenu,&columns);
}

static int native_text_margin(HDC dc,UINT dpi)
{
    /* USER adds two native menu-font average character cells to owner-draw
     * widths. Account for that host layout allowance once, without using
     * any host-font advances to shape or scale our System glyphs. */
    NONCLIENTMETRICSW ncm={.cbSize=sizeof(ncm)};HFONT font;HGDIOBJ old;SIZE size;int result=0;
    if(!SystemParametersInfoForDpi(SPI_GETNONCLIENTMETRICS,sizeof(ncm),&ncm,0,dpi))return 0;
    font=CreateFontIndirectW(&ncm.lfMenuFont);if(!font)return 0;
    old=SelectObject(dc,font);
    if(old && GetTextExtentPoint32W(dc,L"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ",52,&size))
        result=2*((size.cx/26+1)/2);
    if(old)SelectObject(dc,old);
    DeleteObject(font);return result;
}

BOOL win16_style_menu_size(win16_style_menu_item *items,MEASUREITEMSTRUCT *measure,HFONT font,UINT dpi)
{
    win16_style_menu_item *item;HDC dc;win16_style_menu_columns columns;BOOL ok,arrows;
    if(measure->CtlType!=ODT_MENU || !(item=find(items,measure->itemData)))return FALSE;
    dc=CreateCompatibleDC(NULL);if(!dc)return FALSE;
    ok=columns_read(items,item->menu,dc,font,dpi,&columns,&arrows);
    int margin=ok?native_text_margin(dc,dpi):0;
    DeleteDC(dc);if(ok){measure->itemWidth=max(1,win16_style_menu_columns_width(&columns,dpi,arrows)-margin);
        measure->itemHeight=(item->type&MFT_SEPARATOR)?2*max(1,MulDiv(1,(int)dpi,96)):columns.height;}return ok;
}
