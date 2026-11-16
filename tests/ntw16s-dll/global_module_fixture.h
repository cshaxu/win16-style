#ifndef TEST_GLOBAL_MODULE_FIXTURE_H
#define TEST_GLOBAL_MODULE_FIXTURE_H
#include "lib-win32/process-global/load.h"
#include "lib-win32/process-global/remote_loader.h"
#include "lib-win32/process-global/remote_export.h"
static inline DWORD fixture_remote_global_thread(HANDLE process,DWORD pid,const WCHAR *path,void **address)
{return win32_process_global_remote_export(process,pid,path,sizeof(void*)==8?L"ntw16s64.dll":L"ntw16s32.dll","Ntw16sGlobalThread",address);}

static inline DWORD fixture_global_load_begin(HANDLE process,DWORD pid,const WCHAR *path,win32_process_global_load *ticket)
{return win32_process_global_load_begin(process,pid,path,sizeof(void*)==8?L"ntw16s64.dll":L"ntw16s32.dll",win32_process_global_remote_loadlibrary,ticket);}
static inline DWORD fixture_global_load_finish(win32_process_global_load *ticket,DWORD timeout,void **worker)
{return win32_process_global_load_finish(ticket,timeout,fixture_remote_global_thread,worker);}

#endif
