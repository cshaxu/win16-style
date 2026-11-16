#ifndef STYLE_GLOBAL_ATTACH_H
#define STYLE_GLOBAL_ATTACH_H
#include <windows.h>
#include "lib-win32/process-global/policy.h"
#include "lib-win32/process-global/load.h"
#include "start.h"

typedef struct style_global_attach_config {
    const style_global_start_config *start;
    PCWSTR shared_module;
    const char *shared_export;
    win32_process_global_image_filter allowed;
    PCWSTR control;
    LPARAM apply,remove,query;
    DWORD_PTR attached;
} style_global_attach_config;
/* Config and referenced strings/operations are borrowed until close succeeds.
 * This object owns the process/load/worker tickets, identity, path and phase. */
typedef struct style_global_attach {
    const style_global_attach_config *config;
    win32_process_global_identity identity;
    win32_process_global_load load;
    win32_process_global_worker start;
    HANDLE process;
    HWND window;
    WCHAR *dll_path;
    DWORD cookie,phase,control_error;
    BOOL loading_shared;
} style_global_attach;

/* Each operation is nonblocking except for one bounded control message. The
 * caller retains tickets across ERROR_IO_PENDING and signals the session stop
 * event before close; no operation terminates a target process or thread. */
/* A null window selects the existing thread-only path. */
DWORD style_global_attach_begin(const style_global_attach_config *,const win32_process_global_policy *,
    HWND window,DWORD pid,DWORD thread_id,WORD machine,DWORD cookie,const WCHAR *dll_path,style_global_attach *);
DWORD style_global_attach_advance(style_global_attach *ticket,SECURITY_ATTRIBUTES *security);
DWORD style_global_attach_apply(style_global_attach *ticket);
DWORD style_global_attach_remove(style_global_attach *ticket);
DWORD style_global_attach_close(style_global_attach *ticket);
#endif
