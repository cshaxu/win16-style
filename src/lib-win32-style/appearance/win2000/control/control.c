#include "control.h"
#include "button_paint.h"
#include "static_paint.h"
#include "edit_paint.h"
#include "list_combo.h"
#include "scroll.h"
#include "tab_header.h"
#include "lib-win32-style/application/control/combo.h"
#include "lib-win32-style/application/control/list.h"
#include "lib-win32-style/application/control/edit.h"
#include "lib-win32-style/application/control/notify.h"
#include "lib-win32-style/application/control/basic.h"
#include "lib-win32-style/application/control/retire.h"
#include "lib-win32-style/application/graphics/present.h"
#include "lib-win32-style/application/lifecycle/install.h"
#include "lib-win32-style/application/lifecycle/dispatch.h"
#include "lib-win32-style/application/window/window_identity.h"
#include <stdlib.h>

typedef struct win2k_control {
    style_instance instance;
    style_control_state state;
    style_native_theme_fn native_theme;
    BOOL frame_owned;
    style_control_notify_state parent_binding;
    style_control_combo combo;
    HWND combo_parent;
    win2k_scroll scroll;
    style_control_native_state tab;
    BOOL tab_control,header_control;
} win2k_control;
static LRESULT CALLBACK control_proc(HWND,UINT,WPARAM,LPARAM,UINT_PTR,DWORD_PTR);
static LRESULT CALLBACK parent_proc(HWND,UINT,WPARAM,LPARAM,UINT_PTR,DWORD_PTR);
static win2k_control *lookup(HWND window)
{return (win2k_control *)style_instance_lookup(window,control_proc);}
static BOOL attach(HWND,style_native_theme_fn,HWND);
static BOOL class_is(HWND window,const WCHAR *expected)
{
    WCHAR name[32];
    return GetClassNameW(window,name,ARRAYSIZE(name)) && !lstrcmpiW(name,expected);
}
static style_control_state *lookup_state(HWND window)
{
    win2k_control *control=lookup(window);
    return control?&control->state:NULL;
}
static BOOL list_style(DWORD style)
{return !(style&(LBS_OWNERDRAWFIXED|LBS_OWNERDRAWVARIABLE|LBS_NODATA));}
static BOOL static_class(HWND window)
{WCHAR name[32];return GetClassNameW(window,name,ARRAYSIZE(name)) && !lstrcmpiW(name,L"STATIC");}
static style_control_basic_kind basic_kind(HWND window)
{
    if(!static_class(window))return STYLE_CONTROL_BUTTON;
    UINT type=(UINT)GetWindowLongPtrW(window,GWL_STYLE)&SS_TYPEMASK;
    if(type==SS_ICON || type==SS_BITMAP)return STYLE_CONTROL_BITMAP;
    if(type>=SS_ETCHEDHORZ && type<=SS_ETCHEDFRAME)return STYLE_CONTROL_SEPARATOR;
    return STYLE_CONTROL_STATIC;
}
BOOL win2k_control_supported(HWND window)
{
    WCHAR name[32];DWORD style=(DWORD)GetWindowLongPtrW(window,GWL_STYLE);
    if(!(style&WS_CHILD) || (GetWindowLongPtrW(window,GWL_EXSTYLE)&(WS_EX_LAYOUTRTL|WS_EX_RTLREADING)) ||
       !GetClassNameW(window,name,ARRAYSIZE(name)))return FALSE;
    if(!lstrcmpiW(name,L"LISTBOX"))return list_style(style);
    if(!lstrcmpiW(name,WC_TABCONTROLW))return win2k_tab_supported(window);
    if(!lstrcmpiW(name,WC_HEADERW))return win2k_header_supported(window);
    if(!lstrcmpiW(name,L"SCROLLBAR"))return TRUE;
    if(!lstrcmpiW(name,L"COMBOBOX"))
        return (style&3) && !(style&(CBS_OWNERDRAWFIXED|CBS_OWNERDRAWVARIABLE));
    if(!lstrcmpiW(name,L"STATIC")){
        UINT type=style&SS_TYPEMASK;
        return type==SS_LEFT || type==SS_CENTER || type==SS_RIGHT || type==SS_SIMPLE || type==SS_LEFTNOWORDWRAP ||
            type==SS_ICON || type==SS_BITMAP || (type>=SS_BLACKRECT && type<=SS_WHITEFRAME) ||
            ((type>=SS_ETCHEDHORZ && type<=SS_ETCHEDFRAME) && !(style&SS_SUNKEN));
    }
    win2k_edit_kind edit=win2k_edit_class(window);
    if(edit!=WIN2K_EDIT_NONE){
        DWORD ex=(DWORD)GetWindowLongPtrW(window,GWL_EXSTYLE);
        if((style&WS_CAPTION)==WS_CAPTION || (style&WS_THICKFRAME) || (ex&WS_EX_DLGMODALFRAME))return FALSE;
        return edit==WIN2K_EDIT_STANDARD || (style&(WS_BORDER|WS_DLGFRAME)) ||
            (ex&(WS_EX_CLIENTEDGE|WS_EX_WINDOWEDGE|WS_EX_STATICEDGE));
    }
    if(lstrcmpiW(name,L"BUTTON"))return FALSE;
    UINT type=style&BS_TYPEMASK;
    if(type==BS_GROUPBOX)return !(style&(BS_ICON|BS_BITMAP|BS_PUSHLIKE|BS_MULTILINE));
    BOOL push=type<=BS_DEFPUSHBUTTON || (style&BS_PUSHLIKE);
    BOOL choice=type==BS_CHECKBOX || type==BS_AUTOCHECKBOX || type==BS_RADIOBUTTON ||
        type==BS_AUTORADIOBUTTON || type==BS_3STATE || type==BS_AUTO3STATE;
    return (type<=BS_DEFPUSHBUTTON || choice) && (push || !(style&(BS_ICON|BS_BITMAP)));
}
static void dispose(void *context)
{
    win2k_control *control=context;
    style_control_state_dispose(&control->state);free(control);
}
static style_control_retired release_theme(HWND window,void *context,BOOL terminal)
{
    win2k_control *control=context;
    win2k_scroll_retire(window,&control->scroll,TRUE);
    BOOL list_removed=style_control_combo_list_retire(&control->combo,&control->instance,
        lookup_state,win2k_control_remove);
    style_control_notify_unbind(&control->parent_binding,parent_proc,control);
    if(terminal)(void)style_theme_terminal(window,control);
    else style_theme_remove(window,control,control->native_theme);
    BOOL result=terminal || !control->frame_owned || !IsWindow(window) ||
        SetWindowPos(window,NULL,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE|SWP_FRAMECHANGED);
    return (style_control_retired){list_removed && result,FALSE};
}
static void finish_input(HWND window,void *context)
{
    win2k_control *control=context;
    win2k_scroll_retire(window,&control->scroll,FALSE);
}
static BOOL retire(HWND window,win2k_control *control,BOOL terminal)
{
    static const style_control_retire_ops ops={finish_input,release_theme};
    return style_control_retire(window,&control->instance,&control->state,terminal,control,&ops);
}
static void paint(HWND window,void *context,HDC dc)
{
    win2k_control *control=context;
    BOOL painted=TRUE;
    if(control->tab_control){
        unsigned epoch=control->tab.epoch;
        HFONT font=(HFONT)SendMessageW(window,WM_GETFONT,0,0);
        painted=style_control_native_current(window,&control->instance,&control->tab,epoch) &&
            win2k_tab_paint(window,dc,font,&control->instance,&control->tab);
    }else if(control->combo_parent || class_is(window,L"LISTBOX")){
        HWND parent=control->combo_parent?control->combo_parent:GetParent(window);
        win2k_list_paint(window,&control->instance,parent,dc);
    }else if(class_is(window,L"COMBOBOX")){
        painted=win2k_combo_paint(window,&control->instance,dc);
    }else painted=static_class(window)?win2k_static_paint(window,&control->instance,dc,&win2k_standard_palette):
        win2k_button_paint(window,&control->instance,dc,&win2k_standard_palette);
    if(!painted &&
       !control->instance.detached){
        int saved=SaveDC(dc);
        if(saved){DefSubclassProc(window,WM_PRINTCLIENT,(WPARAM)dc,PRF_CLIENT);RestoreDC(dc,saved);}
    }
}
static BOOL is_group(HWND window,void *context)
{(void)context;return class_is(window,L"BUTTON") && (GetWindowLongPtrW(window,GWL_STYLE)&BS_TYPEMASK)==BS_GROUPBOX;}
static void render(HWND window,void *context,HDC dc,BOOL buffered)
{
    win2k_control *control=context;
    if(buffered){const style_present_ops ops={paint,is_group};
        style_present(window,&control->instance,control,win2k_standard_palette.face,&ops,dc);}
    else paint(window,control,dc);
}
static void edit_border(HWND window,void *context,HDC dc)
{(void)context;(void)win2k_edit_border(window,dc,&win2k_standard_palette);}
static BOOL embedded_input(HWND window,void *context,UINT message,WPARAM a,LPARAM b)
{
    win2k_control *control=context;
    const style_scroll_ops ops=win2k_scroll_operations(&control->scroll);
    if(message==WM_ENABLE && !a)
        style_scroll_finish(window,&control->instance,&control->scroll.controller,&ops,TRUE);
    return control->instance.detached || style_scroll_embedded_input(window,&control->instance,&control->state,
        &control->scroll.controller,&ops,message,a,b);
}
static void embedded_paint(HWND window,void *context,HDC dc)
{
    win2k_control *control=context;
    win2k_scroll_paint_nc(window,&control->scroll,dc);
}
static BOOL control_supported(HWND window,const win2k_control *control)
{
    if(!control->combo_parent)return win2k_control_supported(window);
    win2k_control *parent=lookup(control->combo_parent);
    return parent && !parent->instance.detached &&
        (!control->state.owner || control->state.owner==&parent->instance) &&
        list_style((DWORD)GetWindowLongPtrW(window,GWL_STYLE));
}
static LRESULT dispatch(void *context,HWND window,UINT message,WPARAM a,LPARAM b)
{
    win2k_control *control=context;LRESULT result;
    if(control->tab_control)style_control_native_observe(&control->tab,STYLE_CONTROL_TAB,message);
    if(message==WM_NCDESTROY){
        (void)retire(window,control,TRUE);result=DefSubclassProc(window,message,a,b);
    }else if(!control_supported(window,control) ||
        (control->parent_binding.parent && GetParent(window)!=control->parent_binding.parent)){
        result=DefSubclassProc(window,message,a,b);
        if(!control->instance.detached)(void)retire(window,control,FALSE);
    }else if(control->tab_control){
        const style_control_draw draw={control,render};
        result=style_control_native_message(window,&control->instance,&control->tab,STYLE_CONTROL_TAB,
            message,a,b,win2k_tab_supported,&draw);
    }else if(control->header_control){
        result=DefSubclassProc(window,message,a,b);
    }else if(class_is(window,L"SCROLLBAR")){
        if(GetWindowLongPtrW(window,GWL_STYLE)&(SBS_SIZEBOX|SBS_SIZEGRIP)){
            const style_control_draw draw={control,win2k_scroll_sizebox};
            result=style_scroll_sizebox_message(window,message,a,b,&draw);
        }else{
            const style_scroll_ops ops=win2k_scroll_operations(&control->scroll);
            result=style_scroll_control_message(window,&control->instance,&control->scroll.controller,&message,&a,&b,&ops);
        }
    }else if(control->scroll.controller.resources.list){
        if(GetWindowLongPtrW(window,GWL_STYLE)&LBS_USETABSTOPS){
            /* Native tab positions can predate attachment and have no getter. */
            const style_control_edit_ops ops={.context=control,.input=embedded_input,.scrollbars=embedded_paint,.border=edit_border};
            result=style_control_edit_message(window,&control->instance,&control->state,NULL,message,a,b,&ops);
        }else{
            const style_control_list_ops ops={.context=control,.input=embedded_input,.draw=render,
                .border=edit_border,.scrollbars=embedded_paint,.native_nonclient=TRUE};
            result=style_control_list_message(window,&control->instance,&control->state,message,a,b,&ops);
        }
    }else if(class_is(window,L"COMBOBOX")){
        const style_control_draw draw={control,render};
        result=style_control_combo_message(window,&control->instance,&control->state,message,a,b,&draw);
    }else if(win2k_edit_class(window)!=WIN2K_EDIT_NONE){
        const style_control_edit_ops ops={.context=control,.input=embedded_input,.scrollbars=embedded_paint,.border=edit_border};
        result=style_control_edit_message(window,&control->instance,&control->state,NULL,message,a,b,&ops);
    }else{
        const style_control_draw draw={control,render};
        result=style_control_basic_message(window,&control->instance,&control->state,message,a,b,basic_kind(window),&draw);
    }
    return result;
}
static void finish_call(void *context,BOOL completed)
{win2k_control *control=context;(void)completed;style_instance_release(&control->instance);}
static LRESULT parent_dispatch(void *context,HWND parent,UINT message,WPARAM a,LPARAM b)
{
    win2k_control *control=context;
    if(control->header_control){
        const style_control_notify_ops ops={&control->instance,win2k_header_supported,win2k_header_paint,NULL};
        return style_control_notify_message(parent,message,a,b,(UINT_PTR)control->parent_binding.window,
            &control->instance,&control->parent_binding,parent_proc,NULL,&ops);
    }
    if(message==WM_NCDESTROY)style_control_notify_unbind(&control->parent_binding,parent_proc,control);
    LRESULT result=DefSubclassProc(parent,message,a,b);DWORD error=GetLastError();
    if(!control->instance.detached && (message==WM_CTLCOLOREDIT || message==WM_CTLCOLORSTATIC) &&
       (HWND)b==control->parent_binding.window && GetParent((HWND)b)==parent)
        result=(LRESULT)win2k_edit_brush((HDC)a,(HBRUSH)result,&win2k_standard_palette);
    SetLastError(error);return result;
}
static LRESULT CALLBACK parent_proc(HWND parent,UINT message,WPARAM a,LPARAM b,UINT_PTR id,DWORD_PTR data)
{
    win2k_control *control=(win2k_control *)data;(void)id;
    style_instance_retain(&control->instance);
    const style_dispatch_ops ops={control,control,parent_dispatch,finish_call};
    return style_dispatch_message(parent,message,a,b,&ops);
}
static LRESULT CALLBACK control_proc(HWND window,UINT message,WPARAM a,LPARAM b,UINT_PTR id,DWORD_PTR data)
{
    win2k_control *control=(win2k_control *)data;(void)id;
    style_instance_retain(&control->instance);
    const style_dispatch_ops ops={control,control,dispatch,finish_call};
    return style_dispatch_message(window,message,a,b,&ops);
}
static BOOL install_combo_list(HWND window,void *context)
{
    win2k_control *parent=context;
    if(!class_is(window,L"ComboLBox") || !list_style((DWORD)GetWindowLongPtrW(window,GWL_STYLE))){
        SetLastError(ERROR_NOT_SUPPORTED);
        return FALSE;
    }
    return attach(window,parent->native_theme,parent->instance.window);
}
static BOOL setup(HWND window,void *context)
{
    win2k_control *control=context;
    if((control->header_control || win2k_edit_class(window)==WIN2K_EDIT_STANDARD) &&
       !style_control_notify_bind(window,&control->parent_binding,parent_proc,control))return FALSE;
    if(!style_theme_capture(window) || !style_theme_suppress(window,control,control->native_theme))return FALSE;
    if(class_is(window,L"COMBOBOX")){
        const style_control_combo_ops bindings={.lookup=lookup_state};
        const style_control_combo_setup_ops children={
            .context=control,.get_info=win2k_combo_info,.install_list=install_combo_list,.remove=win2k_control_remove
        };
        return style_control_combo_setup(window,&control->instance,&control->combo,&bindings,&children);
    }
    return !control->frame_owned || SetWindowPos(window,NULL,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE|SWP_FRAMECHANGED);
}
static void rollback(HWND window,void *context)
{(void)retire(window,context,FALSE);}
static BOOL attach(HWND window,style_native_theme_fn native_theme,HWND combo_parent)
{
    if(!win32_style_window_current_thread(window)){SetLastError(ERROR_INVALID_THREAD_ID);return FALSE;}
    if(lookup(window))return TRUE;
    if(!native_theme || (!combo_parent && !win2k_control_supported(window))){SetLastError(ERROR_NOT_SUPPORTED);return FALSE;}
    if(style_theme_retained(window)){SetLastError(ERROR_BUSY);return FALSE;}
    if(!style_instance_preflight(window,control_proc))return FALSE;
    win2k_control *control=calloc(1,sizeof(*control));
    if(!control){SetLastError(ERROR_NOT_ENOUGH_MEMORY);return FALSE;}
    style_instance_init(&control->instance,control,dispose);
    control->tab_control=class_is(window,WC_TABCONTROLW);
    control->header_control=class_is(window,WC_HEADERW);
    style_control_native_init(&control->tab);
    win2k_scroll_init(&control->scroll,&control->instance);
    control->scroll.controller.resources.list=combo_parent || class_is(window,L"LISTBOX");
    control->scroll.controller.resources.edit=win2k_edit_class(window)==WIN2K_EDIT_STANDARD &&
        (GetWindowLongPtrW(window,GWL_STYLE)&ES_MULTILINE);
    control->scroll.controller.resources.frame=win2k_edit_class(window)==WIN2K_EDIT_NATIVE_CLIENT;
    style_control_state_init(&control->state,NULL);control->native_theme=native_theme;
    control->combo_parent=combo_parent;
    control->frame_owned=basic_kind(window)==STYLE_CONTROL_SEPARATOR;
    const style_instance_install_ops ops={control,NULL,setup,rollback};
    BOOL result=style_instance_install(&control->instance,window,control_proc,control,&ops);
    if(result)RedrawWindow(window,NULL,NULL,RDW_INVALIDATE|RDW_FRAME);
    return result;
}
BOOL win2k_control_attach(HWND window,style_native_theme_fn native_theme)
{return attach(window,native_theme,NULL);}
BOOL win2k_control_attached(HWND window)
{win2k_control *control=lookup(window);return control && !control->instance.detached;}
BOOL win2k_control_remove(HWND window)
{
    if(!win32_style_window_current_thread(window)){SetLastError(ERROR_INVALID_THREAD_ID);return FALSE;}
    win2k_control *control=lookup(window);
    if(!control){SetLastError(ERROR_NOT_FOUND);return FALSE;}
    return retire(window,control,FALSE);
}
