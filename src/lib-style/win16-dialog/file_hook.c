/* Classic file template initialization and message adaptation. */
#include "file_hook.h"
#include "file_helpers.h"
#include <dlgs.h>

void win16_dialog_file_initialized(HWND window,void (*chrome)(HWND))
{
    WCHAR dialog_class[32],list_class[32];HWND list=GetDlgItem(window,lst2);
    if(list && GetParent(list)==window && GetClassNameW(window,dialog_class,ARRAYSIZE(dialog_class)) &&
        !lstrcmpW(dialog_class,L"#32770") && GetClassNameW(list,list_class,ARRAYSIZE(list_class)) &&
        !lstrcmpiW(list_class,L"ListBox"))chrome(window);
}

UINT_PTR win16_dialog_file_hook(HWND window,UINT message,WPARAM wparam,LPARAM lparam,
    BOOL resolve_links,void (*chrome)(HWND))
{
    (void)lparam;
    if(message==WM_INITDIALOG)chrome(window);
    if(message==WM_COMMAND&&LOWORD(wparam)==IDOK&&resolve_links)win16_dialog_resolve_file(window);
    return 0;
}

