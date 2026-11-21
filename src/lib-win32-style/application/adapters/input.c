#include "input.h"
#include "surface.h"
#include "lib-win32-style/application/window/frame.h"
#include "lib-win32-style/application/window/caption_slots.h"
#include <windowsx.h>

static int input_hit(style_uwp_frame *v,LPARAM parameter)
{
    POINT point={GET_X_LPARAM(parameter),GET_Y_LPARAM(parameter)};
    return (int)v->ops->hit(v->context,point);
}

static void invoke_caption(style_uwp_frame *frame,int hit,UINT message)
{
    HWND owner=frame->host.owner;
    BOOL can_close=message!=WM_LBUTTONDBLCLK || hit!=HTSYSMENU ||
        style_caption_button_enabled(owner,HTCLOSE);
    BOOL can_maximize=message==WM_LBUTTONDBLCLK && hit==HTCAPTION &&
        frame->ops->can_maximize(frame->context);
    UINT command=style_caption_gesture(hit,message,IsZoomed(owner),can_maximize,can_close);
    if(command)PostMessageW(owner,WM_SYSCOMMAND,command,command==SC_KEYMENU?L' ':0);
}

BOOL style_uwp_input_message(style_uwp_frame *v,UINT message,WPARAM wp,LPARAM lp,LRESULT *result)
{
    (void)wp;*result=0;
    switch(message){
    case WM_MOUSEACTIVATE:*result=MA_NOACTIVATE;return TRUE;
    case WM_SETCURSOR:{
        POINT point;if(!GetCursorPos(&point) || !ScreenToClient(v->host.surface,&point))return FALSE;
        SetCursor(LoadCursorW(NULL,style_frame_resize_cursor((int)v->ops->hit(v->context,point))));
        *result=TRUE;return TRUE;
    }
    case WM_NCHITTEST:*result=HTCLIENT;return TRUE;
    case WM_LBUTTONDOWN:{
        int hit=input_hit(v,lp);SetForegroundWindow(v->host.owner);
        if(v->host.instance.detached)return TRUE;
        if(style_caption_button_enabled(v->host.owner,hit)){
            v->pressed=hit;v->hot=TRUE;SetCapture(v->host.surface);style_uwp_surface_refresh(v);
        }else if(hit==HTCAPTION || (hit>=HTLEFT && hit<=HTBOTTOMRIGHT)){
            POINT point={GET_X_LPARAM(lp),GET_Y_LPARAM(lp)};ClientToScreen(v->host.surface,&point);
            SendMessageW(v->host.owner,WM_NCLBUTTONDOWN,(WPARAM)hit,MAKELPARAM(point.x,point.y));
        }
        return TRUE;
    }
    case WM_LBUTTONDBLCLK:case WM_RBUTTONUP:
        invoke_caption(v,input_hit(v,lp),message);return TRUE;
    case WM_MOUSEMOVE:
        if(v->pressed){BOOL hot=input_hit(v,lp)==v->pressed;
            if(hot!=v->hot){v->hot=hot;style_uwp_surface_refresh(v);}}
        return TRUE;
    case WM_LBUTTONUP:
        if(v->pressed){int hit=v->pressed;BOOL act=input_hit(v,lp)==hit;v->pressed=0;v->hot=FALSE;
            ReleaseCapture();if(!v->host.instance.detached){style_uwp_surface_refresh(v);if(act && !v->host.instance.detached)invoke_caption(v,hit,message);}}
        return TRUE;
    case WM_CANCELMODE:case WM_CAPTURECHANGED:
        if(v->pressed){v->pressed=0;v->hot=FALSE;if(GetCapture()==v->host.surface)ReleaseCapture();style_uwp_surface_refresh(v);}
        return TRUE;
    default:return FALSE;
    }
}
