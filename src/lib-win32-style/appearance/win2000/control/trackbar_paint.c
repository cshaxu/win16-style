/* The original trackbar.c draws a sunken channel and a soft raised thumb.
 * Let the native notification supply client geometry and interaction state. */
#include "trackbar_paint.h"
#include "../common/paint.h"
#include "lib-win32-style/appearance/msstyles/classic_theme.h"

static RECT transpose(RECT r)
{
    return (RECT){r.top,r.left,r.bottom,r.right};
}

static BOOL thumb_fill(HDC dc,const RECT *r,BOOL patterned,const win2k_palette *palette)
{
    return patterned?win2k_dither(dc,r,palette):win2k_fill(dc,r,palette->face);
}

/* The original DrawThumb uses an odd width, a soft raised body with one open
 * side and a 45-degree point. The native item rectangle determines its size. */
static BOOL pointed_thumb(HDC dc,RECT rect,DWORD style,BOOL patterned,const win2k_palette *p)
{
    BOOL vertical=(style&TBS_VERT)!=0,at_start=(style&TBS_TOP)!=0;
    RECT r=vertical?transpose(rect):rect;
    int width=r.right-r.left,nose=(width-1)/2;
    if(!(width&1) || nose<2 || r.bottom-r.top<=nose+2)return FALSE;
    RECT body=r;
    if(at_start)body.top+=nose;
    else body.bottom-=nose;
    if(vertical)body=transpose(body);
    UINT open=vertical?(at_start?BF_LEFT:BF_RIGHT):(at_start?BF_TOP:BF_BOTTOM);
    if(!thumb_fill(dc,&body,patterned,p) ||
       !win2k_edge(dc,&body,EDGE_RAISED,(BF_RECT&~open)|BF_SOFT,p))return FALSE;

    int center=r.left+nose;
    for(int depth=0;depth<nose;++depth){
        int y=at_start?r.top+depth:r.bottom-1-depth;
        RECT row={center-depth,y,center+depth+1,y+1};
        RECT pixels=vertical?transpose(row):row;
        if(!thumb_fill(dc,&pixels,patterned,p))return FALSE;
        RECT dark=row;dark.left=dark.right-1;
        pixels=vertical?transpose(dark):dark;
        if(!win2k_fill(dc,&pixels,p->dark))return FALSE;
        if(depth){
            RECT light=row;light.right=light.left+1;
            RECT shadow=dark;OffsetRect(&shadow,-1,0);
            RECT a=vertical?transpose(light):light,b=vertical?transpose(shadow):shadow;
            if(!win2k_fill(dc,&a,p->highlight) || !win2k_fill(dc,&b,p->shadow))return FALSE;
        }
    }
    return TRUE;
}

BOOL win2k_trackbar_supported(HWND window)
{
    DWORD style=(DWORD)GetWindowLongPtrW(window,GWL_STYLE);
    DWORD extended=(DWORD)GetWindowLongPtrW(window,GWL_EXSTYLE);
    DWORD allowed=TBS_AUTOTICKS|TBS_VERT|TBS_TOP|TBS_BOTH|TBS_NOTICKS|TBS_FIXEDLENGTH|TBS_NOTHUMB;
    return !(style&(0xffffu&~allowed)) && !(style&(WS_BORDER|WS_HSCROLL|WS_VSCROLL)) &&
        !(extended&(WS_EX_CLIENTEDGE|WS_EX_STATICEDGE|WS_EX_LAYOUTRTL|WS_EX_RTLREADING));
}

BOOL win2k_trackbar_part(HWND window,const NMCUSTOMDRAW *draw,const void *context)
{
    BOOL channel=draw->dwItemSpec==TBCD_CHANNEL;
    DWORD style=(DWORD)GetWindowLongPtrW(window,GWL_STYLE);
    BOOL thumb=draw->dwItemSpec==TBCD_THUMB && !(style&TBS_NOTHUMB);
    RECT r=draw->rc,client;
    HDC dc=draw->hdc;
    if(!dc || (!channel && !thumb) || !GetClientRect(window,&client) ||
       IsRectEmpty(&r) || r.left<0 || r.top<0 || r.right>client.right || r.bottom>client.bottom)return FALSE;
    int saved=SaveDC(dc);
    if(!saved)return FALSE;
    const classic_trackbar_colors *axes=context;
    const classic_trackbar_colors *colors=&axes[(style&TBS_VERT)!=0];
    win2k_palette palette=win2k_standard_palette;
    if(colors->present){
        palette.face=colors->thumb;
        palette.dark=colors->edge;
    }
    const win2k_palette *p=&palette;
    BOOL patterned=thumb && ((draw->uItemState&CDIS_SELECTED) || !IsWindowEnabled(window));
    BOOL painted=IntersectClipRect(dc,r.left,r.top,r.right,r.bottom)!=ERROR;
    if(painted && thumb && !(style&TBS_BOTH))painted=pointed_thumb(dc,r,style,patterned,p);
    else if(painted)painted=win2k_edge(dc,&r,channel?EDGE_SUNKEN:EDGE_RAISED,BF_RECT|BF_ADJUST|(thumb?BF_SOFT:0),p) &&
        (thumb?thumb_fill(dc,&r,patterned,p):win2k_fill(dc,&r,colors->present?colors->channel:p->highlight));
    return RestoreDC(dc,saved) && painted;
}
