#ifndef WIN16_DIALOG_FILE_HOOK_H
#define WIN16_DIALOG_FILE_HOOK_H
#include <windows.h>
void win16_dialog_file_initialized(HWND window,void (*chrome)(HWND));
UINT_PTR win16_dialog_file_hook(HWND window,UINT message,WPARAM wparam,LPARAM lparam,
    BOOL resolve_links,void (*chrome)(HWND));
#endif
