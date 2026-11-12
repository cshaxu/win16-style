#include "surface.h"
#include "lib-style/common/frame.h"

BOOL style_uwp_surface_refresh(const style_uwp_surface_view *view)
{
    if(*view->detached || *view->updating)return !*view->detached;
    if(!*view->surface)return TRUE;
    if(IsIconic(view->owner) || !IsWindowVisible(view->owner)){
        ShowWindow(*view->surface,SW_HIDE);return TRUE;
    }
    *view->updating=TRUE;
    RECT outer,hole;BOOL ok=view->geometry(view->context,&outer,&hole);
    if(ok){
        SIZE size={outer.right-outer.left,outer.bottom-outer.top};
        ok=style_bitmap_ensure(view->bitmap,NULL,size);
        if(ok){
            ZeroMemory(view->bitmap->pixels,(SIZE_T)size.cx*(SIZE_T)size.cy*4);
            view->paint(view->context,view->bitmap->dc);
            ok=style_uwp_surface_present(view->owner,*view->surface,&outer,&hole,view->bitmap,view->detached);
        }
    }
    if(!ok && !*view->detached)ShowWindow(*view->surface,SW_HIDE);
    *view->updating=FALSE;return ok && !*view->detached;
}

LRESULT style_uwp_resize_margin_hit(LRESULT hit,POINT point,const RECT *outer,
    const RECT *client,SIZE corner,BOOL sizeable)
{
    if(hit!=HTBORDER || !outer || !client || !sizeable)return hit;
    if(point.x<client->left)return point.y<corner.cy?HTTOPLEFT:
        point.y>=outer->bottom-corner.cy?HTBOTTOMLEFT:HTLEFT;
    if(point.x>=client->right)return point.y<corner.cy?HTTOPRIGHT:
        point.y>=outer->bottom-corner.cy?HTBOTTOMRIGHT:HTRIGHT;
    if(point.y>=client->bottom)return point.x<corner.cx?HTBOTTOMLEFT:
        point.x>=outer->right-corner.cx?HTBOTTOMRIGHT:HTBOTTOM;
    return hit;
}

BOOL style_uwp_frame_bounds(HWND window,HWND title,RECT *outer,RECT *content,BOOL *zoomed)
{
    RECT title_rect;
    if(!GetWindowRect(window,outer) || !GetClientRect(window,content))return FALSE;
    if(title && (!IsWindow(title) || GetParent(title)!=window ||
       !IsWindowVisible(title) || !GetWindowRect(title,&title_rect)))return FALSE;
    *zoomed=IsZoomed(window);
    if(*zoomed){
        RECT visible;
        if(!style_frame_work_area(window,outer,&visible))return FALSE;
        *outer=visible;
    }
    POINT origin={0,0};ClientToScreen(window,&origin);
    OffsetRect(content,origin.x-outer->left,origin.y-outer->top);
    if(title){
        if(title_rect.left<origin.x || title_rect.right!=outer->left+content->right || title_rect.bottom<=origin.y)return FALSE;
        content->top=title_rect.bottom-outer->top;
    }
    if(*zoomed){
        RECT bounds={0,0,outer->right-outer->left,outer->bottom-outer->top};
        if(!IntersectRect(content,content,&bounds))return FALSE;
    }
    return TRUE;
}

BOOL style_uwp_surface_present(HWND owner,HWND surface,const RECT *outer,
    const RECT *hole,style_bitmap *bitmap,const BOOL *detached)
{
    SIZE size=bitmap->size;GdiFlush();
    for(int y=0;y<size.cy;++y){
        BYTE *row=bitmap->pixels+(SIZE_T)y*(SIZE_T)size.cx*4;
        if(y<hole->top || y>=hole->bottom){for(int x=0;x<size.cx;++x)row[(SIZE_T)x*4+3]=255;}
        else{
            for(int x=0;x<hole->left;++x)row[(SIZE_T)x*4+3]=255;
            for(int x=hole->right;x<size.cx;++x)row[(SIZE_T)x*4+3]=255;
        }
    }
    if(*detached)return FALSE;
    HRGN region=CreateRectRgn(0,0,size.cx,size.cy),inside=CreateRectRgnIndirect(hole);
    BOOL ok=region && inside && CombineRgn(region,region,inside,RGN_DIFF)!=ERROR;
    if(inside)DeleteObject(inside);
    if(ok && SetWindowRgn(surface,region,FALSE))region=NULL;else ok=FALSE;
    if(region)DeleteObject(region);
    POINT at={outer->left,outer->top},source={0,0};BLENDFUNCTION blend={AC_SRC_OVER,0,255,AC_SRC_ALPHA};
    if(ok)ok=UpdateLayeredWindow(surface,NULL,&at,&size,bitmap->dc,&source,0,&blend,ULW_ALPHA);
    if(ok){
        HWND above=GetWindow(owner,GW_HWNDPREV);
        if(above==surface)above=GetWindow(surface,GW_HWNDPREV);
        ok=SetWindowPos(surface,above?above:HWND_TOP,0,0,0,0,
            SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE|SWP_NOOWNERZORDER|SWP_SHOWWINDOW);
    }
    return ok;
}
