#ifndef NTW16S_GLOBAL_MANAGER_H
#define NTW16S_GLOBAL_MANAGER_H
#include <windows.h>

typedef struct ntw16s_global_manager ntw16s_global_manager;
typedef struct ntw16s_global_manager_stats {LONG queued,active,failed,overflow;} ntw16s_global_manager_stats;

/* Must be started and stopped on the same message-pumping desktop thread.
 * The session must be active. Before stopping, its owner sets STOPPING,
 * requests window removal, and signals the session stop event. If start fails
 * and returns a non-NULL result, the caller must retry stop on that object. */
DWORD ntw16s_global_manager_start(const WCHAR *dll_path,WORD machine,ntw16s_global_manager **result);
HANDLE ntw16s_global_manager_changed(ntw16s_global_manager *manager);
void ntw16s_global_manager_stats_read(ntw16s_global_manager *manager,ntw16s_global_manager_stats *stats);
BOOL ntw16s_global_manager_tracks(ntw16s_global_manager *manager,DWORD pid,DWORD thread_id);
DWORD ntw16s_global_manager_stop(ntw16s_global_manager *manager,DWORD timeout);
#endif
