/* Authored GDI presentation. Original reference: owner Win3.1 menu captures;
 * black single separators, flat selection and stippled disabled text.
 * This does not import USER's private menu structures or tracking state. */
#include "row_paint.h"
#include "lib-win32-style/application/menu/menu_metrics.h"
#include "lib-win32-style/application/menu/row_text.h"
#include <wchar.h>

BOOL style_menu_columns_read(HDC dc,HFONT font,UINT dpi,const WCHAR *text,
    style_menu_columns *columns)
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
    columns->height=max(columns->height,metric.tmHeight);
    RestoreDC(dc,saved);return TRUE;
}

static style_menu_popup_columns spacing(const style_menu_metrics *metrics,UINT dpi)
{
    const style_menu_metrics declared=metrics?*metrics:(style_menu_metrics){0};
    return style_menu_popup_spacing(dpi,declared.check_column,declared.shortcut_gap,
        declared.arrow_column,declared.plain_column);
}

int style_menu_columns_width(const style_menu_columns *c,UINT dpi,BOOL arrows,
    const style_menu_metrics *metrics)
{
    const style_menu_popup_columns m=spacing(metrics,dpi);
    return m.check+c->label+(c->shortcut?m.gap+c->shortcut:0)+(arrows?m.arrow:m.plain);
}

BOOL style_menu_bar_measure(HDC dc,HFONT font,UINT dpi,const WCHAR *text,
    const style_menu_metrics *metrics,SIZE *size)
{
    style_menu_columns c={0};
    if(!size || !style_menu_columns_read(dc,font,dpi,text,&c))return FALSE;
    unsigned padding=metrics?metrics->bar_padding:0;
    int extra=padding?MulDiv(2*(int)padding,(int)dpi,96):style_menu_native_text_margin(dc,dpi);
    *size=(SIZE){c.label+extra,style_menu_row_height(dpi,c.height,
        metrics?metrics->row_minimum:0,metrics?metrics->row_padding:0)};
    return TRUE;
}

BOOL style_menu_paint_row(HDC dc,const RECT *bounds,HFONT font,UINT dpi,
    const style_window_palette *p,const WCHAR *text,UINT state,style_menu_kind kind,BOOL submenu,
    const style_menu_columns *columns,const classic_menu_colors *colors,const style_menu_metrics *metrics,
    const style_menu_art *art,HBITMAP command)
{
    int saved,u,padding;RECT r;COLORREF background,foreground;
    BOOL result=TRUE,disabled;
    HFONT bold=NULL;
    if(!dc || !bounds || !font || !p || !text || dpi<48 || dpi>768 ||
       bounds->right<=bounds->left || bounds->bottom<=bounds->top ||
       kind<STYLE_MENU_ROW || kind>STYLE_MENU_BAR)return FALSE;
    saved=SaveDC(dc);if(!saved)return FALSE;
    IntersectClipRect(dc,bounds->left,bounds->top,bounds->right,bounds->bottom);
    const style_menu_popup_columns m=spacing(metrics,dpi);
    u=max(1,MulDiv(1,(int)dpi,96));padding=m.check;
    if(kind==STYLE_MENU_BAR)
        padding=metrics && metrics->bar_padding?MulDiv(metrics->bar_padding,(int)dpi,96):
            style_menu_native_text_margin(dc,dpi)/2;
    BOOL selected=(state&ODS_SELECTED)!=0;
    disabled=(state&(ODS_DISABLED|ODS_GRAYED))!=0;
    BOOL themed=colors && colors->present;
    background=themed?(selected?colors->selection:colors->background):(selected?p->active:p->light);
    foreground=themed?style_menu_ink(colors,disabled,selected):
        (selected?p->active_text:p->edge);
    SetDCBrushColor(dc,background);FillRect(dc,bounds,(HBRUSH)GetStockObject(DC_BRUSH));
    if(kind==STYLE_MENU_SEPARATOR){
        r=*bounds;r.top=(r.top+r.bottom)/2;r.bottom=r.top+u;
        SetDCBrushColor(dc,themed?colors->separator:p->edge);FillRect(dc,&r,(HBRUSH)GetStockObject(DC_BRUSH));
    }else{
        const unsigned text_flags=style_menu_text_options(metrics?metrics->text_flags:0);
        if(kind==STYLE_MENU_ROW && (state&ODS_DEFAULT) && (text_flags&STYLE_MENU_TEXT_BOLD_DEFAULT))
            bold=style_menu_bold_font(font);
        SelectObject(dc,bold?bold:font);SetBkMode(dc,TRANSPARENT);SetTextColor(dc,foreground);
        BOOL right=(text_flags&STYLE_MENU_TEXT_SHORTCUT_RIGHT)!=0;
        style_menu_text_layout layout={.label=*bounds,.shortcut=*bounds,.shortcut_right=right,
            .label_center=kind==STYLE_MENU_BAR && (text_flags&STYLE_MENU_TEXT_CENTER_BAR)};
        layout.label.left+=padding;
        layout.label.right-=kind==STYLE_MENU_BAR?padding:submenu?m.arrow:m.plain;
        if(columns && kind==STYLE_MENU_ROW)layout.label.right=layout.label.left+columns->label;
        if(columns && !right){
            layout.shortcut.left+=padding+columns->label+m.gap;
            layout.shortcut.right=layout.shortcut.left+columns->shortcut;
        }else{
            layout.shortcut.left+=padding+(right && columns?columns->label+m.gap:0);
            layout.shortcut.right-=kind==STYLE_MENU_BAR || !columns?padding:submenu?m.arrow:m.plain;
        }
        BOOL stipple=disabled && (text_flags&STYLE_MENU_TEXT_STIPPLE_DISABLED);
        result=style_menu_draw_labels(dc,text,&layout,stipple,foreground);
        if(kind==STYLE_MENU_ROW && (text_flags&STYLE_MENU_COMMAND_GLYPHS))
            style_menu_draw_command(dc,bounds,command,u,foreground);
        if(kind==STYLE_MENU_ROW)
            style_menu_popup_marks(dc,bounds,dpi,foreground,(state&ODS_CHECKED)!=0,submenu,art);
    }
    RestoreDC(dc,saved);
    if(bold)DeleteObject(bold);
    return result;
}
