#include "menu_owner.h"
#include "lib-win32/style/window_identity.h"
BOOL style_menu_owner_mark(HWND w,const void *caller,LPCWSTR class_name,LPCWSTR property,HANDLE marker)
{
    WCHAR cls[64],real[64];HMODULE origin=NULL;
    return w && win32_style_window_current_thread(w) &&
        !(GetWindowLongPtrW(w,GWL_STYLE)&(WS_VISIBLE|WS_CHILD)) &&
        (GetWindowLongPtrW(w,GWL_STYLE)&WS_POPUP) && (GetWindowLongPtrW(w,GWL_EXSTYLE)&WS_EX_TOOLWINDOW) &&
        GetClassNameW(w,cls,64) && !lstrcmpW(cls,class_name) &&
        RealGetWindowClassW(w,real,64) && !lstrcmpW(real,cls) &&
        GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,(LPCWSTR)caller,&origin) && origin==(HMODULE)GetClassLongPtrW(w,GCLP_HMODULE) &&
        SetPropW(w,property,marker);
}
BOOL style_menu_owner_eligible(HWND w,LPCWSTR property,HANDLE marker)
{
    return win32_style_window_current_thread(w) &&
        GetPropW(w,property)==marker && !(GetWindowLongPtrW(w,GWL_STYLE)&(WS_VISIBLE|WS_CHILD)) &&
        (GetWindowLongPtrW(w,GWL_EXSTYLE)&WS_EX_TOOLWINDOW);
}
