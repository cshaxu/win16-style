/* Classic top tabs follow the Windows 2000 comctl32 tab.c geometry. */
#include "tab_header.h"
#include "../common/paint.h"
#include "lib-win32-style/application/control/tab_snapshot.h"

typedef struct tab_paint {
    HFONT font;
    int hot;
    win2k_palette palette;
    COLORREF normal,selected;
} tab_paint;

BOOL win2k_tab_supported(HWND window)
{
    DWORD style=(DWORD)GetWindowLongPtrW(window,GWL_STYLE);
    return !(style&(TCS_VERTICAL|TCS_BOTTOM|TCS_BUTTONS|TCS_OWNERDRAWFIXED|TCS_SCROLLOPPOSITE|
        TCS_FORCEICONLEFT|TCS_FORCELABELLEFT));
}

static void surface(HWND window,void *context,HDC dc,const style_control_tab_snapshot *snapshot)
{
    (void)window;
    const win2k_palette *palette=&((const tab_paint *)context)->palette;
    RECT page=snapshot->page;
    InflateRect(&page,4,4);
    page.top+=2;
    style_paint_color(dc,&snapshot->client,palette->face);
    style_paint_relief(dc,&page,EDGE_RAISED,BF_RECT|BF_SOFT,palette);
}

static void prepare(HWND window,void *context,HDC dc,const style_control_tab_snapshot *snapshot)
{
    (void)window;(void)snapshot;
    HFONT font=((const tab_paint *)context)->font;
    SelectObject(dc,font?font:GetStockObject(DEFAULT_GUI_FONT));
    SetBkMode(dc,TRANSPARENT);
}

static void pixel(HDC dc,int x,int y,COLORREF color)
{
    RECT cell={x,y,x+1,y+1};
    style_paint_color(dc,&cell,color);
}

/* The original DoCorners clears two columns, then draws the two diagonal
 * soft edge layers in the remaining two rows. */
static void corners(HDC dc,const RECT *r,const win2k_palette *palette)
{
    if(r->right-r->left<4 || r->bottom-r->top<3)return;
    RECT left={r->left,r->top,r->left+2,r->top+3};
    RECT right={r->right-2,r->top,r->right,r->top+3};
    style_paint_color(dc,&left,palette->face);
    style_paint_color(dc,&right,palette->face);
    pixel(dc,left.left+1,left.top+1,palette->highlight);
    pixel(dc,left.left,left.top+2,palette->highlight);
    pixel(dc,left.left+1,left.top+2,palette->light);
    pixel(dc,right.right-2,right.top+1,palette->dark);
    pixel(dc,right.right-1,right.top+2,palette->dark);
    pixel(dc,right.right-2,right.top+2,palette->shadow);
}

static void paint_item(HWND window,void *context,HDC dc,const style_control_tab_snapshot *snapshot,
    const style_control_tab_item *item)
{
    const tab_paint *paint=context;
    const win2k_palette *palette=&paint->palette;
    RECT frame=item->rect,content=item->rect;
    BOOL enabled=IsWindowEnabled(window);
    BOOL highlight=(item->native.dwState&TCIS_HIGHLIGHTED)!=0;
    if(item->active){
        InflateRect(&frame,2,2);
        --frame.bottom;
        OffsetRect(&content,0,-2);
    }
    int saved=SaveDC(dc);
    if(!saved)return;
    IntersectClipRect(dc,frame.left,frame.top,frame.right,frame.bottom);
    style_paint_color(dc,&frame,item->active?paint->selected:paint->normal);
    style_paint_relief(dc,&frame,EDGE_RAISED,BF_LEFT|BF_TOP|BF_RIGHT|BF_SOFT,palette);
    corners(dc,&frame,palette);
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
        item->index==paint->hot?GetSysColor(COLOR_HOTLIGHT):palette->text);
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
    const style_control_native_state *state,const classic_tab_colors *colors)
{
    const unsigned epoch=state->epoch;
    tab_paint paint={.font=font,.hot=-1,.palette=win2k_standard_palette,
        .normal=win2k_standard_palette.face,.selected=win2k_standard_palette.face};
    if(colors->present){
        paint.palette.face=colors->pane;
        paint.palette.dark=colors->border;
        paint.normal=colors->normal;
        paint.selected=colors->selected;
    }
    TCHITTESTINFO hit={0};
    if((GetWindowLongPtrW(window,GWL_STYLE)&TCS_HOTTRACK) && IsWindowEnabled(window) &&
       GetCursorPos(&hit.pt) && WindowFromPoint(hit.pt)==window && ScreenToClient(window,&hit.pt))
        paint.hot=(int)SendMessageW(window,TCM_HITTEST,0,(LPARAM)&hit);
    if(!style_control_native_current(window,instance,state,epoch))return FALSE;
    const style_control_tab_painter painter={&paint,surface,prepare,paint_item};
    return style_control_tab_paint(window,dc,instance,state,&painter);
}
