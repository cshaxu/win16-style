#ifndef NTW16S_GLOBAL_POLICY_H
#define NTW16S_GLOBAL_POLICY_H
#include <windows.h>

typedef struct ntw16s_global_policy {
    BYTE sid[SECURITY_MAX_SID_SIZE];
    DWORD sid_bytes,session;
} ntw16s_global_policy;
typedef struct ntw16s_global_identity {
    DWORD pid,thread_id,process_birth_low,process_birth_high,thread_birth_low,thread_birth_high;
    WORD machine;
} ntw16s_global_identity;

BOOL ntw16s_global_image_allowed(const WCHAR *image,const WCHAR *windows);
DWORD ntw16s_global_policy_init(ntw16s_global_policy *policy);
DWORD ntw16s_global_policy_inspect(const ntw16s_global_policy *policy,HWND window,DWORD pid,DWORD thread_id,
                                   WORD expected_machine,ntw16s_global_identity *identity);
#endif
