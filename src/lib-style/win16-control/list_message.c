#include "lib-style/common/control_redraw.h"
#include "list_message.h"
#include <commctrl.h>

typedef struct list_view_state {
    LRESULT top,selection,caret;
} list_view_state;
static list_view_state list_view(HWND w)
{
    list_view_state v;
    v.top=SendMessageW(w,LB_GETTOPINDEX,0,0);
    v.selection=SendMessageW(w,LB_GETCURSEL,0,0);
    v.caret=SendMessageW(w,LB_GETCARETINDEX,0,0);return v;
}

LRESULT win16_control_list_message(HWND w,control_style *s,UINT m,WPARAM a,LPARAM b,const win16_control_list_services *services)
{
    LRESULT result=0;
        if(services->scroll_input(w,s,m,a,b))result=0;
        else if(m==LB_SETTABSTOPS && (GetWindowLongPtrW(w,GWL_STYLE)&LBS_USETABSTOPS))result=services->set_tabs(w,s,a,b);
        else if(m==WM_SETREDRAW){s->list_redraw_off=!a;result=DefSubclassProc(w,m,a,b);}
        else if(m==WM_PAINT){PAINTSTRUCT ps;HDC dc=BeginPaint(w,&ps);services->present(w,s,dc,services->paint);EndPaint(w,&ps);}
        else if(m==WM_PRINTCLIENT)services->paint(w,s,(HDC)a);
        else if(m==WM_NCPAINT || m==WM_PRINT){
            /* This subclass owns the complete list nonclient surface. Native
             * painting first exposes a second scrollbar skin before ours. */
            if(m==WM_PRINT && (b&PRF_CLIENT)){
                HDC dc=(HDC)a;int saved=SaveDC(dc);RECT wr;POINT origin={0,0};
                if(saved){GetWindowRect(w,&wr);ClientToScreen(w,&origin);
                    OffsetViewportOrgEx(dc,origin.x-wr.left,origin.y-wr.top,NULL);
                    services->paint(w,s,dc);RestoreDC(dc,saved);}
            }
            if(!s->detached && (m==WM_NCPAINT || (b&PRF_NONCLIENT))){
                services->border(w,s,m==WM_PRINT?(HDC)a:NULL);
                services->scrollbars(w,s,m==WM_PRINT?(HDC)a:NULL);}
        }
        else if(m==WM_NCMOUSEMOVE || m==WM_NCMOUSELEAVE){
            /* Win3.x has no scrollbar hover skin. Do not start the host's
             * themed hover/leave repaint; clicks still use native tracking. */
            result=0;
        }
        else if(m==WM_ERASEBKGND)result=1;
        else {
            BOOL moving=m==WM_MOUSEMOVE,changed=FALSE;list_view_state before={0},after;
            BOOL suppress=style_control_redraw_can_suppress(w,s->list_redraw_off,s->list_suppression) &&
                (m==WM_MOUSEWHEEL || m==WM_VSCROLL || m==WM_HSCROLL || m==WM_KEYDOWN || m==WM_KEYUP ||
                m==WM_SETFOCUS || m==WM_KILLFOCUS || m==WM_ENABLE || m==WM_SETFONT ||
                m==WM_MOUSEMOVE || m==WM_LBUTTONDOWN || m==WM_LBUTTONUP ||
                m==LB_ADDSTRING || m==LB_INSERTSTRING || m==LB_DELETESTRING || m==LB_RESETCONTENT ||
                m==LB_SETCURSEL || m==LB_SETSEL || m==LB_SELITEMRANGE || m==LB_SELITEMRANGEEX ||
                m==LB_SETCARETINDEX || m==LB_SETTOPINDEX);
            if(moving)before=list_view(w);
            result=style_control_redraw_call(w,m,a,b,suppress,&s->list_suppression,&s->detached,&s->list_redraw_off);
            if(!s->detached && moving){after=list_view(w);
                changed=before.top!=after.top || before.selection!=after.selection || before.caret!=after.caret;}
            if(!s->detached && !s->list_suppression && !s->list_redraw_off && (m==LB_SETCURSEL || m==LB_SETSEL || m==LB_SELITEMRANGE || m==LB_SELITEMRANGEEX ||
                m==LB_SETCARETINDEX || m==LB_SETTOPINDEX || m==LB_ADDSTRING || m==LB_INSERTSTRING ||
                m==LB_DELETESTRING || m==LB_RESETCONTENT || m==LB_SETCOLUMNWIDTH || m==LB_SETHORIZONTALEXTENT || m==WM_SETFONT || m==WM_ENABLE ||
                m==WM_SETFOCUS || m==WM_KILLFOCUS || m==WM_LBUTTONDOWN || m==WM_LBUTTONUP ||
                changed || m==WM_KEYDOWN || m==WM_KEYUP || m==WM_VSCROLL || m==WM_HSCROLL || m==WM_MOUSEWHEEL)){
                RedrawWindow(w,NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW);
                if(!s->detached)services->scrollbars(w,s,NULL);}
        }
        return result;
}
