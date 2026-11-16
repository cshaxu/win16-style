#ifndef STYLE_GLOBAL_SESSION_H
#define STYLE_GLOBAL_SESSION_H
#include <windows.h>

#define STYLE_GLOBAL_NAME_CAP 160
enum { STYLE_GLOBAL_ACTIVE=1, STYLE_GLOBAL_STOPPING=2 };
typedef struct style_global_session {
    DWORD magic,version,owner_pid,owner_birth_low,owner_birth_high,cookie;
    volatile LONG phase;
    DWORD host_pid;
    volatile LONG ready;
    DWORD style;
} style_global_session;
C_ASSERT(sizeof(style_global_session)==40);

/* Configuration strings are immutable and borrowed for process life. The
 * library owns the identity cache; initialize the descriptor/cache to zero
 * before setting prefix, launcher, native module, magic and wire version.
 * Style is an opaque caller-defined ID, unrelated to any renderer. */
typedef struct style_global_namespace {
    PCWSTR prefix,launcher,module;
    DWORD magic,version;
    struct {SRWLOCK lock;DWORD cookie,pid;HMODULE module;} owner;
} style_global_namespace;
BOOL style_global_session_name(const style_global_namespace *,WCHAR *,DWORD);
BOOL style_global_owner_alive(const style_global_namespace *,const style_global_session *);
BOOL style_global_session_matches(const style_global_namespace *,const style_global_session *,DWORD,DWORD);
/* Queries borrow and close their mapping/process handles before returning,
 * and preserve the calling thread's LastError. */
BOOL style_global_policy_active(const style_global_namespace *,DWORD);
BOOL style_global_policy_style(const style_global_namespace *,DWORD,DWORD);
BOOL style_global_owner_matches(style_global_namespace *,const style_global_session *,HMODULE);
#endif
