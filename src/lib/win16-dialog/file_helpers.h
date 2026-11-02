#ifndef WIN16_DIALOG_FILE_HELPERS_H
#define WIN16_DIALOG_FILE_HELPERS_H
#include <windows.h>
void win16_dialog_resolve_file(HWND window);
BOOL ntw16s_old_dialog_folder(HWND window,PWSTR path,size_t capacity);
#endif
