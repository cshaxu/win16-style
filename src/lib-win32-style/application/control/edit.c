#include "control_redraw.h"
#include "lib-win32-style/application/window/frame.h"
#include "edit.h"
#include <commctrl.h>

LRESULT style_control_edit_message(HWND w,style_instance *instance,style_control_state *state,const style_scroll_resources *scroll,UINT m,WPARAM a,LPARAM b,const style_control_edit_ops *ops)
{
    LRESULT result=0;BOOL custom_scroll=scroll && scroll->edit;
        /* EDIT retains its native buffer, selection, caret, undo and IME.
         * The profile supplies its client and nonclient border rendering. */
        if(ops->input && ops->input(w,ops->context,m,a,b))return result;
        if(custom_scroll && ops->suppress_hover && (m==WM_NCMOUSEMOVE || m==WM_NCMOUSELEAVE))return result;
        if(custom_scroll && ops->scrollbars && ops->border && m==WM_NCPAINT){
            ops->scrollbars(w,ops->context,NULL);
            if(!instance->detached && (GetWindowLongPtrW(w,GWL_EXSTYLE)&WS_EX_CLIENTEDGE)){
                if(a==1)ops->border(w,ops->context,NULL);
                else if(a){
                    HDC dc=style_frame_update_dc(w,(HRGN)a);
                    if(dc){ops->border(w,ops->context,dc);ReleaseDC(w,dc);}
                }
            }
            return result;
        }
        if(custom_scroll && m==WM_SETREDRAW)state->redraw_off=!a;
        {BOOL suppress=custom_scroll && style_control_redraw_can_suppress(w,state->redraw_off,state->suppression) &&
            (m==WM_VSCROLL || m==WM_HSCROLL || m==WM_MOUSEWHEEL || m==EM_LINESCROLL || m==WM_SETTEXT ||
            m==EM_REPLACESEL || m==EM_UNDO || m==WM_UNDO || m==WM_KEYDOWN || m==WM_CHAR ||
            (m==WM_MOUSEMOVE && GetCapture()==w && (a&MK_LBUTTON)) || m==WM_LBUTTONDOWN || m==WM_LBUTTONUP);
        if(suppress)style_control_redraw_begin(w,&state->suppression);
        int native_dc=(m==WM_PRINT || m==WM_PRINTCLIENT)?SaveDC((HDC)a):0;
        result=DefSubclassProc(w,m,a,b);
        if(native_dc)RestoreDC((HDC)a,native_dc);
        if(suppress)style_control_redraw_end(w,&state->suppression,&instance->detached,&state->redraw_off);
        if(suppress && !instance->detached)RedrawWindow(w,NULL,NULL,RDW_INVALIDATE|RDW_FRAME|RDW_UPDATENOW);
        }
        if(!instance->detached && ops->client_border && (m==WM_PRINTCLIENT || (m==WM_PRINT && (b&PRF_CLIENT)) || m==WM_PAINT || m==WM_ENABLE))
            ops->client_border(w,ops->context,(m==WM_PRINT || m==WM_PRINTCLIENT)?(HDC)a:NULL);
        if(!instance->detached && (m==WM_NCPAINT || (m==WM_PRINT && (b&PRF_NONCLIENT))))
            {if(ops->scrollbars)ops->scrollbars(w,ops->context,m==WM_PRINT?(HDC)a:NULL);
             if(ops->border)ops->border(w,ops->context,m==WM_PRINT?(HDC)a:NULL);}
        else if(!instance->detached && ops->scrollbars && (m==WM_VSCROLL || m==WM_HSCROLL || m==WM_MOUSEWHEEL ||
            m==EM_LINESCROLL || m==WM_SETTEXT || m==WM_KEYDOWN || m==WM_CHAR))
            ops->scrollbars(w,ops->context,NULL);
        return result;
}
