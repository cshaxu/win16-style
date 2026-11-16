#include "messages.h"
#include "lib-win32-style/application/graphics/present.h"
#include "lib-win32-style/application/window/frame.h"
#include <commctrl.h>

BOOL style_frame_message(style_instance *instance,style_native_frame *state,UINT msg,WPARAM wp,LPARAM lp,const style_frame_message_ops *v,LRESULT *result)
{
    HWND w=instance->window;
    switch(msg){
    case WM_SHOWWINDOW:
        if(wp && state->pending && v->activate){
            RECT client;BOOL normal=!IsIconic(w)&&!IsZoomed(w);
            style_client_screen(w,&client);state->pending=FALSE;
            v->activate(w,v->context,&client,normal);
            if(instance->detached)return TRUE;
        }
        return FALSE;
    case WM_NCCALCSIZE: {
        RECT r=wp?((NCCALCSIZE_PARAMS *)lp)->rgrc[0]:*(RECT *)lp;RECT client;LRESULT native;
        if(!v->client || !v->client(w,v->context,&r,&client))return FALSE;
        /* USER still updates scrollbar state before our geometry is applied. */
        native=DefSubclassProc(w,msg,wp,lp);
        if(instance->detached)return TRUE;
        OffsetRect(&client,r.left,r.top);
        /* Full replacement keeps only native redraw requests, not alignment. */
        LRESULT flags=wp && !(native&WVR_VALIDRECTS)?native&WVR_REDRAW:0;
        *result=style_frame_set_client(wp,lp,&client,flags);
        return TRUE;
    }
    case WM_NCPAINT:
        if(!v->buffered)return FALSE;
        style_frame_ncpaint(w,wp,v->context,v->paint,v->buffered);return TRUE;
    case WM_SETCURSOR:
        if(!style_frame_set_cursor(w,wp,lp))return FALSE;
        *result=TRUE;return TRUE;
    case STYLE_NCUAHDRAWCAPTION:
    case STYLE_NCUAHDRAWFRAME:
    case STYLE_UAHDRAWMENU:
    case STYLE_UAHDRAWMENUITEM:
        return style_frame_suppress_native_theme(msg,IsIconic(w),GetMenu(w)!=NULL);
    case WM_NCACTIVATE:
        state->active=(BOOL)wp;if(IsIconic(w))return FALSE;
        *result=TRUE;v->paint(w,v->context,NULL);return TRUE;
    case WM_NCHITTEST:
        return v->hit_test && v->hit_test(w,v->context,lp,result);
    case WM_PRINT:
        if((lp&PRF_CHECKVISIBLE) && !IsWindowVisible(w))return TRUE;
        if(lp&PRF_NONCLIENT)v->paint(w,v->context,(HDC)wp);
        if(!instance->detached)style_print_client_and_children(w,&instance->detached,(HDC)wp,lp);
        return TRUE;
    case WM_HSCROLL:
    case WM_VSCROLL:
        *result=DefSubclassProc(w,msg,wp,lp);return TRUE;
    case WM_DPICHANGED:
        *result=DefSubclassProc(w,msg,wp,lp);
        {DWORD error=GetLastError();
            if(!instance->detached){
                if(v->refresh_font){v->refresh_font(w,v->context);style_frame_changed(w);}
                else v->paint(w,v->context,NULL);
            }
            SetLastError(error);
        }return TRUE;
    case WM_SETTEXT:
    case WM_SETTINGCHANGE:
    case WM_SYSCOLORCHANGE:
    case WM_THEMECHANGED: {
        *result=DefSubclassProc(w,msg,wp,lp);DWORD error=GetLastError();
        if(!instance->detached)v->paint(w,v->context,NULL);
        SetLastError(error);return TRUE;
    }
    default:return FALSE;
    }
}
