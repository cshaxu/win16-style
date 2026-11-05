#ifndef WIN32_PROCESS_GLOBAL_ENUMERATE_H
#define WIN32_PROCESS_GLOBAL_ENUMERATE_H
#include <windows.h>
#ifdef __cplusplus
extern "C" {
#endif
/* Callback returns FALSE to stop; pid_filter zero accepts all owners.
 * Process and thread IDs are snapshots and must be revalidated by the caller. */
typedef BOOL (*win32_process_global_thread_callback)(DWORD pid,DWORD thread_id,void *context);
DWORD win32_process_global_threads(DWORD pid_filter,win32_process_global_thread_callback callback,void *context);
/* Callers supply policy and decide whether a process already has a target.
 * Thread identity is a snapshot: the callback must revalidate before use.
 * For a new process, ERROR_NOT_READY records a fallback thread; any other
 * nonzero result rejects it. The first accepted thread wins over a fallback. */
typedef DWORD (*win32_process_global_inspect_thread)(DWORD pid,DWORD tid,void *context);
typedef BOOL (*win32_process_global_has_process)(DWORD pid,void *context);
typedef void (*win32_process_global_select_thread)(DWORD pid,DWORD tid,void *context);
DWORD win32_process_global_scan_existing(win32_process_global_has_process,win32_process_global_inspect_thread,win32_process_global_select_thread,void *);
DWORD win32_process_global_scan_process(DWORD,win32_process_global_inspect_thread,win32_process_global_select_thread,void *);

#ifdef __cplusplus
}
#endif
#endif
