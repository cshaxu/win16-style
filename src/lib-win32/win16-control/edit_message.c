#include "edit_message.h"
#include "control_class.h"
#include <commctrl.h>

LRESULT win16_control_edit_message(HWND w,control_style *s,UINT m,WPARAM a,LPARAM b,const win16_control_edit_services *services)
{
    LRESULT result=0;
        /* EDIT retains its native buffer, selection, caret, undo and IME.
         * Original flat EDIT borders are painted inside the client area. */
        if(services->scroll_input(w,s,m,a,b))return result;
        if(s->edit_scrollbars && (m==WM_NCMOUSEMOVE || m==WM_NCMOUSELEAVE))return result;
        if(s->edit_scrollbars && m==WM_NCPAINT){
            services->scrollbars(w,s,NULL);
            if(!s->detached && (GetWindowLongPtrW(w,GWL_EXSTYLE)&WS_EX_CLIENTEDGE)){
                if(a==1)services->border(w,s,NULL);
                else if(a){
                    HRGN region=CreateRectRgn(0,0,0,0);
                    if(region){
                        if(CombineRgn(region,(HRGN)a,NULL,RGN_COPY)!=ERROR){
                            HDC dc=GetDCEx(w,region,DCX_WINDOW|DCX_CACHE|DCX_INTERSECTRGN);
                            if(dc){services->border(w,s,dc);ReleaseDC(w,dc);region=NULL;}
                        }
                        if(region)DeleteObject(region);
                    }
                }
            }
            return result;
        }
        if(s->edit_scrollbars && m==WM_SETREDRAW)s->list_redraw_off=!a;
        {BOOL suppress=s->edit_scrollbars && win16_control_state_can_suppress_redraw(w,s) &&
            (m==WM_VSCROLL || m==WM_HSCROLL || m==WM_MOUSEWHEEL || m==EM_LINESCROLL || m==WM_SETTEXT ||
            m==EM_REPLACESEL || m==EM_UNDO || m==WM_UNDO || m==WM_KEYDOWN || m==WM_CHAR ||
            (m==WM_MOUSEMOVE && GetCapture()==w && (a&MK_LBUTTON)) || m==WM_LBUTTONDOWN || m==WM_LBUTTONUP);
        if(suppress && s->list_suppression++==0)DefSubclassProc(w,WM_SETREDRAW,FALSE,0);
        int native_dc=(m==WM_PRINT || m==WM_PRINTCLIENT)?SaveDC((HDC)a):0;
        result=DefSubclassProc(w,m,a,b);
        if(native_dc)RestoreDC((HDC)a,native_dc);
        if(suppress && --s->list_suppression==0 && !s->detached && !s->list_redraw_off)
            DefSubclassProc(w,WM_SETREDRAW,TRUE,0);
        if(suppress && !s->detached)RedrawWindow(w,NULL,NULL,RDW_INVALIDATE|RDW_FRAME|RDW_UPDATENOW);
        }
        if(!s->detached && (m==WM_PRINTCLIENT || (m==WM_PRINT && (b&PRF_CLIENT)) || m==WM_PAINT || m==WM_ENABLE))
            services->client_border(w,s,(m==WM_PRINT || m==WM_PRINTCLIENT)?(HDC)a:NULL);
        if(!s->detached && (m==WM_NCPAINT || (m==WM_PRINT && (b&PRF_NONCLIENT))))
            {services->scrollbars(w,s,m==WM_PRINT?(HDC)a:NULL);
             if(ntw16s_vb_edit(w) || ntw16s_native_superclass(w,L"EDIT") ||
                (GetWindowLongPtrW(w,GWL_EXSTYLE)&WS_EX_CLIENTEDGE))services->border(w,s,m==WM_PRINT?(HDC)a:NULL);}
        else if(!s->detached && (m==WM_VSCROLL || m==WM_HSCROLL || m==WM_MOUSEWHEEL ||
            m==EM_LINESCROLL || m==WM_SETTEXT || m==WM_KEYDOWN || m==WM_CHAR))
            services->scrollbars(w,s,NULL);
        return result;
}
