#ifndef STYLE_CLASSIC_FILE_HOOK_H
#define STYLE_CLASSIC_FILE_HOOK_H
#include <windows.h>
void style_classic_file_initialized(HWND window,void (*chrome)(HWND));
UINT_PTR style_classic_file_hook(HWND window,UINT message,WPARAM wparam,LPARAM lparam,
    BOOL resolve_links,void (*chrome)(HWND));
#endif
