#include "global_remote.h"
#include "lib-win32/process-global/remote_loader.h"
#include "lib-win32/process-global/remote_export.h"
#include "lib-win32/process-image/remote.h"
#include <tlhelp32.h>
#include <stdint.h>
#include <string.h>
#include <wchar.h>

DWORD ntw16s_remote_loadlibrary(HANDLE process,DWORD pid,void **address)
{return win32_process_global_remote_loadlibrary(process,pid,address);}

DWORD ntw16s_remote_global_thread(HANDLE process,DWORD pid,const WCHAR *path,void **address)
{return win32_process_global_remote_export(process,pid,path,sizeof(void*)==8?L"ntw16s64.dll":L"ntw16s32.dll","Ntw16sGlobalThread",address);}
