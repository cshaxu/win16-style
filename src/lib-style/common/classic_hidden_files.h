#ifndef STYLE_CLASSIC_HIDDEN_FILES_H
#define STYLE_CLASSIC_HIDDEN_FILES_H
#include <windows.h>
#include <stddef.h>
void style_classic_hidden_files(HWND,WCHAR **,BOOL (*)(HWND,PWSTR,size_t));
#endif
