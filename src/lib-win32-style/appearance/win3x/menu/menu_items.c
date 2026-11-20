#include "menu_items.h"
#include "menu_scope.h"
#include "lib-win32-style/application/menu/menu_metrics.h"
#include "lib-win32-style/application/menu/row_text.h"

static BOOL columns_read(style_menu_item *items,HMENU menu,HDC dc,HFONT font,UINT dpi,
    const classic_menu_metrics *metrics,win16_style_menu_columns *columns,BOOL *arrows)
{
    *columns=(win16_style_menu_columns){0};*arrows=FALSE;
    MENUITEMINFOW default_item={.cbSize=sizeof(default_item),.fMask=MIIM_DATA};
    HFONT bold=NULL;
    if(metrics && (metrics->text_flags&STYLE_MENU_TEXT_BOLD_DEFAULT)){
        UINT pos=GetMenuDefaultItem(menu,TRUE,GMDI_USEDISABLED);
        if(pos!=(UINT)-1 && GetMenuItemInfoW(menu,pos,TRUE,&default_item))
            bold=style_menu_bold_font(font);
    }
    BOOL ok=TRUE;
    for(style_menu_item *item=items;item;item=item->next)if(item->menu==menu){
        *arrows|=item->submenu;
        HFONT row_font=bold && default_item.dwItemData==(ULONG_PTR)item?bold:font;
        if(!(item->type&MFT_SEPARATOR) && !win16_style_menu_columns_read(dc,row_font,dpi,item->text,columns)){
            ok=FALSE;break;
        }
    }
    if(bold)DeleteObject(bold);
    return ok;
}

BOOL win16_style_menu_draw(style_menu_item *items,const DRAWITEMSTRUCT *draw,
    HFONT font,UINT dpi,const win16_style_palette *palette,const classic_menu_colors *colors,
    const classic_menu_metrics *metrics,const style_menu_art *art)
{
    style_menu_item *item;win16_style_menu_columns columns;BOOL arrows;
    if(draw->CtlType!=ODT_MENU || !(item=style_menu_item_find(items,draw->itemData)))return FALSE;
    if(!columns_read(items,item->menu,draw->hDC,font,dpi,metrics,&columns,&arrows))return FALSE;
    return win16_style_menu_text_columns(draw->hDC,&draw->rcItem,font,dpi,palette,item->text,draw->itemState,
        (item->type&MFT_SEPARATOR)?WIN16_STYLE_MENU_SEPARATOR:WIN16_STYLE_MENU_ROW,item->submenu,&columns,colors,metrics,art,item->bitmap);
}

BOOL win16_style_menu_size(style_menu_item *items,MEASUREITEMSTRUCT *measure,HFONT font,UINT dpi,
    const classic_menu_metrics *metrics)
{
    style_menu_item *item;HDC dc;win16_style_menu_columns columns;BOOL ok,arrows;
    if(measure->CtlType!=ODT_MENU || !(item=style_menu_item_find(items,measure->itemData)))return FALSE;
    dc=CreateCompatibleDC(NULL);if(!dc)return FALSE;
    ok=columns_read(items,item->menu,dc,font,dpi,metrics,&columns,&arrows);
    int margin=ok?style_menu_native_text_margin(dc,dpi):0;
    DeleteDC(dc);if(ok){measure->itemWidth=max(1,win16_style_menu_columns_width(&columns,dpi,arrows,metrics)-margin);
        measure->itemHeight=(item->type&MFT_SEPARATOR)?2*max(1,MulDiv(1,(int)dpi,96)):columns.height;}return ok;
}

static style_menu_popup_scope *begin_menu(void *context,HWND owner)
{win16_style_menu_theme *t=context;(void)owner;return win16_style_menu_begin(t->palette,t->colors,t->metrics,t->art);}
static void set_system(void *context,style_menu_popup_scope *scope,HMENU menu)
{win16_style_menu_theme *t=context;if(scope)win16_style_menu_system(scope,menu,t->font,t->dpi);}
static BOOL measure_item(void *context,style_menu_item *items,MEASUREITEMSTRUCT *measure)
{win16_style_menu_theme *t=context;return win16_style_menu_size(items,measure,t->font,t->dpi,t->metrics);}
static BOOL draw_item(void *context,style_menu_item *items,const DRAWITEMSTRUCT *draw)
{
    win16_style_menu_theme *t=context;win16_style_palette palette;
    win16_control_resolve_palette(t->palette,&palette);
    return win16_style_menu_draw(items,draw,t->font,t->dpi,&palette,
        win16_control_high_contrast()?NULL:t->colors,t->metrics,t->art);
}
static void select_system(void *context,HWND owner,BOOL selected)
{win16_style_menu_theme *t=context;if(t->selection)t->selection(owner,t->context,selected);}
const style_menu_owner_ops win16_style_menu_operations={begin_menu,set_system,measure_item,draw_item,select_system,TRUE};
