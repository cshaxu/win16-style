#ifndef NTW16S_GLOBAL_DISCOVER_H
#define NTW16S_GLOBAL_DISCOVER_H
#include "lib-win32/process-global/window.h"
typedef win32_process_global_window_callback ntw16s_global_discovered;
typedef win32_process_global_window ntw16s_global_discover;
DWORD ntw16s_global_discover_start(ntw16s_global_discover *,ntw16s_global_discovered,void *);
DWORD ntw16s_global_discover_refresh(ntw16s_global_discover *);
DWORD ntw16s_global_discover_stop(ntw16s_global_discover *);
#endif
