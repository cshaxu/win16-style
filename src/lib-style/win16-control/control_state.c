#include "control_state.h"
#include "../win16-common/font.h"
#include "control_class.h"
#include "tab_paint.h"
#include "header_paint.h"
#include "progress_paint.h"
#include <commctrl.h>
#include <stdlib.h>
control_style *win16_control_state_create(void)
{return (control_style *)calloc(1,sizeof(control_style));}
void win16_control_state_dispose(control_style *s)
{
    for(unsigned i=0;i<s->combo_font_count;++i)win16_style_system_release(s->combo_fonts[i]);
    win16_style_system_release(s->font);
    style_control_protocol_release(s->protocol);
    free(s->tab_positions);free(s);
}
BOOL win16_control_state_prepare_font(HWND w,control_style *s,UINT m,WPARAM *a,BOOL native_font,HFONT *previous,HFONT *outbound)
{
    if(!s->frame_scrollbars && (m==WM_PAINT || m==WM_PRINT || m==WM_PRINTCLIENT) &&
            (native_font || ntw16s_vb_frame(w) || ntw16s_vb_edit(w) || ntw16s_vb_button(w) || ntw16s_vb_combo(w) || ntw16s_vb_list(w))){
        HFONT source=(HFONT)SendMessageW(w,WM_GETFONT,0,0);
        if(!s->detached && source!=s->original_font){
            HFONT replacement=native_font?win16_style_system_acquire(source):win16_style_vb_font_acquire(source);
            *previous=s->font;s->original_font=source;s->font=replacement?replacement:source;
        }
    }
    if(s->detached)return FALSE;
    if(m==WM_SETFONT && s->combo_passthrough){
        s->original_font=win16_style_system_source((HFONT)*a);*a=(WPARAM)s->original_font;
        if(((DWORD)GetWindowLongPtrW(w,GWL_STYLE)&3)==CBS_DROPDOWNLIST){
            HFONT replacement=win16_style_system_acquire(s->original_font);*previous=s->font;s->font=replacement?replacement:s->original_font;
        }
    }
    if(m==WM_SETFONT && !s->frame_scrollbars && !s->combo_passthrough){
        HFONT original=win16_style_system_source((HFONT)*a);
        BOOL vb=ntw16s_vb_frame(w) || ntw16s_vb_edit(w) || ntw16s_vb_button(w) || ntw16s_vb_combo(w) || ntw16s_vb_list(w);
        HFONT replacement=vb?win16_style_vb_font_acquire(original):win16_style_system_acquire(original);
        *previous=s->font;s->original_font=original;
        s->font=replacement?replacement:original;
        *outbound=s->font;win16_style_system_retain(*outbound);
        /* Native tab geometry and VB checklist retain the application's font. */
        *a=(WPARAM)((native_font || ntw16s_vb_checklist_owner(w))?original:s->font);
    }
    return TRUE;
}
/* WM_SETREDRAW(TRUE) sets WS_VISIBLE; don't bracket an application-hidden control. */
BOOL win16_control_state_can_suppress_redraw(HWND w,const control_style *s)
{return !s->list_redraw_off && (s->list_suppression || (GetWindowLongPtrW(w,GWL_STYLE)&WS_VISIBLE));}

LRESULT win16_control_state_call_native(HWND w,control_style *s,UINT m,WPARAM a,LPARAM b,BOOL suppress)
{
    if(suppress && s->list_suppression++==0)DefSubclassProc(w,WM_SETREDRAW,FALSE,0);
    LRESULT result=DefSubclassProc(w,m,a,b);
    if(suppress && --s->list_suppression==0 && !s->detached && !s->list_redraw_off)
        DefSubclassProc(w,WM_SETREDRAW,TRUE,0);
    return result;
}

LRESULT win16_control_combo_message(HWND w,control_style *s,UINT m,WPARAM a,LPARAM b,win16_control_state_present present,win16_control_state_painter paint)
{
    LRESULT result=0;
    if(m==WM_SETREDRAW){s->list_redraw_off=!a;result=DefSubclassProc(w,m,a,b);}
    else if(m==WM_PAINT){PAINTSTRUCT ps;HDC dc=BeginPaint(w,&ps);present(w,s,dc,paint);EndPaint(w,&ps);}
    else if(m==WM_PRINTCLIENT)paint(w,s,(HDC)a);
    else if(m==WM_ERASEBKGND)result=1;
    else {BOOL affects=m==CB_SETCURSEL || m==CB_SHOWDROPDOWN || m==WM_COMMAND || m==WM_ENABLE ||
            m==WM_SETFONT || m==WM_SETFOCUS || m==WM_KILLFOCUS || m==WM_KEYDOWN ||
            m==WM_LBUTTONDOWN || m==WM_LBUTTONUP;
        BOOL suppress=win16_control_state_can_suppress_redraw(w,s) && affects;
        result=win16_control_state_call_native(w,s,m,a,b,suppress);
        if(!s->detached && !s->list_suppression && !s->list_redraw_off && affects)
            RedrawWindow(w,NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW);}
    return result;
}
LRESULT win16_control_tab_message(HWND w,control_style *s,UINT m,WPARAM a,LPARAM b,win16_control_state_present present,win16_control_state_painter paint)
{
    LRESULT result=0;
    if(m==WM_PAINT && win16_style_tab_supported(w)){
        PAINTSTRUCT ps;HDC dc=BeginPaint(w,&ps);present(w,s,dc,paint);EndPaint(w,&ps);
    }else if(m==WM_PRINTCLIENT && win16_style_tab_supported(w))paint(w,s,(HDC)a);
    else result=DefSubclassProc(w,m,a,b);
    if(!s->detached && win16_style_tab_changes(m))RedrawWindow(w,NULL,NULL,RDW_INVALIDATE|(s->calls==1?RDW_UPDATENOW:0));
    return result;
}
LRESULT win16_control_header_message(HWND w,control_style *s,UINT m,WPARAM a,LPARAM b,win16_control_state_present present,win16_control_state_painter paint)
{
    LRESULT result=0;
    if(m==WM_LBUTTONDOWN){
        HDHITTESTINFO hit={.pt={(short)LOWORD(b),(short)HIWORD(b)}};
        LRESULT item=SendMessageW(w,HDM_HITTEST,0,(LPARAM)&hit);
        if(s->detached)return 0;
        s->header_pressed=(item>=0 && (hit.flags&HHT_ONHEADER))?(int)item:-1;
    }
    if(m==WM_LBUTTONUP || m==WM_CANCELMODE || m==WM_CAPTURECHANGED)s->header_pressed=-1;
    if(m==WM_PAINT && win16_style_header_supported(w)){
        PAINTSTRUCT ps;HDC dc=BeginPaint(w,&ps);present(w,s,dc,paint);EndPaint(w,&ps);
    }else if(m==WM_PRINTCLIENT && win16_style_header_supported(w))paint(w,s,(HDC)a);
    else result=DefSubclassProc(w,m,a,b);
    if(!s->detached && win16_style_header_changes(m))RedrawWindow(w,NULL,NULL,RDW_INVALIDATE|(s->calls==1?RDW_UPDATENOW:0));
    return result;
}
LRESULT win16_control_progress_message(HWND w,control_style *s,UINT m,WPARAM a,LPARAM b,win16_control_state_present present,win16_control_state_painter paint)
{
    LRESULT result=0;BOOL classic=win16_style_progress_supported(w);
    if(m==WM_PAINT && classic){PAINTSTRUCT ps;HDC dc=BeginPaint(w,&ps);present(w,s,dc,paint);EndPaint(w,&ps);}
    else if(m==WM_PRINTCLIENT && classic)paint(w,s,(HDC)a);
    else if(m==WM_PRINT && classic && (b&PRF_CLIENT)){
        int saved=SaveDC((HDC)a);if(saved){result=DefSubclassProc(w,m,a,b&~PRF_CLIENT);RestoreDC((HDC)a,saved);}
        if(!s->detached)paint(w,s,(HDC)a);
    }else result=DefSubclassProc(w,m,a,b);
    if(!s->detached && win16_style_progress_changes(m))RedrawWindow(w,NULL,NULL,RDW_INVALIDATE|(s->calls==1?RDW_UPDATENOW:0));
    return result;
}
