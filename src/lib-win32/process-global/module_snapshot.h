#ifndef WIN32_PROCESS_GLOBAL_MODULE_SNAPSHOT_H
#define WIN32_PROCESS_GLOBAL_MODULE_SNAPSHOT_H
#include <windows.h>
#include <tlhelp32.h>
/* Toolhelp may report a changing module list; retain the existing retry bound. */
static inline HANDLE win32_process_global_module_snapshot(DWORD pid)
{
    HANDLE snapshot=INVALID_HANDLE_VALUE;
    for(unsigned attempt=0;attempt<8;++attempt){
        snapshot=CreateToolhelp32Snapshot(TH32CS_SNAPMODULE|TH32CS_SNAPMODULE32,pid);
        if(snapshot!=INVALID_HANDLE_VALUE || GetLastError()!=ERROR_BAD_LENGTH)break;
    }
    return snapshot;
}
#endif
