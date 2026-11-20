#include "modern_frame_layout.h"
#include "lib-win32-style/application/adapters/surface.h"

BOOL win16_uwp_frame_layout(SIZE outer,UINT dpi,DWORD style,BOOL maximized,
    RECT content,const classic_button_group *buttons,const classic_caption_metrics *metrics,win16_style_layout *l)
{
    if(!l || content.left<0 || content.top<0 || content.right>outer.cx ||
       content.bottom>outer.cy || content.left>=content.right || content.top>=content.bottom ||
       (style&WS_CAPTION)!=WS_CAPTION)return FALSE;
    BOOL measured=buttons && buttons->count?
        win16_style_measure_with_buttons(outer.cx,outer.cy,dpi,style,0,maximized,buttons,metrics,NULL,dpi,l):
        win16_style_measure(outer.cx,outer.cy,dpi,style,0,maximized,l);
    if(!measured)return FALSE;
    if(content.top<l->client.top)return FALSE;
    int pitch=l->corner.cx-l->border;
    int count=!!(style&WS_SYSMENU)+!!(style&WS_MAXIMIZEBOX)+!!(style&WS_MINIMIZEBOX);
    if(l->outer.right-2*l->border<(count+1)*pitch)return FALSE;
    l->client=content;
    return TRUE;
}

LRESULT win16_uwp_frame_hit(const win16_style_layout *l,POINT p)
{
    LRESULT hit=win16_style_hit(l,p);
    return l?style_uwp_resize_margin_hit(hit,p,&l->outer,&l->client,l->corner,l->sizeable):hit;
}
