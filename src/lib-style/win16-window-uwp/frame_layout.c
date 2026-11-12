#include "frame_layout.h"

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
    if(hit!=HTBORDER || !l || !l->sizeable)return hit;
    /* Native invisible sizing margins can be wider than the classic border.
     * Keep their resize semantics rather than leaving a dead input band. */
    if(p.x<l->client.left)return p.y<l->corner.cy?HTTOPLEFT:
        p.y>=l->outer.bottom-l->corner.cy?HTBOTTOMLEFT:HTLEFT;
    if(p.x>=l->client.right)return p.y<l->corner.cy?HTTOPRIGHT:
        p.y>=l->outer.bottom-l->corner.cy?HTBOTTOMRIGHT:HTRIGHT;
    if(p.y>=l->client.bottom)return p.x<l->corner.cx?HTBOTTOMLEFT:
        p.x>=l->outer.right-l->corner.cx?HTBOTTOMRIGHT:HTBOTTOM;
    return hit;
}
