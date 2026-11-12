#ifndef WIN16_DIALOG_HIDDEN_FILES_H
#define WIN16_DIALOG_HIDDEN_FILES_H
#include <windows.h>
#include <stddef.h>
void win16_dialog_hidden_files(HWND,WCHAR **,BOOL (*)(HWND,PWSTR,size_t));
void win16_dialog_hidden_schedule(HWND,BOOL);
#endif
