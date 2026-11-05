#include "global_load.h"
#include "global_remote.h"
DWORD ntw16s_global_load_begin(HANDLE process,DWORD pid,const WCHAR *path,ntw16s_global_load *ticket)
{return win32_process_global_load_begin(process,pid,path,sizeof(void*)==8?L"ntw16s64.dll":L"ntw16s32.dll",ntw16s_remote_loadlibrary,ticket);}
DWORD ntw16s_global_load_finish(ntw16s_global_load *ticket,DWORD timeout,void **worker)
{return win32_process_global_load_finish(ticket,timeout,ntw16s_remote_global_thread,worker);}
DWORD ntw16s_global_load_discard(ntw16s_global_load *ticket)
{return win32_process_global_load_discard(ticket);}
