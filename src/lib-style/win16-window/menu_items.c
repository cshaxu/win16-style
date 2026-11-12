#include "menu_items.h"
#include "lib-style/common/menu_metrics.h"

static BOOL columns_read(style_menu_item *items,HMENU menu,HDC dc,HFONT font,UINT dpi,
    win16_style_menu_columns *columns,BOOL *arrows)
{
    *columns=(win16_style_menu_columns){0};*arrows=FALSE;
    for(style_menu_item *item=items;item;item=item->next)if(item->menu==menu){
        *arrows|=item->submenu;
        if(!(item->type&MFT_SEPARATOR) && !win16_style_menu_columns_read(dc,font,dpi,item->text,columns))return FALSE;
    }
    return TRUE;
}

BOOL win16_style_menu_draw(style_menu_item *items,const DRAWITEMSTRUCT *draw,
    HFONT font,UINT dpi,const win16_style_palette *palette)
{
    style_menu_item *item;win16_style_menu_columns columns;BOOL arrows;
    if(draw->CtlType!=ODT_MENU || !(item=style_menu_item_find(items,draw->itemData)))return FALSE;
    if(!columns_read(items,item->menu,draw->hDC,font,dpi,&columns,&arrows))return FALSE;
    return win16_style_menu_text_columns(draw->hDC,&draw->rcItem,font,dpi,palette,item->text,draw->itemState,
        (item->type&MFT_SEPARATOR)?WIN16_STYLE_MENU_SEPARATOR:WIN16_STYLE_MENU_ROW,item->submenu,&columns);
}

BOOL win16_style_menu_size(style_menu_item *items,MEASUREITEMSTRUCT *measure,HFONT font,UINT dpi)
{
    style_menu_item *item;HDC dc;win16_style_menu_columns columns;BOOL ok,arrows;
    if(measure->CtlType!=ODT_MENU || !(item=style_menu_item_find(items,measure->itemData)))return FALSE;
    dc=CreateCompatibleDC(NULL);if(!dc)return FALSE;
    ok=columns_read(items,item->menu,dc,font,dpi,&columns,&arrows);
    int margin=ok?style_menu_native_text_margin(dc,dpi):0;
    DeleteDC(dc);if(ok){measure->itemWidth=max(1,win16_style_menu_columns_width(&columns,dpi,arrows)-margin);
        measure->itemHeight=(item->type&MFT_SEPARATOR)?2*max(1,MulDiv(1,(int)dpi,96)):columns.height;}return ok;
}
