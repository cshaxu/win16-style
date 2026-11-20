#include "trackbar_paint.h"
#include <commctrl.h>

static void outlined_part(HDC dc,RECT r,UINT dpi,BOOL channel,BOOL pressed,
    COLORREF trough,const style_relief_palette *palette)
{
    int unit=max(1,MulDiv(1,(int)dpi,96));
    style_paint_outlined_face(dc,r,unit,!channel,pressed,channel?trough:palette->face,palette);
}

static RECT transpose(RECT r)
{
    return (RECT){r.top,r.left,r.bottom,r.right};
}

static BOOL thumb_fill(HDC dc,const RECT *r,BOOL patterned,const style_relief_palette *palette)
{
    return patterned?style_paint_dither(dc,r,palette):style_paint_color(dc,r,palette->face);
}

/* The original DrawThumb uses an odd width, a soft raised body with one open
 * side and a 45-degree point. The native item rectangle determines its size. */
static BOOL pointed_thumb(HDC dc,RECT rect,DWORD style,BOOL patterned,const style_relief_palette *p)
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
       !style_paint_relief(dc,&body,EDGE_RAISED,(BF_RECT&~open)|BF_SOFT,p))return FALSE;

    int center=r.left+nose;
    for(int depth=0;depth<nose;++depth){
        int y=at_start?r.top+depth:r.bottom-1-depth;
        RECT row={center-depth,y,center+depth+1,y+1};
        RECT pixels=vertical?transpose(row):row;
        if(!thumb_fill(dc,&pixels,patterned,p))return FALSE;
        RECT dark=row;dark.left=dark.right-1;
        pixels=vertical?transpose(dark):dark;
        if(!style_paint_color(dc,&pixels,p->dark))return FALSE;
        if(depth){
            RECT light=row;light.right=light.left+1;
            RECT shadow=dark;OffsetRect(&shadow,-1,0);
            RECT a=vertical?transpose(light):light,b=vertical?transpose(shadow):shadow;
            if(!style_paint_color(dc,&a,p->highlight) || !style_paint_color(dc,&b,p->shadow))return FALSE;
        }
    }
    return TRUE;
}

BOOL style_trackbar_paint(HDC dc,RECT r,DWORD style,UINT dpi,BOOL channel,BOOL pressed,
    BOOL disabled,style_trackbar_profile profile,COLORREF trough,const style_relief_palette *palette)
{
    int saved=SaveDC(dc);
    if(!saved)return FALSE;
    BOOL painted=TRUE;
    if(profile==STYLE_TRACKBAR_OUTLINED){
        outlined_part(dc,r,dpi,channel,pressed,trough,palette);
    }else{
        painted=IntersectClipRect(dc,r.left,r.top,r.right,r.bottom)!=ERROR;
        if(painted && !channel && !(style&TBS_BOTH))
            painted=pointed_thumb(dc,r,style,pressed||disabled,palette);
        else if(painted)
            painted=style_paint_relief(dc,&r,channel?EDGE_SUNKEN:EDGE_RAISED,
                BF_RECT|BF_ADJUST|(channel?0:BF_SOFT),palette) &&
                (channel?style_paint_color(dc,&r,trough):thumb_fill(dc,&r,pressed||disabled,palette));
    }
    return RestoreDC(dc,saved) && painted;
}
