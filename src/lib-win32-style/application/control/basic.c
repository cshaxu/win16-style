#include "lib-win32-style/application/control/control_redraw.h"
#include "basic.h"
#include <commctrl.h>

LRESULT style_control_basic_message(HWND w,style_instance *instance,style_control_state *state,UINT m,WPARAM a,LPARAM b,
    style_control_basic_kind kind,const style_control_draw *draw)
{
    LRESULT result=0;BOOL static_control=kind==STYLE_CONTROL_STATIC || kind==STYLE_CONTROL_BITMAP || kind==STYLE_CONTROL_SEPARATOR;
    /* Bitmap statics have no themed chrome or text. Keep the native image
     * painter and image ownership, rather than inventing a second blitter. */
    if(kind==STYLE_CONTROL_BITMAP){
        if(m==WM_PRINTCLIENT){
            HDC dc=(HDC)a;int saved=SaveDC(dc);RECT r;
            /* The centered native painter excludes its image rectangle while
             * filling the border. Do not leak that clip into the caller DC. */
            if(saved){GetClientRect(w,&r);IntersectClipRect(dc,r.left,r.top,r.right,r.bottom);
                result=DefSubclassProc(w,m,a,b);RestoreDC(dc,saved);}
        }else result=DefSubclassProc(w,m,a,b);
    }
    /* Native etched STATIC adds WS_EX_STATICEDGE and can leave zero client
     * height. Own its complete line rectangle without changing the stored
     * style or its outer geometry; detach reinstates native NC calculation. */
    else if(kind==STYLE_CONTROL_SEPARATOR && (m==WM_NCCALCSIZE || m==WM_NCPAINT))result=0;
    else if(kind==STYLE_CONTROL_NATIVE_OVERLAY){
        /* A native overlay retains its owner's child/windowless painting.
         * Run the original painter before the renderer operation. */
        result=DefSubclassProc(w,m,a,b);
        if(!instance->detached && (m==WM_PAINT || m==WM_PRINTCLIENT ||
                (m==WM_PRINT && (b&PRF_CLIENT)))){
            HDC dc=(m==WM_PAINT)?GetDC(w):(HDC)a;
            if(dc){draw->render(w,draw->context,dc,FALSE);if(m==WM_PAINT)ReleaseDC(w,dc);}
        }
    }
    else if(m==WM_SETREDRAW){state->redraw_off=!a;result=DefSubclassProc(w,m,a,b);}
    else if(m==WM_PAINT){PAINTSTRUCT ps;HDC dc=BeginPaint(w,&ps);draw->render(w,draw->context,dc,TRUE);EndPaint(w,&ps);}
    else if(m==WM_PRINT && kind==STYLE_CONTROL_SEPARATOR){
        /* STATIC prints etched shapes internally rather than forwarding
         * WM_PRINTCLIENT. Keep printed and on-screen separators identical. */
        if((b&PRF_CLIENT) && (!(b&PRF_CHECKVISIBLE) || IsWindowVisible(w)))draw->render(w,draw->context,(HDC)a,FALSE);
    }
    else if(m==WM_PRINTCLIENT)draw->render(w,draw->context,(HDC)a,FALSE);
    else if(m==WM_ERASEBKGND)result=1;
    else {
        BOOL moving=m==WM_MOUSEMOVE && !static_control,changed=FALSE;
        LRESULT previous=moving?SendMessageW(w,BM_GETSTATE,0,0):0;
        /* Only bracket native drawing leaves, not BM_CLICK or mouse/key
         * dispatch: automatic radio-group traversal must see the original
         * visibility throughout the outer input operation. */
        BOOL suppress=!static_control && style_control_redraw_can_suppress(w,state->redraw_off,state->suppression) &&
            (m==BM_SETCHECK || m==BM_SETSTATE || m==BM_SETSTYLE || m==WM_ENABLE ||
             m==WM_SETFOCUS || m==WM_KILLFOCUS || m==WM_SETTEXT || m==WM_SETFONT ||
             m==WM_MOUSEMOVE);
        result=style_control_redraw_call(w,m,a,b,suppress,&state->suppression,&instance->detached,&state->redraw_off);
        if(!instance->detached && moving)changed=previous!=SendMessageW(w,BM_GETSTATE,0,0);
        if(!instance->detached && instance->calls==1 && !state->suppression && !state->redraw_off && (m==BM_SETCHECK || m==BM_SETSTATE || m==BM_SETSTYLE || m==BM_CLICK || m==WM_ENABLE ||
            m==WM_SETFOCUS || m==WM_KILLFOCUS || m==WM_SETTEXT || m==WM_SETFONT ||
            m==BM_SETIMAGE || m==BCM_SETIMAGELIST || (static_control && (m==STM_SETICON || m==STM_SETIMAGE)) ||
            m==WM_LBUTTONDOWN || m==WM_LBUTTONUP || changed || m==WM_KEYDOWN || m==WM_KEYUP))
            RedrawWindow(w,NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW);
    }
    return result;
}
