#include "header_paint.h"
#include <commctrl.h>
#include <stdlib.h>
#include <wchar.h>

BOOL win16_style_header_supported(HWND w)
{return !(GetWindowLongPtrW(w,GWL_STYLE)&(HDS_FILTERBAR|HDS_CHECKBOXES|HDS_OVERFLOW|HDS_HIDDEN));}
BOOL win16_style_header_changes(UINT m)
{
    switch(m){
    case WM_SIZE:case WM_SETFONT:case WM_STYLECHANGED:case WM_ENABLE:case WM_THEMECHANGED:
    case WM_MOUSEMOVE:case WM_LBUTTONDOWN:case WM_LBUTTONUP:case WM_LBUTTONDBLCLK:
    case WM_CANCELMODE:case WM_CAPTURECHANGED:case WM_SETFOCUS:case WM_KILLFOCUS:
    case WM_KEYDOWN:case WM_KEYUP:case WM_UPDATEUISTATE:
    case HDM_INSERTITEMA:case HDM_INSERTITEMW:case HDM_SETITEMA:case HDM_SETITEMW:case HDM_DELETEITEM:
    case HDM_SETORDERARRAY:case HDM_SETIMAGELIST:case HDM_SETBITMAPMARGIN:case HDM_SETUNICODEFORMAT:
        return TRUE;
    default:return FALSE;
    }
}
typedef struct header_item {RECT rect;WCHAR *text;int format;} header_item;
static BOOL valid(HWND w,const BOOL *detached,const unsigned *epoch,unsigned captured)
{return !*detached && *epoch==captured && IsWindow(w);}
static WCHAR *item_text(HWND w,int index,int *format,const BOOL *detached,const unsigned *epoch,unsigned captured)
{
    for(int capacity=128;capacity<=32768;capacity*=2){
        WCHAR *text=calloc((size_t)capacity,sizeof(*text));if(!text)return NULL;
        HDITEMW item={.mask=HDI_TEXT|HDI_FORMAT,.pszText=text,.cchTextMax=capacity};
        BOOL ok=(BOOL)SendMessageW(w,HDM_GETITEMW,index,(LPARAM)&item);text[capacity-1]=0;
        if(!ok || !valid(w,detached,epoch,captured) || item.pszText!=text ||
            (item.fmt&~(HDF_STRING|HDF_JUSTIFYMASK|HDF_RTLREADING))){free(text);return NULL;}
        if(wcslen(text)<(size_t)capacity-1){*format=item.fmt;return text;}
        free(text);
    }
    return NULL;
}
static void fill(HDC dc,RECT r,COLORREF color)
{SetDCBrushColor(dc,color);FillRect(dc,&r,GetStockObject(DC_BRUSH));}
BOOL win16_style_header_paint(HWND w,HDC dc,const win16_style_palette *p,HFONT font,
    const BOOL *detached,const unsigned *epoch,int pressed)
{
    unsigned captured=*epoch;BOOL painted=FALSE;int saved=0;RECT client;
    if(!dc || !win16_style_header_supported(w))return FALSE;
    LRESULT count=SendMessageW(w,HDM_GETITEMCOUNT,0,0);
    if(!valid(w,detached,epoch,captured) || count<0 || count>1024 || !GetClientRect(w,&client))return FALSE;
    header_item *items=count?calloc((size_t)count,sizeof(*items)):NULL;if(count && !items)return FALSE;
    /* Gather and validate BEFORE touching pixels: special per-item formats
     * preserve their original native/owner-draw rendering as one control. */
    for(int i=0;i<count;++i){
        BOOL ok=(BOOL)SendMessageW(w,HDM_GETITEMRECT,i,(LPARAM)&items[i].rect);
        if(!ok || !valid(w,detached,epoch,captured))goto done;
        items[i].text=item_text(w,i,&items[i].format,detached,epoch,captured);
        if(!items[i].text)goto done;
    }
    saved=SaveDC(dc);if(!saved)goto done;
    IntersectClipRect(dc,client.left,client.top,client.right,client.bottom);
    fill(dc,client,p->face);SelectObject(dc,font?font:GetStockObject(SYSTEM_FONT));SetBkMode(dc,TRANSPARENT);
    SetTextColor(dc,IsWindowEnabled(w)?p->edge:p->shadow);
    int u=max(1,MulDiv(1,(int)GetDpiForWindow(w),96));
    RECT bottom=client;bottom.top=max(bottom.top,bottom.bottom-u);fill(dc,bottom,p->edge);
    for(int i=0;i<count;++i){
        RECT r=items[i].rect,visible;if(!IntersectRect(&visible,&client,&r))continue;
        int inner=SaveDC(dc);if(!inner)goto done;
        IntersectClipRect(dc,r.left,r.top,r.right,r.bottom);
        for(int n=0;n<u && !IsRectEmpty(&r);++n){SetDCBrushColor(dc,p->edge);FrameRect(dc,&r,GetStockObject(DC_BRUSH));InflateRect(&r,-1,-1);}
        InflateRect(&r,-3*u,0);
        if(i==pressed && GetCapture()==w && (GetWindowLongPtrW(w,GWL_STYLE)&HDS_BUTTONS))OffsetRect(&r,u,u);
        UINT flags=DT_VCENTER|DT_SINGLELINE|DT_NOPREFIX|DT_END_ELLIPSIS;
        int alignment=items[i].format&HDF_JUSTIFYMASK;
        flags|=alignment==HDF_RIGHT?DT_RIGHT:alignment==HDF_CENTER?DT_CENTER:DT_LEFT;
        if((items[i].format&HDF_RTLREADING) || (GetWindowLongPtrW(w,GWL_EXSTYLE)&WS_EX_RTLREADING))flags|=DT_RTLREADING;
        DrawTextW(dc,items[i].text,-1,&r,flags);RestoreDC(dc,inner);
    }
    painted=valid(w,detached,epoch,captured);
done:
    if(saved)RestoreDC(dc,saved);
    for(int i=0;i<count;++i)free(items[i].text);
    free(items);return painted;
}
