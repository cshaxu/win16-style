#ifndef STYLE_PRINT_DIRECTORY_H
#define STYLE_PRINT_DIRECTORY_H
#include <windows.h>

/* S_OK returns a selected queue name owned by the caller (CoTaskMemFree).
 * S_FALSE is cancellation. The directory dialog does not add a queue. */
HRESULT style_print_directory_select(HWND owner,PWSTR *queue);

#endif
