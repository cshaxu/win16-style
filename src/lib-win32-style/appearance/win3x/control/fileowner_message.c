#include "fileowner_message.h"
#include "control_class.h"
#include <commctrl.h>

LRESULT win16_control_fileowner_message(HWND w,control_style *s,UINT m,WPARAM a,LPARAM b,const win16_control_fileowner_services *services)
{
    LRESULT result=0;
        /* Runtime owner-draw remains authoritative. Overlay chrome only after
         * its native paint; never clear or recreate the image/text item. */
        if(ntw16s_vb_owner_file(w) && style_control_vb_directory(w) && services->scroll_input(w,s,m,a,b))result=0;
        else result=DefSubclassProc(w,m,a,b);
        if(!s->detached && (ntw16s_vb_owner_file(w) ||
                (s->common_dialog_combo && style_control_dialog_combo_valid(w,s->common_dialog_parent,style_control_is_combo)))){
            if(m==WM_PAINT){
                if(style_control_vb_drive(w) || s->common_dialog_combo){HDC dc=GetDC(w);if(dc){services->combo(w,s,dc);ReleaseDC(w,dc);}}
                else {services->border(w,s,NULL);services->scrollbars(w,s,NULL);}
            }else if(m==WM_PRINTCLIENT && (style_control_vb_drive(w) || s->common_dialog_combo))services->combo(w,s,(HDC)a);
            else if(m==WM_NCPAINT || (m==WM_PRINT && (b&PRF_NONCLIENT))){
                if(style_control_vb_directory(w)){services->border(w,s,m==WM_PRINT?(HDC)a:NULL);services->scrollbars(w,s,m==WM_PRINT?(HDC)a:NULL);}
            }
        }
        return result;
}
