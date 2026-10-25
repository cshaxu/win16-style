#include "tab_paint.h"
#include <commctrl.h>
#include <stdlib.h>
#include <wchar.h>

BOOL win16_style_tab_supported(HWND w)
{
    DWORD style=(DWORD)GetWindowLongPtrW(w,GWL_STYLE);
    return !(style&(TCS_VERTICAL|TCS_BOTTOM|TCS_BUTTONS|TCS_OWNERDRAWFIXED|TCS_SCROLLOPPOSITE|
        TCS_FORCEICONLEFT|TCS_FORCELABELLEFT));
}
BOOL win16_style_tab_changes(UINT m)
{
    switch(m){
    case WM_SETFONT:case WM_SETFOCUS:case WM_KILLFOCUS:case WM_ENABLE:case WM_SIZE:
    case WM_STYLECHANGED:case WM_UPDATEUISTATE:case WM_THEMECHANGED:case WM_MOUSEMOVE:case WM_MOUSELEAVE:
    case WM_LBUTTONDOWN:case WM_LBUTTONUP:case WM_KEYDOWN:case WM_KEYUP:
    case TCM_INSERTITEMA:case TCM_INSERTITEMW:case TCM_DELETEITEM:case TCM_DELETEALLITEMS:
    case TCM_SETCURSEL:case TCM_SETCURFOCUS:case TCM_SETITEMA:case TCM_SETITEMW:
    case TCM_SETPADDING:case TCM_SETITEMSIZE:case TCM_SETIMAGELIST:case TCM_SETMINTABWIDTH:
    case TCM_HIGHLIGHTITEM:case TCM_DESELECTALL:case TCM_SETEXTENDEDSTYLE:return TRUE;
    default:return FALSE;
    }
}
static BOOL valid(HWND w,const BOOL *detached,const unsigned *epoch,unsigned captured)
{return !*detached && *epoch==captured && IsWindow(w);}
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
/* No TCIF_PARAM: an application may have enlarged the item-data contract
 * using TCM_SETITEMEXTRA. Only public fixed header fields are requested. */
static WCHAR *item_text(HWND w,int index,TCITEMW *item,const BOOL *detached,const unsigned *epoch,unsigned captured)
{
    for(int capacity=128;capacity<=32768;capacity*=2){
        WCHAR *text=calloc((size_t)capacity,sizeof(*text));if(!text)return NULL;
        *item=(TCITEMW){.mask=TCIF_TEXT|TCIF_IMAGE|TCIF_STATE,.dwStateMask=TCIS_BUTTONPRESSED|TCIS_HIGHLIGHTED,
            .pszText=text,.cchTextMax=capacity};
        BOOL ok=(BOOL)SendMessageW(w,TCM_GETITEMW,index,(LPARAM)item);
        text[capacity-1]=0;
        if(!ok || !valid(w,detached,epoch,captured)){free(text);return NULL;}
        if(wcslen(text)<(size_t)capacity-1)return text;
        free(text);
    }
    return NULL;
}
BOOL win16_style_tab_paint(HWND w,HDC dc,const win16_style_palette *p,HFONT font,
    const BOOL *detached,const unsigned *epoch)
{
    if(!dc || !win16_style_tab_supported(w))return FALSE;
    unsigned captured=*epoch;BOOL painted=FALSE;int saved=SaveDC(dc);if(!saved)return FALSE;
    RECT client,page,body;LRESULT count=SendMessageW(w,TCM_GETITEMCOUNT,0,0);
    if(!valid(w,detached,epoch,captured) || count<0 || count>4096)goto done;
    if(!GetClientRect(w,&client))goto done;
    if(IsRectEmpty(&client)){painted=TRUE;goto done;}
    IntersectClipRect(dc,client.left,client.top,client.right,client.bottom);
    page=client;SendMessageW(w,TCM_ADJUSTRECT,FALSE,(LPARAM)&page);
    if(!valid(w,detached,epoch,captured) || IsRectEmpty(&page))goto done;
    /* Paint the strip/border, never an application's page interior. Native
     * child windows (including the single-row scroll buttons) also keep their
     * own pixels. No parent/page styles or coordinates are changed. */
    ExcludeClipRect(dc,page.left,page.top,page.right,page.bottom);
    unsigned children=0;
    for(HWND child=GetWindow(w,GW_CHILD);child && children++<256;child=GetWindow(child,GW_HWNDNEXT)){
        RECT r;if(IsWindowVisible(child) && GetWindowRect(child,&r)){
            MapWindowPoints(NULL,w,(POINT*)&r,2);ExcludeClipRect(dc,r.left,r.top,r.right,r.bottom);}
    }
    int u=max(1,MulDiv(1,(int)GetDpiForWindow(w),96));body=client;body.top=max(client.top,page.top-3*u);
    fill(dc,&client,p->face);edge(dc,body,p->edge,u,TRUE);
    LRESULT selected=SendMessageW(w,TCM_GETCURSEL,0,0);
    LRESULT focus=SendMessageW(w,TCM_GETCURFOCUS,0,0);
    LRESULT cues=SendMessageW(w,WM_QUERYUISTATE,0,0);
    if(!valid(w,detached,epoch,captured))goto done;
    SelectObject(dc,font?font:GetStockObject(SYSTEM_FONT));SetBkMode(dc,TRANSPARENT);
    for(unsigned pass=0;pass<2;++pass)for(int index=0;index<count;++index){
        BOOL active=index==selected;if(active!=(BOOL)pass)continue;
        RECT r,visible;TCITEMW item;WCHAR *text=NULL;
        BOOL rect_ok=(BOOL)SendMessageW(w,TCM_GETITEMRECT,index,(LPARAM)&r);
        if(!valid(w,detached,epoch,captured) || !rect_ok)goto done;
        if(!IntersectRect(&visible,&r,&client))continue;
        text=item_text(w,index,&item,detached,epoch,captured);if(!text)goto done;
        HIMAGELIST images=(HIMAGELIST)SendMessageW(w,TCM_GETIMAGELIST,0,0);
        if(!valid(w,detached,epoch,captured)){free(text);goto done;}
        /* No application messages between obtaining the borrowed list and
         * drawing it; a getter callback may replace the previous list. */
        int cx=0,cy=0;BOOL image=images && item.iImage>=0 && item.iImage<ImageList_GetImageCount(images) &&
            ImageList_GetIconSize(images,&cx,&cy);
        if(active){InflateRect(&r,u,u);r.bottom=max(r.bottom,body.top+u);}
        BOOL highlighted=(item.dwState&TCIS_HIGHLIGHTED)!=0;
        fill(dc,&r,highlighted?p->active:active?p->light:p->face);edge(dc,r,p->edge,u,!active);
        RECT content=r;InflateRect(&content,-2*u,-u);SIZE size={0};
        GetTextExtentPoint32W(dc,text,(int)wcslen(text),&size);
        int gap=image && *text?3*u:0,total=size.cx+(image?cx+gap:0);
        int left=content.left+max(0,(content.right-content.left-total)/2);
        if(image){int y=content.top+(content.bottom-content.top-cy)/2;int image_dc=SaveDC(dc);
            if(image_dc){IntersectClipRect(dc,content.left,content.top,content.right,content.bottom);
                ImageList_DrawEx(images,item.iImage,dc,left,y,cx,cy,CLR_NONE,p->shadow,IsWindowEnabled(w)?ILD_NORMAL:ILD_BLEND50);
                RestoreDC(dc,image_dc);}
            left+=cx+gap;
        }
        content.left=left;SetTextColor(dc,!IsWindowEnabled(w)?p->shadow:highlighted?p->active_text:p->edge);
        UINT flags=DT_LEFT|DT_VCENTER|DT_SINGLELINE|DT_NOPREFIX|DT_END_ELLIPSIS;
        if(GetWindowLongPtrW(w,GWL_EXSTYLE)&WS_EX_RTLREADING)flags|=DT_RTLREADING;
        DrawTextW(dc,text,-1,&content,flags);
        if(GetFocus()==w && index==focus && !(cues&UISF_HIDEFOCUS)){
            RECT outline=content;outline.right=min(outline.right,outline.left+size.cx);
            outline.top+=(outline.bottom-outline.top-size.cy)/2;outline.bottom=outline.top+size.cy;
            DrawFocusRect(dc,&outline);
        }
        free(text);
    }
    painted=valid(w,detached,epoch,captured);
done:
    RestoreDC(dc,saved);return painted;
}
