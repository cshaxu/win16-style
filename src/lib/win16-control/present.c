#include "present.h"
#include "control_class.h"

void win16_control_present(HWND w,const win16_control_present_view *view,HDC target)
{
    RECT dirty,client;HDC memory=NULL;HBITMAP bitmap=NULL,old=NULL;BOOL ready=FALSE;
    BOOL group;int target_saved=0;
    if(!target || GetClipBox(target,&dirty)==ERROR || !GetClientRect(w,&client) ||
        !IntersectRect(&dirty,&dirty,&client))return;
    group=ntw16s_class_is(w,L"BUTTON") &&
        (GetWindowLongPtrW(w,GWL_STYLE)&BS_TYPEMASK)==BS_GROUPBOX;
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
            SetDCBrushColor(memory,view->palette->light);FillRect(memory,&client,(HBRUSH)GetStockObject(DC_BRUSH));
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
