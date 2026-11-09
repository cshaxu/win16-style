#ifndef WIN16_XAML_RUNTIME_FIND_H
#define WIN16_XAML_RUNTIME_FIND_H
#include <windows.h>

/* Same-width target. The caller owns process, with query and VM-read access.
 * Outputs are cleared on failure; ERROR_PARTIAL_COPY indicates a width or
 * snapshot limitation, not that the target lacks WinUI. */
DWORD win16_xaml_runtime_find(HANDLE process, WCHAR *loader, DWORD loader_capacity,
                             WCHAR *xaml, DWORD xaml_capacity);
#endif
