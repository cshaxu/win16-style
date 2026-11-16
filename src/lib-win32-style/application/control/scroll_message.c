#include "scroll_message.h"
#include "control_protocol.h"
#include <commctrl.h>

LRESULT style_scroll_frame_message(HWND w,style_instance *instance,const style_control_state *owner,style_scroll_controller *scroll,UINT m,WPARAM a,LPARAM b,const style_scroll_ops *ops,const style_scroll_frame_ops *frame)
{
    LRESULT result=0;
        if(style_scroll_embedded_input(w,instance,owner,scroll,ops,m,a,b))result=0;
        else if(frame->suppress_hover && m==WM_NCMOUSEMOVE && (a==HTHSCROLL || a==HTVSCROLL))result=0;
        else {if(m==WM_ENABLE && !a)style_scroll_finish(w,instance,scroll,ops,TRUE);
            int saved=(scroll->resources.native_border && m==WM_PRINT)?SaveDC((HDC)a):0;
            if(!instance->detached)result=DefSubclassProc(w,m,a,b);
            if(saved)RestoreDC((HDC)a,saved);}
        if(!instance->detached && scroll->resources.native_border && (m==WM_NCPAINT || (m==WM_PRINT && (b&PRF_NONCLIENT)))){
            frame->border(w,frame->context,m==WM_PRINT?(HDC)a:NULL);ops->nonclient(w,ops->context,m==WM_PRINT?(HDC)a:NULL);
        }
        if(!instance->detached && frame->overlay && (m==WM_NCPAINT || (m==WM_PRINT && (b&PRF_NONCLIENT))))
            frame->overlay(w,frame->context,m==WM_PRINT?(HDC)a:NULL);
        /* Native controls can repaint their own non-client bars while handling
         * input or position changes without issuing WM_NCPAINT. Restore both
         * axes after the native handler, including programmatic list scrolling. */
        if(!instance->detached && (m==WM_VSCROLL || m==WM_HSCROLL || m==WM_MOUSEWHEEL || m==WM_MOUSEHWHEEL ||
            m==WM_KEYDOWN || m==WM_KEYUP || m==WM_LBUTTONUP || m==WM_ENABLE || m==WM_SIZE ||
            m==LB_SETTOPINDEX || m==LB_SETCURSEL || m==LB_SETSEL || m==EM_LINESCROLL))
            ops->nonclient(w,ops->context,NULL);
        return result;
}
LRESULT style_scroll_sizebox_message(HWND w,UINT m,WPARAM a,LPARAM b,const style_control_draw *draw)
{
    LRESULT result=0;
        if(m==WM_PAINT){PAINTSTRUCT ps;HDC dc=BeginPaint(w,&ps);draw->render(w,draw->context,dc,TRUE);EndPaint(w,&ps);}
        else if(m==WM_PRINTCLIENT)draw->render(w,draw->context,(HDC)a,FALSE);
        else if(m==WM_ERASEBKGND)result=1;
        else result=DefSubclassProc(w,m,a,b);
        return result;
}
LRESULT style_scroll_control_message(HWND w,style_instance *instance,style_scroll_controller *scroll,UINT *message,WPARAM *wparam,LPARAM *lparam,const style_scroll_ops *ops)
{
    UINT m=*message;WPARAM a=*wparam;LPARAM b=*lparam;LRESULT result=0;
        if(style_control_protocol_is(w,L"SCROLLBAR") && !IsWindowEnabled(w) && (m==WM_KEYDOWN || m==WM_KEYUP || m==WM_LBUTTONDOWN || m==WM_LBUTTONDBLCLK || m==WM_LBUTTONUP))result=DefSubclassProc(w,m,a,b);
        else if(style_scroll_control_input(w,instance,scroll,ops,m,a,b))result=0;
        else if(m==WM_SETFOCUS || m==WM_SIZE)ops->caret(w,ops->context);
        else if(m==WM_KILLFOCUS)style_scroll_caret_destroy(w,&scroll->resources);
        else if(m==SBM_GETSCROLLINFO){SCROLLINFO *info=(SCROLLINFO *)b;
            result=DefSubclassProc(w,m,a,b);
            if(result && info && info->cbSize==sizeof(*info) && (info->fMask&SIF_TRACKPOS)){
                if(scroll->reporting)info->nTrackPos=scroll->report;
                else if(scroll->track.active)info->nTrackPos=scroll->track.position;}}
        else if(m==WM_PAINT){PAINTSTRUCT ps;HDC dc=BeginPaint(w,&ps);ops->draw(w,ops->context,dc,OBJID_CLIENT,(GetWindowLongPtrW(w,GWL_STYLE)&SBS_VERT)!=0);EndPaint(w,&ps);ops->caret(w,ops->context);}
        else if(m==WM_PRINTCLIENT)ops->draw(w,ops->context,(HDC)a,OBJID_CLIENT,(GetWindowLongPtrW(w,GWL_STYLE)&SBS_VERT)!=0);
        else if(m==WM_ERASEBKGND)result=1;
        else {BOOL redraw=m==SBM_SETPOS?b!=0:m==SBM_SETSCROLLINFO?a!=0:m!=SBM_SETRANGE;
            if(m==WM_ENABLE && !a)style_scroll_finish(w,instance,scroll,ops,TRUE);
            if(instance->detached)goto done;
            if(m==SBM_SETPOS)b=FALSE;
            if(m==SBM_SETSCROLLINFO)a=FALSE;
            if(m==SBM_SETRANGEREDRAW)m=SBM_SETRANGE;
            result=DefSubclassProc(w,m,a,b);
            if(redraw && !instance->detached && (m==SBM_SETPOS || m==SBM_SETSCROLLINFO || m==SBM_SETRANGE || m==SBM_SETRANGEREDRAW ||
                m==SBM_ENABLE_ARROWS || m==WM_ENABLE || m==WM_SETFOCUS || m==WM_KILLFOCUS ||
                m==WM_LBUTTONDOWN || m==WM_LBUTTONUP || m==WM_KEYDOWN))RedrawWindow(w,NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW);
            if(redraw)ops->caret(w,ops->context);}
    done:
    *message=m;*wparam=a;*lparam=b;return result;
}
