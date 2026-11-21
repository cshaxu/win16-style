/* Classic top tabs follow the Windows 2000 comctl32 tab.c geometry. */
#include "tab_header.h"
#include "../common/palette.h"
#include "lib-win32-style/application/control/tab_snapshot.h"

typedef struct tab_paint {
    HFONT font;
    int hot;
    win2k_palette palette;
    COLORREF normal,selected,hot_text;
    style_line_profile profile;
    RECT page;
    int unit;
} tab_paint;

static void surface(HWND window,void *context,HDC dc,const style_control_tab_snapshot *snapshot)
{
    tab_paint *paint=context;
    paint->unit=max(1,MulDiv(1,(int)GetDpiForWindow(window),96));
    paint->page=style_paint_tab_page(dc,snapshot->client,snapshot->page,paint->unit,paint->profile,&paint->palette);
}

static void prepare(HWND window,void *context,HDC dc,const style_control_tab_snapshot *snapshot)
{
    (void)window;(void)snapshot;
    HFONT font=((const tab_paint *)context)->font;
    SelectObject(dc,font?font:GetStockObject(DEFAULT_GUI_FONT));
    SetBkMode(dc,TRANSPARENT);
}

static void paint_item(HWND window,void *context,HDC dc,const style_control_tab_snapshot *snapshot,
    const style_control_tab_item *item)
{
    const tab_paint *paint=context;
    const win2k_palette *palette=&paint->palette;
    RECT content=item->rect;
    BOOL enabled=IsWindowEnabled(window);
    BOOL highlight=(item->native.dwState&TCIS_HIGHLIGHTED)!=0;
    if(item->active)OffsetRect(&content,0,-2);
    int saved=SaveDC(dc);
    if(!saved)return;
    RECT frame=style_paint_tab_item(dc,item->rect,paint->page,item->active,paint->unit,paint->profile,
        item->active?paint->selected:paint->normal,palette);
    IntersectClipRect(dc,frame.left,frame.top,frame.right,frame.bottom);
    content.left+=2;
    content.right-=2;
    RECT measured={0};
    DrawTextW(dc,item->text,-1,&measured,DT_CALCRECT|DT_SINGLELINE);
    int gap=item->image && *item->text?3:0;
    int width=measured.right+(item->image?item->image_size.cx+gap:0);
    content.left+=max(0,(content.right-content.left-width)/2);
    if(item->image){
        int y=content.top+(content.bottom-content.top-item->image_size.cy)/2;
        ImageList_DrawEx(item->images,item->native.iImage,dc,content.left,y,0,0,CLR_NONE,
            highlight?palette->selection:CLR_DEFAULT,highlight?ILD_BLEND50:ILD_NORMAL);
        content.left+=item->image_size.cx+gap;
    }
    RECT label=content;
    label.right=min(label.right,label.left+measured.right);
    label.top+=(label.bottom-label.top-measured.bottom)/2;
    label.bottom=label.top+measured.bottom;
    if(highlight)style_paint_color(dc,&label,palette->selection);
    SetTextColor(dc,highlight?palette->selection_text:
        item->index==paint->hot?paint->hot_text:palette->text);
    UINT flags=DT_LEFT|DT_VCENTER|DT_SINGLELINE;
    if(snapshot->cues&UISF_HIDEACCEL)flags|=DT_HIDEPREFIX;
    DrawTextW(dc,item->text,-1,&content,flags);
    if(enabled && GetFocus()==window && item->index==snapshot->focus && !(snapshot->cues&UISF_HIDEFOCUS)){
        InflateRect(&label,1,1);
        DrawFocusRect(dc,&label);
    }
    RestoreDC(dc,saved);
}

BOOL win2k_tab_paint(HWND window,HDC dc,HFONT font,const style_instance *instance,
    const style_control_native_state *state,const classic_tab_colors *colors,style_line_profile profile,const win2k_palette *palette)
{
    if(!profile)return FALSE;
    const unsigned epoch=state->epoch;
    tab_paint paint={.font=font,.hot=style_control_tab_hot_item(window),.palette=*palette,
        .normal=palette->face,.selected=palette->face,
        .hot_text=colors->has_hot_text?colors->hot_text:GetSysColor(COLOR_HOTLIGHT),
        .profile=profile};
    if(colors->present){
        paint.palette.face=colors->pane;
        paint.palette.dark=colors->border;
        paint.normal=colors->normal;
        paint.selected=colors->selected;
    }
    if(!style_control_native_current(window,instance,state,epoch))return FALSE;
    const style_control_tab_painter painter={&paint,surface,prepare,paint_item};
    return style_control_tab_paint(window,dc,instance,state,&painter);
}
