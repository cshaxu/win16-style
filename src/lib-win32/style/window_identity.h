#ifndef WIN32_STYLE_WINDOW_IDENTITY_H
#define WIN32_STYLE_WINDOW_IDENTITY_H
#include <windows.h>
#include <wchar.h>

/* No ownership is acquired; callers choose their error and revalidation policy. */
static inline BOOL win32_style_window_current_thread(HWND window)
{
    DWORD process=0;
    return GetWindowThreadProcessId(window,&process)==GetCurrentThreadId() && process==GetCurrentProcessId();
}
/* A borrowed child and parent still belong to the same local module/thread. */
static inline BOOL win32_style_window_owned_child(HWND window,HMODULE owner,HWND parent)
{
    return window && parent && owner && IsWindow(window) && GetParent(window)==parent &&
        win32_style_window_current_thread(window) &&
        GetWindowThreadProcessId(parent,NULL)==GetCurrentThreadId() &&
        (HMODULE)GetClassLongPtrW(window,GCLP_HMODULE)==owner &&
        (HMODULE)GetClassLongPtrW(parent,GCLP_HMODULE)==owner &&
        (GetWindowLongPtrW(window,GWL_STYLE)&WS_CHILD);
}
/* Exact class identity; the fixed buffer matches the native host class names. */
static inline BOOL win32_style_window_class(HWND window,PCWSTR expected)
{
    WCHAR value[80];
    return GetClassNameW(window,value,ARRAYSIZE(value)) && !wcscmp(value,expected);
}
/* The class name is a caller policy; this only verifies its procedure owner. */
static inline BOOL win32_style_class_procedure_owned(HMODULE owner,PCWSTR name)
{
    WNDCLASSEXW cls={.cbSize=sizeof(cls)};HMODULE procedure_owner=NULL;
    return GetClassInfoExW(owner,name,&cls) && cls.lpfnWndProc &&
        GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
            (LPCWSTR)cls.lpfnWndProc,&procedure_owner) && procedure_owner==owner;
}
#endif
