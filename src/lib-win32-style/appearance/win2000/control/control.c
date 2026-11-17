#include "control.h"
#include "button_paint.h"
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
} win2k_control;
static LRESULT CALLBACK control_proc(HWND,UINT,WPARAM,LPARAM,UINT_PTR,DWORD_PTR);
static win2k_control *lookup(HWND window)
{return (win2k_control *)style_instance_lookup(window,control_proc);}
BOOL win2k_control_supported(HWND window)
{
    WCHAR name[32];
    DWORD style=(DWORD)GetWindowLongPtrW(window,GWL_STYLE);
    return (style&WS_CHILD) && (style&BS_TYPEMASK)<=BS_DEFPUSHBUTTON &&
        !(GetWindowLongPtrW(window,GWL_EXSTYLE)&(WS_EX_LAYOUTRTL|WS_EX_RTLREADING)) &&
        GetClassNameW(window,name,ARRAYSIZE(name)) && !lstrcmpiW(name,L"BUTTON");
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
    return (style_control_retired){TRUE,FALSE};
}
static BOOL retire(HWND window,win2k_control *control,BOOL terminal)
{
    static const style_control_retire_ops ops={NULL,release_theme};
    return style_control_retire(window,&control->instance,&control->state,terminal,control,&ops);
}
static void paint(HWND window,void *context,HDC dc)
{
    win2k_control *control=context;
    if(!win2k_button_paint(window,&control->instance,dc,&win2k_control_standard_palette) &&
       !control->instance.detached){
        int saved=SaveDC(dc);
        if(saved){DefSubclassProc(window,WM_PRINTCLIENT,(WPARAM)dc,PRF_CLIENT);RestoreDC(dc,saved);}
    }
}
static BOOL is_group(HWND window,void *context)
{(void)window;(void)context;return FALSE;}
static void render(HWND window,void *context,HDC dc,BOOL buffered)
{
    win2k_control *control=context;
    if(buffered){const style_present_ops ops={paint,is_group};
        style_present(window,&control->instance,control,win2k_control_standard_palette.face,&ops,dc);}
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
        result=style_control_basic_message(window,&control->instance,&control->state,message,a,b,STYLE_CONTROL_BUTTON,&draw);
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
    return style_theme_capture(window) && style_theme_suppress(window,control,control->native_theme);
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
