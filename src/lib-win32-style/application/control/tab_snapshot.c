#include "tab_snapshot.h"
#include <stdlib.h>
#include <wchar.h>

BOOL style_control_tab_supported(HWND window)
{
    DWORD style=(DWORD)GetWindowLongPtrW(window,GWL_STYLE);
    return !(style&(TCS_VERTICAL|TCS_BOTTOM|TCS_BUTTONS|TCS_OWNERDRAWFIXED|TCS_SCROLLOPPOSITE|
        TCS_FORCEICONLEFT|TCS_FORCELABELLEFT));
}

int style_control_tab_hot_item(HWND window)
{
    TCHITTESTINFO hit={0};
    if(!(GetWindowLongPtrW(window,GWL_STYLE)&TCS_HOTTRACK) || !IsWindowEnabled(window) ||
       !GetCursorPos(&hit.pt) || WindowFromPoint(hit.pt)!=window || !ScreenToClient(window,&hit.pt))return -1;
    return (int)SendMessageW(window,TCM_HITTEST,0,(LPARAM)&hit);
}

static WCHAR *item_text(HWND w,int index,TCITEMW *item,const style_instance *instance,const style_control_native_state *state,unsigned epoch)
{
    for(int capacity=128;capacity<=32768;capacity*=2){
        WCHAR *text=calloc((size_t)capacity,sizeof(*text));if(!text)return NULL;
        *item=(TCITEMW){.mask=TCIF_TEXT|TCIF_IMAGE|TCIF_STATE,.dwStateMask=TCIS_BUTTONPRESSED|TCIS_HIGHLIGHTED,
            .pszText=text,.cchTextMax=capacity};
        BOOL ok=(BOOL)SendMessageW(w,TCM_GETITEMW,index,(LPARAM)item);
        text[capacity-1]=0;
        if(!ok || !style_control_native_current(w,instance,state,epoch)){free(text);return NULL;}
        if(wcslen(text)<(size_t)capacity-1)return text;
        free(text);
    }
    return NULL;
}
BOOL style_control_tab_paint(HWND w,HDC dc,const style_instance *instance,const style_control_native_state *state,
    const style_control_tab_painter *painter)
{
    if(!dc)return FALSE;
    style_control_tab_snapshot snapshot={.epoch=state->epoch};
    BOOL painted=FALSE;int saved=SaveDC(dc);if(!saved)return FALSE;
    LRESULT count=SendMessageW(w,TCM_GETITEMCOUNT,0,0);
    if(!style_control_native_current(w,instance,state,snapshot.epoch) || count<0 || count>4096)goto done;
    snapshot.count=(int)count;
    if(!GetClientRect(w,&snapshot.client))goto done;
    if(IsRectEmpty(&snapshot.client)){painted=TRUE;goto done;}
    IntersectClipRect(dc,snapshot.client.left,snapshot.client.top,snapshot.client.right,snapshot.client.bottom);
    snapshot.page=snapshot.client;SendMessageW(w,TCM_ADJUSTRECT,FALSE,(LPARAM)&snapshot.page);
    if(!style_control_native_current(w,instance,state,snapshot.epoch) || IsRectEmpty(&snapshot.page))goto done;
    ExcludeClipRect(dc,snapshot.page.left,snapshot.page.top,snapshot.page.right,snapshot.page.bottom);
    unsigned children=0;
    for(HWND child=GetWindow(w,GW_CHILD);child && children++<256;child=GetWindow(child,GW_HWNDNEXT)){
        RECT r;if(IsWindowVisible(child) && GetWindowRect(child,&r)){
            MapWindowPoints(NULL,w,(POINT*)&r,2);ExcludeClipRect(dc,r.left,r.top,r.right,r.bottom);}
    }
    painter->surface(w,painter->context,dc,&snapshot);
    snapshot.selected=(int)SendMessageW(w,TCM_GETCURSEL,0,0);
    snapshot.focus=(int)SendMessageW(w,TCM_GETCURFOCUS,0,0);
    snapshot.cues=(UINT)SendMessageW(w,WM_QUERYUISTATE,0,0);
    if(!style_control_native_current(w,instance,state,snapshot.epoch))goto done;
    painter->prepare(w,painter->context,dc,&snapshot);
    for(unsigned pass=0;pass<2;++pass)for(int index=0;index<snapshot.count;++index){
        BOOL active=index==snapshot.selected;if(active!=(BOOL)pass)continue;
        style_control_tab_item item={.index=index,.active=active};RECT visible;
        BOOL rect_ok=(BOOL)SendMessageW(w,TCM_GETITEMRECT,index,(LPARAM)&item.rect);
        if(!style_control_native_current(w,instance,state,snapshot.epoch) || !rect_ok)goto done;
        if(!IntersectRect(&visible,&item.rect,&snapshot.client))continue;
        WCHAR *text=item_text(w,index,&item.native,instance,state,snapshot.epoch);if(!text)goto done;
        item.text=text;item.images=(HIMAGELIST)SendMessageW(w,TCM_GETIMAGELIST,0,0);
        if(!style_control_native_current(w,instance,state,snapshot.epoch)){free(text);goto done;}
        /* No application messages between borrowing the list and consuming it. */
        int cx=0,cy=0;
        item.image=item.images && item.native.iImage>=0 && item.native.iImage<ImageList_GetImageCount(item.images) &&
            ImageList_GetIconSize(item.images,&cx,&cy);
        item.image_size=(SIZE){cx,cy};
        painter->item(w,painter->context,dc,&snapshot,&item);
        free(text);
    }
    painted=style_control_native_current(w,instance,state,snapshot.epoch);
done:
    RestoreDC(dc,saved);return painted;
}

typedef struct tab_view {
    style_relief_palette palette;
    COLORREF normal,selected,hot_color;
    HFONT font;
    style_line_profile edge;
    BOOL literal;
    int unit,hot;
    RECT page;
} tab_view;

static void draw_page(HWND window,void *context,HDC dc,const style_control_tab_snapshot *snapshot)
{
    tab_view *view=context;
    view->unit=max(1,MulDiv(1,(int)GetDpiForWindow(window),96));
    view->page=style_paint_tab_page(dc,snapshot->client,snapshot->page,view->unit,view->edge,&view->palette);
}

static void select_font(HWND window,void *context,HDC dc,const style_control_tab_snapshot *snapshot)
{
    (void)window;(void)snapshot;
    const tab_view *view=context;
    SelectObject(dc,view->font?view->font:GetStockObject(view->literal?SYSTEM_FONT:DEFAULT_GUI_FONT));
    SetBkMode(dc,TRANSPARENT);
}

static void draw_image(HDC dc,const style_control_tab_item *item,const RECT *content,
    const tab_view *view,BOOL enabled,BOOL highlighted)
{
    int y=content->top+(content->bottom-content->top-item->image_size.cy)/2;
    int saved=view->literal?SaveDC(dc):0;
    if(view->literal){
        if(!saved)return;
        IntersectClipRect(dc,content->left,content->top,content->right,content->bottom);
    }
    COLORREF ink=view->literal?view->palette.gray:highlighted?view->palette.selection:CLR_DEFAULT;
    BOOL blend=view->literal?!enabled:highlighted;
    ImageList_DrawEx(item->images,item->native.iImage,dc,content->left,y,
        view->literal?item->image_size.cx:0,view->literal?item->image_size.cy:0,
        CLR_NONE,ink,blend?ILD_BLEND50:ILD_NORMAL);
    if(saved)RestoreDC(dc,saved);
}

static void draw_tab(HWND window,void *context,HDC dc,const style_control_tab_snapshot *snapshot,
    const style_control_tab_item *item)
{
    const tab_view *view=context;
    const style_relief_palette *ink=&view->palette;
    BOOL enabled=IsWindowEnabled(window);
    BOOL highlighted=(item->native.dwState&TCIS_HIGHLIGHTED)!=0;
    int saved=SaveDC(dc);
    if(!saved)return;
    COLORREF face=view->literal && highlighted?ink->selection:item->active?view->selected:view->normal;
    RECT frame=style_paint_tab_item(dc,item->rect,view->page,item->active,view->unit,view->edge,face,ink);
    RECT content=view->literal?frame:item->rect;
    int unit=view->literal?view->unit:1;
    InflateRect(&content,-2*unit,view->literal?-unit:0);
    if(!view->literal){
        if(item->active)OffsetRect(&content,0,-2);
        IntersectClipRect(dc,frame.left,frame.top,frame.right,frame.bottom);
    }
    SIZE size={0};
    if(view->literal)GetTextExtentPoint32W(dc,item->text,(int)wcslen(item->text),&size);
    else{
        RECT measured={0};
        DrawTextW(dc,item->text,-1,&measured,DT_CALCRECT|DT_SINGLELINE);
        size=(SIZE){measured.right,measured.bottom};
    }
    int image_width=item->image?item->image_size.cx+(*item->text?3*unit:0):0;
    content.left+=max(0,(content.right-content.left-size.cx-image_width)/2);
    if(item->image){draw_image(dc,item,&content,view,enabled,highlighted);content.left+=image_width;}
    RECT label=content;
    label.right=min(label.right,label.left+size.cx);
    label.top+=(label.bottom-label.top-size.cy)/2;
    label.bottom=label.top+size.cy;
    if(!view->literal && highlighted)style_paint_color(dc,&label,ink->selection);
    SetTextColor(dc,view->literal && !enabled?ink->gray:highlighted?ink->selection_text:
        item->index==view->hot?view->hot_color:ink->text);
    UINT flags=DT_LEFT|DT_VCENTER|DT_SINGLELINE;
    if(view->literal){
        flags|=DT_NOPREFIX|DT_END_ELLIPSIS;
        if(GetWindowLongPtrW(window,GWL_EXSTYLE)&WS_EX_RTLREADING)flags|=DT_RTLREADING;
    }else if(snapshot->cues&UISF_HIDEACCEL)flags|=DT_HIDEPREFIX;
    DrawTextW(dc,item->text,-1,&content,flags);
    if((view->literal || enabled) && GetFocus()==window &&
        item->index==snapshot->focus && !(snapshot->cues&UISF_HIDEFOCUS)){
        if(!view->literal)InflateRect(&label,1,1);
        DrawFocusRect(dc,&label);
    }
    RestoreDC(dc,saved);
}

BOOL style_control_tab_present(HWND window,HDC dc,HFONT font,const style_instance *instance,
    const style_control_native_state *state,const style_tab_colors *colors,style_line_profile edge,
    style_tab_text text,const style_relief_palette *palette)
{
    if(!dc || !edge || !style_control_tab_supported(window))return FALSE;
    unsigned epoch=state->epoch;
    BOOL literal=text==STYLE_TAB_TEXT_LITERAL;
    tab_view view={.font=font,.edge=edge,.literal=literal,.palette=*palette,
        .normal=palette->face,.selected=literal?palette->highlight:palette->face,
        .hot_color=colors->has_hot_text?colors->hot_text:GetSysColor(COLOR_HOTLIGHT),
        .hot=(!literal || colors->has_hot_text)?style_control_tab_hot_item(window):-1};
    if(!style_control_native_current(window,instance,state,epoch))return FALSE;
    if(colors->present){
        view.palette.face=colors->pane;
        view.palette.dark=colors->border;
        view.normal=colors->normal;
        view.selected=colors->selected;
    }
    const style_control_tab_painter painter={&view,draw_page,select_font,draw_tab};
    return style_control_tab_paint(window,dc,instance,state,&painter);
}
