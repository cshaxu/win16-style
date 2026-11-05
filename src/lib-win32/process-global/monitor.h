#ifndef WIN32_PROCESS_GLOBAL_MONITOR_H
#define WIN32_PROCESS_GLOBAL_MONITOR_H
#include <windows.h>
#include <evntcons.h>
typedef struct win32_process_global_monitor win32_process_global_monitor;
typedef void (*win32_process_global_monitor_callback)(DWORD pid,void *context);
DWORD win32_process_global_monitor_event_pid(const EVENT_RECORD *record);
DWORD win32_process_global_monitor_start(const WCHAR *name,win32_process_global_monitor_callback callback,void *context,win32_process_global_monitor **result);
DWORD win32_process_global_monitor_stop(win32_process_global_monitor *monitor);
#endif
