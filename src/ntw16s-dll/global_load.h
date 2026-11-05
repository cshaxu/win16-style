#ifndef NTW16S_GLOBAL_LOAD_H
#define NTW16S_GLOBAL_LOAD_H
#include "lib-win32/process-global/load.h"
typedef win32_process_global_load ntw16s_global_load;
DWORD ntw16s_global_load_begin(HANDLE,DWORD,const WCHAR *,ntw16s_global_load *);
DWORD ntw16s_global_load_finish(ntw16s_global_load *,DWORD,void **);
DWORD ntw16s_global_load_discard(ntw16s_global_load *);
#endif
