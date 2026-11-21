#include "client_frame_renderer.h"
#include "client_frame_paint.h"
#include "lib-win32-style/application/menu/client_bar.h"
#include "lib-win32-style/application/graphics/present.h"
#include "lib-win32-style/application/window/window_text.h"
#include "lib-win32-style/application/window/window_icon.h"
#include <stdlib.h>
#include <windowsx.h>

void style_client_frame_button_press(HWND w,const style_client_frame_button_view *s,int hit)
{
    (void)style_caption_button_begin(w,s->state,hit,FALSE);s->paint(w,s->context,hit);
}
BOOL style_client_frame_button_motion(HWND w,const style_client_frame_button_view *s,UINT msg,LPARAM lp)
{
    style_full_client_layout l;RECT r;POINT point;
    if(!s->state->pressed || !s->layout(w,s->context,&l))return FALSE;
    int pressed=s->state->pressed;BOOL was_hot=s->state->hot;
    point.x=GET_X_LPARAM(lp);point.y=GET_Y_LPARAM(lp);ClientToScreen(w,&point);
    GetWindowRect(w,&r);point.x-=r.left;point.y-=r.top;
    BOOL invoke=style_caption_button_move(w,s->state,style_full_client_hit(&l,point)==pressed,msg==WM_LBUTTONUP,TRUE);
    if(msg==WM_LBUTTONUP){
        if(invoke && style_caption_button_enabled(w,pressed))PostMessageW(w,WM_SYSCOMMAND,style_caption_command(style_caption_role(pressed),IsZoomed(w)),
            pressed==HTSYSMENU?L' ':0);
    }
    if(!s->instance->detached && (msg==WM_LBUTTONUP || was_hot!=s->state->hot))s->paint(w,s->context,pressed);
    return TRUE;
}
BOOL style_client_frame_button_cancel(HWND w,const style_client_frame_button_view *s,UINT msg)
{
    if(!s->state->pressed)return FALSE;
    int pressed=style_caption_button_cancel(w,s->state,msg);
    if(!s->instance->detached)s->paint(w,s->context,pressed);
    return TRUE;
}

static void render_caption(HWND window,const style_instance *instance,HDC dc,
    const style_full_client_layout *layout,const style_window_palette *palette,
    void *context,style_client_frame_state_read read)
{
    WCHAR *title=style_window_text_copy(window);
    if(!instance->detached){
        style_client_frame_paint_state state;read(context,&state);
        if(!IsRectEmpty(&layout->buttons[CLASSIC_BUTTON_CLOSE]))
            state.close_disabled=!style_caption_button_enabled(window,HTCLOSE);
        BOOL child=(GetWindowLongPtrW(window,GWL_STYLE)&WS_CHILD)!=0;
        state.icon=IsRectEmpty(&layout->icon)?NULL:style_window_caption_icon(window);
        if(!instance->detached)style_client_frame_caption(dc,layout,palette,&state,title,child,IsZoomed(window));
    }
    free(title);
}

void style_client_frame_paint(HWND w,style_instance *instance,void *context,const style_client_frame_renderer_ops *ops,HDC target)
{
    style_full_client_layout l;
    style_window_palette p;style_client_frame_paint_state state;
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
    const style_border_palette palette={p.face,p.edge,p.light,p.shadow,state.active?p.active:p.inactive};
    if(state.border.fill)style_frame_border_paint(dc,l.outer,l.border,l.unit,l.dialog_frame,
        l.sizeable?&l.corner:NULL,&state.border,&palette);
    else style_frame_solid_fill(dc,l.outer,p.face);
    if(!IsRectEmpty(&l.menu)){
        style_menu_bar_snapshot bar;POINT origin={l.menu.left,l.menu.top};
        const classic_menu_colors *menu=state.menu_bar && state.menu_bar->present &&
            !style_window_high_contrast()?state.menu_bar:NULL;
        style_frame_solid_fill(dc,l.menu,menu?menu->background:p.light);
        if(style_menu_client_bar_read(w,GetMenu(w),state.fonts->menu,state.fonts->dpi,&l,origin,l.menu.right-l.menu.left,&bar)){
            ops->state(context,&state);
            if(state.bar_selected>=0 && (UINT)state.bar_selected<bar.count)
                bar.items[state.bar_selected].state|=MFS_HILITE;
            style_menu_client_bar_paint(&bar,dc,state.fonts->menu,state.fonts->dpi,&p,menu,state.menu_metrics,state.art,&state.presentation);
            style_menu_bar_free(&bar);
            if(instance->detached){RestoreDC(dc,saved);if(!target)ReleaseDC(w,dc);return;}
        }
        r=l.menu;r.top=r.bottom-l.unit;style_frame_solid_fill(dc,r,p.edge);
    }
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
    const style_client_frame_renderer_ops *ops;
} frame_paint_call;
static void paint_callback(HWND w,void *context,HDC dc)
{
    const frame_paint_call *call=context;
    style_client_frame_paint(w,call->instance,call->context,call->ops,dc);
}
static void paint_buffered(HWND w,style_instance *instance,void *context,const style_client_frame_renderer_ops *ops,HDC dc,const RECT *bounds)
{
    frame_paint_call call={instance,context,ops};
    style_present_buffered(w,instance,&call,paint_callback,dc,bounds);
}

void style_client_frame_buffered(HWND w,style_instance *instance,void *context,const style_client_frame_renderer_ops *ops,HDC dc)
{
    style_full_client_layout l;RECT bounds;int saved=SaveDC(dc);
    if(!saved){style_client_frame_paint(w,instance,context,ops,dc);return;}
    if(ops->layout(w,context,&l) && !instance->detached &&
        ExcludeClipRect(dc,l.client.left,l.client.top,l.client.right,l.client.bottom)!=ERROR &&
        GetClipBox(dc,&bounds)!=ERROR && bounds.right>bounds.left && bounds.bottom>bounds.top)
        paint_buffered(w,instance,context,ops,dc,&bounds);
    else if(!instance->detached)style_client_frame_paint(w,instance,context,ops,dc);
    RestoreDC(dc,saved);
}

/* Called only from the pinned subclass invocation. Application menu-init and
 * native tracking can reenter/remove/destroy the frame; never paint afterwards
 * unless its attachment remains live. */
/* Menu selection changes no caption, sizing band or scrollbar pixels.
 * Reuse the frame painter with a bounded clip instead of clearing the frame. */
void style_client_frame_menu_state(HWND w,style_instance *instance,void *context,const style_client_frame_renderer_ops *ops)
{
    style_full_client_layout l;HDC dc;HRGN clip,system;int saved;RECT bounds;
    if(!ops->layout(w,context,&l))return;
    dc=GetWindowDC(w);if(!dc)return;saved=SaveDC(dc);
    if(!saved){ReleaseDC(w,dc);return;}
    clip=CreateRectRgnIndirect(&l.menu);system=CreateRectRgnIndirect(&l.buttons[CLASSIC_BUTTON_SYSTEM]);
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
void style_client_frame_button_state(HWND w,style_instance *instance,void *context,const style_client_frame_renderer_ops *ops,int hit)
{
    style_full_client_layout l;RECT r;HDC dc;int saved;
    if(!ops->layout(w,context,&l))return;
    r=l.buttons[style_caption_role(hit)];
    if(IsRectEmpty(&r))return;
    dc=GetWindowDC(w);if(!dc)return;saved=SaveDC(dc);
    if(saved){if(IntersectClipRect(dc,r.left,r.top,r.right,r.bottom)!=ERROR){
        paint_buffered(w,instance,context,ops,dc,&r);
        }
        RestoreDC(dc,saved);}
    ReleaseDC(w,dc);
}
