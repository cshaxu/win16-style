#include "control_redraw.h"
#include "control_list.h"
#include "list.h"
#include <commctrl.h>

static LRESULT forward_tabs(HWND window,WPARAM count,LPARAM values)
{return DefSubclassProc(window,LB_SETTABSTOPS,count,values);}
LRESULT style_control_list_message(HWND w,style_instance *instance,style_control_state *state,UINT m,WPARAM a,LPARAM b,const style_control_list_ops *ops)
{
    LRESULT result=0;
        if(ops->input(w,ops->context,m,a,b))result=0;
        else if(m==LB_SETTABSTOPS && (GetWindowLongPtrW(w,GWL_STYLE)&LBS_USETABSTOPS))result=style_control_tabs_set(w,instance,state,a,b,forward_tabs);
        else if(m==WM_SETREDRAW){state->redraw_off=!a;result=DefSubclassProc(w,m,a,b);}
        else if(m==WM_PAINT){PAINTSTRUCT ps;HDC dc=BeginPaint(w,&ps);ops->draw(w,ops->context,dc,TRUE);EndPaint(w,&ps);}
        else if(m==WM_PRINTCLIENT)ops->draw(w,ops->context,(HDC)a,FALSE);
        else if(m==WM_NCPAINT || m==WM_PRINT){
            /* This subclass owns the complete list nonclient surface. Native
             * painting first exposes a second scrollbar skin before ours. */
            if(m==WM_PRINT && (b&PRF_CLIENT)){
                HDC dc=(HDC)a;int saved=SaveDC(dc);RECT wr;POINT origin={0,0};
                if(saved){GetWindowRect(w,&wr);ClientToScreen(w,&origin);
                    OffsetViewportOrgEx(dc,origin.x-wr.left,origin.y-wr.top,NULL);
                    ops->draw(w,ops->context,dc,FALSE);RestoreDC(dc,saved);}
            }
            if(!instance->detached && (m==WM_NCPAINT || (b&PRF_NONCLIENT))){
                ops->border(w,ops->context,m==WM_PRINT?(HDC)a:NULL);
                ops->scrollbars(w,ops->context,m==WM_PRINT?(HDC)a:NULL);}
        }
        else if(ops->suppress_hover && (m==WM_NCMOUSEMOVE || m==WM_NCMOUSELEAVE)){
            /* The profile owns whether native hover repaint is suppressed. */
            result=0;
        }
        else if(m==WM_ERASEBKGND)result=1;
        else {
            BOOL moving=m==WM_MOUSEMOVE,changed=FALSE;style_control_list_view before={0},after;
            BOOL suppress=style_control_redraw_can_suppress(w,state->redraw_off,state->suppression) &&
                (m==WM_MOUSEWHEEL || m==WM_VSCROLL || m==WM_HSCROLL || m==WM_KEYDOWN || m==WM_KEYUP ||
                m==WM_SETFOCUS || m==WM_KILLFOCUS || m==WM_ENABLE || m==WM_SETFONT ||
                m==WM_MOUSEMOVE || m==WM_LBUTTONDOWN || m==WM_LBUTTONUP ||
                m==LB_ADDSTRING || m==LB_INSERTSTRING || m==LB_DELETESTRING || m==LB_RESETCONTENT ||
                m==LB_SETCURSEL || m==LB_SETSEL || m==LB_SELITEMRANGE || m==LB_SELITEMRANGEEX ||
                m==LB_SETCARETINDEX || m==LB_SETTOPINDEX);
            if(moving)before=style_control_list_view_read(w);
            result=style_control_redraw_call(w,m,a,b,suppress,&state->suppression,&instance->detached,&state->redraw_off);
            if(!instance->detached && moving){after=style_control_list_view_read(w);
                changed=style_control_list_view_changed(&before,&after);}
            if(!instance->detached && !state->suppression && !state->redraw_off && (m==LB_SETCURSEL || m==LB_SETSEL || m==LB_SELITEMRANGE || m==LB_SELITEMRANGEEX ||
                m==LB_SETCARETINDEX || m==LB_SETTOPINDEX || m==LB_ADDSTRING || m==LB_INSERTSTRING ||
                m==LB_DELETESTRING || m==LB_RESETCONTENT || m==LB_SETCOLUMNWIDTH || m==LB_SETHORIZONTALEXTENT || m==WM_SETFONT || m==WM_ENABLE ||
                m==WM_SETFOCUS || m==WM_KILLFOCUS || m==WM_LBUTTONDOWN || m==WM_LBUTTONUP ||
                changed || m==WM_KEYDOWN || m==WM_KEYUP || m==WM_VSCROLL || m==WM_HSCROLL || m==WM_MOUSEWHEEL)){
                RedrawWindow(w,NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW);
                if(!instance->detached)ops->scrollbars(w,ops->context,NULL);}
        }
        return result;
}
