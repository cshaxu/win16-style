#include "header_paint.h"
#include <commctrl.h>
#include <stdlib.h>
#include <wchar.h>
#include "lib-win32-style/application/control/header_snapshot.h"

BOOL win16_style_header_supported(HWND w)
{return !(GetWindowLongPtrW(w,GWL_STYLE)&(HDS_FILTERBAR|HDS_CHECKBOXES|HDS_OVERFLOW|HDS_HIDDEN));}

static void fill(HDC dc,RECT r,COLORREF color)
{SetDCBrushColor(dc,color);FillRect(dc,&r,GetStockObject(DC_BRUSH));}
BOOL win16_style_header_paint(HWND w,HDC dc,const win16_style_palette *p,HFONT font,
    const style_instance *instance,const style_control_native_state *state)
{
    BOOL painted=FALSE;int saved=0;style_header_snapshot snapshot;
    if(!dc || !win16_style_header_supported(w) || !style_header_snapshot_read(w,instance,state,HDF_STRING|HDF_JUSTIFYMASK|HDF_RTLREADING,&snapshot))return FALSE;
    RECT client=snapshot.client;int count=snapshot.count;style_header_item *items=snapshot.items;
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
        if(i==state->pressed && GetCapture()==w && (GetWindowLongPtrW(w,GWL_STYLE)&HDS_BUTTONS))OffsetRect(&r,u,u);
        UINT flags=DT_VCENTER|DT_SINGLELINE|DT_NOPREFIX|DT_END_ELLIPSIS;
        int alignment=items[i].format&HDF_JUSTIFYMASK;
        flags|=alignment==HDF_RIGHT?DT_RIGHT:alignment==HDF_CENTER?DT_CENTER:DT_LEFT;
        if((items[i].format&HDF_RTLREADING) || (GetWindowLongPtrW(w,GWL_EXSTYLE)&WS_EX_RTLREADING))flags|=DT_RTLREADING;
        DrawTextW(dc,items[i].text,-1,&r,flags);RestoreDC(dc,inner);
    }
    painted=style_control_native_current(w,instance,state,snapshot.epoch);
done:
    if(saved)RestoreDC(dc,saved);
    style_header_snapshot_clear(&snapshot);return painted;
}
