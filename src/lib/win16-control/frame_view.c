#include "frame_view.h"
#include "frame_paint.h"
#include "menu_bar.h"

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
void win16_control_frame_buffered(HWND w,const win16_control_frame_view *view,HDC dc)
{
    win16_style_layout l;RECT bounds;int saved=SaveDC(dc);BOOL ready=FALSE;
    HDC memory=NULL;HBITMAP bitmap=NULL,old=NULL;
    if(!saved){win16_control_paint_frame(w,view,dc);return;}
    if(view->layout(w,view->context,&l) && !*view->detached &&
        ExcludeClipRect(dc,l.client.left,l.client.top,l.client.right,l.client.bottom)!=ERROR &&
        GetClipBox(dc,&bounds)!=ERROR && bounds.right>bounds.left && bounds.bottom>bounds.top){
        memory=CreateCompatibleDC(dc);
        if(memory)bitmap=CreateCompatibleBitmap(dc,bounds.right-bounds.left,bounds.bottom-bounds.top);
        if(bitmap)old=(HBITMAP)SelectObject(memory,bitmap);
        if(old && old!=(HBITMAP)HGDI_ERROR &&
            BitBlt(memory,0,0,bounds.right-bounds.left,bounds.bottom-bounds.top,dc,bounds.left,bounds.top,SRCCOPY) &&
            SetWindowOrgEx(memory,bounds.left,bounds.top,NULL)){
            win16_control_paint_frame(w,view,memory);
            if(!*view->detached)ready=BitBlt(dc,bounds.left,bounds.top,bounds.right-bounds.left,
                bounds.bottom-bounds.top,memory,bounds.left,bounds.top,SRCCOPY);
        }
    }
    if(old && old!=(HBITMAP)HGDI_ERROR)SelectObject(memory,old);
    if(bitmap)DeleteObject(bitmap);
    if(memory)DeleteDC(memory);
    if(!ready && !*view->detached)win16_control_paint_frame(w,view,dc);
    RestoreDC(dc,saved);
}

/* Called only from the pinned subclass invocation. Application menu-init and
 * native tracking can reenter/remove/destroy the frame; never paint afterwards
 * unless its attachment remains live. */
/* Menu selection changes no caption, sizing band or scrollbar pixels.
 * Reuse the frame painter with a bounded clip instead of clearing the frame. */
void win16_control_frame_menu_state(HWND w,const win16_control_frame_view *view)
{
    win16_style_layout l;HDC dc,memory=NULL;HRGN clip,system;int saved;
    RECT bounds;HBITMAP bitmap=NULL,old=NULL;BOOL ready=FALSE;
    if(!view->layout(w,view->context,&l))return;
    dc=GetWindowDC(w);if(!dc)return;saved=SaveDC(dc);
    if(!saved){ReleaseDC(w,dc);return;}
    clip=CreateRectRgnIndirect(&l.menu);system=CreateRectRgnIndirect(&l.system);
    if(clip && system && CombineRgn(clip,clip,system,RGN_OR)!=ERROR &&
        ExtSelectClipRgn(dc,clip,RGN_AND)!=ERROR && GetRgnBox(clip,&bounds)!=ERROR &&
        bounds.right>bounds.left && bounds.bottom>bounds.top){
        memory=CreateCompatibleDC(dc);
        if(memory)bitmap=CreateCompatibleBitmap(dc,bounds.right-bounds.left,bounds.bottom-bounds.top);
        if(bitmap)old=(HBITMAP)SelectObject(memory,bitmap);
        if(old && old!=(HBITMAP)HGDI_ERROR &&
            BitBlt(memory,0,0,bounds.right-bounds.left,bounds.bottom-bounds.top,dc,bounds.left,bounds.top,SRCCOPY) &&
            SetWindowOrgEx(memory,bounds.left,bounds.top,NULL)){
            win16_control_paint_frame(w,view,memory);
            if(!*view->detached)ready=BitBlt(dc,bounds.left,bounds.top,bounds.right-bounds.left,
                bounds.bottom-bounds.top,memory,bounds.left,bounds.top,SRCCOPY);
        }
        if(old && old!=(HBITMAP)HGDI_ERROR)SelectObject(memory,old);
        if(bitmap)DeleteObject(bitmap);
        if(memory)DeleteDC(memory);
        if(!ready && !*view->detached)win16_control_paint_frame(w,view,dc);
    }
    if(system)DeleteObject(system);
    if(clip)DeleteObject(clip);
    RestoreDC(dc,saved);ReleaseDC(w,dc);
}

/* Press tracking changes one button, not the sizing band or other controls. */
void win16_control_frame_button_state(HWND w,const win16_control_frame_view *view,int hit)
{
    win16_style_layout l;RECT r;HDC dc,memory;HBITMAP bitmap=NULL,old=NULL;int saved;BOOL ready=FALSE;
    if(!view->layout(w,view->context,&l))return;
    r=hit==HTSYSMENU?l.system:hit==HTMINBUTTON?l.minimize:l.maximize;
    if(IsRectEmpty(&r))return;
    dc=GetWindowDC(w);if(!dc)return;saved=SaveDC(dc);
    if(saved){if(IntersectClipRect(dc,r.left,r.top,r.right,r.bottom)!=ERROR){
        memory=CreateCompatibleDC(dc);
        if(memory)bitmap=CreateCompatibleBitmap(dc,r.right-r.left,r.bottom-r.top);
        if(bitmap)old=(HBITMAP)SelectObject(memory,bitmap);
        if(old && old!=(HBITMAP)HGDI_ERROR &&
            BitBlt(memory,0,0,r.right-r.left,r.bottom-r.top,dc,r.left,r.top,SRCCOPY) &&
            SetWindowOrgEx(memory,r.left,r.top,NULL)){
            win16_control_paint_frame(w,view,memory);
            if(!*view->detached)ready=BitBlt(dc,r.left,r.top,r.right-r.left,r.bottom-r.top,memory,r.left,r.top,SRCCOPY);
        }
        if(old && old!=(HBITMAP)HGDI_ERROR)SelectObject(memory,old);
        if(bitmap)DeleteObject(bitmap);
        if(memory)DeleteDC(memory);
        if(!ready && !*view->detached)win16_control_paint_frame(w,view,dc);
        }
        RestoreDC(dc,saved);}
    ReleaseDC(w,dc);
}
