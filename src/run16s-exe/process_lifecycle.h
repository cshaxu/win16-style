#ifndef WIN32_PROCESS_LAUNCH_PROCESS_H
#define WIN32_PROCESS_LAUNCH_PROCESS_H
#include <windows.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef DWORD (*win32_process_prepare)(HANDLE process,void *context);
enum { WIN32_PROCESS_PREPARE_FAILED=1,WIN32_PROCESS_RESUME_FAILED=2 };
/* Takes ownership of a newly created suspended child's handles. On success
 * closes its thread handle and returns the process handle to the caller. On
 * failure terminates only that uncommitted child and closes both handles. */
DWORD win32_process_activate(PROCESS_INFORMATION *child,win32_process_prepare prepare,void *context,unsigned *phase);
/* Borrows a process handle; caller owns and closes it. */
DWORD win32_process_wait(HANDLE process,DWORD *exit_code);
#ifdef __cplusplus
}
#endif
#endif
