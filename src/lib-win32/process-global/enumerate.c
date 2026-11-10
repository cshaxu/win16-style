#include "enumerate.h"
#include <tlhelp32.h>
DWORD win32_process_global_threads(DWORD pid_filter,win32_process_global_thread_callback callback,void *context)
{
    if(!callback)return ERROR_INVALID_PARAMETER;
    HANDLE snapshot=CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD,0);
    if(snapshot==INVALID_HANDLE_VALUE)return GetLastError();
    THREADENTRY32 entry={.dwSize=sizeof(entry)};DWORD error=ERROR_SUCCESS;
    if(Thread32First(snapshot,&entry)){
        for(;;){
            if((!pid_filter || entry.th32OwnerProcessID==pid_filter) &&
               !callback(entry.th32OwnerProcessID,entry.th32ThreadID,context))break;
            if(!Thread32Next(snapshot,&entry)){
                error=GetLastError();
                if(error==ERROR_NO_MORE_FILES)error=ERROR_SUCCESS;
                break;
            }
        }
    }else{
        error=GetLastError();
        if(error==ERROR_NO_MORE_FILES)error=ERROR_SUCCESS;
    }
    CloseHandle(snapshot);return error;
}

typedef struct scan_existing_context {
    win32_process_global_has_process has;
    win32_process_global_inspect_thread inspect;
    win32_process_global_select_thread select;
    void *context;
} scan_existing_context;
static BOOL scan_existing(DWORD pid,DWORD tid,void *parameter)
{
    scan_existing_context *s=parameter;
    if(!s->has(pid,s->context) && !s->inspect(pid,tid,s->context))s->select(pid,tid,s->context);
    return TRUE;
}
DWORD win32_process_global_scan_existing(win32_process_global_has_process has,win32_process_global_inspect_thread inspect,win32_process_global_select_thread select,void *context)
{
    if(!has || !inspect || !select)return ERROR_INVALID_PARAMETER;
    scan_existing_context state={has,inspect,select,context};
    return win32_process_global_threads(0,scan_existing,&state);
}
typedef struct scan_process_context {
    win32_process_global_inspect_thread inspect;
    win32_process_global_select_thread select;
    void *context;DWORD pending;BOOL selected;
} scan_process_context;
static BOOL scan_process(DWORD pid,DWORD tid,void *parameter)
{
    scan_process_context *s=parameter;DWORD status=s->inspect(pid,tid,s->context);
    if(!status){s->select(pid,tid,s->context);s->selected=TRUE;return FALSE;}
    if(status==ERROR_NOT_READY && !s->pending)s->pending=tid;
    return TRUE;
}
DWORD win32_process_global_scan_process(DWORD pid,win32_process_global_inspect_thread inspect,win32_process_global_select_thread select,void *context)
{
    if(!pid || !inspect || !select)return ERROR_INVALID_PARAMETER;
    scan_process_context state={inspect,select,context,0,FALSE};
    DWORD error=win32_process_global_threads(pid,scan_process,&state);
    if(!state.selected && state.pending)select(pid,state.pending,context);
    return error;
}
