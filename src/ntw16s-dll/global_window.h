#ifndef NTW16S_GLOBAL_WINDOW_H
#define NTW16S_GLOBAL_WINDOW_H
#include <windows.h>
/* GUI-thread only. Idempotent within a live attachment; unsupported windows
 * remain native. Removal includes windows already attached by run16s, per the
 * global-exit contract. No process-wide startup hooks are modified. */
BOOL ntw16s_global_window_apply(HWND window);
BOOL ntw16s_global_window_remove(HWND window);
BOOL ntw16s_global_window_attached(HWND window);
#endif
