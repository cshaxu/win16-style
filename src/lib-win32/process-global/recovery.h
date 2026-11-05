#ifndef WIN32_PROCESS_GLOBAL_RECOVERY_H
#define WIN32_PROCESS_GLOBAL_RECOVERY_H
#include <windows.h>
/* Parser accepts only sessions owned by the caller's application. */
typedef BOOL (*win32_process_global_trace_name_parser)(const WCHAR *,DWORD *,DWORD *,DWORD *);
BOOL win32_process_global_trace_orphan(DWORD pid,DWORD birth_high,DWORD birth_low);
void win32_process_global_recover_orphans(win32_process_global_trace_name_parser parse);
#endif
