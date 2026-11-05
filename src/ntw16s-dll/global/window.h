#ifndef NTW16S_GLOBAL_WINDOW_H
#define NTW16S_GLOBAL_WINDOW_H
#include <windows.h>
/* Existing-window attachment shares the automatic classifier, runs on the
 * window thread, and refuses an existing owner or an unsupported window. */
BOOL __cdecl Ntw16sWindowAttach(HWND window);
/* GUI-thread only. Idempotent within a live attachment; unsupported windows
 * remain native. Existing run16s attachments remain independently owned.
 * No process-wide startup hooks are modified. */
BOOL ntw16s_global_window_apply(HWND window);
BOOL ntw16s_global_window_remove(HWND window);
BOOL ntw16s_global_window_attached(HWND window);
#endif
