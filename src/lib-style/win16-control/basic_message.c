#include "lib-style/common/control_redraw.h"
#include "basic_message.h"
#include "control_class.h"
#include <commctrl.h>

LRESULT win16_control_basic_message(HWND w,control_style *s,UINT m,WPARAM a,LPARAM b,win16_control_state_present present,win16_control_state_painter paint)
{
    LRESULT result=0;
    /* Bitmap statics have no themed chrome or text. Keep the native image
     * painter and image ownership, rather than inventing a second blitter. */
    if(ntw16s_is_static(w) && (GetWindowLongPtrW(w,GWL_STYLE)&SS_TYPEMASK)==SS_BITMAP){
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
    else if(ntw16s_is_separator(w) && (m==WM_NCCALCSIZE || m==WM_NCPAINT))result=0;
    else if(ntw16s_vb_frame(w)){
        /* Unlike native GROUPBOX, a VB Frame owns children and paints its
         * windowless labels. Always run the original painter first. */
        result=DefSubclassProc(w,m,a,b);
        if(!s->detached && (m==WM_PAINT || m==WM_PRINTCLIENT ||
                (m==WM_PRINT && (b&PRF_CLIENT)))){
            HDC dc=(m==WM_PAINT)?GetDC(w):(HDC)a;
            if(dc){paint(w,s,dc);if(m==WM_PAINT)ReleaseDC(w,dc);}
        }
    }
    else if(m==WM_SETREDRAW){s->list_redraw_off=!a;result=DefSubclassProc(w,m,a,b);}
    else if(m==WM_PAINT){PAINTSTRUCT ps;HDC dc=BeginPaint(w,&ps);present(w,s,dc,paint);EndPaint(w,&ps);}
    else if(m==WM_PRINT && ntw16s_is_separator(w)){
        /* STATIC prints etched shapes internally rather than forwarding
         * WM_PRINTCLIENT. Keep printed and on-screen separators identical. */
        if((b&PRF_CLIENT) && (!(b&PRF_CHECKVISIBLE) || IsWindowVisible(w)))paint(w,s,(HDC)a);
    }
    else if(m==WM_PRINTCLIENT)paint(w,s,(HDC)a);
    else if(m==WM_ERASEBKGND)result=1;
    else {
        BOOL moving=m==WM_MOUSEMOVE && !ntw16s_is_static(w),changed=FALSE;
        LRESULT previous=moving?SendMessageW(w,BM_GETSTATE,0,0):0;
        /* Only bracket native drawing leaves, not BM_CLICK or mouse/key
         * dispatch: automatic radio-group traversal must see the original
         * visibility throughout the outer input operation. */
        BOOL suppress=!ntw16s_is_static(w) && style_control_redraw_can_suppress(w,s->list_redraw_off,s->list_suppression) &&
            (m==BM_SETCHECK || m==BM_SETSTATE || m==BM_SETSTYLE || m==WM_ENABLE ||
             m==WM_SETFOCUS || m==WM_KILLFOCUS || m==WM_SETTEXT || m==WM_SETFONT ||
             m==WM_MOUSEMOVE);
        result=style_control_redraw_call(w,m,a,b,suppress,&s->list_suppression,&s->detached,&s->list_redraw_off);
        if(!s->detached && moving)changed=previous!=SendMessageW(w,BM_GETSTATE,0,0);
        if(!s->detached && s->calls==1 && !s->list_suppression && !s->list_redraw_off && (m==BM_SETCHECK || m==BM_SETSTATE || m==BM_SETSTYLE || m==BM_CLICK || m==WM_ENABLE ||
            m==WM_SETFOCUS || m==WM_KILLFOCUS || m==WM_SETTEXT || m==WM_SETFONT ||
            m==BM_SETIMAGE || m==BCM_SETIMAGELIST || (ntw16s_is_static(w) && (m==STM_SETICON || m==STM_SETIMAGE)) ||
            m==WM_LBUTTONDOWN || m==WM_LBUTTONUP || changed || m==WM_KEYDOWN || m==WM_KEYUP))
            RedrawWindow(w,NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW);
    }
    return result;
}
