/* Project-authored implementation from documented binary observations.
 * Original code and system binaries are not build or runtime dependencies. */
#include "move.h"
#include <windowsx.h>
#include <stdio.h>
#include <wchar.h>
#include <dwmapi.h>
#include <limits.h>

typedef struct move_loop {
    HWND window;
    BOOL cancelled;
} move_loop;
static _Thread_local move_loop *tracking;

void win16_style_move_cancel(HWND window)
{
    if(tracking && tracking->window==window && !tracking->cancelled){
        tracking->cancelled=TRUE;
        /* Sent cancellation must also wake a GetMessage blocked on an empty queue. */
        PostThreadMessageW(GetCurrentThreadId(),WM_NULL,0,0);
    }
}

/* Compositor-safe, per-transaction overlay. Hide and flush our own preview
 * before capture: current DWM can include it even without CAPTUREBLT.
 * Destroying this owned HWND removes the preview without XORing changed apps.
 * USER's 55/AA pattern and PATINVERT are evaluated over captured RGB pixels. */
typedef struct outline_surface {
    HWND window;
    HINSTANCE instance;
    WCHAR name[80];
    HDC dc,screen;
    HBITMAP bitmap;
    HGDIOBJ previous;
    DWORD *bits;
    int width,height,stroke,unit;
    POINT origin;
    UINT physical_dpi,logical_dpi;
    DPI_AWARENESS awareness;
    RECT work[32];UINT monitors;
} outline_surface;
static LRESULT CALLBACK surface_proc(HWND window,UINT message,WPARAM wp,LPARAM lp)
{
    if(message==WM_NCCREATE){
        const CREATESTRUCTW *creation=(const CREATESTRUCTW *)lp;
        SetWindowLongPtrW(window,GWLP_USERDATA,(LONG_PTR)creation->lpCreateParams);
        if(GetWindowLongPtrW(window,GWLP_USERDATA)!=(LONG_PTR)creation->lpCreateParams)return FALSE;
    }
    return DefWindowProcW(window,message,wp,lp);
}
static BOOL CALLBACK cache_work_area(HMONITOR monitor,HDC dc,RECT *rect,LPARAM data)
{
    outline_surface *s=(outline_surface *)data;MONITORINFO info={.cbSize=sizeof(info)};
    (void)dc;(void)rect;
    if(s->monitors==ARRAYSIZE(s->work) || !GetMonitorInfoW(monitor,&info))return FALSE;
    s->work[s->monitors++]=info.rcWork;return TRUE;
}
static void surface_close(outline_surface *s)
{
    if(s->window && GetWindowLongPtrW(s->window,GWLP_USERDATA)==(LONG_PTR)s)DestroyWindow(s->window);
    if(s->name[0])UnregisterClassW(s->name,s->instance);
    if(s->previous)SelectObject(s->dc,s->previous);
    if(s->bitmap)DeleteObject(s->bitmap);
    if(s->dc)DeleteDC(s->dc);
    if(s->screen)ReleaseDC(NULL,s->screen);
}
static BOOL surface_draw_physical(outline_surface *s,const RECT *r)
{
    POINT destination={r->left,r->top},source={0,0};SIZE size={s->width,s->height};
    BLENDFUNCTION blend={AC_SRC_OVER,0,255,AC_SRC_ALPHA};
    BOOL visible=IsWindowVisible(s->window),result;
    if(visible){ShowWindow(s->window,SW_HIDE);DwmFlush();}
    if(!BitBlt(s->dc,0,0,s->width,s->stroke,s->screen,r->left,r->top,SRCCOPY) ||
       !BitBlt(s->dc,0,s->height-s->stroke,s->width,s->stroke,s->screen,r->left,r->bottom-s->stroke,SRCCOPY) ||
       !BitBlt(s->dc,0,s->stroke,s->stroke,s->height-2*s->stroke,s->screen,r->left,r->top+s->stroke,SRCCOPY) ||
       !BitBlt(s->dc,s->width-s->stroke,s->stroke,s->stroke,s->height-2*s->stroke,s->screen,r->right-s->stroke,r->top+s->stroke,SRCCOPY))return FALSE;
    GdiFlush();
    for(int y=0;y<s->height;++y)for(int x=0;x<s->width;++x){
        if(y>=s->stroke && y<s->height-s->stroke && x==s->stroke)x=s->width-s->stroke;
        DWORD *pixel=&s->bits[y*s->width+x];
        if(x<s->stroke || x>=s->width-s->stroke || y<s->stroke || y>=s->height-s->stroke){
            LONGLONG px=(LONGLONG)x+r->left-s->origin.x,py=(LONGLONG)y+r->top-s->origin.y;
            /* Floor division keeps the phase across negative monitor coordinates. */
            px=px>=0?px/s->unit:-((-px+s->unit-1)/s->unit);
            py=py>=0?py/s->unit:-((-py+s->unit-1)/s->unit);
            *pixel=0xff000000|((*pixel^(((px^py)&1)?0xffffff:0))&0xffffff);
        }else *pixel=0;
    }
    result=UpdateLayeredWindow(s->window,NULL,&destination,&size,s->dc,&source,0,&blend,ULW_ALPHA);
    if(result && visible)SetWindowPos(s->window,HWND_TOPMOST,0,0,0,0,
        SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE|SWP_SHOWWINDOW);
    return result;
}
/* 0=render failure,1=outside every work area (retain last preview),2=drawn. */
static int surface_draw(outline_surface *s,const RECT *r)
{
    RECT physical,intersection;BOOL result,visible=FALSE;DPI_AWARENESS_CONTEXT previous;
    LONGLONG dx=(LONGLONG)r->left*s->physical_dpi,dy=(LONGLONG)r->top*s->physical_dpi;
    LONGLONG x,y;
    if(s->awareness==DPI_AWARENESS_PER_MONITOR_AWARE || s->physical_dpi==s->logical_dpi){x=r->left;y=r->top;}
    else{
        /* Same absolute quantization as native SetWindowPos, including negative
         * deltas on the primary display. Do not round an origin-relative delta. */
        x=(dx+(dx>=0?s->logical_dpi/2:-(LONGLONG)(s->logical_dpi/2)))/s->logical_dpi;
        y=(dy+(dy>=0?s->logical_dpi/2:-(LONGLONG)(s->logical_dpi/2)))/s->logical_dpi;
    }
    if(x<LONG_MIN || x>LONG_MAX-s->width || y<LONG_MIN || y>LONG_MAX-s->height)return FALSE;
    physical.left=(LONG)x;physical.top=(LONG)y;
    physical.right=physical.left+s->width;physical.bottom=physical.top+s->height;
    for(UINT i=0;i<s->monitors;++i)if(IntersectRect(&intersection,&physical,&s->work[i])){visible=TRUE;break;}
    if(!visible)return 1;
    previous=SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    if(!previous)return FALSE;
    result=surface_draw_physical(s,&physical);SetThreadDpiAwarenessContext(previous);return result?2:0;
}
static BOOL surface_open_physical(outline_surface *s,HWND owner,const RECT *r,DWORD style)
{
    WNDCLASSW cls={0};BITMAPINFO info={0};UINT dpi;
    s->width=r->right-r->left;s->height=r->bottom-r->top;s->origin.x=r->left;s->origin.y=r->top;
    if(s->width<3 || s->height<3 || s->width>8192 || s->height>8192 ||
       (ULONGLONG)s->width*s->height>16777216)return FALSE;
    if(!EnumDisplayMonitors(NULL,NULL,cache_work_area,(LPARAM)s) || !s->monitors)return FALSE;
    if(s->awareness==DPI_AWARENESS_UNAWARE && s->monitors!=1)return FALSE;
    {RECT intersection;BOOL visible=FALSE;
    for(UINT i=0;i<s->monitors;++i)if(IntersectRect(&intersection,r,&s->work[i])){visible=TRUE;break;}
    if(!visible)return FALSE;}
    if(!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
        (LPCWSTR)(ULONG_PTR)&surface_open_physical,&s->instance))return FALSE;
    swprintf_s(s->name,80,L"Win16Outline-%lu-%p",GetCurrentThreadId(),(void *)s);
    cls.lpfnWndProc=surface_proc;cls.hInstance=s->instance;cls.lpszClassName=s->name;
    if(!RegisterClassW(&cls)){s->name[0]=0;return FALSE;}
    s->window=CreateWindowExW(WS_EX_LAYERED|WS_EX_TRANSPARENT|WS_EX_NOACTIVATE|WS_EX_TOOLWINDOW,
        s->name,L"",WS_POPUP,r->left,r->top,s->width,s->height,owner,NULL,s->instance,s);
    if(!s->window)return FALSE;
    dpi=GetDpiForWindow(s->window);if(dpi<48 || dpi>768)return FALSE;s->physical_dpi=dpi;
    if(s->awareness==DPI_AWARENESS_SYSTEM_AWARE && dpi!=s->logical_dpi)return FALSE;
    s->stroke=max(1,MulDiv((style&WS_THICKFRAME)?3:1,(int)dpi,96));
    s->unit=max(1,MulDiv(1,(int)dpi,96));
    if(s->width<=2*s->stroke || s->height<=2*s->stroke)return FALSE;
    s->screen=GetDC(NULL);s->dc=CreateCompatibleDC(NULL);
    if(!s->screen || !s->dc)return FALSE;
    info.bmiHeader.biSize=sizeof(info.bmiHeader);info.bmiHeader.biWidth=s->width;
    info.bmiHeader.biHeight=-s->height;info.bmiHeader.biPlanes=1;info.bmiHeader.biBitCount=32;
    s->bitmap=CreateDIBSection(NULL,&info,DIB_RGB_COLORS,(void **)&s->bits,NULL,0);
    if(!s->bitmap)return FALSE;
    ZeroMemory(s->bits,(SIZE_T)s->width*s->height*4);
    s->previous=SelectObject(s->dc,s->bitmap);
    if(!s->previous || s->previous==HGDI_ERROR){s->previous=NULL;return FALSE;}
    return surface_draw_physical(s,r);
}
static BOOL surface_open(outline_surface *s,HWND owner,const RECT *r,DWORD style)
{
    RECT physical;BOOL result;DPI_AWARENESS_CONTEXT previous;
    (void)r;
    s->awareness=GetAwarenessFromDpiAwarenessContext(GetWindowDpiAwarenessContext(owner));
    if(s->awareness==DPI_AWARENESS_INVALID)return FALSE;
    s->logical_dpi=GetDpiForWindow(owner);if(!s->logical_dpi)return FALSE;
    previous=SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    if(!previous)return FALSE;
    result=GetWindowRect(owner,&physical) && surface_open_physical(s,owner,&physical,style);
    SetThreadDpiAwarenessContext(previous);return result;
}

BOOL win16_style_move(HWND window,UINT dpi,BOOL mouse,LPARAM start,
    const void *owner,win16_style_move_valid valid)
{
    DWORD style=(DWORD)GetWindowLongPtrW(window,GWL_STYLE);
    RECT original,preview,mouse_base;POINT anchor;MSG message;move_loop loop={window,FALSE};
    outline_surface surface={0};
    BOOL commit=FALSE,entered=FALSE,handled=FALSE,got_quit=FALSE;int quit_code=0;
    HMENU menu;UINT move_state;
    if(tracking)return TRUE; /* Reject reentry without releasing capture. */
    if((style&WS_CHILD) || IsZoomed(window) || IsIconic(window) ||
       !IsWindowVisible(window) || !IsWindowEnabled(window) || GetCapture() ||
       (GetWindowLongPtrW(window,GWL_EXSTYLE)&WS_EX_LAYOUTRTL) ||
       GetPropW(window,L"Win16Style.MoveTransaction") || !GetWindowRect(window,&original))return FALSE;
    menu=GetSystemMenu(window,FALSE);move_state=menu?GetMenuState(menu,SC_MOVE,MF_BYCOMMAND):(UINT)-1;
    if(move_state!=(UINT)-1 && (move_state&(MF_DISABLED|MF_GRAYED)))return FALSE;
    if(!surface_open(&surface,window,&original,style))goto cleanup;
    preview=original;mouse_base=original;
    if(mouse){anchor.x=GET_X_LPARAM(start);anchor.y=GET_Y_LPARAM(start);}
    else if(!GetCursorPos(&anchor))goto cleanup;
    tracking=&loop;
    if(!SetPropW(window,L"Win16Style.MoveTransaction",&loop))goto cleanup;
    SetActiveWindow(window);
    if(!valid(window,owner))goto cleanup;
    SetCapture(window);
    if(GetCapture()!=window)goto cleanup;
    handled=TRUE;entered=TRUE;SendMessageW(window,WM_ENTERSIZEMOVE,0,0);
    if(loop.cancelled || !valid(window,owner))goto cleanup;
    SetWindowPos(surface.window,HWND_TOPMOST,0,0,0,0,
        SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE|SWP_SHOWWINDOW);
    while(!loop.cancelled && valid(window,owner)){
        int status=GetMessageW(&message,NULL,0,0);POINT point;RECT next;BOOL keyboard=FALSE;
        if(status<=0){if(!status){got_quit=TRUE;quit_code=(int)message.wParam;}break;}
        if(loop.cancelled || !valid(window,owner) || GetCapture()!=window)break;
        if(message.message==WM_KEYDOWN && message.wParam==VK_ESCAPE)break;
        if((message.message==WM_LBUTTONUP || message.message==WM_NCLBUTTONUP) ||
           (message.message==WM_KEYDOWN && message.wParam==VK_RETURN)){commit=TRUE;break;}
        next=preview;
        if(message.message==WM_MOUSEMOVE && message.hwnd==window){
            if(mouse && !(message.wParam&MK_LBUTTON))continue;
            point.x=GET_X_LPARAM(message.lParam);point.y=GET_Y_LPARAM(message.lParam);
            ClientToScreen(window,&point);
            next=mouse_base;OffsetRect(&next,point.x-anchor.x,point.y-anchor.y);
        }else if(message.message==WM_KEYDOWN && message.wParam>=VK_LEFT && message.wParam<=VK_DOWN){
            /* USER6:0F9D/0FDE: Ctrl=1, otherwise half the VGA caption-button
             * metrics (18x18 ->9). Profile grid overrides are not imported. */
            int step=max(1,MulDiv((GetKeyState(VK_CONTROL)&0x8000)?1:9,(int)dpi,96));
            keyboard=TRUE;
            OffsetRect(&next,message.wParam==VK_LEFT?-step:message.wParam==VK_RIGHT?step:0,
                message.wParam==VK_UP?-step:message.wParam==VK_DOWN?step:0);
        }else if((message.message>=WM_KEYFIRST && message.message<=WM_KEYLAST)){
            continue;
        }else{
            TranslateMessage(&message);DispatchMessageW(&message);continue;
        }
        SendMessageW(window,WM_MOVING,0,(LPARAM)&next);
        if(loop.cancelled || !valid(window,owner))break;
        /* Moving cannot silently become resizing through an application callback. */
        if((LONGLONG)next.left+original.right-original.left>LONG_MAX ||
           (LONGLONG)next.top+original.bottom-original.top>LONG_MAX)break;
        next.right=next.left+original.right-original.left;next.bottom=next.top+original.bottom-original.top;
        if(!EqualRect(&next,&preview)){
            int painted=surface_draw(&surface,&next);if(!painted)break;
            if(painted==2){preview=next;if(keyboard){mouse_base=preview;if(!GetCursorPos(&anchor))break;}}
        }
    }
cleanup:
    surface_close(&surface);
    if(tracking==&loop){
        /* Unbind before releasing our capture: its notification is not cancellation. */
        tracking=NULL;
        if(GetCapture()==window)ReleaseCapture();
        if(commit && !loop.cancelled && valid(window,owner))
            SetWindowPos(window,NULL,preview.left,preview.top,0,0,SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
        if(GetPropW(window,L"Win16Style.MoveTransaction")==&loop){
            RemovePropW(window,L"Win16Style.MoveTransaction");
            if(entered)SendMessageW(window,WM_EXITSIZEMOVE,0,0);
        }
        if(got_quit)PostQuitMessage(quit_code);
        return handled;
    }
    return FALSE;
}
