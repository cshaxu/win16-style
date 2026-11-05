#include "enumerate.h"
#include <tlhelp32.h>
DWORD win32_process_global_threads(DWORD pid_filter,win32_process_global_thread_callback callback,void *context)
{
    if(!callback)return ERROR_INVALID_PARAMETER;
    HANDLE snapshot=CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD,0);
    if(snapshot==INVALID_HANDLE_VALUE)return GetLastError();
    THREADENTRY32 entry={.dwSize=sizeof(entry)};DWORD error=ERROR_SUCCESS;
    if(Thread32First(snapshot,&entry)){
        do{
            if((!pid_filter || entry.th32OwnerProcessID==pid_filter) &&
               !callback(entry.th32OwnerProcessID,entry.th32ThreadID,context))break;
        }while(Thread32Next(snapshot,&entry));
    }else{
        error=GetLastError();
        if(error==ERROR_NO_MORE_FILES)error=ERROR_SUCCESS;
    }
    CloseHandle(snapshot);return error;
}
