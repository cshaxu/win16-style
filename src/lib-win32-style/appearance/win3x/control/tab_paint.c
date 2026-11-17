#include "tab_paint.h"
#include <commctrl.h>
#include "lib-win32-style/application/control/tab_snapshot.h"
#include <wchar.h>

BOOL win16_style_tab_supported(HWND w)
{
    DWORD style=(DWORD)GetWindowLongPtrW(w,GWL_STYLE);
    return !(style&(TCS_VERTICAL|TCS_BOTTOM|TCS_BUTTONS|TCS_OWNERDRAWFIXED|TCS_SCROLLOPPOSITE|
        TCS_FORCEICONLEFT|TCS_FORCELABELLEFT));
}

static void fill(HDC dc,const RECT *r,COLORREF color)
{SetDCBrushColor(dc,color);FillRect(dc,r,GetStockObject(DC_BRUSH));}
static void edge(HDC dc,RECT r,COLORREF color,int width,BOOL bottom)
{
    for(int i=0;i<width && r.right>r.left && r.bottom>r.top;++i){
        RECT line=r;line.bottom=line.top+1;fill(dc,&line,color);
        line=r;line.right=line.left+1;fill(dc,&line,color);
        line=r;line.left=line.right-1;fill(dc,&line,color);
        if(bottom){line=r;line.top=line.bottom-1;fill(dc,&line,color);}
        InflateRect(&r,-1,-1);
    }
}
typedef struct tab_paint {
    const win16_style_palette *palette;
    HFONT font;
    int unit;
    RECT body;
} tab_paint;
static void surface(HWND w,void *context,HDC dc,const style_control_tab_snapshot *snapshot)
{
    tab_paint *paint=context;paint->unit=max(1,MulDiv(1,(int)GetDpiForWindow(w),96));
    paint->body=snapshot->client;paint->body.top=max(snapshot->client.top,snapshot->page.top-3*paint->unit);
    fill(dc,&snapshot->client,paint->palette->face);edge(dc,paint->body,paint->palette->edge,paint->unit,TRUE);
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
    RECT r=item->rect,body=paint->body;BOOL active=item->active,image=item->image;
    HIMAGELIST images=item->images;const WCHAR *text=item->text;
    if(active){InflateRect(&r,u,u);r.bottom=max(r.bottom,body.top+u);}
    BOOL highlighted=(item->native.dwState&TCIS_HIGHLIGHTED)!=0;
    fill(dc,&r,highlighted?p->active:active?p->light:p->face);edge(dc,r,p->edge,u,!active);
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
    const style_instance *instance,const style_control_native_state *state)
{
    if(!dc || !win16_style_tab_supported(w))return FALSE;
    tab_paint paint={.palette=p,.font=font};
    const style_control_tab_painter painter={&paint,surface,prepare,paint_item};
    return style_control_tab_paint(w,dc,instance,state,&painter);
}
