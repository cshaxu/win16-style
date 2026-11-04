#ifndef PROCESS_HOOK_STAGING_H
#define PROCESS_HOOK_STAGING_H
#include <windows.h>
/* Only a newly created, suspended, uncommitted child is accepted by policy;
 * its creator owns rollback on failure. The payload is an opaque byte span. */
typedef DWORD (*process_hook_payload_consumer)(const void *,DWORD,DWORD,void *);
DWORD process_hook_stage_process(HANDLE child,const char *dll,const void *payload,DWORD bytes);
DWORD process_hook_stage_read(process_hook_payload_consumer consume,void *context,BOOL *found);
DWORD process_hook_stage_restore(void);
#endif
