#ifndef NTW16S_GLOBAL_PROCESS_H
#define NTW16S_GLOBAL_PROCESS_H
#include <windows.h>
#include "lib-win32/process-global/monitor.h"

BOOL ntw16s_global_process_parse_name(const WCHAR *name,DWORD *pid,DWORD *high,DWORD *low);
DWORD ntw16s_global_process_start(win32_process_global_monitor_callback callback,void *context,
                                 win32_process_global_monitor **result);
/* Stop the known peer's logger using its retained, terminated process object. */
DWORD ntw16s_global_process_stop_peer(HANDLE process,DWORD pid);
#endif
