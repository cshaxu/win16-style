#include "frame_view.h"
#include "frame_paint.h"
#include "menu_bar.h"
#include <windowsx.h>

void win16_control_frame_button_press(HWND w,const win16_control_frame_button_view *s,int hit)
{
    *s->pressed=hit;*s->hot=TRUE;SetCapture(w);s->paint(w,s->context,hit);
}
BOOL win16_control_frame_button_motion(HWND w,const win16_control_frame_button_view *s,UINT msg,LPARAM lp)
{
    win16_style_layout l;RECT r;POINT point;
    if(!*s->pressed || !s->layout(w,s->context,&l))return FALSE;
    int pressed=*s->pressed;BOOL was_hot=*s->hot;
    point.x=GET_X_LPARAM(lp);point.y=GET_Y_LPARAM(lp);ClientToScreen(w,&point);
    GetWindowRect(w,&r);point.x-=r.left;point.y-=r.top;
    *s->hot=win16_style_hit(&l,point)==pressed;
    if(msg==WM_LBUTTONUP){
        BOOL invoke=*s->hot;*s->pressed=0;ReleaseCapture();
        if(invoke)PostMessageW(w,WM_SYSCOMMAND,pressed==HTSYSMENU?SC_KEYMENU:
            pressed==HTMINBUTTON?SC_MINIMIZE:(IsZoomed(w)?SC_RESTORE:SC_MAXIMIZE),
            pressed==HTSYSMENU?L' ':0);
    }
    if(!*s->detached && (msg==WM_LBUTTONUP || was_hot!=*s->hot))s->paint(w,s->context,pressed);
    return TRUE;
}
BOOL win16_control_frame_button_cancel(HWND w,const win16_control_frame_button_view *s,UINT msg)
{
    if(!*s->pressed)return FALSE;
    int pressed=*s->pressed;*s->pressed=0;*s->hot=FALSE;
    if(msg==WM_CANCELMODE && GetCapture()==w)ReleaseCapture();
    if(!*s->detached)s->paint(w,s->context,pressed);
    return TRUE;
}

void win16_control_frame_ncpaint(HWND w,WPARAM wp,void *context,
    void (*paint)(HWND,void *,HDC),void (*buffered)(HWND,void *,HDC))
{
    if(wp!=1 && wp){
        /* GetDCEx takes ownership of the copied update region. */
        HRGN region=CreateRectRgn(0,0,0,0);HDC dc=NULL;
        if(region && CombineRgn(region,(HRGN)wp,NULL,RGN_COPY)!=ERROR){
            dc=GetDCEx(w,region,DCX_WINDOW|DCX_INTERSECTRGN|DCX_CACHE);
            if(dc){buffered(w,context,dc);ReleaseDC(w,dc);region=NULL;}
        }
        if(region)DeleteObject(region);
        if(!dc)paint(w,context,NULL);
    }else {HDC dc=GetWindowDC(w);if(dc){buffered(w,context,dc);ReleaseDC(w,dc);}}
}
void win16_control_paint_frame(HWND w,const win16_control_frame_view *view,HDC target)
{
    win16_style_layout l;
    win16_style_palette p;
    HDC dc=target?target:GetWindowDC(w);
    RECT r;int saved;
    if(!dc)return;
    if(!view->layout(w,view->context,&l) || *view->detached){if(!target)ReleaseDC(w,dc);return;}
    if(!l.border && IsRectEmpty(&l.caption)){if(!target)ReleaseDC(w,dc);return;}
    view->colors(view->context,&p);saved=SaveDC(dc);
    /* A borrowed WM_PRINT DC must remain untouched if its state cannot be
     * saved. This is failure containment, not a menu-animation workaround. */
    if(!saved){if(!target)ReleaseDC(w,dc);return;}
    ExcludeClipRect(dc,l.client.left,l.client.top,l.client.right,l.client.bottom);
    win16_control_fill(dc,l.outer,l.dialog_frame?(*view->active?p.active:p.inactive):p.face);
    win16_control_outline(dc,l.outer,l.unit,p.edge);
    /* The reference frame has a grey band between two dark outlines, not
     * the white upper/left bevel used by later Windows non-client frames. */
    r=l.outer;InflateRect(&r,-l.border+l.unit,-l.border+l.unit);
    win16_control_outline(dc,r,l.unit,l.dialog_frame?p.light:p.edge);
    if(!IsRectEmpty(&l.menu)){
        win16_style_bar bar;POINT origin={l.menu.left,l.menu.top};
        win16_control_fill(dc,l.menu,p.light);
        if(win16_style_bar_read_for_window(w,GetMenu(w),*view->menu_font,*view->dpi,origin,l.menu.right-l.menu.left,&bar)){
            if(*view->bar_selected>=0 && (UINT)*view->bar_selected<bar.count)
                bar.items[*view->bar_selected].state|=MFS_HILITE;
            win16_style_bar_paint(&bar,dc,*view->menu_font,*view->dpi,&p);win16_style_bar_free(&bar);
            if(*view->detached){RestoreDC(dc,saved);if(!target)ReleaseDC(w,dc);return;}
        }
        r=l.menu;r.top=r.bottom-l.unit;win16_control_fill(dc,r,p.edge);
    }
    win16_control_frame_sizing(dc,&l,&p);
    view->scrollbars(w,dc,&p);
    if(IsRectEmpty(&l.caption)){
        RestoreDC(dc,saved);if(!target)ReleaseDC(w,dc);return;
    }
    win16_control_frame_caption(w,dc,&l,&p,view->active,view->system_menu,view->bar_keyboard,
        view->bar_selected,view->pressed,view->hot,view->title_font);
    RestoreDC(dc,saved);if(!target)ReleaseDC(w,dc);
}


/* USER can request the entire frame when a popup changes. Compose the
 * requested non-client pixels off-screen so the background and text do not
 * become visible as separate drawing operations. */
static void paint_buffered(HWND w,const win16_control_frame_view *view,HDC dc,const RECT *bounds)
{
    HDC memory=CreateCompatibleDC(dc);HBITMAP bitmap=NULL,old=NULL;BOOL ready=FALSE;
    int width=bounds->right-bounds->left,height=bounds->bottom-bounds->top;
    if(memory)bitmap=CreateCompatibleBitmap(dc,width,height);
    if(bitmap)old=(HBITMAP)SelectObject(memory,bitmap);
    if(old && old!=(HBITMAP)HGDI_ERROR &&
        BitBlt(memory,0,0,width,height,dc,bounds->left,bounds->top,SRCCOPY) &&
        SetWindowOrgEx(memory,bounds->left,bounds->top,NULL)){
        win16_control_paint_frame(w,view,memory);
        if(!*view->detached)ready=BitBlt(dc,bounds->left,bounds->top,width,height,memory,bounds->left,bounds->top,SRCCOPY);
    }
    if(old && old!=(HBITMAP)HGDI_ERROR)SelectObject(memory,old);
    if(bitmap)DeleteObject(bitmap);
    if(memory)DeleteDC(memory);
    if(!ready && !*view->detached)win16_control_paint_frame(w,view,dc);
}

void win16_control_frame_buffered(HWND w,const win16_control_frame_view *view,HDC dc)
{
    win16_style_layout l;RECT bounds;int saved=SaveDC(dc);
    if(!saved){win16_control_paint_frame(w,view,dc);return;}
    if(view->layout(w,view->context,&l) && !*view->detached &&
        ExcludeClipRect(dc,l.client.left,l.client.top,l.client.right,l.client.bottom)!=ERROR &&
        GetClipBox(dc,&bounds)!=ERROR && bounds.right>bounds.left && bounds.bottom>bounds.top)
        paint_buffered(w,view,dc,&bounds);
    else if(!*view->detached)win16_control_paint_frame(w,view,dc);
    RestoreDC(dc,saved);
}

/* Called only from the pinned subclass invocation. Application menu-init and
 * native tracking can reenter/remove/destroy the frame; never paint afterwards
 * unless its attachment remains live. */
/* Menu selection changes no caption, sizing band or scrollbar pixels.
 * Reuse the frame painter with a bounded clip instead of clearing the frame. */
void win16_control_frame_menu_state(HWND w,const win16_control_frame_view *view)
{
    win16_style_layout l;HDC dc;HRGN clip,system;int saved;RECT bounds;
    if(!view->layout(w,view->context,&l))return;
    dc=GetWindowDC(w);if(!dc)return;saved=SaveDC(dc);
    if(!saved){ReleaseDC(w,dc);return;}
    clip=CreateRectRgnIndirect(&l.menu);system=CreateRectRgnIndirect(&l.system);
    if(clip && system && CombineRgn(clip,clip,system,RGN_OR)!=ERROR &&
        ExtSelectClipRgn(dc,clip,RGN_AND)!=ERROR && GetRgnBox(clip,&bounds)!=ERROR &&
        bounds.right>bounds.left && bounds.bottom>bounds.top){
        paint_buffered(w,view,dc,&bounds);
    }
    if(system)DeleteObject(system);
    if(clip)DeleteObject(clip);
    RestoreDC(dc,saved);ReleaseDC(w,dc);
}

/* Press tracking changes one button, not the sizing band or other controls. */
void win16_control_frame_button_state(HWND w,const win16_control_frame_view *view,int hit)
{
    win16_style_layout l;RECT r;HDC dc;int saved;
    if(!view->layout(w,view->context,&l))return;
    r=hit==HTSYSMENU?l.system:hit==HTMINBUTTON?l.minimize:l.maximize;
    if(IsRectEmpty(&r))return;
    dc=GetWindowDC(w);if(!dc)return;saved=SaveDC(dc);
    if(saved){if(IntersectClipRect(dc,r.left,r.top,r.right,r.bottom)!=ERROR){
        paint_buffered(w,view,dc,&r);
        }
        RestoreDC(dc,saved);}
    ReleaseDC(w,dc);
}
