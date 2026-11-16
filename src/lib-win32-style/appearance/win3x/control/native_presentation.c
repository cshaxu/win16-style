#include "native_presentation.h"
#include "../common/font.h"
#include "tab_paint.h"
#include "header_paint.h"
#include "progress_paint.h"
#include <commctrl.h>

void win16_control_draw_tab(HWND w,control_style *s,HDC dc)
{
    HFONT font=s->state.font;win16_style_system_retain(font);
    if(!win16_style_tab_paint(w,dc,&s->palette,font,&s->instance.detached,&s->tab_epoch) && !s->instance.detached)
        DefSubclassProc(w,WM_PRINTCLIENT,(WPARAM)dc,PRF_CLIENT);
    win16_style_system_release(font);
}
void win16_control_draw_header(HWND w,control_style *s,HDC dc)
{
    HFONT font=s->state.font;win16_style_system_retain(font);
    if(!win16_style_header_paint(w,dc,&s->palette,font,&s->instance.detached,&s->header_epoch,s->header_pressed) && !s->instance.detached)
        DefSubclassProc(w,WM_PRINTCLIENT,(WPARAM)dc,PRF_CLIENT);
    win16_style_system_release(font);
}
void win16_control_draw_progress(HWND w,control_style *s,HDC dc)
{
    if(!win16_style_progress_paint(w,dc,&s->palette,&s->instance.detached,&s->progress_epoch) && !s->instance.detached)
        DefSubclassProc(w,WM_PRINTCLIENT,(WPARAM)dc,PRF_CLIENT);
}
