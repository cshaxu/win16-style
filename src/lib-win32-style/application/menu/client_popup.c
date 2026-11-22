#include "client_popup.h"
#include "row_paint.h"
#include "../window/frame_border.h"
#include "lib-win32-style/application/menu/menu_snapshot.h"
#include "lib-win32-style/application/graphics/present.h"
#include "lib-win32-style/application/menu/row_text.h"
#include <stdlib.h>

typedef struct menu_presentation {
    style_window_palette palette;
    classic_menu_colors colors;
    style_menu_metrics metrics;
    style_menu_art art;
    HMENU system;
    HFONT font;
    UINT dpi;
    const style_menu_owner *owner;
} menu_presentation;

static void paint_border(style_menu_popup *p,HDC target)
{
    HWND w=style_menu_popup_window(p);menu_presentation *view=style_menu_popup_context(p);
    RECT outer,client,r;HDC dc=target?target:GetWindowDC(w);int saved,u;
    if(!dc)return;
    saved=SaveDC(dc);if(!saved){if(!target)ReleaseDC(w,dc);return;}
    if(!GetWindowRect(w,&outer) || !GetClientRect(w,&client))goto done;
    MapWindowPoints(w,NULL,(POINT *)&client,2);
    OffsetRect(&client,-outer.left,-outer.top);
    SetRect(&r,0,0,outer.right-outer.left,outer.bottom-outer.top);
    ExcludeClipRect(dc,client.left,client.top,client.right,client.bottom);
    style_frame_solid_fill(dc,r,view->colors.present?view->colors.background:view->palette.light);
    u=max(1,MulDiv(1,(int)GetDpiForWindow(w),96));
    style_frame_solid_outline(dc,r,u,view->palette.edge);
done:
    RestoreDC(dc,saved);if(!target)ReleaseDC(w,dc);
}

static UINT item_draw_state(UINT state)
{
    return ((state&MFS_HILITE)?ODS_SELECTED:0) | ((state&MFS_DISABLED)?ODS_DISABLED:0) |
        ((state&MFS_CHECKED)?ODS_CHECKED:0) | ((state&MFS_DEFAULT)?ODS_DEFAULT:0);
}

static BOOL paint_system(style_menu_popup *p,HDC target,BOOL window_coordinates)
{
    HWND w=style_menu_popup_window(p);menu_presentation *s=style_menu_popup_context(p);
    MENUBARINFO info={.cbSize=sizeof(info)};
    if(!s->system||!s->font)return FALSE;
    HMENU menu=s->system;HFONT font=s->font;UINT dpi=s->dpi;
    if(!GetMenuBarInfo(w,OBJID_CLIENT,0,&info)||!style_menu_popup_live(p)||info.hMenu!=menu){return FALSE;}
    int count=GetMenuItemCount(menu);if(!style_menu_popup_live(p)||count<1||count>64){return FALSE;}
    style_menu_row *rows=calloc((size_t)count,sizeof(*rows));if(!rows){return FALSE;}
    HDC dc=target?target:GetDC(w);if(!dc){free(rows);return FALSE;}
    int saved=SaveDC(dc);style_menu_columns columns={0};BOOL valid=saved!=0,arrows=FALSE;
    POINT delta={0,0};if(valid&&!style_menu_client_offset(w,window_coordinates,&delta))valid=FALSE;
    HFONT bold=(style_menu_text_options(s->metrics.text_flags)&STYLE_MENU_TEXT_BOLD_DEFAULT)?style_menu_bold_font(font):NULL;
    for(int i=0;valid&&i<count;++i){
        if(!style_menu_read_row(menu,(UINT)i,&rows[i],NULL)||!style_menu_popup_live(p))valid=FALSE;
        else if(!(rows[i].type&MFT_SEPARATOR) && !style_menu_columns_read(dc,
            bold&&(rows[i].state&MFS_DEFAULT)?bold:font,dpi,rows[i].text,&columns))valid=FALSE;
        if(valid)arrows|=rows[i].submenu;
    }
    if(bold)DeleteObject(bold);
    /* Compose the popup before presenting it; clearing each row directly on
     * the screen exposes blank text/background between GDI calls. */
    RECT client;HDC output=dc;style_present_buffer buffer;BOOL attempted=FALSE;
    if(valid&&!target&&GetClientRect(w,&client)){
        attempted=TRUE;
        if(style_present_buffer_begin(&buffer,dc,&client,TRUE))dc=buffer.memory;
    }
    int theme_width=style_menu_columns_width(&columns,dpi,arrows,&s->metrics);
    for(int i=0;valid&&i<count&&style_menu_popup_live(p);++i){RECT r=rows[i].rect;
        MapWindowPoints(NULL,w,(POINT *)&r,2);OffsetRect(&r,delta.x,delta.y);
        style_menu_metrics metrics=s->metrics;
        if(metrics.check_column && theme_width>r.right-r.left && columns.shortcut){
            int deficit=MulDiv(theme_width-(r.right-r.left),96,(int)dpi);
            metrics.shortcut_gap=max(0,metrics.shortcut_gap-deficit);
        }
        (void)style_menu_paint_row(dc,&r,font,dpi,&s->palette,rows[i].text,item_draw_state(rows[i].state),
            (rows[i].type&MFT_SEPARATOR)?STYLE_MENU_SEPARATOR:STYLE_MENU_ROW,
            rows[i].submenu,&columns,&s->colors,&metrics,&s->art,rows[i].bitmap);
    }
    if(attempted)(void)style_present_buffer_end(&buffer,dc==buffer.memory&&style_menu_popup_live(p));
    if(saved)RestoreDC(output,saved);
    if(!target)ReleaseDC(w,output);
    free(rows);
    return valid && style_menu_popup_live(p);
}

static BOOL paint_owned_arrows(style_menu_popup *p,HDC target,BOOL window_coordinates)
{
    HWND window=style_menu_popup_window(p);
    menu_presentation *s=style_menu_popup_context(p);
    if(!s->owner || !s->art.available || !s->dpi)return FALSE;

    MENUBARINFO info={.cbSize=sizeof(info)};
    POINT delta={0,0};
    if(!GetMenuBarInfo(window,OBJID_CLIENT,0,&info) || !style_menu_popup_live(p) ||
       !info.hMenu || !style_menu_client_offset(window,window_coordinates,&delta))return FALSE;
    int count=GetMenuItemCount(info.hMenu);
    if(count<=0)return FALSE;

    HDC dc=target?target:GetDC(window);
    if(!dc)return FALSE;
    int saved=SaveDC(dc);
    BOOL painted=FALSE;
    int extent=style_menu_arrow_extent(s->dpi);
    /* USER may draw a submenu arrow after WM_DRAWITEM. Replace the reserved
     * column only for a row that this owner converted to themed owner-draw. */
    for(int i=0;saved && i<count && style_menu_popup_live(p);++i){
        MENUITEMINFOW data={.cbSize=sizeof(data),.fMask=MIIM_DATA|MIIM_SUBMENU|MIIM_STATE|MIIM_FTYPE};
        if(!GetMenuItemInfoW(info.hMenu,(UINT)i,TRUE,&data) || !style_menu_popup_live(p) ||
           !data.hSubMenu || !(data.fType&MFT_OWNERDRAW))continue;
        const style_menu_item *item=style_menu_item_find(s->owner->items,data.dwItemData);
        if(!item || item->menu!=info.hMenu || !item->submenu || (item->type&MFT_SEPARATOR))continue;
        RECT row;
        if(!GetMenuItemRect(NULL,info.hMenu,(UINT)i,&row))continue;
        MapWindowPoints(NULL,window,(POINT *)&row,2);
        OffsetRect(&row,delta.x,delta.y);
        if(row.right-row.left<extent || row.bottom-row.top<extent || !style_menu_popup_live(p))continue;
        RECT gutter={row.right-extent,row.top,row.right,row.bottom};
        COLORREF background,foreground;
        style_menu_row_colors(&s->palette,&s->colors,item_draw_state(data.fState),&background,&foreground);
        SetDCBrushColor(dc,background);FillRect(dc,&gutter,(HBRUSH)GetStockObject(DC_BRUSH));
        style_menu_draw_mark(dc,&row,TRUE,s->dpi,foreground,&s->art.arrow[0][0]);
        painted=TRUE;
    }
    if(saved)RestoreDC(dc,saved);
    if(!target)ReleaseDC(window,dc);
    return painted;
}

static BOOL paint_popup(style_menu_popup *p,HDC dc,BOOL window_coordinates)
{
    return paint_system(p,dc,window_coordinates) || paint_owned_arrows(p,dc,window_coordinates);
}

static BOOL popup_selection(style_menu_popup *p,ULONGLONG *selected)
{
    HWND w=style_menu_popup_window(p);menu_presentation *s=style_menu_popup_context(p);
    MENUBARINFO info={.cbSize=sizeof(info)};
    if(!GetMenuBarInfo(w,OBJID_CLIENT,0,&info) || !style_menu_popup_live(p) ||
       !info.hMenu || (info.hMenu!=s->system && (!s->owner || !s->art.available)))return FALSE;
    int count=GetMenuItemCount(info.hMenu);if(count<1||count>64)return FALSE;
    *selected=0;
    for(int i=0;i<count;++i){MENUITEMINFOW item={.cbSize=sizeof(item),.fMask=MIIM_STATE};
        if(!GetMenuItemInfoW(info.hMenu,(UINT)i,TRUE,&item))return FALSE;
        if(item.fState&MFS_HILITE)*selected|=(ULONGLONG)1<<i;
    }
    return TRUE;
}

static BOOL accept_popup(void *context,HWND window)
{(void)context;(void)window;return TRUE;}
static const style_menu_popup_ops popup_ops={accept_popup,paint_border,paint_popup,popup_selection,free,TRUE,TRUE};

void style_menu_client_popup_system(style_menu_popup_scope *scope,HMENU menu,HFONT font,UINT dpi)
{
    menu_presentation *s=style_menu_popup_scope_context(scope);
    if(s){s->system=menu;s->font=font;s->dpi=dpi;}
}
style_menu_popup_scope *style_menu_client_popup_begin(const style_window_palette *palette,
    const classic_menu_colors *colors,const style_menu_metrics *metrics,const style_menu_art *art,
    const style_menu_owner *owner,UINT dpi)
{
    menu_presentation *s=calloc(1,sizeof(*s));if(!s)return NULL;
    if(palette)style_window_palette_resolve(palette,&s->palette);else style_window_palette_default(&s->palette);
    s->owner=owner;s->dpi=dpi;
    if(colors && !style_window_high_contrast())s->colors=*colors;
    if(metrics)s->metrics=*metrics;
    if(art)s->art=*art;
    style_menu_popup_scope *scope=style_menu_popup_begin(NULL,&popup_ops,s);
    if(!scope)free(s);
    return scope;
}
