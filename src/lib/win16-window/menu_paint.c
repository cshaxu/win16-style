/* Authored GDI presentation. Original reference: owner Win3.1 menu captures;
 * black single separators, flat selection and stippled disabled text.
 * This does not import USER's private menu structures or tracking state. */
#include "menu_paint.h"
#include "menu_glyphs.h"
#include <wchar.h>

typedef struct menu_text {
    const WCHAR *text;
    int count;
    RECT bounds;
    UINT flags;
} menu_text;

static BOOL CALLBACK draw_text(HDC dc,LPARAM data,WPARAM unused,int width,int height)
{
    menu_text *t=(menu_text *)data;RECT r=t->bounds;
    (void)unused;(void)width;(void)height;
    return DrawTextW(dc,t->text,t->count,&r,t->flags)!=0;
}

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

int win16_style_menu_columns_width(const win16_style_menu_columns *c,UINT dpi,BOOL arrows)
{
    return MulDiv(18,(int)dpi,96)+c->label+
        (c->shortcut?MulDiv(8,(int)dpi,96)+c->shortcut:0)+MulDiv(arrows?12:4,(int)dpi,96);
}

BOOL win16_style_menu_measure(HDC dc,HFONT font,UINT dpi,const WCHAR *text,
    win16_style_menu_kind kind,SIZE *size)
{
    win16_style_menu_columns c={0};
    if(!size || kind<WIN16_STYLE_MENU_ROW || kind>WIN16_STYLE_MENU_BAR ||
        !win16_style_menu_columns_read(dc,font,dpi,text,&c))return FALSE;
    /* The exported row helper has no submenu flag: reserve an arrow cell.
     * Native popup adapters know the complete HMENU and omit it when unused. */
    size->cx=kind==WIN16_STYLE_MENU_BAR?c.label+MulDiv(12,(int)dpi,96):win16_style_menu_columns_width(&c,dpi,TRUE);
    size->cy=kind==WIN16_STYLE_MENU_SEPARATOR?2*max(1,MulDiv(1,(int)dpi,96)):c.height;
    return TRUE;
}

static BOOL render_text(HDC dc,menu_text item,BOOL disabled,COLORREF color)
{
    HBRUSH brush;BOOL result;int x,y;
    if(!item.count || !item.text[0])return TRUE;
    if(!disabled)return draw_text(dc,(LPARAM)&item,0,0,0);
    brush=CreateSolidBrush(color);if(!brush)return FALSE;
    x=item.bounds.left;y=item.bounds.top;OffsetRect(&item.bounds,-x,-y);
    result=DrawStateW(dc,brush,draw_text,(LPARAM)&item,0,x,y,
        item.bounds.right,item.bounds.bottom,DST_COMPLEX|DSS_UNION|DSS_MONO);
    DeleteObject(brush);return result;
}

static void glyph(HDC dc,int left,int top,const char *bits,int width,int height,UINT dpi,COLORREF color)
{
    int x,y;RECT pixel;SetDCBrushColor(dc,color);
    for(y=0;y<height;++y)for(x=0;x<width;++x)if(bits[y*(width+1)+x]=='0'){
        SetRect(&pixel,left+MulDiv(x,(int)dpi,96),top+MulDiv(y,(int)dpi,96),
            left+MulDiv(x+1,(int)dpi,96),top+MulDiv(y+1,(int)dpi,96));
        FillRect(dc,&pixel,(HBRUSH)GetStockObject(DC_BRUSH));
    }
}

BOOL win16_style_menu_text_columns(HDC dc,const RECT *bounds,HFONT font,UINT dpi,
    const win16_style_palette *p,const WCHAR *text,UINT state,win16_style_menu_kind kind,BOOL submenu,const win16_style_menu_columns *columns)
{
    int saved,u,padding;RECT r;COLORREF background,foreground;const WCHAR *tab;
    menu_text item;BOOL result=TRUE,disabled;
    if(!dc || !bounds || !font || !p || !text || dpi<48 || dpi>768 ||
       bounds->right<=bounds->left || bounds->bottom<=bounds->top ||
       kind<WIN16_STYLE_MENU_ROW || kind>WIN16_STYLE_MENU_BAR)return FALSE;
    saved=SaveDC(dc);if(!saved)return FALSE;
    IntersectClipRect(dc,bounds->left,bounds->top,bounds->right,bounds->bottom);
    u=max(1,MulDiv(1,(int)dpi,96));padding=MulDiv(kind==WIN16_STYLE_MENU_BAR?6:18,(int)dpi,96);
    background=(state&ODS_SELECTED)?p->active:p->light;
    foreground=(state&ODS_SELECTED)?p->active_text:p->edge;
    SetDCBrushColor(dc,background);FillRect(dc,bounds,(HBRUSH)GetStockObject(DC_BRUSH));
    if(kind==WIN16_STYLE_MENU_SEPARATOR){
        r=*bounds;r.top=(r.top+r.bottom)/2;r.bottom=r.top+u;
        SetDCBrushColor(dc,p->edge);FillRect(dc,&r,(HBRUSH)GetStockObject(DC_BRUSH));
    }else{
        SelectObject(dc,font);SetBkMode(dc,TRANSPARENT);SetTextColor(dc,foreground);
        tab=wcschr(text,L'\t');item.text=text;item.count=tab?(int)(tab-text):-1;
        item.bounds=*bounds;item.bounds.left+=padding;item.bounds.right-=MulDiv(kind==WIN16_STYLE_MENU_BAR?6:submenu?12:4,(int)dpi,96);
        if(columns && kind==WIN16_STYLE_MENU_ROW)item.bounds.right=item.bounds.left+columns->label;
        item.flags=DT_LEFT|DT_SINGLELINE|DT_VCENTER;
        disabled=(state&(ODS_DISABLED|ODS_GRAYED))!=0;
        /* DSS_UNION produces the original stippled silhouette, unlike the
         * raised/shadowed DSS_DISABLED treatment of later Windows styles. */
        result=render_text(dc,item,disabled,foreground);
        if(tab && tab[1]){
            item.text=tab+1;item.count=-1;item.flags=DT_LEFT|DT_SINGLELINE|DT_VCENTER|DT_NOPREFIX;
            if(columns){item.bounds.left=bounds->left+padding+columns->label+MulDiv(8,(int)dpi,96);
                item.bounds.right=item.bounds.left+columns->shortcut;}
            else {item.bounds=*bounds;item.bounds.left+=padding;item.bounds.right-=padding;item.flags|=DT_RIGHT;}
            result=render_text(dc,item,disabled,foreground)&&result;
        }
        if(kind==WIN16_STYLE_MENU_ROW && (state&ODS_CHECKED))glyph(dc,bounds->left+MulDiv(2,(int)dpi,96),
            (bounds->top+bounds->bottom-MulDiv(14,(int)dpi,96))/2,&menu_check[0][0],14,14,dpi,foreground);
        if(kind==WIN16_STYLE_MENU_ROW && submenu)glyph(dc,bounds->right-MulDiv(11,(int)dpi,96),
            (bounds->top+bounds->bottom-MulDiv(11,(int)dpi,96))/2,&menu_arrow[0][0],7,11,dpi,foreground);
    }
    RestoreDC(dc,saved);return result;
}

BOOL win16_style_menu_text(HDC dc,const RECT *bounds,HFONT font,UINT dpi,
    const win16_style_palette *p,const WCHAR *text,UINT state,win16_style_menu_kind kind,BOOL submenu)
{
    win16_style_menu_columns c={0};
    if(!win16_style_menu_columns_read(dc,font,dpi,text,&c))return FALSE;
    return win16_style_menu_text_columns(dc,bounds,font,dpi,p,text,state,kind,submenu,&c);
}
