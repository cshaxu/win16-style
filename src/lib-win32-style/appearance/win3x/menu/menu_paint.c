/* Authored GDI presentation. Original reference: owner Win3.1 menu captures;
 * black single separators, flat selection and stippled disabled text.
 * This does not import USER's private menu structures or tracking state. */
#include "menu_paint.h"
#include "../resources/menu_glyphs.h"
#include "lib-win32-style/appearance/msstyles/classic_presets.h"
#include "lib-win32-style/application/menu/row_text.h"
#include <wchar.h>

BOOL win16_style_menu_columns_read(HDC dc,HFONT font,UINT dpi,const WCHAR *text,
    win16_style_menu_columns *columns)
{
    int saved;RECT label={0},shortcut={0};const WCHAR *tab;TEXTMETRICW metric;
    if(!dc || !font || !text || !columns || dpi<48 || dpi>768)return FALSE;
    saved=SaveDC(dc);if(!saved)return FALSE;
    SelectObject(dc,font);
    if(!GetTextMetricsW(dc,&metric)){RestoreDC(dc,saved);return FALSE;}
    tab=wcschr(text,L'\t');
    DrawTextW(dc,text,tab?(int)(tab-text):-1,&label,DT_CALCRECT|DT_SINGLELINE);
    if(tab)DrawTextW(dc,tab+1,-1,&shortcut,DT_CALCRECT|DT_SINGLELINE|DT_NOPREFIX);
    columns->label=max(columns->label,label.right);
    columns->shortcut=max(columns->shortcut,shortcut.right);
    columns->height=max(columns->height,max(metric.tmHeight,MulDiv(14,(int)dpi,96))+2*max(1,MulDiv(1,(int)dpi,96)));
    RestoreDC(dc,saved);return TRUE;
}

typedef struct popup_spacing {int check,gap,arrow,plain;} popup_spacing;

static popup_spacing spacing(const classic_menu_metrics *metrics,UINT dpi)
{
    static const classic_menu_metrics fallback=WIN16_POPUP_MENU_METRICS;
    if(!metrics || !metrics->check_column)metrics=&fallback;
    return (popup_spacing){MulDiv(metrics->check_column,(int)dpi,96),
        MulDiv(metrics->shortcut_gap,(int)dpi,96),MulDiv(metrics->arrow_column,(int)dpi,96),
        MulDiv(metrics->plain_column,(int)dpi,96)};
}

int win16_style_menu_columns_width(const win16_style_menu_columns *c,UINT dpi,BOOL arrows,
    const classic_menu_metrics *metrics)
{
    popup_spacing m=spacing(metrics,dpi);
    return m.check+c->label+(c->shortcut?m.gap+c->shortcut:0)+(arrows?m.arrow:m.plain);
}

BOOL win16_style_menu_measure(HDC dc,HFONT font,UINT dpi,const WCHAR *text,
    win16_style_menu_kind kind,SIZE *size)
{
    win16_style_menu_columns c={0};
    if(!size || kind<WIN16_STYLE_MENU_ROW || kind>WIN16_STYLE_MENU_BAR ||
        !win16_style_menu_columns_read(dc,font,dpi,text,&c))return FALSE;
    /* The exported row helper has no submenu flag: reserve an arrow cell.
     * Native popup adapters know the complete HMENU and omit it when unused. */
    size->cx=kind==WIN16_STYLE_MENU_BAR?c.label+MulDiv(12,(int)dpi,96):
        win16_style_menu_columns_width(&c,dpi,TRUE,NULL);
    size->cy=kind==WIN16_STYLE_MENU_SEPARATOR?2*max(1,MulDiv(1,(int)dpi,96)):c.height;
    return TRUE;
}

BOOL win16_style_menu_text_columns(HDC dc,const RECT *bounds,HFONT font,UINT dpi,
    const win16_style_palette *p,const WCHAR *text,UINT state,win16_style_menu_kind kind,BOOL submenu,
    const win16_style_menu_columns *columns,const classic_menu_colors *colors,const classic_menu_metrics *metrics,
    const style_menu_art *art,HBITMAP command)
{
    int saved,u,padding;RECT r;COLORREF background,foreground;
    BOOL result=TRUE,disabled;
    HFONT bold=NULL;
    if(!dc || !bounds || !font || !p || !text || dpi<48 || dpi>768 ||
       bounds->right<=bounds->left || bounds->bottom<=bounds->top ||
       kind<WIN16_STYLE_MENU_ROW || kind>WIN16_STYLE_MENU_BAR)return FALSE;
    saved=SaveDC(dc);if(!saved)return FALSE;
    IntersectClipRect(dc,bounds->left,bounds->top,bounds->right,bounds->bottom);
    popup_spacing m=spacing(metrics,dpi);
    u=max(1,MulDiv(1,(int)dpi,96));padding=kind==WIN16_STYLE_MENU_BAR?MulDiv(6,(int)dpi,96):m.check;
    BOOL selected=(state&ODS_SELECTED)!=0;
    disabled=(state&(ODS_DISABLED|ODS_GRAYED))!=0;
    BOOL themed=colors && colors->present;
    background=themed?(selected?colors->selection:colors->background):(selected?p->active:p->light);
    foreground=themed?(disabled?colors->disabled_text:selected?colors->selected_text:colors->text):
        (selected?p->active_text:p->edge);
    SetDCBrushColor(dc,background);FillRect(dc,bounds,(HBRUSH)GetStockObject(DC_BRUSH));
    if(kind==WIN16_STYLE_MENU_SEPARATOR){
        r=*bounds;r.top=(r.top+r.bottom)/2;r.bottom=r.top+u;
        SetDCBrushColor(dc,themed?colors->separator:p->edge);FillRect(dc,&r,(HBRUSH)GetStockObject(DC_BRUSH));
    }else{
        unsigned text_flags=metrics?metrics->text_flags:0;
        if(kind==WIN16_STYLE_MENU_ROW && (state&ODS_DEFAULT) && (text_flags&STYLE_MENU_TEXT_BOLD_DEFAULT))
            bold=style_menu_bold_font(font);
        SelectObject(dc,bold?bold:font);SetBkMode(dc,TRANSPARENT);SetTextColor(dc,foreground);
        BOOL right=text_flags?(text_flags&STYLE_MENU_TEXT_SHORTCUT_RIGHT)!=0:columns==NULL;
        style_menu_text_layout layout={.label=*bounds,.shortcut=*bounds,.shortcut_right=right,
            .label_center=kind==WIN16_STYLE_MENU_BAR && (text_flags&STYLE_MENU_TEXT_CENTER_BAR)};
        layout.label.left+=padding;
        layout.label.right-=kind==WIN16_STYLE_MENU_BAR?padding:submenu?m.arrow:m.plain;
        if(columns && kind==WIN16_STYLE_MENU_ROW)layout.label.right=layout.label.left+columns->label;
        if(columns && !right){
            layout.shortcut.left+=padding+columns->label+m.gap;
            layout.shortcut.right=layout.shortcut.left+columns->shortcut;
        }else{
            layout.shortcut.left+=padding+(right && columns?columns->label+m.gap:0);
            layout.shortcut.right-=kind==WIN16_STYLE_MENU_BAR || !columns?padding:submenu?m.arrow:m.plain;
        }
        BOOL stipple=disabled && (!text_flags || (text_flags&STYLE_MENU_TEXT_STIPPLE_DISABLED));
        result=style_menu_draw_labels(dc,text,&layout,stipple,foreground);
        if(kind==WIN16_STYLE_MENU_ROW && (text_flags&STYLE_MENU_COMMAND_GLYPHS))
            style_menu_draw_command(dc,bounds,command,u,foreground);
        if(kind==WIN16_STYLE_MENU_ROW && (state&ODS_CHECKED))
            style_menu_draw_mark(dc,bounds,FALSE,dpi,foreground,art && art->available?&art->check[0][0]:&menu_check[0][0]);
        if(kind==WIN16_STYLE_MENU_ROW && submenu)
            style_menu_draw_mark(dc,bounds,TRUE,dpi,foreground,art && art->available?&art->arrow[0][0]:&menu_arrow[0][0]);
    }
    RestoreDC(dc,saved);
    if(bold)DeleteObject(bold);
    return result;
}

BOOL win16_style_menu_text(HDC dc,const RECT *bounds,HFONT font,UINT dpi,
    const win16_style_palette *p,const WCHAR *text,UINT state,win16_style_menu_kind kind,BOOL submenu)
{
    win16_style_menu_columns c={0};
    if(!win16_style_menu_columns_read(dc,font,dpi,text,&c))return FALSE;
    return win16_style_menu_text_columns(dc,bounds,font,dpi,p,text,state,kind,submenu,&c,NULL,NULL,NULL,NULL);
}
