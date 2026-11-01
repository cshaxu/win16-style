#ifndef NTW16S_GLOBAL_PROCESS_H
#define NTW16S_GLOBAL_PROCESS_H
#include <windows.h>
#include <evntcons.h>

typedef struct ntw16s_global_process ntw16s_global_process;
typedef void (*ntw16s_global_process_callback)(DWORD pid,void *context);
DWORD ntw16s_global_process_event_pid(const EVENT_RECORD *record);
BOOL ntw16s_global_process_parse_name(const WCHAR *name,DWORD *pid,DWORD *high,DWORD *low);
DWORD ntw16s_global_process_start(ntw16s_global_process_callback callback,void *context,
                                 ntw16s_global_process **result);
DWORD ntw16s_global_process_stop(ntw16s_global_process *monitor);
/* Stop the known peer's logger using its retained, terminated process object. */
DWORD ntw16s_global_process_stop_peer(HANDLE process,DWORD pid);
#endif
