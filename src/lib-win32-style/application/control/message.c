#include "message.h"
#include "control_class.h"
#include "control_vcl_spin.h"
LRESULT style_control_message_dispatch(HWND w,style_instance *instance,style_control_state *state,style_control_bindings *bindings,
    UINT m,WPARAM a,LPARAM b,const style_control_message_ops *ops)
{
    LRESULT result=0;HFONT pinned=state->font;style_control_message call={.message=m,.parameter=a,.value=b};
    style_instance_retain(instance);
    RECT spin_before={0};HWND spin_button=style_control_vcl_spin_before(w,m,&spin_before);
    call.tab=style_control_is_tab(w);call.header=style_control_is_header(w);call.progress=style_control_class_is(w,PROGRESS_CLASSW);
    call.native_font=call.tab || call.header;
    BOOL combo_paint=style_control_vb_combo(w) && (m==WM_PAINT || m==WM_PRINT || m==WM_PRINTCLIENT);
    if(combo_paint)++state->native_paints;
    if(call.progress)style_control_native_observe(&bindings->progress,STYLE_CONTROL_PROGRESS,m);
    if(call.tab)style_control_native_observe(&bindings->tab,STYLE_CONTROL_TAB,m);
    if(call.header)style_control_native_observe(&bindings->header,STYLE_CONTROL_HEADER,m);
    state->fonts->retain(pinned);
    if(style_control_guard_message(w,instance,state,m,a,b,&ops->guard,&result))goto finished;
    if(!ops->prepare_font(w,ops->context,&call))goto finished;
    result=ops->dispatch(w,ops->context,&call);
finished:
    style_control_message_finish(w,instance,state,call.message,call.value,spin_button,&spin_before,combo_paint,
        call.previous,call.outbound,pinned,ops->lookup);
    return result;
}
