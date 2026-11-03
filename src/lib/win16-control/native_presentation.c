#include "native_presentation.h"
#include "../win16-common/font.h"
#include "tab_paint.h"
#include "header_paint.h"
#include "progress_paint.h"
#include <commctrl.h>

void win16_control_nonclient_scrollbars(HWND w,control_style *s,HDC target,win16_control_scroll_painter draw_scroll)
{
    DWORD style=(DWORD)GetWindowLongPtrW(w,GWL_STYLE);HDC dc;
    if(!(style&(WS_HSCROLL|WS_VSCROLL)))return;
    dc=target?target:GetWindowDC(w);if(!dc)return;
    if(style&WS_HSCROLL)draw_scroll(w,s,dc,OBJID_HSCROLL,FALSE);
    if(style&WS_VSCROLL)draw_scroll(w,s,dc,OBJID_VSCROLL,TRUE);
    if(!target)ReleaseDC(w,dc);
}


void win16_control_draw_tab(HWND w,control_style *s,HDC dc)
{
    HFONT font=s->font;win16_style_system_retain(font);
    if(!win16_style_tab_paint(w,dc,&s->palette,font,&s->detached,&s->tab_epoch) && !s->detached)
        DefSubclassProc(w,WM_PRINTCLIENT,(WPARAM)dc,PRF_CLIENT);
    win16_style_system_release(font);
}
void win16_control_draw_header(HWND w,control_style *s,HDC dc)
{
    HFONT font=s->font;win16_style_system_retain(font);
    if(!win16_style_header_paint(w,dc,&s->palette,font,&s->detached,&s->header_epoch,s->header_pressed) && !s->detached)
        DefSubclassProc(w,WM_PRINTCLIENT,(WPARAM)dc,PRF_CLIENT);
    win16_style_system_release(font);
}
void win16_control_draw_progress(HWND w,control_style *s,HDC dc)
{
    if(!win16_style_progress_paint(w,dc,&s->palette,&s->detached,&s->progress_epoch) && !s->detached)
        DefSubclassProc(w,WM_PRINTCLIENT,(WPARAM)dc,PRF_CLIENT);
}
