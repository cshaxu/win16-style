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
#ifdef __cplusplus
}
#endif
#endif
