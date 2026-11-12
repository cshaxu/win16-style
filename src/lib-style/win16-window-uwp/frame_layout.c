#include "frame_layout.h"
#include "lib-style/uwp/surface.h"

BOOL win16_uwp_frame_layout(SIZE outer,UINT dpi,DWORD style,BOOL maximized,
    RECT content,win16_style_layout *l)
{
    if(!l || content.left<0 || content.top<0 || content.right>outer.cx ||
       content.bottom>outer.cy || content.left>=content.right || content.top>=content.bottom ||
       (style&WS_CAPTION)!=WS_CAPTION ||
       !win16_style_measure(outer.cx,outer.cy,dpi,style,0,maximized,l))return FALSE;
    int top=l->border,bottom=content.top-l->unit;
    if(bottom-top<max(20,MulDiv(18,(int)dpi,96)))return FALSE;
    int pitch=bottom-top+l->unit;
    int buttons=!!(style&WS_SYSMENU)+!!(style&WS_MAXIMIZEBOX)+!!(style&WS_MINIMIZEBOX);
    if(l->outer.right-2*l->border< (buttons+1)*pitch)return FALSE;
    l->client=content;
    SetRect(&l->caption,l->border,top,outer.cx-l->border,bottom);
    l->title=l->caption;
    SetRectEmpty(&l->system);SetRectEmpty(&l->minimize);SetRectEmpty(&l->maximize);
    if(style&WS_SYSMENU){l->system=l->caption;l->system.right=l->system.left+pitch;l->title.left=l->system.right;}
    int right=l->caption.right;
    if(style&WS_MAXIMIZEBOX){l->maximize=l->caption;l->maximize.left=right-pitch;right-=pitch;}
    if(style&WS_MINIMIZEBOX){l->minimize=l->caption;l->minimize.right=right;l->minimize.left=right-pitch;right-=pitch;}
    l->title.right=right;
    l->corner.cx=l->border+pitch;l->corner.cy=content.top;
    return TRUE;
}

LRESULT win16_uwp_frame_hit(const win16_style_layout *l,POINT p)
{
    LRESULT hit=win16_style_hit(l,p);
    return l?style_uwp_resize_margin_hit(hit,p,&l->outer,&l->client,l->corner,l->sizeable):hit;
}
