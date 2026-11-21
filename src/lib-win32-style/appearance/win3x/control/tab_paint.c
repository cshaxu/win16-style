#include "tab_paint.h"
#include <commctrl.h>
#include "lib-win32-style/application/control/tab_snapshot.h"
#include "lib-win32-style/appearance/msstyles/classic_theme.h"
#include <wchar.h>

BOOL win16_style_tab_supported(HWND w)
{
    DWORD style=(DWORD)GetWindowLongPtrW(w,GWL_STYLE);
    return !(style&(TCS_VERTICAL|TCS_BOTTOM|TCS_BUTTONS|TCS_OWNERDRAWFIXED|TCS_SCROLLOPPOSITE|
        TCS_FORCEICONLEFT|TCS_FORCELABELLEFT));
}

typedef struct tab_paint {
    const win16_style_palette *palette;
    classic_tab_colors colors;
    style_relief_palette frame;
    style_line_profile profile;
    HFONT font;
    int unit;
    RECT body;
} tab_paint;
static void surface(HWND w,void *context,HDC dc,const style_control_tab_snapshot *snapshot)
{
    tab_paint *paint=context;paint->unit=max(1,MulDiv(1,(int)GetDpiForWindow(w),96));
    paint->body=style_paint_tab_page(dc,snapshot->client,snapshot->page,paint->unit,paint->profile,&paint->frame);
}
static void prepare(HWND w,void *context,HDC dc,const style_control_tab_snapshot *snapshot)
{
    (void)w;(void)snapshot;tab_paint *paint=context;
    SelectObject(dc,paint->font?paint->font:GetStockObject(SYSTEM_FONT));SetBkMode(dc,TRANSPARENT);
}
static void paint_item(HWND w,void *context,HDC dc,const style_control_tab_snapshot *snapshot,const style_control_tab_item *item)
{
    tab_paint *paint=context;const win16_style_palette *p=paint->palette;
    int u=paint->unit,cx=item->image_size.cx,cy=item->image_size.cy;
    BOOL image=item->image,highlighted=(item->native.dwState&TCIS_HIGHLIGHTED)!=0;
    HIMAGELIST images=item->images;const WCHAR *text=item->text;
    COLORREF face=highlighted?p->active:item->active?paint->colors.selected:paint->colors.normal;
    RECT r=style_paint_tab_item(dc,item->rect,paint->body,item->active,u,paint->profile,face,&paint->frame);
    RECT content=r;InflateRect(&content,-2*u,-u);SIZE size={0};
    GetTextExtentPoint32W(dc,text,(int)wcslen(text),&size);
    int gap=image && *text?3*u:0,total=size.cx+(image?cx+gap:0);
    int left=content.left+max(0,(content.right-content.left-total)/2);
    if(image){int y=content.top+(content.bottom-content.top-cy)/2;int image_dc=SaveDC(dc);
        if(image_dc){IntersectClipRect(dc,content.left,content.top,content.right,content.bottom);
            ImageList_DrawEx(images,item->native.iImage,dc,left,y,cx,cy,CLR_NONE,p->shadow,IsWindowEnabled(w)?ILD_NORMAL:ILD_BLEND50);
            RestoreDC(dc,image_dc);}
        left+=cx+gap;
    }
    content.left=left;SetTextColor(dc,!IsWindowEnabled(w)?p->shadow:highlighted?p->active_text:p->edge);
    UINT flags=DT_LEFT|DT_VCENTER|DT_SINGLELINE|DT_NOPREFIX|DT_END_ELLIPSIS;
    if(GetWindowLongPtrW(w,GWL_EXSTYLE)&WS_EX_RTLREADING)flags|=DT_RTLREADING;
    DrawTextW(dc,text,-1,&content,flags);
    if(GetFocus()==w && item->index==snapshot->focus && !(snapshot->cues&UISF_HIDEFOCUS)){
        RECT outline=content;outline.right=min(outline.right,outline.left+size.cx);
        outline.top+=(outline.bottom-outline.top-size.cy)/2;outline.bottom=outline.top+size.cy;
        DrawFocusRect(dc,&outline);
    }
}
BOOL win16_style_tab_paint(HWND w,HDC dc,const win16_style_palette *p,HFONT font,
    const style_instance *instance,const style_control_native_state *state,const classic_tab_colors *colors,style_line_profile profile)
{
    if(!dc || !profile || !win16_style_tab_supported(w))return FALSE;
    tab_paint paint={.palette=p,.font=font,.profile=profile,
        .colors=colors->present?*colors:(classic_tab_colors){p->face,p->face,p->light,p->edge,FALSE}};
    paint.frame=(style_relief_palette){.face=paint.colors.pane,.dark=paint.colors.border,
        .light=p->face,.highlight=p->light,.shadow=p->shadow};
    const style_control_tab_painter painter={&paint,surface,prepare,paint_item};
    return style_control_tab_paint(w,dc,instance,state,&painter);
}
