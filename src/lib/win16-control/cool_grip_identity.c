#include "cool_grip_identity.h"
#include "special_paint.h"

BOOL win16_control_cool_grip_identity(HWND w,void *owner,const NMCUSTOMDRAW *n,WNDPROC native,BOOL (*valid)(HWND,void *))
{
    RECT band={0},border={0},client={0},child={0},overlap;POINT wo={0},vo={0};WCHAR text[2]={0xffff,0xffff};
    REBARBANDINFOW info={.cbSize=REBARBANDINFOW_V3_SIZE,.fMask=RBBIM_BACKGROUND|RBBIM_STYLE|RBBIM_CHILD|RBBIM_TEXT|RBBIM_IMAGE|RBBIM_COLORS,.lpText=text,.cch=ARRAYSIZE(text),.iImage=-1};
    if(!valid(w,owner) || !native || !n->hdc || WindowFromDC(n->hdc)!=w || GetMapMode(n->hdc)!=MM_TEXT || (GetLayout(n->hdc)&LAYOUT_RTL) ||
       !GetWindowOrgEx(n->hdc,&wo) || !GetViewportOrgEx(n->hdc,&vo) || wo.x || wo.y || vo.x || vo.y ||
       (GetWindowLongPtrW(w,GWL_STYLE)&CCS_VERT))return FALSE;
    LRESULT i=CallWindowProcW(native,w,RB_IDTOINDEX,n->dwItemSpec,0);
    if(i<0 || !valid(w,owner) || !CallWindowProcW(native,w,RB_GETRECT,(WPARAM)i,(LPARAM)&band) || !valid(w,owner) ||
       !CallWindowProcW(native,w,RB_GETBANDINFOW,(WPARAM)i,(LPARAM)&info) || !valid(w,owner))return FALSE;
    CallWindowProcW(native,w,RB_GETBANDBORDERS,(WPARAM)i,(LPARAM)&border);
    if(!valid(w,owner))return FALSE;
    COLORREF barback=(COLORREF)CallWindowProcW(native,w,RB_GETBKCOLOR,0,0);
    if(!valid(w,owner))return FALSE;
    COLORREF barfore=(COLORREF)CallWindowProcW(native,w,RB_GETTEXTCOLOR,0,0);
    if(!valid(w,owner) || (barback!=CLR_NONE && barback!=GetSysColor(COLOR_BTNFACE)) ||
       (barfore!=CLR_NONE && barfore!=GetSysColor(COLOR_BTNTEXT)) ||
       info.clrBack!=GetSysColor(COLOR_BTNFACE) || info.clrFore!=GetSysColor(COLOR_BTNTEXT) || info.hbmBack || text[0] || info.cch>ARRAYSIZE(text) || info.iImage!=-1 || !info.hwndChild || GetParent(info.hwndChild)!=w ||
       (info.fStyle&~(RBBS_BREAK|RBBS_FIXEDBMP|RBBS_GRIPPERALWAYS)) || !GetClientRect(w,&client) || !GetWindowRect(info.hwndChild,&child))return FALSE;
    MapWindowPoints(NULL,w,(POINT*)&child,2);UINT dpi=GetDpiForWindow(w);RECT r=n->rc;
    return valid(w,owner) && dpi && !IsRectEmpty(&r) && r.left==band.left && r.top==band.top && r.bottom==band.bottom &&
        border.left>0 && r.right<=band.left+border.left && r.right-r.left<=MulDiv(12,dpi,96) &&
        r.left>=client.left && r.top>=client.top && r.right<=client.right && r.bottom<=client.bottom && !IntersectRect(&overlap,&r,&child);
}
LRESULT win16_control_cool_notify(HWND parent,UINT m,WPARAM p,LPARAM l,void *owner,HWND window,BOOL *ours,BOOL *verified,BOOL (*valid)(HWND,void *),BOOL (*grip)(HWND,void *,const NMCUSTOMDRAW *))
{
    if(m!=WM_NOTIFY || !l || !ours || window!=((NMHDR*)l)->hwndFrom || ((NMHDR*)l)->code!=NM_CUSTOMDRAW || !valid(window,owner))return DefSubclassProc(parent,m,p,l);
    NMCUSTOMDRAW *n=(NMCUSTOMDRAW*)l;
    if(n->dwDrawStage==CDDS_PREPAINT){LRESULT base=DefSubclassProc(parent,m,p,l);*ours=base==CDRF_DODEFAULT && valid(window,owner);return *ours?CDRF_NOTIFYITEMDRAW:base;}
    if(!*ours || !valid(window,owner))return DefSubclassProc(parent,m,p,l);
    if(n->dwDrawStage==CDDS_ITEMPREPAINT)return CDRF_NOTIFYPOSTPAINT;
    if(n->dwDrawStage==CDDS_ITEMPOSTPAINT){
        if(grip(window,owner,n) && win16_control_cool_grip(n->hdc,&n->rc))*verified=TRUE;
        return CDRF_DODEFAULT;
    }
    return DefSubclassProc(parent,m,p,l);
}
