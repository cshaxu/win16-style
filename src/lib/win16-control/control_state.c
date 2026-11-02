#include "control_state.h"
#include "font.h"
#include "control_class.h"
#include <stdlib.h>
control_style *win16_control_state_create(void)
{return (control_style *)calloc(1,sizeof(control_style));}
void win16_control_state_dispose(control_style *s)
{
    for(unsigned i=0;i<s->combo_font_count;++i)win16_style_system_release(s->combo_fonts[i]);
    win16_style_system_release(s->font);
    ntw16s_control_protocol_release(s->protocol);
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
