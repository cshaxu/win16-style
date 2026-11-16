#ifndef WIN32_STYLE_WINDOW_DISPATCH_H
#define WIN32_STYLE_WINDOW_DISPATCH_H
#include <windows.h>
/* The caller owns its message protocol and UI-thread action. This operation
 * clears the result property before and after the bounded synchronous request. */
DWORD win32_style_window_request(HWND,UINT,PCWSTR,WPARAM,LPARAM,DWORD);
/* Borrows the loaded module/callback. Installs one temporary thread hook,
 * revalidates target identity, sends the request and removes the hook. Zero
 * target PID/TID is rejected before installation; no desktop hook is used. */
DWORD win32_style_window_dispatch_loaded(HWND,DWORD,DWORD,HMODULE,HOOKPROC,
    PCWSTR,PCWSTR,WPARAM,LPARAM,DWORD);
/* The caller validates target identity, bitness and module before calling.
 * The named message and result property belong to the caller's protocol.
 * The target writes error+1 to the property; no property is invalid data. */
DWORD win32_style_window_dispatch(HWND window,DWORD thread,PCWSTR module_path,
    const char *hook_export,PCWSTR message_name,PCWSTR result_property,
    WPARAM action,LPARAM parameter,DWORD timeout);
#endif
