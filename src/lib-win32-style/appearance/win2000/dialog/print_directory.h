#ifndef WIN2K_PRINT_DIRECTORY_H
#define WIN2K_PRINT_DIRECTORY_H
#include <windows.h>

/* S_OK returns a selected queue name owned by the caller (CoTaskMemFree).
 * S_FALSE is cancellation. The directory dialog does not add a queue. */
HRESULT win2k_print_directory_select(HWND owner,PWSTR *queue);

#endif
