#include "lib-win32-style/appearance/win3x/menu/menu_paint.h"
#include "lib-win32-style/application/menu/menu_metrics.h"

extern IMAGE_DOS_HEADER __ImageBase;

static classic_menu_metrics read_menu_theme(style_menu_art *art)
{
    DWORD incoming=GetLastError();
    classic_menu_metrics metrics={0};
    classic_theme_mapping source={0};
    if(!classic_theme_attachment_layout((HMODULE)&__ImageBase,CLASSIC_WINDOW_PROVIDER_FULL_CLIENT,&source) && source.resources){
        metrics=source.model.menu_metrics;
        if(art)(void)style_menu_art_read(source.resources,source.model.menu_images,art);
        classic_theme_close(&source);
    }
    SetLastError(incoming);return metrics;
}

BOOL win16_style_menu_measure(HDC dc,HFONT font,UINT dpi,const WCHAR *text,
    win16_style_menu_kind kind,SIZE *size)
{
    win16_style_menu_columns c={0};
    if(!size || kind<WIN16_STYLE_MENU_ROW || kind>WIN16_STYLE_MENU_BAR)return FALSE;
    classic_menu_metrics metrics=read_menu_theme(NULL);
    if(kind==WIN16_STYLE_MENU_BAR)
        return win16_style_menu_bar_measure(dc,font,dpi,text,&metrics,size);
    if(!win16_style_menu_columns_read(dc,font,dpi,text,&c))return FALSE;
    /* The exported row helper has no submenu flag: reserve an arrow cell.
     * Native popup adapters know the complete HMENU and omit it when unused. */
    size->cx=win16_style_menu_columns_width(&c,dpi,TRUE,&metrics);
    size->cy=kind==WIN16_STYLE_MENU_SEPARATOR?2*max(1,MulDiv(1,(int)dpi,96)):
        style_menu_row_height(dpi,c.height,metrics.row_minimum,metrics.row_padding);
    return TRUE;
}

BOOL win16_style_menu_text(HDC dc,const RECT *bounds,HFONT font,UINT dpi,
    const win16_style_palette *p,const WCHAR *text,UINT state,win16_style_menu_kind kind,BOOL submenu)
{
    win16_style_menu_columns c={0};
    if(!win16_style_menu_columns_read(dc,font,dpi,text,&c))return FALSE;
    style_menu_art art={0};
    BOOL needs_art=kind==WIN16_STYLE_MENU_ROW && ((state&ODS_CHECKED) || submenu);
    classic_menu_metrics metrics=read_menu_theme(needs_art?&art:NULL);
    return win16_style_menu_text_columns(dc,bounds,font,dpi,p,text,state,kind,submenu,&c,NULL,&metrics,&art,NULL);
}
