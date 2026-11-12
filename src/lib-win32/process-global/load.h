#ifndef WIN32_PROCESS_GLOBAL_LOAD_H
#define WIN32_PROCESS_GLOBAL_LOAD_H
#include <windows.h>
/* Zero-initialize; on timeout retain the ticket until the remote read ends. */
typedef struct win32_process_global_load {HANDLE process,thread,file;void *remote_path;WCHAR *path;DWORD pid;} win32_process_global_load;
typedef DWORD (*win32_process_global_loader_fn)(HANDLE,DWORD,void **);
typedef DWORD (*win32_process_global_resolve_fn)(HANDLE,DWORD,const WCHAR *,void **);
DWORD win32_process_global_load_begin(HANDLE,DWORD,const WCHAR *,const WCHAR *,win32_process_global_loader_fn,win32_process_global_load *);
DWORD win32_process_global_load_finish(win32_process_global_load *,DWORD,win32_process_global_resolve_fn,void **);
DWORD win32_process_global_load_discard(win32_process_global_load *);
/* For a still-running loader only: close local resources without freeing the
 * remote path it may still read. That allocation remains until target exit. */
DWORD win32_process_global_load_abandon(win32_process_global_load *);
#endif
