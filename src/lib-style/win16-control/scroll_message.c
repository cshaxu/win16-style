#include "scroll_message.h"
#include "special_paint.h"
#include "../common/control_protocol.h"
#include <commctrl.h>

LRESULT win16_control_frame_scroll_message(HWND w,control_style *s,UINT m,WPARAM a,LPARAM b,const win16_control_scroll_services *services)
{
    LRESULT result=0;
        if(services->embedded_input(w,s,m,a,b))result=0;
        else if(m==WM_NCMOUSEMOVE && (a==HTHSCROLL || a==HTVSCROLL))result=0;
        else {if(m==WM_ENABLE && !a)services->finish(w,s,TRUE);
            int saved=(s->native_border && m==WM_PRINT)?SaveDC((HDC)a):0;
            if(!s->detached)result=DefSubclassProc(w,m,a,b);
            if(saved)RestoreDC((HDC)a,saved);}
        if(!s->detached && s->native_border && (m==WM_NCPAINT || (m==WM_PRINT && (b&PRF_NONCLIENT)))){
            services->border(w,s,m==WM_PRINT?(HDC)a:NULL);services->scrollbars(w,s,m==WM_PRINT?(HDC)a:NULL);
        }
        if(!s->detached && s->legacy_progress_border && (m==WM_NCPAINT || (m==WM_PRINT && (b&PRF_NONCLIENT))))
            win16_control_legacy_progress_border(w,&s->palette,m==WM_PRINT?(HDC)a:NULL);
        /* Native controls can repaint their own non-client bars while handling
         * input or position changes without issuing WM_NCPAINT. Restore both
         * axes after the native handler, including programmatic list scrolling. */
        if(!s->detached && (m==WM_VSCROLL || m==WM_HSCROLL || m==WM_MOUSEWHEEL || m==WM_MOUSEHWHEEL ||
            m==WM_KEYDOWN || m==WM_KEYUP || m==WM_LBUTTONUP || m==WM_ENABLE || m==WM_SIZE ||
            m==LB_SETTOPINDEX || m==LB_SETCURSEL || m==LB_SETSEL || m==EM_LINESCROLL))
            services->scrollbars(w,s,NULL);
        return result;
}
LRESULT win16_control_sizebox_message(HWND w,control_style *s,UINT m,WPARAM a,LPARAM b,const win16_control_scroll_services *services)
{
    LRESULT result=0;
        if(m==WM_PAINT){PAINTSTRUCT ps;HDC dc=BeginPaint(w,&ps);services->present(w,s,dc,services->sizebox);EndPaint(w,&ps);}
        else if(m==WM_PRINTCLIENT)win16_control_draw_sizebox(w,(HDC)a);
        else if(m==WM_ERASEBKGND)result=1;
        else result=DefSubclassProc(w,m,a,b);
        return result;
}
LRESULT win16_control_scroll_message(HWND w,control_style *s,UINT *message,WPARAM *wparam,LPARAM *lparam,const win16_control_scroll_services *services)
{
    UINT m=*message;WPARAM a=*wparam;LPARAM b=*lparam;LRESULT result=0;
        if(style_control_protocol_is(w,L"SCROLLBAR") && !IsWindowEnabled(w) && (m==WM_KEYDOWN || m==WM_KEYUP || m==WM_LBUTTONDOWN || m==WM_LBUTTONDBLCLK || m==WM_LBUTTONUP))result=DefSubclassProc(w,m,a,b);
        else if(services->input(w,s,m,a,b))result=0;
        else if(m==WM_SETFOCUS || m==WM_SIZE)services->caret_sync(w,s);
        else if(m==WM_KILLFOCUS)services->caret_destroy(w,s);
        else if(m==SBM_GETSCROLLINFO){SCROLLINFO *info=(SCROLLINFO *)b;
            result=DefSubclassProc(w,m,a,b);
            if(result && info && info->cbSize==sizeof(*info) && (info->fMask&SIF_TRACKPOS)){
                if(s->scroll_reporting)info->nTrackPos=s->scroll_report;
                else if(s->scroll.active)info->nTrackPos=s->scroll.position;}}
        else if(m==WM_PAINT){PAINTSTRUCT ps;HDC dc=BeginPaint(w,&ps);services->draw(w,s,dc,OBJID_CLIENT,(GetWindowLongPtrW(w,GWL_STYLE)&SBS_VERT)!=0);EndPaint(w,&ps);services->caret_sync(w,s);}
        else if(m==WM_PRINTCLIENT)services->draw(w,s,(HDC)a,OBJID_CLIENT,(GetWindowLongPtrW(w,GWL_STYLE)&SBS_VERT)!=0);
        else if(m==WM_ERASEBKGND)result=1;
        else {BOOL redraw=m==SBM_SETPOS?b!=0:m==SBM_SETSCROLLINFO?a!=0:m!=SBM_SETRANGE;
            if(m==WM_ENABLE && !a)services->finish(w,s,TRUE);
            if(s->detached)goto done;
            if(m==SBM_SETPOS)b=FALSE;
            if(m==SBM_SETSCROLLINFO)a=FALSE;
            if(m==SBM_SETRANGEREDRAW)m=SBM_SETRANGE;
            result=DefSubclassProc(w,m,a,b);
            if(redraw && !s->detached && (m==SBM_SETPOS || m==SBM_SETSCROLLINFO || m==SBM_SETRANGE || m==SBM_SETRANGEREDRAW ||
                m==SBM_ENABLE_ARROWS || m==WM_ENABLE || m==WM_SETFOCUS || m==WM_KILLFOCUS ||
                m==WM_LBUTTONDOWN || m==WM_LBUTTONUP || m==WM_KEYDOWN))RedrawWindow(w,NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW);
            if(redraw)services->caret_sync(w,s);}
    done:
    *message=m;*wparam=a;*lparam=b;return result;
}
