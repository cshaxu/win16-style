#include "dialog_combo.h"
#include "control_native_combo.h"
#include "control_class.h"
static _Thread_local HWND pending;
HWND style_control_dialog_combo_pending(void){return pending;}
BOOL style_control_dialog_combo_install(HWND window,const void *owner,const style_dialog_combo_ops *ops)
{
    HWND previous=pending;
    if(!style_control_dialog_combo_valid(window,GetParent(window),style_control_is_combo)){
        SetLastError(ERROR_NOT_SUPPORTED);return FALSE;}
    pending=window;
    BOOL result=ops->apply(window,STYLE_DIALOG_COMBO,FALSE,owner);
    pending=previous;
    if(result && IsWindow(window)){
        COMBOBOXINFO info={.cbSize=sizeof(info)};
        if(ops->get_info(window,&info)){
            if(info.hwndList)(void)ops->apply(info.hwndList,STYLE_DIALOG_COMBO_LIST,FALSE,owner);
            if(info.hwndItem && info.hwndItem!=window && style_control_is_edit(info.hwndItem))
                (void)ops->apply(info.hwndItem,STYLE_DIALOG_COMBO_EDIT,
                    (GetWindowLongPtrW(info.hwndItem,GWL_STYLE)&WS_BORDER)!=0,owner);
        }
    }
    return result;
}
