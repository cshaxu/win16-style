#ifndef WIN32_STYLE_WINDOW_DISPATCH_H
#define WIN32_STYLE_WINDOW_DISPATCH_H
#include <windows.h>
/* The caller validates target identity, bitness and module before calling.
 * The named message and result property belong to the caller's protocol.
 * The target writes error+1 to the property; no property is invalid data. */
DWORD win32_style_window_dispatch(HWND window,DWORD thread,PCWSTR module_path,
    const char *hook_export,PCWSTR message_name,PCWSTR result_property,
    WPARAM action,LPARAM parameter,DWORD timeout);
#endif
