#ifndef WIN32_PROCESS_GLOBAL_WORKER_H
#define WIN32_PROCESS_GLOBAL_WORKER_H
#include <windows.h>
/* Zero-initialize, then supply an owned, initially nonsignaled ready event.
 * begin borrows process/packet and retains its own process handle and packet copy.
 * Call close after any begin failure; timeout retains the ticket for retry.
 * The worker must copy the packet before signaling ready and never read the
 * remote packet afterwards. ready releases that allocation; close owns and
 * closes the supplied ready event as well as the ticket's other handles. */
typedef struct win32_process_global_worker {HANDLE process,thread,ready;void *remote_packet;} win32_process_global_worker;
DWORD win32_process_global_worker_begin(win32_process_global_worker *ticket,HANDLE process,void *worker,const void *packet,SIZE_T bytes);
DWORD win32_process_global_worker_ready(win32_process_global_worker *ticket,DWORD timeout);
DWORD win32_process_global_worker_close(win32_process_global_worker *ticket);
#endif
