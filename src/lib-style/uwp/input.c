#include "input.h"
#include "lib-style/common/frame.h"
#include <windowsx.h>

static int input_hit(const style_uwp_input_view *v,LPARAM parameter)
{
    POINT point={GET_X_LPARAM(parameter),GET_Y_LPARAM(parameter)};
    return (int)v->hit(v->context,point);
}

BOOL style_uwp_input_message(const style_uwp_input_view *v,UINT message,WPARAM wp,LPARAM lp,LRESULT *result)
{
    (void)wp;*result=0;
    switch(message){
    case WM_MOUSEACTIVATE:*result=MA_NOACTIVATE;return TRUE;
    case WM_SETCURSOR:{
        POINT point;if(!GetCursorPos(&point) || !ScreenToClient(v->surface,&point))return FALSE;
        SetCursor(LoadCursorW(NULL,style_frame_resize_cursor((int)v->hit(v->context,point))));
        *result=TRUE;return TRUE;
    }
    case WM_NCHITTEST:*result=HTCLIENT;return TRUE;
    case WM_LBUTTONDOWN:{
        int hit=input_hit(v,lp);SetForegroundWindow(v->owner);
        if(*v->detached)return TRUE;
        if(v->button(v->context,hit)){
            *v->pressed=hit;*v->hot=TRUE;SetCapture(v->surface);v->refresh(v->context);
        }else if(hit==HTCAPTION || (hit>=HTLEFT && hit<=HTBOTTOMRIGHT)){
            POINT point={GET_X_LPARAM(lp),GET_Y_LPARAM(lp)};ClientToScreen(v->surface,&point);
            SendMessageW(v->owner,WM_NCLBUTTONDOWN,(WPARAM)hit,MAKELPARAM(point.x,point.y));
        }
        return TRUE;
    }
    case WM_LBUTTONDBLCLK:case WM_RBUTTONUP:
        v->invoke(v->context,input_hit(v,lp),message);return TRUE;
    case WM_MOUSEMOVE:
        if(*v->pressed){BOOL hot=input_hit(v,lp)==*v->pressed;
            if(hot!=*v->hot){*v->hot=hot;v->refresh(v->context);}}
        return TRUE;
    case WM_LBUTTONUP:
        if(*v->pressed){int hit=*v->pressed;BOOL act=input_hit(v,lp)==hit;*v->pressed=0;*v->hot=FALSE;
            ReleaseCapture();if(!*v->detached){v->refresh(v->context);if(act && !*v->detached)v->invoke(v->context,hit,message);}}
        return TRUE;
    case WM_CANCELMODE:case WM_CAPTURECHANGED:
        if(*v->pressed){*v->pressed=0;*v->hot=FALSE;if(GetCapture()==v->surface)ReleaseCapture();v->refresh(v->context);}
        return TRUE;
    default:return FALSE;
    }
}
