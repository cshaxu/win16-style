#ifndef NTW16S_GLOBAL_LOAD_H
#define NTW16S_GLOBAL_LOAD_H
#include <windows.h>

/* Zero-initialize before use. After a timeout, keep the ticket and call finish
 * again; the target may still be reading remote_path. Never kill the target. */
typedef struct ntw16s_global_load {
    HANDLE process,thread,file;
    void *remote_path;
    WCHAR *path;
    DWORD pid;
} ntw16s_global_load;

DWORD ntw16s_global_load_begin(HANDLE process,DWORD pid,const WCHAR *path,ntw16s_global_load *ticket);
DWORD ntw16s_global_load_finish(ntw16s_global_load *ticket,DWORD timeout,void **worker);
DWORD ntw16s_global_load_discard(ntw16s_global_load *ticket);
#endif
