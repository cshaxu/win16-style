#ifndef WIN32_PROCESS_GLOBAL_POLICY_H
#define WIN32_PROCESS_GLOBAL_POLICY_H
#include <windows.h>
typedef struct win32_process_global_policy {BYTE sid[SECURITY_MAX_SID_SIZE];DWORD sid_bytes,session;} win32_process_global_policy;
typedef struct win32_process_global_identity {DWORD pid,thread_id,process_birth_low,process_birth_high,thread_birth_low,thread_birth_high;WORD machine;} win32_process_global_identity;
typedef BOOL (*win32_process_global_image_filter)(const WCHAR *,const WCHAR *);
DWORD win32_process_global_policy_init(win32_process_global_policy *);
DWORD win32_process_global_policy_inspect(const win32_process_global_policy *,HWND,DWORD,DWORD,WORD,win32_process_global_identity *,win32_process_global_image_filter);
DWORD win32_process_global_policy_inspect_thread(const win32_process_global_policy *,DWORD,DWORD,WORD,win32_process_global_identity *,win32_process_global_image_filter);
DWORD win32_process_global_policy_revalidate(HANDLE,HWND,const win32_process_global_identity *);
DWORD win32_process_global_policy_revalidate_thread(HANDLE,const win32_process_global_identity *);
#endif
