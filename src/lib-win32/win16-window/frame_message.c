#include "frame_message.h"
#include "frame_layout.h"
#include "frame_view.h"
#include "../win16-common/present.h"
#include <commctrl.h>

/* Observed host non-client draw messages; payloads remain uninterpreted. */
#define STYLE_NCUAHDRAWCAPTION 0x00ae
#define STYLE_NCUAHDRAWFRAME   0x00af
#define STYLE_UAHDRAWMENU     0x0091
#define STYLE_UAHDRAWMENUITEM 0x0092
BOOL win16_control_frame_message(HWND w,style_window *s,UINT msg,WPARAM wp,LPARAM lp,const win16_control_frame_message_services *v,LRESULT *result)
{
    switch(msg){
    case WM_SHOWWINDOW:
        if(wp && s->pending_frame){
            RECT client;BOOL normal=!IsIconic(w)&&!IsZoomed(w);
            win16_control_client_screen(w,&client);s->pending_frame=FALSE;
            v->activate(w,s,&client,normal);
            if(s->detached)return TRUE;
        }
        return FALSE;
    case WM_CTLCOLORDLG:
    case WM_CTLCOLORSTATIC:
        if(!s->dialog)return FALSE;
        *result=win16_control_dialog_colors((HDC)wp,&s->palette);return TRUE;
    case WM_NCCALCSIZE: {
        RECT r=wp?((NCCALCSIZE_PARAMS *)lp)->rgrc[0]:*(RECT *)lp;win16_style_layout l;
        if(!v->measure(w,s,&r,&l))return FALSE;
        /* USER still updates scrollbar state before our geometry is applied. */
        DefSubclassProc(w,msg,wp,lp);
        if(s->detached)return TRUE;
        OffsetRect(&l.client,r.left,r.top);
        if(wp)((NCCALCSIZE_PARAMS *)lp)->rgrc[0]=l.client;else *(RECT *)lp=l.client;
        return TRUE;
    }
    case WM_NCPAINT:
        win16_control_frame_ncpaint(w,wp,s,v->paint_callback,v->buffered_callback);return TRUE;
    case WM_SETCURSOR:
        if(!win16_control_frame_set_cursor(w,wp,lp))return FALSE;
        *result=TRUE;return TRUE;
    case STYLE_NCUAHDRAWCAPTION:
    case STYLE_NCUAHDRAWFRAME:
        return !IsIconic(w);
    case STYLE_UAHDRAWMENU:
    case STYLE_UAHDRAWMENUITEM:
        return !IsIconic(w) && GetMenu(w)!=NULL;
    case WM_NCACTIVATE:
        s->active=(BOOL)wp;if(IsIconic(w))return FALSE;
        *result=TRUE;v->paint(w,s,NULL);return TRUE;
    case WM_NCHITTEST:
        return v->hit_test(w,lp,result);
    case WM_PRINT:
        if((lp&PRF_CHECKVISIBLE) && !IsWindowVisible(w))return TRUE;
        if(lp&PRF_NONCLIENT)v->paint(w,s,(HDC)wp);
        if(!s->detached)win16_control_print_client_and_children(w,&s->detached,(HDC)wp,lp);
        return TRUE;
    case WM_SETTEXT:
        *result=DefSubclassProc(w,msg,wp,lp);
        if(!s->detached)v->paint(w,s,NULL);
        return TRUE;
    case WM_HSCROLL:
    case WM_VSCROLL:
        *result=DefSubclassProc(w,msg,wp,lp);return TRUE;
    case WM_DPICHANGED:
        *result=DefSubclassProc(w,msg,wp,lp);
        if(!s->detached){v->refresh_font(w,s);win16_control_frame_changed(w);}return TRUE;
    case WM_SETTINGCHANGE:
    case WM_SYSCOLORCHANGE:
    case WM_THEMECHANGED:
        *result=DefSubclassProc(w,msg,wp,lp);
        if(!s->detached)v->paint(w,s,NULL);
        return TRUE;
    default:return FALSE;
    }
}
