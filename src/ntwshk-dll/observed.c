#include "observed.h"
#include "lib-win32-style/appearance/win2000/window/frame.h"
#include "lib-win32-style/appearance/win2000/control/control.h"
#include "lib-win32-style/application/lifecycle/global_window.h"
#include "lib-win32-style/application/lifecycle/theme_state.h"
#include "lib-win32-style/application/window/window_identity.h"
#include <wchar.h>

static void attach_frame(HWND window)
{if(window && !style_global_window_managed(window))win2k_frame_attach(window);}
static int control_owner;

static BOOL control_local(HWND window)
{
    if(win32_style_window_current_thread(window))return TRUE;
    SetLastError(ERROR_INVALID_THREAD_ID);return FALSE;
}
BOOL ntwshk_observed_control_attached(HWND window)
{return control_local(window) && (win2k_control_attached(window) || style_theme_owned(window,&control_owner));}
BOOL ntwshk_observed_control_attach(HWND window)
{
    if(!control_local(window))return FALSE;
    if(!(GetWindowLongPtrW(window,GWL_STYLE)&WS_CHILD)){SetLastError(ERROR_NOT_SUPPORTED);return FALSE;}
    if(ntwshk_observed_control_attached(window))return TRUE;
    BOOL native_requested;
    if(win2k_control_attach(window,ntwshk_observed_native_theme,&native_requested))return TRUE;
    if(native_requested || GetLastError()!=ERROR_NOT_SUPPORTED)return FALSE;
    if(style_theme_retained(window)){SetLastError(ERROR_BUSY);return FALSE;}
    WCHAR name[32];
    if(!GetClassNameW(window,name,ARRAYSIZE(name)))return FALSE;
    static const WCHAR *const classes[]={L"Button",L"Edit",L"ComboBox",L"ListBox",L"ScrollBar"};
    for(unsigned i=0;i<ARRAYSIZE(classes);++i){
        if(!_wcsicmp(name,classes[i])){
            if(!style_theme_capture(window))return FALSE;
            if(style_theme_suppress(window,&control_owner,ntwshk_observed_native_theme))return TRUE;
            (void)style_theme_terminal(window,&control_owner);return FALSE;
        }
    }
    SetLastError(ERROR_NOT_SUPPORTED);return FALSE;
}
BOOL ntwshk_observed_control_remove(HWND window)
{
    if(!control_local(window))return FALSE;
    if(win2k_control_attached(window))return win2k_control_remove(window);
    if(!ntwshk_observed_control_attached(window)){SetLastError(ERROR_NOT_FOUND);return FALSE;}
    style_theme_remove(window,&control_owner,ntwshk_observed_native_theme);return TRUE;
}

void ntwshk_observed_window_created(HWND window,DWORD style)
{
    if(style&WS_CHILD)(void)ntwshk_observed_control_attach(window);
    else if(window)attach_frame(window);
}

static BOOL CALLBACK attach_dialog_child(HWND child,LPARAM context)
{(void)context;(void)ntwshk_observed_control_attach(child);return TRUE;}
void ntwshk_observed_dialog_initialize(HWND dialog)
{attach_frame(dialog);if(IsWindow(dialog))EnumChildWindows(dialog,attach_dialog_child,0);}
