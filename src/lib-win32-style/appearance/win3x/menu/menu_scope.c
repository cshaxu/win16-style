#include "menu_scope.h"
#include "menu_paint.h"
#include "../window/frame_paint.h"
#include "lib-win32-style/application/menu/menu_snapshot.h"
#include "lib-win32-style/application/graphics/present.h"
#include <stdlib.h>

typedef struct menu_presentation {
    win16_style_palette palette;
    classic_menu_colors colors;
    classic_menu_metrics metrics;
    win16_menu_art art;
    HMENU system;
    HFONT font;
    UINT dpi;
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
    win16_control_fill(dc,r,view->colors.present?view->colors.background:view->palette.light);
    u=max(1,MulDiv(1,(int)GetDpiForWindow(w),96));
    win16_control_outline(dc,r,u,view->palette.edge);
done:
    RestoreDC(dc,saved);if(!target)ReleaseDC(w,dc);
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
    int saved=SaveDC(dc);win16_style_menu_columns columns={0};BOOL valid=saved!=0,arrows=FALSE;
    POINT delta={0,0};if(valid&&!style_menu_client_offset(w,window_coordinates,&delta))valid=FALSE;
    for(int i=0;valid&&i<count;++i){
        if(!style_menu_read_row(menu,(UINT)i,&rows[i],NULL)||!style_menu_popup_live(p))valid=FALSE;
        else if(!(rows[i].type&MFT_SEPARATOR)&&!win16_style_menu_columns_read(dc,font,dpi,rows[i].text,&columns))valid=FALSE;
        if(valid)arrows|=rows[i].submenu;
    }
    /* Compose the popup before presenting it; clearing each row directly on
     * the screen exposes blank text/background between GDI calls. */
    RECT client;HDC output=dc;style_present_buffer buffer;BOOL attempted=FALSE;
    if(valid&&!target&&GetClientRect(w,&client)){
        attempted=TRUE;
        if(style_present_buffer_begin(&buffer,dc,&client,TRUE))dc=buffer.memory;
    }
    int theme_width=win16_style_menu_columns_width(&columns,dpi,arrows,&s->metrics);
    for(int i=0;valid&&i<count&&style_menu_popup_live(p);++i){RECT r=rows[i].rect;
        MapWindowPoints(NULL,w,(POINT *)&r,2);OffsetRect(&r,delta.x,delta.y);
        const classic_menu_metrics *metrics=theme_width<=r.right-r.left?&s->metrics:NULL;
        UINT state=0;if(rows[i].state&MFS_HILITE)state|=ODS_SELECTED;if(rows[i].state&MFS_DISABLED)state|=ODS_DISABLED;if(rows[i].state&MFS_CHECKED)state|=ODS_CHECKED;
        (void)win16_style_menu_text_columns(dc,&r,font,dpi,&s->palette,rows[i].text,state,
            (rows[i].type&MFT_SEPARATOR)?WIN16_STYLE_MENU_SEPARATOR:WIN16_STYLE_MENU_ROW,
            rows[i].submenu,&columns,&s->colors,metrics,&s->art);
    }
    if(attempted)(void)style_present_buffer_end(&buffer,dc==buffer.memory&&style_menu_popup_live(p));
    if(saved)RestoreDC(output,saved);
    if(!target)ReleaseDC(w,output);
    free(rows);
    return valid && style_menu_popup_live(p);
}

static BOOL system_selection(style_menu_popup *p,ULONGLONG *selected)
{
    HWND w=style_menu_popup_window(p);menu_presentation *s=style_menu_popup_context(p);
    MENUBARINFO info={.cbSize=sizeof(info)};
    if(!s->system||!GetMenuBarInfo(w,OBJID_CLIENT,0,&info)||
        info.hMenu!=s->system)return FALSE;
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
static const style_menu_popup_ops popup_ops={accept_popup,paint_border,paint_system,system_selection,free,TRUE,FALSE};

void win16_style_menu_system(win16_style_menu_scope *scope,HMENU menu,HFONT font,UINT dpi)
{
    menu_presentation *s=style_menu_popup_scope_context(scope);
    if(s){s->system=menu;s->font=font;s->dpi=dpi;}
}
win16_style_menu_scope *win16_style_menu_begin(const win16_style_palette *palette,
    const classic_menu_colors *colors,const classic_menu_metrics *metrics,const win16_menu_art *art)
{
    menu_presentation *s=calloc(1,sizeof(*s));if(!s)return NULL;
    if(palette)s->palette=*palette;else win16_style_default_palette(&s->palette);
    if(colors && !win16_control_high_contrast())s->colors=*colors;
    if(metrics)s->metrics=*metrics;
    if(art)s->art=*art;
    style_menu_popup_scope *scope=style_menu_popup_begin(NULL,&popup_ops,s);
    if(!scope)free(s);
    return scope;
}
