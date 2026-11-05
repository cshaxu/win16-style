#include "recovery.h"
#include "identity.h"
#include <evntrace.h>
#include <stdlib.h>
#include <wchar.h>

BOOL win32_process_global_trace_orphan(DWORD pid,DWORD high,DWORD low)
{
    HANDLE process=OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION|SYNCHRONIZE,FALSE,pid);
    BOOL orphan=FALSE;
    if(process){
        DWORD actual_low=0,actual_high=0;
        if(win32_process_global_birth(process,&actual_low,&actual_high))
            orphan=actual_high!=high || actual_low!=low || WaitForSingleObject(process,0)==WAIT_OBJECT_0;
        CloseHandle(process);
    }else orphan=GetLastError()==ERROR_INVALID_PARAMETER;
    return orphan;
}

DWORD win32_process_global_trace_stop_peer(HANDLE process,DWORD pid,const WCHAR *name)
{
    if(!process || !pid || !name || !*name || GetProcessId(process)!=pid || WaitForSingleObject(process,0)!=WAIT_OBJECT_0)return ERROR_INVALID_PARAMETER;
    size_t length=wcsnlen_s(name,32768);if(length>=32768)return ERROR_INVALID_NAME;
    size_t bytes=sizeof(EVENT_TRACE_PROPERTIES)+(length+1)*sizeof(WCHAR);
    EVENT_TRACE_PROPERTIES *p=(EVENT_TRACE_PROPERTIES *)calloc(1,bytes);
    if(!p)return ERROR_NOT_ENOUGH_MEMORY;
    p->Wnode.BufferSize=(ULONG)bytes;p->LoggerNameOffset=sizeof(*p);
    DWORD error=ControlTraceW(0,name,p,EVENT_TRACE_CONTROL_STOP);free(p);
    return error==ERROR_WMI_INSTANCE_NOT_FOUND || error==ERROR_ACTIVE_CONNECTIONS?ERROR_SUCCESS:error;
}

void win32_process_global_recover_orphans(win32_process_global_trace_name_parser parse)
{
    if(!parse)return;
    enum {capacity=256,name_capacity=128};
    EVENT_TRACE_PROPERTIES *entries[capacity]={0};ULONG count=0;
    size_t bytes=sizeof(EVENT_TRACE_PROPERTIES)+name_capacity*sizeof(WCHAR);
    for(unsigned i=0;i<capacity;++i){
        entries[i]=(EVENT_TRACE_PROPERTIES *)calloc(1,bytes);
        if(!entries[i])goto done;
        entries[i]->Wnode.BufferSize=(ULONG)bytes;entries[i]->LoggerNameOffset=sizeof(EVENT_TRACE_PROPERTIES);
    }
    if(QueryAllTracesW(entries,capacity,&count)!=ERROR_SUCCESS)goto done;
    for(ULONG i=0;i<count && i<capacity;++i){
        WCHAR *name=(WCHAR *)((BYTE *)entries[i]+entries[i]->LoggerNameOffset);
        DWORD pid=0,high=0,low=0;
        if(!parse(name,&pid,&high,&low) || pid==GetCurrentProcessId())continue;
        BOOL orphan=win32_process_global_trace_orphan(pid,high,low);
        if(orphan)(void)ControlTraceW(0,name,entries[i],EVENT_TRACE_CONTROL_STOP);
    }
done:
    for(unsigned i=0;i<capacity;++i)free(entries[i]);
}
