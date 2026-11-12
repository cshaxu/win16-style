#include "native_combo_identity.h"
#include "combo_paint.h"
#include <oleacc.h>

/* The application owns owner-drawn swatches and text. Only the documented
 * native button rectangle is eligible for post-paint chrome. */
void win16_control_color_button(HWND w,const style_control_native_combo_view *v,HDC dc,const RECT *dirty)
{
    COMBOBOXINFO info={.cbSize=sizeof(info)};
    if(!style_control_color_identity(w,v,&info))return;
    BOOL pressed=SendMessageW(w,CB_GETDROPPEDSTATE,0,0)!=0;
    if(!*v->detached && IsWindow(w) && GetParent(w)==*v->parent)
        win16_control_native_combo_button(w,v->detached,dc,dirty,info.rcButton,pressed);
}
void win16_control_date_button(HWND w,const style_control_native_combo_view *v,HDC dc,const RECT *dirty)
{
    DATETIMEPICKERINFO info={.cbSize=sizeof(info)};
    if(style_control_date_identity(w,v,&info) && !*v->detached && GetParent(w)==*v->parent)
        win16_control_native_combo_button(w,v->detached,dc,dirty,info.rcButton,(info.stateButton&STATE_SYSTEM_PRESSED)!=0);
}
