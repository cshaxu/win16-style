#ifndef STYLE_COMMON_FRAME_H
#define STYLE_COMMON_FRAME_H
#include <windows.h>
#include <oleacc.h>
#include <commctrl.h>

#define STYLE_NCUAHDRAWCAPTION 0x00ae
#define STYLE_NCUAHDRAWFRAME   0x00af
#define STYLE_UAHDRAWMENU     0x0091
#define STYLE_UAHDRAWMENUITEM 0x0092

/* Standard USER hit codes use the same resize cursors for each style. */
static inline LPCWSTR style_frame_resize_cursor(int hit)
{
    switch(hit){
    case HTLEFT: case HTRIGHT:return (LPCWSTR)IDC_SIZEWE;
    case HTTOP: case HTBOTTOM:return (LPCWSTR)IDC_SIZENS;
    case HTSIZE: case HTTOPLEFT: case HTBOTTOMRIGHT:return (LPCWSTR)IDC_SIZENWSE;
    case HTTOPRIGHT: case HTBOTTOMLEFT:return (LPCWSTR)IDC_SIZENESW;
    default:return (LPCWSTR)IDC_ARROW;
    }
}

static inline BOOL style_frame_suppress_native_theme(UINT message,BOOL iconic,BOOL has_menu)
{
    if(iconic)return FALSE;
    if(message==STYLE_NCUAHDRAWCAPTION || message==STYLE_NCUAHDRAWFRAME)return TRUE;
    return has_menu && (message==STYLE_UAHDRAWMENU || message==STYLE_UAHDRAWMENUITEM);
}

/* The frame and optional scrollbar painters are supplied by the style. */
static inline void style_frame_ncpaint(HWND window,WPARAM update,void *context,
    void (*paint)(HWND,void *,HDC),void (*buffered)(HWND,void *,HDC))
{
    if(update!=1 && update){
        HRGN region=CreateRectRgn(0,0,0,0);HDC dc=NULL;
        if(region && CombineRgn(region,(HRGN)update,NULL,RGN_COPY)!=ERROR){
            dc=GetDCEx(window,region,DCX_WINDOW|DCX_INTERSECTRGN|DCX_CACHE);
            if(dc){buffered(window,context,dc);ReleaseDC(window,dc);region=NULL;}
        }
        if(region)DeleteObject(region);
        if(!dc)paint(window,context,NULL);
    }else {HDC dc=GetWindowDC(window);if(dc){buffered(window,context,dc);ReleaseDC(window,dc);}}
}

/* Keep native non-client services, including scrollbars, outside our client. */
static inline HRGN style_frame_native_region(HWND window,WPARAM incoming)
{
    RECT outer,client;POINT origin={0,0};
    if(!GetWindowRect(window,&outer) || !GetClientRect(window,&client) || !ClientToScreen(window,&origin))return NULL;
    OffsetRect(&client,origin.x,origin.y);
    HRGN region=CreateRectRgnIndirect(&outer),inside=CreateRectRgnIndirect(&client);
    if(!region || !inside){if(region)DeleteObject(region);if(inside)DeleteObject(inside);return NULL;}
    int combined=CombineRgn(region,region,inside,RGN_DIFF);DeleteObject(inside);
    if(combined!=ERROR && incoming && incoming!=1)combined=CombineRgn(region,region,(HRGN)incoming,RGN_AND);
    if(combined==ERROR){DeleteObject(region);return NULL;}
    return region;
}

/* USER can redraw its original menu after a custom frame has moved it. */
static inline void style_invalidate_native_menu_overlap(HWND window)
{
    MENUBARINFO menu={.cbSize=sizeof(menu)};RECT client,overlap;POINT origin={0,0};
    if(!GetMenu(window) || !GetMenuBarInfo(window,OBJID_MENU,0,&menu) ||
       !GetClientRect(window,&client) || !ClientToScreen(window,&origin))return;
    OffsetRect(&client,origin.x,origin.y);
    if(!IntersectRect(&overlap,&client,&menu.rcBar))return;
    /* USER also paints the native menu's bottom rule just outside rcBar. */
    overlap.left=client.left;overlap.right=client.right;
    overlap.bottom=min(client.bottom,overlap.bottom+1);
    OffsetRect(&overlap,-origin.x,-origin.y);
    RedrawWindow(window,&overlap,NULL,RDW_INVALIDATE|RDW_ERASE|RDW_ALLCHILDREN);
}

static inline BOOL style_frame_xaml_bridge(HWND w,BOOL visible_only)
{
    static const WCHAR *const classes[]={L"Windows.UI.Composition.DesktopWindowContentBridge",L"Microsoft.UI.Content.DesktopChildSiteBridge"};
    for(unsigned i=0;i<sizeof(classes)/sizeof(classes[0]);++i){
        HWND child=NULL;
        while((child=FindWindowExW(w,child,classes[i],NULL))!=NULL)
            if(!visible_only || IsWindowVisible(child))return TRUE;
    }
    return FALSE;
}

/* Let XAML process WM_GETICON/WM_SETICON without a native caption redraw.
 * The caller retains its state during synchronous window-message reentry. */
static inline BOOL style_frame_xaml_icon(HWND w,UINT message,WPARAM wp,LPARAM lp,
    BOOL *updating,const BOOL *detached,LRESULT *result)
{
    LONG_PTR style=GetWindowLongPtrW(w,GWL_STYLE);
    if(*updating || (style&WS_CHILD) || !(style&WS_DLGFRAME) || !style_frame_xaml_bridge(w,FALSE))return FALSE;
    *updating=TRUE;SetLastError(0);
    LONG_PTR old=SetWindowLongPtrW(w,GWL_STYLE,style&~WS_DLGFRAME);
    if(old || !GetLastError()){
        if(*detached || !IsWindow(w)){
            if(IsWindow(w)){
                SetWindowLongPtrW(w,GWL_STYLE,GetWindowLongPtrW(w,GWL_STYLE)|WS_DLGFRAME);
                *result=DefSubclassProc(w,message,wp,lp);
            }
            *updating=FALSE;return TRUE;
        }
        *result=DefSubclassProc(w,message,wp,lp);DWORD error=GetLastError();
        if(IsWindow(w)){
            LONG_PTR current=GetWindowLongPtrW(w,GWL_STYLE);
            SetWindowLongPtrW(w,GWL_STYLE,current|WS_DLGFRAME);
            if(!*detached)SetWindowPos(w,NULL,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|
                SWP_NOZORDER|SWP_NOACTIVATE|SWP_FRAMECHANGED);
        }
        *updating=FALSE;SetLastError(error);return TRUE;
    }
    *updating=FALSE;return FALSE;
}

/* Only replace the output on a successful intersection. Callers decide
 * whether failure means native geometry or an unsupported surface. */
static inline BOOL style_frame_work_area(HWND window,const RECT *outer,RECT *visible)
{
    MONITORINFO monitor={.cbSize=sizeof(monitor)};RECT intersection;
    if(!GetMonitorInfoW(MonitorFromWindow(window,MONITOR_DEFAULTTONEAREST),&monitor) ||
       !IntersectRect(&intersection,outer,&monitor.rcWork))return FALSE;
    *visible=intersection;return TRUE;
}

/* A composed window near the work-area edge can lose its native title if
 * preserving client coordinates would move the outer frame off-screen. */
static inline BOOL style_frame_keep_outer(HWND window,const RECT *wanted,const RECT *client)
{
    RECT outer;MONITORINFO monitor={.cbSize=sizeof(monitor)};
    if(!wanted || !client || (GetWindowLongPtrW(window,GWL_STYLE)&WS_CHILD) ||
       !style_frame_xaml_bridge(window,FALSE) || !GetWindowRect(window,&outer) ||
       !GetMonitorInfoW(MonitorFromRect(&outer,MONITOR_DEFAULTTONEAREST),&monitor))return FALSE;
    return (outer.top>=monitor.rcWork.top && wanted->top-client->top<monitor.rcWork.top) ||
           (outer.left>=monitor.rcWork.left && wanted->left-client->left<monitor.rcWork.left);
}

static inline void style_frame_changed(HWND window)
{
    SetWindowPos(window,NULL,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE|SWP_FRAMECHANGED);
    RedrawWindow(window,NULL,NULL,RDW_FRAME|RDW_INVALIDATE|RDW_UPDATENOW);
}
static inline void style_client_screen(HWND window,RECT *rect)
{
    POINT origin={0,0};GetClientRect(window,rect);ClientToScreen(window,&origin);OffsetRect(rect,origin.x,origin.y);
}
static inline void style_restore_client(HWND window,const RECT *wanted)
{
    RECT current,outer;POINT origin;
    style_client_screen(window,&current);GetWindowRect(window,&outer);
    origin.x=outer.left+wanted->left-current.left;origin.y=outer.top+wanted->top-current.top;
    if(GetWindowLongPtrW(window,GWL_STYLE)&WS_CHILD)ScreenToClient(GetParent(window),&origin);
    SetWindowPos(window,NULL,origin.x,origin.y,
        outer.right-outer.left+(wanted->right-wanted->left)-(current.right-current.left),
        outer.bottom-outer.top+(wanted->bottom-wanted->top)-(current.bottom-current.top),SWP_NOZORDER|SWP_NOACTIVATE);
}
static inline void style_frame_changed_restore(HWND window,const RECT *client,BOOL normal)
{
    style_frame_changed(window);
    if(normal && IsWindow(window))style_restore_client(window,client);
}
#endif
