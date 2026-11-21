#include "system_menu.h"
#include "lib-win32-style/application/menu/menu_snapshot.h"
#include "lib-win32-style/application/menu/popup.h"
#include "lib-win32-style/application/menu/row_text.h"
#include "lib-win32-style/application/menu/menu_metrics.h"
#include "lib-win32-style/application/graphics/present.h"
#include <oleacc.h>
#include <commctrl.h>
#include <stdlib.h>

classic_menu_colors win2k_menu_colors(const classic_menu_colors *defined)
{
    if(defined && defined->present)return *defined;
    return (classic_menu_colors){GetSysColor(COLOR_MENU),GetSysColor(COLOR_HIGHLIGHT),
        GetSysColor(COLOR_MENUTEXT),GetSysColor(COLOR_HIGHLIGHTTEXT),
        GetSysColor(COLOR_GRAYTEXT),GetSysColor(COLOR_3DSHADOW),TRUE};
}

void win2k_menu_style_read(win2k_menu_style *style,const classic_theme_mapping *source)
{
    *style=(win2k_menu_style){.font=source->model.menu_font,.colors=source->model.menu_popup,
        .metrics=source->model.menu_metrics};
    if(source->resources)
        (void)style_menu_art_read(source->resources,source->model.menu_images,&style->art);
}

BOOL win2k_system_menu_paint(HWND popup,HMENU menu,HDC dc,BOOL window_coordinates,
    const win2k_menu_style *style)
{
    MENUBARINFO info={.cbSize=sizeof(info)};
    if(!popup || !menu || !dc || !GetMenuBarInfo(popup,OBJID_CLIENT,0,&info) || info.hMenu!=menu)return FALSE;
    int count=GetMenuItemCount(menu);
    if(count<1 || count>64)return FALSE;
    style_menu_row *rows=calloc((size_t)count,sizeof(*rows));if(!rows)return FALSE;
    BOOL valid=TRUE;
    for(int i=0;i<count;++i){
        if(!style_menu_read_row(menu,(UINT)i,&rows[i],NULL)){valid=FALSE;break;}
    }
    int saved=valid?SaveDC(dc):0;
    if(!saved){free(rows);return FALSE;}
    UINT dpi=GetDpiForWindow(popup);if(!dpi)dpi=96;
    int unit=max(1,MulDiv(1,(int)dpi,96));
    const msstyles_font *defined=style?&style->font:NULL;
    const classic_menu_colors colors=win2k_menu_colors(style?&style->colors:NULL);
    const classic_menu_metrics metrics=style?style->metrics:(classic_menu_metrics){0};
    const style_menu_popup_columns columns=style_menu_popup_spacing(dpi,metrics.check_column,
        metrics.shortcut_gap,metrics.arrow_column,metrics.plain_column);
    HFONT font=style_menu_font_acquire(dpi,defined,false);
    HFONT normal=font?font:(HFONT)GetCurrentObject(dc,OBJ_FONT);
    const unsigned text_flags=style_menu_text_options(metrics.text_flags);
    HFONT bold=(text_flags&STYLE_MENU_TEXT_BOLD_DEFAULT)?style_menu_bold_font(normal):NULL;
    if(normal)SelectObject(dc,normal);
    SetBkMode(dc,TRANSPARENT);
    POINT origin={0,0};
    if(!style_menu_client_offset(popup,window_coordinates,&origin))valid=FALSE;
    for(int i=0;valid && i<count;++i){
        RECT r=rows[i].rect;MapWindowPoints(NULL,popup,(POINT *)&r,2);OffsetRect(&r,origin.x,origin.y);
        BOOL selected=(rows[i].state&MFS_HILITE)!=0;
        HBRUSH background=CreateSolidBrush(selected?colors.selection:colors.background);
        if(!background){valid=FALSE;break;}
        FillRect(dc,&r,background);DeleteObject(background);
        if(rows[i].type&MFT_SEPARATOR){
            RECT line={r.left+unit,r.top+(r.bottom-r.top)/2,r.right-unit,r.top+(r.bottom-r.top)/2+unit};
            HBRUSH shadow=CreateSolidBrush(colors.separator);
            if(shadow){FillRect(dc,&line,shadow);DeleteObject(shadow);}continue;
        }
        if((rows[i].state&MFS_DEFAULT) && bold)SelectObject(dc,bold);
        else if(normal)SelectObject(dc,normal);
        COLORREF text=(rows[i].state&MFS_DISABLED)?colors.disabled_text:selected?colors.selected_text:colors.text;
        SetTextColor(dc,text);
        if(text_flags&STYLE_MENU_COMMAND_GLYPHS)
            style_menu_draw_command(dc,&r,rows[i].bitmap,unit,text);
        RECT label=r;
        label.left+=columns.check;
        label.right-=rows[i].submenu?columns.arrow:columns.plain;
        BOOL right=(text_flags&STYLE_MENU_TEXT_SHORTCUT_RIGHT)!=0;
        style_menu_text_layout layout={.label=label,.shortcut=label,.shortcut_right=right,.shortcut_first=right};
        layout.shortcut.left+=columns.gap;
        if(!right)layout.shortcut.left+=style_menu_label_width(dc,rows[i].text);
        BOOL stipple=(rows[i].state&MFS_DISABLED) && (text_flags&STYLE_MENU_TEXT_STIPPLE_DISABLED);
        (void)style_menu_draw_labels(dc,rows[i].text,&layout,stipple,text);
        const style_menu_art *art=style && style->art.available?&style->art:NULL;
        if(rows[i].state&MFS_CHECKED)style_menu_draw_mark(dc,&r,FALSE,dpi,text,art?&art->check[0][0]:NULL);
        if(rows[i].submenu)style_menu_draw_mark(dc,&r,TRUE,dpi,text,art?&art->arrow[0][0]:NULL);
    }
    RestoreDC(dc,saved);if(bold)DeleteObject(bold);style_menu_font_release(font);free(rows);return valid;
}

typedef struct menu_presentation {
    HMENU menu;
    win2k_menu_style style;
} menu_presentation;
static BOOL accept_popup(void *context,HWND window)
{
    menu_presentation *s=context;
    return s->menu && style_menu_popup_is(window,s->menu);
}
static BOOL selected_item(style_menu_popup *popup,ULONGLONG *selected)
{
    menu_presentation *s=style_menu_popup_context(popup);int count=GetMenuItemCount(s->menu);
    if(count<0)return FALSE;
    *selected=0;
    for(int i=0;i<count;++i){MENUITEMINFOW item={.cbSize=sizeof(item),.fMask=MIIM_STATE};
        if(GetMenuItemInfoW(s->menu,(UINT)i,TRUE,&item) && (item.fState&MFS_HILITE)){
            *selected=(ULONGLONG)i+1;break;}}
    return TRUE;
}
static BOOL popup_paint(style_menu_popup *popup,HDC target,BOOL window_coordinates)
{
    HWND window=style_menu_popup_window(popup);menu_presentation *s=style_menu_popup_context(popup);
    if(!s->menu)return FALSE;
    HDC dc=target?target:GetDC(window);if(!dc)return FALSE;
    BOOL painted=FALSE;RECT client;
    if(!target && GetClientRect(window,&client)){
        style_present_buffer buffer;
        BOOL ready=style_present_buffer_begin(&buffer,dc,&client,TRUE);
        if(ready)painted=win2k_system_menu_paint(window,s->menu,buffer.memory,FALSE,&s->style);
        BOOL committed=style_present_buffer_end(&buffer,ready && painted && style_menu_popup_live(popup));
        painted=painted && committed;
    }
    if(!painted && style_menu_popup_live(popup))painted=win2k_system_menu_paint(window,s->menu,dc,window_coordinates,&s->style);
    if(!target)ReleaseDC(window,dc);
    return painted;
}
static const style_menu_popup_ops popup_ops={accept_popup,NULL,popup_paint,selected_item,free,FALSE,TRUE};
static style_menu_popup_scope *begin_menu(void *context,HWND owner)
{
    const win2k_menu_style *style=context;
    menu_presentation *s=calloc(1,sizeof(*s));if(!s)return NULL;
    if(style)s->style=*style;
    style_menu_popup_scope *scope=style_menu_popup_begin(owner,&popup_ops,s);
    if(!scope)free(s);
    return scope;
}
static void set_system(void *context,style_menu_popup_scope *scope,HMENU menu)
{
    (void)context;menu_presentation *s=style_menu_popup_scope_context(scope);
    if(s)s->menu=menu;
}
const style_menu_owner_ops win2k_system_menu_operations={.begin=begin_menu,.system=set_system};
