#include "control.h"
#include "button_paint.h"
#include "static_paint.h"
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
} win2k_control;
static LRESULT CALLBACK control_proc(HWND,UINT,WPARAM,LPARAM,UINT_PTR,DWORD_PTR);
static win2k_control *lookup(HWND window)
{return (win2k_control *)style_instance_lookup(window,control_proc);}
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
    if(!lstrcmpiW(name,L"STATIC")){
        UINT type=style&SS_TYPEMASK;
        return type==SS_LEFT || type==SS_CENTER || type==SS_RIGHT || type==SS_SIMPLE || type==SS_LEFTNOWORDWRAP ||
            type==SS_ICON || type==SS_BITMAP || (type>=SS_BLACKRECT && type<=SS_WHITEFRAME) ||
            ((type>=SS_ETCHEDHORZ && type<=SS_ETCHEDFRAME) && !(style&SS_SUNKEN));
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
    if(terminal)(void)style_theme_terminal(window,control);
    else style_theme_remove(window,control,control->native_theme);
    BOOL result=terminal || !control->frame_owned || !IsWindow(window) ||
        SetWindowPos(window,NULL,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE|SWP_FRAMECHANGED);
    return (style_control_retired){result,FALSE};
}
static BOOL retire(HWND window,win2k_control *control,BOOL terminal)
{
    static const style_control_retire_ops ops={NULL,release_theme};
    return style_control_retire(window,&control->instance,&control->state,terminal,control,&ops);
}
static void paint(HWND window,void *context,HDC dc)
{
    win2k_control *control=context;
    BOOL painted=static_class(window)?win2k_static_paint(window,&control->instance,dc,&win2k_standard_palette):
        win2k_button_paint(window,&control->instance,dc,&win2k_standard_palette);
    if(!painted &&
       !control->instance.detached){
        int saved=SaveDC(dc);
        if(saved){DefSubclassProc(window,WM_PRINTCLIENT,(WPARAM)dc,PRF_CLIENT);RestoreDC(dc,saved);}
    }
}
static BOOL is_group(HWND window,void *context)
{(void)context;return !static_class(window) && (GetWindowLongPtrW(window,GWL_STYLE)&BS_TYPEMASK)==BS_GROUPBOX;}
static void render(HWND window,void *context,HDC dc,BOOL buffered)
{
    win2k_control *control=context;
    if(buffered){const style_present_ops ops={paint,is_group};
        style_present(window,&control->instance,control,win2k_standard_palette.face,&ops,dc);}
    else paint(window,control,dc);
}
static LRESULT dispatch(void *context,HWND window,UINT message,WPARAM a,LPARAM b)
{
    win2k_control *control=context;LRESULT result;
    if(message==WM_NCDESTROY){
        (void)retire(window,control,TRUE);result=DefSubclassProc(window,message,a,b);
    }else if(!win2k_control_supported(window)){
        result=DefSubclassProc(window,message,a,b);
        if(!control->instance.detached)(void)retire(window,control,FALSE);
    }else{
        const style_control_draw draw={control,render};
        result=style_control_basic_message(window,&control->instance,&control->state,message,a,b,basic_kind(window),&draw);
    }
    return result;
}
static void finish_call(void *context,BOOL completed)
{win2k_control *control=context;(void)completed;style_instance_release(&control->instance);}
static LRESULT CALLBACK control_proc(HWND window,UINT message,WPARAM a,LPARAM b,UINT_PTR id,DWORD_PTR data)
{
    win2k_control *control=(win2k_control *)data;(void)id;
    style_instance_retain(&control->instance);
    const style_dispatch_ops ops={control,control,dispatch,finish_call};
    return style_dispatch_message(window,message,a,b,&ops);
}
static BOOL setup(HWND window,void *context)
{
    win2k_control *control=context;
    if(!style_theme_capture(window) || !style_theme_suppress(window,control,control->native_theme))return FALSE;
    return !control->frame_owned || SetWindowPos(window,NULL,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE|SWP_FRAMECHANGED);
}
static void rollback(HWND window,void *context)
{(void)retire(window,context,FALSE);}
BOOL win2k_control_attach(HWND window,style_native_theme_fn native_theme)
{
    if(!win32_style_window_current_thread(window)){SetLastError(ERROR_INVALID_THREAD_ID);return FALSE;}
    if(lookup(window))return TRUE;
    if(!native_theme || !win2k_control_supported(window)){SetLastError(ERROR_NOT_SUPPORTED);return FALSE;}
    if(style_theme_retained(window)){SetLastError(ERROR_BUSY);return FALSE;}
    if(!style_instance_preflight(window,control_proc))return FALSE;
    win2k_control *control=calloc(1,sizeof(*control));
    if(!control){SetLastError(ERROR_NOT_ENOUGH_MEMORY);return FALSE;}
    style_instance_init(&control->instance,control,dispose);
    style_control_state_init(&control->state,NULL);control->native_theme=native_theme;
    control->frame_owned=basic_kind(window)==STYLE_CONTROL_SEPARATOR;
    const style_instance_install_ops ops={control,NULL,setup,rollback};
    BOOL result=style_instance_install(&control->instance,window,control_proc,control,&ops);
    if(result)RedrawWindow(window,NULL,NULL,RDW_INVALIDATE|RDW_FRAME);
    return result;
}
BOOL win2k_control_attached(HWND window)
{win2k_control *control=lookup(window);return control && !control->instance.detached;}
BOOL win2k_control_remove(HWND window)
{
    if(!win32_style_window_current_thread(window)){SetLastError(ERROR_INVALID_THREAD_ID);return FALSE;}
    win2k_control *control=lookup(window);
    if(!control){SetLastError(ERROR_NOT_FOUND);return FALSE;}
    return retire(window,control,FALSE);
}
