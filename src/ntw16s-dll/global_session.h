#ifndef NTW16S_GLOBAL_SESSION_H
#define NTW16S_GLOBAL_SESSION_H
#include <windows.h>
#define NTW16S_GLOBAL_MAGIC 0x47363157u
#define NTW16S_GLOBAL_VERSION 1u
#define NTW16S_GLOBAL_ACTIVE 1
#define NTW16S_GLOBAL_STOPPING 2
#define NTW16S_GLOBAL_APPLY 1
#define NTW16S_GLOBAL_REMOVE 2
#define NTW16S_GLOBAL_NAME_CAP 160
#define NTW16S_GLOBAL_CONTROL L"Win16Style.Global.Control.1"
typedef struct ntw16s_global_session {
    DWORD magic,version,owner_pid,owner_birth_low,owner_birth_high,cookie;
    volatile LONG phase;
    DWORD host_pid;
    /* Both width-specific observers are installed; attachment may continue. */
    volatile LONG ready;
} ntw16s_global_session;
C_ASSERT(sizeof(ntw16s_global_session)==36);
BOOL ntw16s_global_name(WCHAR *name,DWORD capacity);
BOOL ntw16s_global_owner_alive(const ntw16s_global_session *session);
BOOL ntw16s_global_birth(HANDLE process,DWORD *low,DWORD *high);
#endif
