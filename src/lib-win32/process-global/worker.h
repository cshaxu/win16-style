#ifndef WIN32_PROCESS_GLOBAL_WORKER_H
#define WIN32_PROCESS_GLOBAL_WORKER_H
#include <windows.h>
/* Caller zero-initializes; on timeout preserve resources and retry. */
typedef struct win32_process_global_worker {HANDLE process,thread,ready;void *remote_packet;} win32_process_global_worker;
DWORD win32_process_global_worker_begin(win32_process_global_worker *ticket,HANDLE process,void *worker,const void *packet,SIZE_T bytes);
DWORD win32_process_global_worker_ready(win32_process_global_worker *ticket,DWORD timeout);
DWORD win32_process_global_worker_close(win32_process_global_worker *ticket);
#endif
