#include "present.h"

void style_present(HWND w,const style_present_view *view,HDC target)
{
    RECT dirty,client;HDC memory=NULL;HBITMAP bitmap=NULL,old=NULL;BOOL ready=FALSE;
    BOOL group;int target_saved=0;
    if(!target || GetClipBox(target,&dirty)==ERROR || !GetClientRect(w,&client) ||
        !IntersectRect(&dirty,&dirty,&client))return;
    group=view->is_group && view->is_group(w,view->classification);
    if(group){HWND sibling;RECT r;
        target_saved=SaveDC(target);if(!target_saved)return;
        /* Group interiors belong to the parent background. Never overwrite
         * the sibling controls visually enclosed by this transparent group. */
        for(sibling=GetWindow(GetParent(w),GW_CHILD);sibling;sibling=GetWindow(sibling,GW_HWNDNEXT))
            if(sibling!=w && IsWindowVisible(sibling) && GetWindowRect(sibling,&r)){
                MapWindowPoints(NULL,w,(POINT *)&r,2);
                ExcludeClipRect(target,r.left,r.top,r.right,r.bottom);}
    }
    memory=CreateCompatibleDC(target);
    if(memory)bitmap=CreateCompatibleBitmap(target,dirty.right-dirty.left,dirty.bottom-dirty.top);
    if(bitmap)old=(HBITMAP)SelectObject(memory,bitmap);
    if(old && old!=(HBITMAP)HGDI_ERROR &&
        BitBlt(memory,0,0,dirty.right-dirty.left,dirty.bottom-dirty.top,target,dirty.left,dirty.top,SRCCOPY) &&
        SetWindowOrgEx(memory,dirty.left,dirty.top,NULL)){
        if(group){HWND parent=GetParent(w);POINT origin={0,0};int saved=SaveDC(memory);
            SetDCBrushColor(memory,view->background);FillRect(memory,&client,(HBRUSH)GetStockObject(DC_BRUSH));
            if(saved && parent){MapWindowPoints(w,parent,&origin,1);
                OffsetViewportOrgEx(memory,-origin.x,-origin.y,NULL);
                SendMessageW(parent,WM_PRINTCLIENT,(WPARAM)memory,PRF_CLIENT);}
            if(saved)RestoreDC(memory,saved);
        }
        if(!*view->detached)view->paint(w,view->context,memory);
        if(!*view->detached)ready=BitBlt(target,dirty.left,dirty.top,dirty.right-dirty.left,
            dirty.bottom-dirty.top,memory,dirty.left,dirty.top,SRCCOPY);
    }
    if(old && old!=(HBITMAP)HGDI_ERROR)SelectObject(memory,old);
    if(bitmap)DeleteObject(bitmap);
    if(memory)DeleteDC(memory);
    if(!ready && !*view->detached)view->paint(w,view->context,target);
    if(target_saved)RestoreDC(target,target_saved);
}

typedef struct print_children {
    HWND parent;
    HDC dc;
    POINT origin;
    LPARAM flags;
    BOOL owned;
} print_children;

static BOOL CALLBACK print_child(HWND child,LPARAM data)
{
    print_children *p=(print_children *)data;RECT r;int saved;
    if(!IsWindow(p->parent))return FALSE;
    if((p->owned?GetWindow(child,GW_OWNER):GetParent(child))!=p->parent || !IsWindowVisible(child))return TRUE;
    if(!GetWindowRect(child,&r) || !(saved=SaveDC(p->dc)))return TRUE;
    OffsetViewportOrgEx(p->dc,r.left-p->origin.x,r.top-p->origin.y,NULL);
    IntersectClipRect(p->dc,0,0,r.right-r.left,r.bottom-r.top);
    SendMessageW(child,WM_PRINT,(WPARAM)p->dc,p->flags|PRF_NONCLIENT|PRF_CLIENT|PRF_ERASEBKGND);
    RestoreDC(p->dc,saved);return TRUE;
}

/* Custom nonclient geometry cannot delegate native frame printing. Preserve
 * DWP's frame/client/child ordering using the public HWND/DC boundaries.
 * See the S4 print audit for the original xxxDWPPrint reference. */
void style_print_client_and_children(HWND w,const BOOL *detached,HDC dc,LPARAM flags)
{
    RECT wr,client;POINT origin={0,0};int saved;print_children children;
    if(!GetWindowRect(w,&wr) || !GetClientRect(w,&client) || !ClientToScreen(w,&origin))return;
    children.parent=w;children.dc=dc;children.origin=origin;children.flags=flags;children.owned=FALSE;
    if(flags&PRF_NONCLIENT){children.origin.x=wr.left;children.origin.y=wr.top;}
    if(flags&PRF_CLIENT){
        saved=SaveDC(dc);if(!saved)return;
        OffsetViewportOrgEx(dc,origin.x-children.origin.x,origin.y-children.origin.y,NULL);
        IntersectClipRect(dc,0,0,client.right,client.bottom);
        if(flags&PRF_ERASEBKGND)SendMessageW(w,WM_ERASEBKGND,(WPARAM)dc,0);
        if(!*detached)SendMessageW(w,WM_PRINTCLIENT,(WPARAM)dc,flags);
        RestoreDC(dc,saved);
    }
    if(!*detached && (flags&PRF_CHILDREN))EnumChildWindows(w,print_child,(LPARAM)&children);
    if(!*detached && (flags&PRF_OWNED)){
        children.owned=TRUE;EnumThreadWindows(GetCurrentThreadId(),print_child,(LPARAM)&children);
    }
}
