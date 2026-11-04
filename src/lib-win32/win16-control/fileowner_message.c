#include "fileowner_message.h"
#include "control_class.h"
#include <commctrl.h>

BOOL win16_control_common_dialog_combo_valid(HWND w,HWND parent)
{
    DWORD style=(DWORD)GetWindowLongPtrW(w,GWL_STYLE);
    int id=GetDlgCtrlID(w);
    return w && parent && GetParent(w)==parent && GetWindowThreadProcessId(w,NULL)==GetCurrentThreadId() &&
        ntw16s_class_is(w,L"COMBOBOX") && id>=1136 && id<=1140 && (style&CBS_OWNERDRAWFIXED) &&
        (style&CBS_HASSTRINGS) && !(style&CBS_OWNERDRAWVARIABLE);
}

LRESULT win16_control_fileowner_message(HWND w,control_style *s,UINT m,WPARAM a,LPARAM b,const win16_control_fileowner_services *services)
{
    LRESULT result=0;
        /* Runtime owner-draw remains authoritative. Overlay chrome only after
         * its native paint; never clear or recreate the image/text item. */
        if(ntw16s_vb_owner_file(w) && ntw16s_vb_directory(w) && services->scroll_input(w,s,m,a,b))result=0;
        else result=DefSubclassProc(w,m,a,b);
        if(!s->detached && (ntw16s_vb_owner_file(w) ||
                (s->common_dialog_combo && win16_control_common_dialog_combo_valid(w,s->common_dialog_parent)))){
            if(m==WM_PAINT){
                if(ntw16s_vb_drive(w) || s->common_dialog_combo){HDC dc=GetDC(w);if(dc){services->combo(w,s,dc);ReleaseDC(w,dc);}}
                else {services->border(w,s,NULL);services->scrollbars(w,s,NULL);}
            }else if(m==WM_PRINTCLIENT && (ntw16s_vb_drive(w) || s->common_dialog_combo))services->combo(w,s,(HDC)a);
            else if(m==WM_NCPAINT || (m==WM_PRINT && (b&PRF_NONCLIENT))){
                if(ntw16s_vb_directory(w)){services->border(w,s,m==WM_PRINT?(HDC)a:NULL);services->scrollbars(w,s,m==WM_PRINT?(HDC)a:NULL);}
            }
        }
        return result;
}
