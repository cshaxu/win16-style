#include "modern_frame_layout.h"
#include "lib-win32-style/application/adapters/surface.h"

BOOL win16_uwp_frame_layout(SIZE outer,UINT dpi,DWORD style,DWORD ex_style,BOOL maximized,
    RECT content,const classic_button_group *buttons,const classic_caption_metrics *metrics,
    const style_frame_geometry *frame,style_full_client_layout *l)
{
    if(!l || content.left<0 || content.top<0 || content.right>outer.cx ||
       content.bottom>outer.cy || content.left>=content.right || content.top>=content.bottom ||
       (style&WS_CAPTION)!=WS_CAPTION)return FALSE;
    BOOL measured=style_full_client_measure(outer.cx,outer.cy,dpi,style,ex_style&WS_EX_CONTEXTHELP,maximized,
        buttons,metrics,frame,dpi,l);
    if(!measured)return FALSE;
    if(content.top<l->client.top)return FALSE;
    int pitch=l->corner.cx-l->border;
    int count=0;
    for(unsigned role=CLASSIC_BUTTON_SYSTEM;role<=CLASSIC_BUTTON_CLOSE;++role)
        if(!IsRectEmpty(&l->buttons[role]))++count;
    if(l->outer.right-2*l->border<(count+1)*pitch)return FALSE;
    l->client=content;
    return TRUE;
}

LRESULT win16_uwp_frame_hit(const style_full_client_layout *l,POINT p)
{
    LRESULT hit=style_full_client_hit(l,p);
    return l?style_uwp_resize_margin_hit(hit,p,&l->outer,&l->client,l->corner,l->sizeable):hit;
}
