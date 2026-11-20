#include "row_text.h"
#include <wchar.h>

typedef struct text_run {
    const WCHAR *text;
    int count;
    RECT bounds;
    UINT flags;
} text_run;

static BOOL CALLBACK draw_text(HDC dc,LPARAM data,WPARAM unused,int width,int height)
{
    const text_run *run=(const text_run *)data;
    RECT bounds=run->bounds;
    (void)unused;(void)width;(void)height;
    return DrawTextW(dc,run->text,run->count,&bounds,run->flags)!=0;
}

static BOOL draw_run(HDC dc,text_run run,BOOL stippled,COLORREF color)
{
    if(!run.count || !run.text[0])return TRUE;
    if(!stippled)return draw_text(dc,(LPARAM)&run,0,0,0);
    if(IsRectEmpty(&run.bounds))return TRUE;
    HBRUSH brush=CreateSolidBrush(color);
    if(!brush)return FALSE;
    int x=run.bounds.left,y=run.bounds.top;
    OffsetRect(&run.bounds,-x,-y);
    BOOL result=DrawStateW(dc,brush,draw_text,(LPARAM)&run,0,x,y,
        run.bounds.right,run.bounds.bottom,DST_COMPLEX|DSS_UNION|DSS_MONO);
    DeleteObject(brush);
    return result;
}

int style_menu_label_width(HDC dc,const WCHAR *text)
{
    const WCHAR *tab=wcschr(text,L'\t');
    RECT bounds={0};
    if(tab!=text)DrawTextW(dc,text,tab?(int)(tab-text):-1,&bounds,DT_CALCRECT|DT_SINGLELINE);
    return bounds.right;
}

HFONT style_menu_bold_font(HFONT font)
{
    LOGFONTW logical;
    if(!font || GetObjectW(font,sizeof(logical),&logical)!=sizeof(logical))return NULL;
    logical.lfWeight=FW_BOLD;
    return CreateFontIndirectW(&logical);
}

BOOL style_menu_draw_label(HDC dc,const WCHAR *text,const RECT *bounds,BOOL centered,BOOL stippled,COLORREF color)
{
    return draw_run(dc,(text_run){text,-1,*bounds,DT_SINGLELINE|DT_VCENTER|(centered?DT_CENTER:DT_LEFT)},
        stippled,color);
}

BOOL style_menu_draw_labels(HDC dc,const WCHAR *text,const style_menu_text_layout *layout,
    BOOL stippled,COLORREF color)
{
    const WCHAR *tab=wcschr(text,L'\t');
    const UINT flags=DT_SINGLELINE|DT_VCENTER;
    text_run label={text,tab?(int)(tab-text):-1,layout->label,flags|(layout->label_center?DT_CENTER:DT_LEFT)};
    if(!tab || !tab[1])return draw_run(dc,label,stippled,color);
    text_run shortcut={tab+1,-1,layout->shortcut,flags|DT_NOPREFIX|
        (layout->shortcut_right?DT_RIGHT:DT_LEFT)};
    if(layout->shortcut_first){
        BOOL first=draw_run(dc,shortcut,stippled,color);
        return draw_run(dc,label,stippled,color)&&first;
    }
    BOOL first=draw_run(dc,label,stippled,color);
    return draw_run(dc,shortcut,stippled,color)&&first;
}
