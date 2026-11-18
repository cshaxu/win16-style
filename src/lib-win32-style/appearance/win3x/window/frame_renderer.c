#include "frame_renderer.h"
#include "frame_paint.h"
#include "../menu/menu_bar.h"
#include "lib-win32-style/application/graphics/present.h"
#include "lib-win32-style/application/window/window_text.h"
#include <stdlib.h>
#include <windowsx.h>

void win16_control_frame_button_press(HWND w,const win16_control_frame_button_view *s,int hit)
{
    (void)style_caption_button_begin(w,s->state,hit,FALSE);s->paint(w,s->context,hit);
}
BOOL win16_control_frame_button_motion(HWND w,const win16_control_frame_button_view *s,UINT msg,LPARAM lp)
{
    win16_style_layout l;RECT r;POINT point;
    if(!s->state->pressed || !s->layout(w,s->context,&l))return FALSE;
    int pressed=s->state->pressed;BOOL was_hot=s->state->hot;
    point.x=GET_X_LPARAM(lp);point.y=GET_Y_LPARAM(lp);ClientToScreen(w,&point);
    GetWindowRect(w,&r);point.x-=r.left;point.y-=r.top;
    BOOL invoke=style_caption_button_move(w,s->state,win16_style_hit(&l,point)==pressed,msg==WM_LBUTTONUP,TRUE);
    if(msg==WM_LBUTTONUP){
        if(invoke)PostMessageW(w,WM_SYSCOMMAND,pressed==HTSYSMENU?SC_KEYMENU:
            pressed==HTMINBUTTON?SC_MINIMIZE:(IsZoomed(w)?SC_RESTORE:SC_MAXIMIZE),
            pressed==HTSYSMENU?L' ':0);
    }
    if(!s->instance->detached && (msg==WM_LBUTTONUP || was_hot!=s->state->hot))s->paint(w,s->context,pressed);
    return TRUE;
}
BOOL win16_control_frame_button_cancel(HWND w,const win16_control_frame_button_view *s,UINT msg)
{
    if(!s->state->pressed)return FALSE;
    int pressed=style_caption_button_cancel(w,s->state,msg);
    if(!s->instance->detached)s->paint(w,s->context,pressed);
    return TRUE;
}

static void render_caption(HWND window,const style_instance *instance,HDC dc,
    const win16_style_layout *layout,const win16_style_palette *palette,
    void *context,win16_frame_state_read read)
{
    WCHAR *title=style_window_text_copy(window);
    if(!instance->detached){
        win16_frame_paint_state state;read(context,&state);
        BOOL child=(GetWindowLongPtrW(window,GWL_STYLE)&WS_CHILD)!=0;
        win16_control_frame_caption(dc,layout,palette,&state,title,child,IsZoomed(window));
    }
    free(title);
}

void win16_control_paint_frame(HWND w,style_instance *instance,void *context,const win16_frame_renderer_ops *ops,HDC target)
{
    win16_style_layout l;
    win16_style_palette p;win16_frame_paint_state state;
    HDC dc=target?target:GetWindowDC(w);
    RECT r;int saved;
    if(!dc)return;
    if(!ops->layout(w,context,&l) || instance->detached){if(!target)ReleaseDC(w,dc);return;}
    if(!l.border && IsRectEmpty(&l.caption)){if(!target)ReleaseDC(w,dc);return;}
    ops->colors(context,&p);ops->state(context,&state);saved=SaveDC(dc);
    /* A borrowed WM_PRINT DC must remain untouched if its state cannot be
     * saved. This is failure containment, not a menu-animation workaround. */
    if(!saved){if(!target)ReleaseDC(w,dc);return;}
    ExcludeClipRect(dc,l.client.left,l.client.top,l.client.right,l.client.bottom);
    win16_control_fill(dc,l.outer,l.dialog_frame?(state.active?p.active:p.inactive):p.face);
    win16_control_outline(dc,l.outer,l.unit,p.edge);
    /* The reference frame has a grey band between two dark outlines, not
     * the white upper/left bevel used by later Windows non-client frames. */
    r=l.outer;InflateRect(&r,-l.border+l.unit,-l.border+l.unit);
    win16_control_outline(dc,r,l.unit,l.dialog_frame?p.light:p.edge);
    if(!IsRectEmpty(&l.menu)){
        style_menu_bar_snapshot bar;POINT origin={l.menu.left,l.menu.top};
        win16_control_fill(dc,l.menu,p.light);
        if(win16_style_bar_read_for_window(w,GetMenu(w),state.fonts->menu,state.fonts->dpi,origin,l.menu.right-l.menu.left,&bar)){
            ops->state(context,&state);
            if(state.bar_selected>=0 && (UINT)state.bar_selected<bar.count)
                bar.items[state.bar_selected].state|=MFS_HILITE;
            win16_style_bar_paint(&bar,dc,state.fonts->menu,state.fonts->dpi,&p);style_menu_bar_free(&bar);
            if(instance->detached){RestoreDC(dc,saved);if(!target)ReleaseDC(w,dc);return;}
        }
        r=l.menu;r.top=r.bottom-l.unit;win16_control_fill(dc,r,p.edge);
    }
    win16_control_frame_sizing(dc,&l,&p);
    ops->scrollbars(w,dc,&p);
    if(IsRectEmpty(&l.caption)){
        RestoreDC(dc,saved);if(!target)ReleaseDC(w,dc);return;
    }
    render_caption(w,instance,dc,&l,&p,context,ops->state);
    RestoreDC(dc,saved);if(!target)ReleaseDC(w,dc);
}


/* USER can request the entire frame when a popup changes. Compose the
 * requested non-client pixels off-screen so the background and text do not
 * become visible as separate drawing operations. */
typedef struct frame_paint_call {
    style_instance *instance;
    void *context;
    const win16_frame_renderer_ops *ops;
} frame_paint_call;
static void paint_callback(HWND w,void *context,HDC dc)
{
    const frame_paint_call *call=context;
    win16_control_paint_frame(w,call->instance,call->context,call->ops,dc);
}
static void paint_buffered(HWND w,style_instance *instance,void *context,const win16_frame_renderer_ops *ops,HDC dc,const RECT *bounds)
{
    frame_paint_call call={instance,context,ops};
    style_present_buffered(w,instance,&call,paint_callback,dc,bounds);
}

void win16_control_frame_buffered(HWND w,style_instance *instance,void *context,const win16_frame_renderer_ops *ops,HDC dc)
{
    win16_style_layout l;RECT bounds;int saved=SaveDC(dc);
    if(!saved){win16_control_paint_frame(w,instance,context,ops,dc);return;}
    if(ops->layout(w,context,&l) && !instance->detached &&
        ExcludeClipRect(dc,l.client.left,l.client.top,l.client.right,l.client.bottom)!=ERROR &&
        GetClipBox(dc,&bounds)!=ERROR && bounds.right>bounds.left && bounds.bottom>bounds.top)
        paint_buffered(w,instance,context,ops,dc,&bounds);
    else if(!instance->detached)win16_control_paint_frame(w,instance,context,ops,dc);
    RestoreDC(dc,saved);
}

/* Called only from the pinned subclass invocation. Application menu-init and
 * native tracking can reenter/remove/destroy the frame; never paint afterwards
 * unless its attachment remains live. */
/* Menu selection changes no caption, sizing band or scrollbar pixels.
 * Reuse the frame painter with a bounded clip instead of clearing the frame. */
void win16_control_frame_menu_state(HWND w,style_instance *instance,void *context,const win16_frame_renderer_ops *ops)
{
    win16_style_layout l;HDC dc;HRGN clip,system;int saved;RECT bounds;
    if(!ops->layout(w,context,&l))return;
    dc=GetWindowDC(w);if(!dc)return;saved=SaveDC(dc);
    if(!saved){ReleaseDC(w,dc);return;}
    clip=CreateRectRgnIndirect(&l.menu);system=CreateRectRgnIndirect(&l.system);
    if(clip && system && CombineRgn(clip,clip,system,RGN_OR)!=ERROR &&
        ExtSelectClipRgn(dc,clip,RGN_AND)!=ERROR && GetRgnBox(clip,&bounds)!=ERROR &&
        bounds.right>bounds.left && bounds.bottom>bounds.top){
        paint_buffered(w,instance,context,ops,dc,&bounds);
    }
    if(system)DeleteObject(system);
    if(clip)DeleteObject(clip);
    RestoreDC(dc,saved);ReleaseDC(w,dc);
}

/* Press tracking changes one button, not the sizing band or other controls. */
void win16_control_frame_button_state(HWND w,style_instance *instance,void *context,const win16_frame_renderer_ops *ops,int hit)
{
    win16_style_layout l;RECT r;HDC dc;int saved;
    if(!ops->layout(w,context,&l))return;
    r=hit==HTSYSMENU?l.system:hit==HTMINBUTTON?l.minimize:l.maximize;
    if(IsRectEmpty(&r))return;
    dc=GetWindowDC(w);if(!dc)return;saved=SaveDC(dc);
    if(saved){if(IntersectClipRect(dc,r.left,r.top,r.right,r.bottom)!=ERROR){
        paint_buffered(w,instance,context,ops,dc,&r);
        }
        RestoreDC(dc,saved);}
    ReleaseDC(w,dc);
}
