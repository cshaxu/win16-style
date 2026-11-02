#ifndef WIN16_DIALOG_FOLDER_CREATE_H
#define WIN16_DIALOG_FOLDER_CREATE_H
#include <windows.h>
BOOL win16_dialog_create_folder(HWND owner,PCWSTR parent,PWSTR path,size_t capacity);
#endif
