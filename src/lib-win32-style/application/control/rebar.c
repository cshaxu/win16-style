#include "rebar.h"
BOOL style_control_rebar_read(const style_control_rebar_scope *scope,const NMCUSTOMDRAW *draw,DWORD rejected,style_control_rebar_probe *probe)
{
    *probe=(style_control_rebar_probe){0};HWND w=scope->window;RECT band={0},border={0};POINT wo={0},vo={0};WCHAR text[2]={0xffff,0xffff};
    REBARBANDINFOW info={.cbSize=REBARBANDINFOW_V3_SIZE,.fMask=RBBIM_BACKGROUND|RBBIM_STYLE|RBBIM_CHILD|RBBIM_TEXT|RBBIM_IMAGE|RBBIM_COLORS,.lpText=text,.cch=ARRAYSIZE(text),.iImage=-1};
    if(!scope->valid(w,scope->owner) || !scope->native || !draw->hdc || WindowFromDC(draw->hdc)!=w || GetMapMode(draw->hdc)!=MM_TEXT || (GetLayout(draw->hdc)&LAYOUT_RTL) ||
       !GetWindowOrgEx(draw->hdc,&wo) || !GetViewportOrgEx(draw->hdc,&vo) || wo.x || wo.y || vo.x || vo.y || (GetWindowLongPtrW(w,GWL_STYLE)&rejected))return FALSE;
    LRESULT i=CallWindowProcW(scope->native,w,RB_IDTOINDEX,draw->dwItemSpec,0);
    if(i<0 || !scope->valid(w,scope->owner) || !CallWindowProcW(scope->native,w,RB_GETRECT,(WPARAM)i,(LPARAM)&band) || !scope->valid(w,scope->owner) ||
       !CallWindowProcW(scope->native,w,RB_GETBANDINFOW,(WPARAM)i,(LPARAM)&info) || !scope->valid(w,scope->owner))return FALSE;
    CallWindowProcW(scope->native,w,RB_GETBANDBORDERS,(WPARAM)i,(LPARAM)&border);
    if(!scope->valid(w,scope->owner))return FALSE;
    COLORREF back=(COLORREF)CallWindowProcW(scope->native,w,RB_GETBKCOLOR,0,0);
    if(!scope->valid(w,scope->owner))return FALSE;
    COLORREF fore=(COLORREF)CallWindowProcW(scope->native,w,RB_GETTEXTCOLOR,0,0);
    if(!scope->valid(w,scope->owner))return FALSE;
    *probe=(style_control_rebar_probe){.band=band,.border=border,.child=info.hwndChild,.bitmap=info.hbmBack,.style=info.fStyle,.text_capacity=info.cch,
        .text={text[0],text[1]},.image=info.iImage,.bar_back=back,.bar_fore=fore,.back=info.clrBack,.fore=info.clrFore};return TRUE;
}
BOOL style_control_rebar_geometry(const style_control_rebar_scope *scope,style_control_rebar_probe *probe)
{
    RECT client,child;if(!GetClientRect(scope->window,&client) || !GetWindowRect(probe->child,&child))return FALSE;
    MapWindowPoints(NULL,scope->window,(POINT *)&child,2);UINT dpi=GetDpiForWindow(scope->window);
    if(!scope->valid(scope->window,scope->owner) || !dpi)return FALSE;
    probe->client=client;probe->child_rect=child;probe->dpi=dpi;return TRUE;
}
