/* DrawStatusTextEx in Win2000 status.c uses one edge per ordinary pane.
 * Let native painting finish first, including any owner draw and icon. */
#include "bars.h"
#include "../common/palette.h"
#include "lib-win32-style/application/graphics/bitmap.h"

static DWORD pixel(COLORREF color)
{
    return ((DWORD)GetRValue(color)<<16)|((DWORD)GetGValue(color)<<8)|GetBValue(color);
}

static void tint_grip(HWND window,HDC dc,const RECT *client,int pane_end,const style_instance *instance,
    COLORREF shadow)
{
    if(!(GetWindowLongPtrW(window,GWL_STYLE)&SBARS_SIZEGRIP) || IsZoomed(GetParent(window)))return;
    SIZE size={client->right-pane_end,client->bottom};
    if(size.cx<3 || size.cx>64 || size.cy<3 || size.cy>128)return;
    POINT point={client->right-3,client->bottom-3};
    if(!ClientToScreen(window,&point) ||
        SendMessageW(window,WM_NCHITTEST,0,MAKELPARAM(point.x,point.y))!=HTBOTTOMRIGHT || instance->detached)return;
    DWORD native=pixel(GetSysColor(COLOR_3DSHADOW)),classic=pixel(shadow);
    if(native==classic || GetMapMode(dc)!=MM_TEXT || (GetLayout(dc)&LAYOUT_RTL))return;
    style_bitmap grip={0};
    if(!style_bitmap_ensure(&grip,dc,size))return;
    if(BitBlt(grip.dc,0,0,size.cx,size.cy,dc,pane_end,0,SRCCOPY) && GdiFlush()){
        DWORD *pixels=(DWORD *)grip.pixels;
        size_t count=(size_t)size.cx*(size_t)size.cy;
        /* A custom background can itself be the native shadow color. */
        if((pixels[size.cx+1]&0xffffffu)!=native){
            BOOL changed=FALSE;
            for(size_t i=(size_t)size.cx*2;i<count;++i)
                if((pixels[i]&0xffffffu)==native){pixels[i]=classic;changed=TRUE;}
            if(changed)BitBlt(dc,pane_end,0,size.cx,size.cy,grip.dc,0,0,SRCCOPY);
        }
    }
    style_bitmap_dispose(&grip);
}

void win2k_status_panes(HWND window,HDC dc,const style_instance *instance,
    const style_control_status_state *state,const win2k_palette *palette,const classic_bar_colors *colors)
{
    if(!dc || instance->detached || GetMapMode(dc)!=MM_TEXT || (GetLayout(dc)&LAYOUT_RTL))return;
    unsigned epoch=state->epoch;
    if(!style_control_status_identity(window,instance,state,NULL,NULL))return;
    if(SendMessageW(window,SB_ISSIMPLE,0,0))return;
    LRESULT count=SendMessageW(window,SB_GETPARTS,0,0);
    if(count<1 || count>64)return;
    RECT client,panes[64];
    UINT flags[64];
    if(!GetClientRect(window,&client))return;
    win2k_palette edge=*palette;
    edge.highlight=classic_bar_color(colors,CLASSIC_BAR_HIGHLIGHT,edge.highlight);
    edge.shadow=classic_bar_color(colors,CLASSIC_BAR_SHADOW,edge.shadow);
    for(LRESULT i=0;i<count;++i){
        WPARAM pane=(WPARAM)i;
        flags[i]=HIWORD(SendMessageW(window,SB_GETTEXTLENGTHW,pane,0));
        if(instance->detached || epoch!=state->epoch)return;
        if(!SendMessageW(window,SB_GETRECT,pane,(LPARAM)&panes[i]))SetRectEmpty(&panes[i]);
        if(instance->detached || epoch!=state->epoch)return;
    }
    for(LRESULT i=0;i<count;++i){
        RECT pane=panes[i],inside;
        if((flags[i]&SBT_NOBORDERS) || !IntersectRect(&inside,&client,&pane) || !EqualRect(&inside,&pane))continue;
        style_paint_relief(dc,&pane,(flags[i]&SBT_POPOUT)?BDR_RAISEDINNER:BDR_SUNKENOUTER,BF_RECT,&edge);
    }
    /* The native layout keeps the grip outside the last pane. */
    if(panes[count-1].right>0 && (flags[count-1]&SBT_OWNERDRAW)==0)
        tint_grip(window,dc,&client,panes[count-1].right,instance,palette->shadow);
}
