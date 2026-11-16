#ifndef STYLE_GLOBAL_MANAGER_H
#define STYLE_GLOBAL_MANAGER_H
#include <windows.h>
#include "attach.h"
#include "uwp_global.h"
#include "lib-win32/process-global/monitor.h"

typedef struct style_global_manager style_global_manager;
typedef struct style_global_manager_stats {LONG queued,active,failed,overflow;} style_global_manager_stats;

enum style_global_report_kind {STYLE_GLOBAL_PROCESS_SCAN,STYLE_GLOBAL_TRACE_UNAVAILABLE,
    STYLE_GLOBAL_LOADER_RETAINED,STYLE_GLOBAL_TARGET_STOP,STYLE_GLOBAL_MONITOR_STOP,
    STYLE_GLOBAL_DISCOVERY_STOP,STYLE_GLOBAL_TARGETS_STOP,STYLE_GLOBAL_UWP_STOP};
typedef struct style_global_report {
    enum style_global_report_kind kind;
    DWORD pid,thread,error,worker,loader,phase;
} style_global_report;
/* Borrowed configuration outlives manager and any retained failed stop.
 * The manager owns discovery/session mapping/targets/tickets and shutdown.
 * Callbacks provide application policy and final reporting, not state access. */
typedef struct style_global_manager_config {
    const style_global_attach_config *attach;
    PCWSTR trace_prefix;
    BOOL (*uwp_services)(DWORD style,DWORD cookie,WORD machine,style_uwp_global_services *,void *);
    void (*report)(const style_global_report *,void *);
    void *context;
} style_global_manager_config;

/* Must be started and stopped on the same message-pumping desktop thread.
 * The session must be active. Before stopping, its owner sets STOPPING,
 * requests window removal, and signals the session stop event. If start fails
 * and returns a non-NULL result, the caller must retry stop on that object. */
DWORD style_global_manager_start(const style_global_manager_config *,const WCHAR *dll_path,WORD machine,style_global_manager **result);
DWORD style_global_manager_refresh(style_global_manager *manager);
/* Pump the owner thread while startup work drains; ERROR_TIMEOUT is a
 * bounded partial-start result, not a claim that all targets were attached. */
DWORD style_global_manager_wait_idle(style_global_manager *manager,DWORD timeout);
HANDLE style_global_manager_changed(style_global_manager *manager);
void style_global_manager_stats_read(style_global_manager *manager,style_global_manager_stats *stats);
BOOL style_global_manager_tracks(style_global_manager *manager,DWORD pid,DWORD thread_id);
#ifdef STYLE_GLOBAL_MANAGER_TEST
void style_global_manager_test_process(style_global_manager *manager,DWORD pid);
/* Restrict the real observer to a test-owned app and its associated UWP frame. */
void style_global_manager_test_filter(DWORD pid);
void style_global_manager_test_second(DWORD pid);
DWORD style_global_manager_test_abandon(void);
#endif
DWORD style_global_manager_stop(style_global_manager *manager,DWORD timeout);
#endif
