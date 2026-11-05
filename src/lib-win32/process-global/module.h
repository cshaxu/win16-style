#ifndef WIN32_PROCESS_GLOBAL_MODULE_H
#define WIN32_PROCESS_GLOBAL_MODULE_H
#include <windows.h>
#include "load.h"
/* Compare locked regular files byte-for-byte; failures return FALSE. */
BOOL win32_process_global_files_equal(const WCHAR *,const WCHAR *);
typedef BOOL (*win32_process_global_files_equal_fn)(const WCHAR *,const WCHAR *);
typedef DWORD (*win32_process_global_mapped_identity_fn)(HANDLE,DWORD,HMODULE,const WCHAR *,const WCHAR *);
DWORD win32_process_global_mapped_identity(HANDLE,DWORD,HMODULE,const WCHAR *,const WCHAR *,const char *,win32_process_global_resolve_fn);
DWORD win32_process_global_select_path(HANDLE,DWORD,const WCHAR *,WCHAR *,DWORD,win32_process_global_files_equal_fn,win32_process_global_mapped_identity_fn);
#endif
