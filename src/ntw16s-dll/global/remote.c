#include "ntw16s-dll/global/remote.h"
#include "lib-win32/process-global/remote_loader.h"
#include "lib-win32/process-global/remote_export.h"
DWORD ntw16s_remote_global_thread(HANDLE process,DWORD pid,const WCHAR *path,void **address)
{return win32_process_global_remote_export(process,pid,path,sizeof(void*)==8?L"ntw16s64.dll":L"ntw16s32.dll","Ntw16sGlobalThread",address);}

DWORD ntw16s_global_load_begin(HANDLE process,DWORD pid,const WCHAR *path,win32_process_global_load *ticket)
{return win32_process_global_load_begin(process,pid,path,sizeof(void*)==8?L"ntw16s64.dll":L"ntw16s32.dll",win32_process_global_remote_loadlibrary,ticket);}
DWORD ntw16s_global_load_finish(win32_process_global_load *ticket,DWORD timeout,void **worker)
{return win32_process_global_load_finish(ticket,timeout,ntw16s_remote_global_thread,worker);}
