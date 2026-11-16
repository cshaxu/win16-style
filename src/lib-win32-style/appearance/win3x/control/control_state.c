#include "lib-win32-style/application/control/control_redraw.h"
#include "control_state.h"
#include "../common/font.h"
#include "control_class.h"
#include "tab_paint.h"
#include "header_paint.h"
#include "progress_paint.h"
#include <commctrl.h>
#include <stdlib.h>
static HFONT acquire_font(HWND w,HFONT source,BOOL native)
{
    BOOL vb=!native && (style_control_vb_frame(w) || style_control_vb_edit(w) || style_control_vb_button(w) || style_control_vb_combo(w) || style_control_vb_list(w));
    return vb?win16_style_vb_font_acquire(source):win16_style_system_acquire(source);
}
static const style_control_fonts font_operations={win16_style_system_source,acquire_font,win16_style_system_retain,win16_style_system_release};
control_style *win16_control_state_create(void)
{
    control_style *s=calloc(1,sizeof(*s));if(!s)return NULL;
    style_instance_init(&s->instance,s,win16_control_state_dispose);style_control_state_init(&s->state,&font_operations);return s;
}
void win16_control_state_dispose(void *context)
{
    control_style *s=context;
    style_control_combo_dispose(&s->combo,&s->state);
    style_control_state_dispose(&s->state);free(s);
}
BOOL win16_control_state_prepare_font(HWND w,control_style *s,UINT m,WPARAM *a,BOOL native_font,HFONT *previous,HFONT *outbound)
{
    style_control_font_mode mode=STYLE_CONTROL_FONT_NONE;
    if(!s->scroll.resources.frame && (m==WM_PAINT || m==WM_PRINT || m==WM_PRINTCLIENT) &&
        (native_font || style_control_vb_frame(w) || style_control_vb_edit(w) || style_control_vb_button(w) || style_control_vb_combo(w) || style_control_vb_list(w)))mode=STYLE_CONTROL_FONT_OBSERVE;
    if(m==WM_SETFONT && s->combo_passthrough)
        mode=((DWORD)GetWindowLongPtrW(w,GWL_STYLE)&3)==CBS_DROPDOWNLIST?STYLE_CONTROL_FONT_CACHE:STYLE_CONTROL_FONT_NORMALIZE;
    if(m==WM_SETFONT && !s->scroll.resources.frame && !s->combo_passthrough)mode=STYLE_CONTROL_FONT_REPLACE;
    return style_control_font_message(w,&s->instance,&s->state,a,mode,native_font,
        mode==STYLE_CONTROL_FONT_REPLACE && (native_font || ntw16s_vb_checklist_owner(w)),previous,outbound);
}
LRESULT win16_control_tab_message(HWND w,control_style *s,UINT m,WPARAM a,LPARAM b,win16_control_state_present present,win16_control_state_painter paint)
{
    LRESULT result=0;
    if(m==WM_PAINT && win16_style_tab_supported(w)){
        PAINTSTRUCT ps;HDC dc=BeginPaint(w,&ps);present(w,s,dc,paint);EndPaint(w,&ps);
    }else if(m==WM_PRINTCLIENT && win16_style_tab_supported(w))paint(w,s,(HDC)a);
    else result=DefSubclassProc(w,m,a,b);
    if(!s->instance.detached && win16_style_tab_changes(m))RedrawWindow(w,NULL,NULL,RDW_INVALIDATE|(s->instance.calls==1?RDW_UPDATENOW:0));
    return result;
}
LRESULT win16_control_header_message(HWND w,control_style *s,UINT m,WPARAM a,LPARAM b,win16_control_state_present present,win16_control_state_painter paint)
{
    LRESULT result=0;
    if(m==WM_LBUTTONDOWN){
        HDHITTESTINFO hit={.pt={(short)LOWORD(b),(short)HIWORD(b)}};
        LRESULT item=SendMessageW(w,HDM_HITTEST,0,(LPARAM)&hit);
        if(s->instance.detached)return 0;
        s->header_pressed=(item>=0 && (hit.flags&HHT_ONHEADER))?(int)item:-1;
    }
    if(m==WM_LBUTTONUP || m==WM_CANCELMODE || m==WM_CAPTURECHANGED)s->header_pressed=-1;
    if(m==WM_PAINT && win16_style_header_supported(w)){
        PAINTSTRUCT ps;HDC dc=BeginPaint(w,&ps);present(w,s,dc,paint);EndPaint(w,&ps);
    }else if(m==WM_PRINTCLIENT && win16_style_header_supported(w))paint(w,s,(HDC)a);
    else result=DefSubclassProc(w,m,a,b);
    if(!s->instance.detached && win16_style_header_changes(m))RedrawWindow(w,NULL,NULL,RDW_INVALIDATE|(s->instance.calls==1?RDW_UPDATENOW:0));
    return result;
}
LRESULT win16_control_progress_message(HWND w,control_style *s,UINT m,WPARAM a,LPARAM b,win16_control_state_present present,win16_control_state_painter paint)
{
    LRESULT result=0;BOOL classic=win16_style_progress_supported(w);
    if(m==WM_PAINT && classic){PAINTSTRUCT ps;HDC dc=BeginPaint(w,&ps);present(w,s,dc,paint);EndPaint(w,&ps);}
    else if(m==WM_PRINTCLIENT && classic)paint(w,s,(HDC)a);
    else if(m==WM_PRINT && classic && (b&PRF_CLIENT)){
        int saved=SaveDC((HDC)a);if(saved){result=DefSubclassProc(w,m,a,b&~PRF_CLIENT);RestoreDC((HDC)a,saved);}
        if(!s->instance.detached)paint(w,s,(HDC)a);
    }else result=DefSubclassProc(w,m,a,b);
    if(!s->instance.detached && win16_style_progress_changes(m))RedrawWindow(w,NULL,NULL,RDW_INVALIDATE|(s->instance.calls==1?RDW_UPDATENOW:0));
    return result;
}
