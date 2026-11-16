#ifndef STYLE_GLOBAL_TRACE_H
#define STYLE_GLOBAL_TRACE_H
#include "lib-win32/process-global/monitor.h"
/* The caller owns the namespace prefix. Canonical PID/birth spelling and
 * monitor/recovery assembly are shared; prefix/context are borrowed only for
 * the call. Stopping requires the retained exited peer process object. */
BOOL style_global_trace_parse(const WCHAR *prefix,const WCHAR *name,DWORD *pid,DWORD *high,DWORD *low);
DWORD style_global_trace_start(const WCHAR *prefix,win32_process_global_monitor_callback,void *,win32_process_global_monitor **);
DWORD style_global_trace_stop_peer(const WCHAR *prefix,HANDLE process,DWORD pid);
#endif
