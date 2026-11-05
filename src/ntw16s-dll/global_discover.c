#include "global_discover.h"
DWORD ntw16s_global_discover_start(ntw16s_global_discover *w,ntw16s_global_discovered cb,void *ctx)
{return win32_process_global_window_start(w,cb,ctx);}
DWORD ntw16s_global_discover_refresh(ntw16s_global_discover *w)
{return win32_process_global_window_refresh(w);}
DWORD ntw16s_global_discover_stop(ntw16s_global_discover *w)
{return win32_process_global_window_stop(w);}
